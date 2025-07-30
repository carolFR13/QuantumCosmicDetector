
#include "QDEventAction.hh"

#include <iomanip>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "QDBarHit.hh"
#include "QDRunAction.hh"

G4bool QDEventAction::fCRYOutput = false;

void QDEventAction::BeginOfEventAction(const G4Event *event) {
    G4cout << "Beginning of event action" << G4endl;
}

QDBarHitsCollection *
QDEventAction::GetHitsCollection(G4int barID, const G4Event *event) const

{
    auto hitsCollection = static_cast<QDBarHitsCollection *>(event->GetHCofThisEvent()->GetHC(barID));

    if (!hitsCollection) {
        G4ExceptionDescription msg;
        msg << "Cannot access hits collection with ID " << barID;
        G4Exception("QDEventAction::GetHitsCollection", "MyCode0001", FatalException, msg);
    }
    return hitsCollection;
}

void QDEventAction::EndOfEventAction(const G4Event *event) {
    // assign the fRunAction pointer before using it
    if (!fRunAction) {
        fRunAction = static_cast<const QDRunAction *>(
            G4RunManager::GetRunManager()->GetUserRunAction());
    }

    // Get hits collections IDs (only once)
    if (fBarHCID == -1) {
        fBarHCID = G4SDManager::GetSDMpointer()->GetCollectionID("barCollection");
    }

    // Get hits collections
    auto barHC = GetHitsCollection(fBarHCID, event);

    // protection against no-hit events
    if (barHC->entries() == 0)
        return;

    for(G4int i = 0; i < barHC->entries(); ++i) {

        // Get the hit from the collection
        auto barHit = (*barHC)[i];

        // Print per event (modulo n)
        auto eventID = event->GetEventID();
        if (eventID != barHit->GetEventID()) {
            G4cout << "Warning: Event ID mismatch in hits collection: "
                   << "Event ID = " << eventID
                   << ", Hit Event ID = " << barHit->GetEventID() << G4endl;
            continue; // Skip this hit if IDs do not match
        }
        auto printModulo = G4RunManager::GetRunManager()->GetPrintProgress();
        if ((printModulo > 0) && (eventID % printModulo == 0)) {
            G4cout << "---> Energy deposition " << barHit->GetEdep() << G4endl;
            G4cout << "--> End of event: " << eventID << "\n"
                << G4endl;
        }

        // Fill ntuples
        G4int ntBar = fRunAction->GetNtBarHitsId();

        auto analysisManager = G4AnalysisManager::Instance();

        analysisManager->FillNtupleIColumn(ntBar, 0, barHit->GetEventID()); // event ID
        analysisManager->FillNtupleIColumn(ntBar, 1, barHit->GetBarID());         // barID
        analysisManager->FillNtupleSColumn(ntBar, 2, barHit->GetParticleName());  // particle name

        analysisManager->FillNtupleDColumn(ntBar, 3, barHit->GetPos().x() / mm);  // x
        analysisManager->FillNtupleDColumn(ntBar, 4, barHit->GetPos().y() / mm);  // y
        analysisManager->FillNtupleDColumn(ntBar, 5, barHit->GetPos().z() / mm);  // z

        // local position
        analysisManager->FillNtupleDColumn(ntBar, 6, barHit->GetLocalPos().x() / mm);  // x
        analysisManager->FillNtupleDColumn(ntBar, 7, barHit->GetLocalPos().y() / mm);  // y
        analysisManager->FillNtupleDColumn(ntBar, 8, barHit->GetLocalPos().z() / mm);  // z

        // times
        analysisManager->FillNtupleDColumn(ntBar, 9, barHit->GetGlobalTime() / ns);  // tG
        analysisManager->FillNtupleDColumn(ntBar, 10, barHit->GetTime1() / ns);      // barID
        analysisManager->FillNtupleDColumn(ntBar, 11, barHit->GetTime2() / ns);

        analysisManager->FillNtupleDColumn(ntBar, 12, barHit->GetEdep() / MeV);
        analysisManager->FillNtupleIColumn(ntBar, 13, barHit->GetPlaneID()); // planeID
        analysisManager->FillNtupleSColumn(ntBar, 14, barHit->GetVolumeName()); // volumeName

        analysisManager->AddNtupleRow(ntBar);

        G4cout << " Bar " << barHit->GetBarID()
            << " Edep " << barHit->GetEdep() / MeV << " MeV"
            << " T1 " << barHit->GetTime1() / ns << " ns"
            << " T2 " << barHit->GetTime2() / ns << " ns"
            << G4endl;
    }

    G4cout << "CRY output enabled: " << fCRYOutput << G4endl;

    if (fCRYOutput && event->GetNumberOfPrimaryVertex() > 0) {
        auto vertex = event->GetPrimaryVertex(0);
        G4int ntCry = fRunAction->GetNtCryId();

        // Add safety check
        if (ntCry < 0) {
            G4cout << "Warning: Cosmic ray ntuple not created, skipping fill" << G4endl;
            return;
        }


        if (vertex && vertex->GetNumberOfParticle() > 0) {
            auto primary = vertex->GetPrimary(0);
            if (primary) {

            G4ThreeVector pos = vertex -> GetPosition();
            G4ThreeVector dir = primary->GetMomentumDirection();
            G4double energy = primary->GetKineticEnergy();
            G4String pName = primary->GetParticleDefinition()->GetParticleName();
            G4double time = vertex ->GetT0(); // time in seconds

            // debug prints
            G4cout << "Saving primary particle data:" << G4endl;
            G4cout << "Position: " << pos << G4endl;
            G4cout << "Direction: " << dir << G4endl;
            G4cout << "Energy: " << energy << G4endl;

            auto analysisManager = G4AnalysisManager::Instance();
            

            analysisManager->FillNtupleDColumn(ntCry, 0, time);
            analysisManager->FillNtupleIColumn(ntCry, 1, event->GetEventID());
            analysisManager->FillNtupleSColumn(ntCry, 2, pName);
            analysisManager->FillNtupleDColumn(ntCry, 3, energy/CLHEP::MeV);
            analysisManager->FillNtupleDColumn(ntCry, 4, dir.x());
            analysisManager->FillNtupleDColumn(ntCry, 5, dir.y());
            analysisManager->FillNtupleDColumn(ntCry, 6, dir.z());
            analysisManager->FillNtupleDColumn(ntCry, 7, pos.x()/CLHEP::mm);
            analysisManager->FillNtupleDColumn(ntCry, 8, pos.y()/CLHEP::mm);
            analysisManager->FillNtupleDColumn(ntCry, 9, pos.z()/CLHEP::mm);
            analysisManager->AddNtupleRow(ntCry);

            }
        }
    }

    G4cout << "End of event action completed" << G4endl;
    return;
}
