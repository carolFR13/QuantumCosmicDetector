
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
#include "QDQPUHit.hh"
#include "QDRunAction.hh"

G4bool QDEventAction::fCRYOutput = false;

void QDEventAction::BeginOfEventAction(const G4Event *event) {
    G4cout << "Beginning of event action" << G4endl;
}

QDBarHitsCollection *QDEventAction::GetBarHitsCollection(G4int barID, const G4Event *event) const {

    auto hitsCollection = static_cast<QDBarHitsCollection *>(event->GetHCofThisEvent()->GetHC(barID));

    if (!hitsCollection) {
        G4ExceptionDescription msg;
        msg << "Cannot access bar hits collection with ID " << barID;
        G4Exception("QDEventAction::GetHitsCollection", "MyCode0001", FatalException, msg);
    }
    return hitsCollection;
}

QDQPUHitsCollection* QDEventAction::GetQPUHitsCollection(G4int qpuID, const G4Event* event) const
{
    auto hitsCollection = static_cast<QDQPUHitsCollection*>(event->GetHCofThisEvent()->GetHC(qpuID));

    if (!hitsCollection) {
        G4ExceptionDescription msg;
        msg << "Cannot access QPU hits collection with ID " << qpuID;
        G4Exception("QDEventAction::GetQPUHitsCollection", "MyCode0002",FatalException, msg);
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
    if (fQPUHCID == -1) {
        fQPUHCID = G4SDManager::GetSDMpointer()->GetCollectionID("qpuCollection");
    }

    // Get hits collections
    auto barHC = GetBarHitsCollection(fBarHCID, event);
    auto qpuHC = GetQPUHitsCollection(fQPUHCID, event);

    // protection against no-hit events
    if (barHC->entries() == 0)
        return;

    if (qpuHC->entries() == 0) 
        return;

    
    auto analysisManager = G4AnalysisManager::Instance();
    
    // Fill bar hits ntuple
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

        analysisManager->FillNtupleIColumn(ntBar, 0, barHit->GetEventID()); // event ID
        analysisManager->FillNtupleIColumn(ntBar, 1, barHit->GetBarID());         // barID
        analysisManager->FillNtupleSColumn(ntBar, 2, barHit->GetParticleName());  // particle name

        // earliest position
        analysisManager->FillNtupleDColumn(ntBar, 3, barHit->GetEarliestPos().x() / mm);  // x
        analysisManager->FillNtupleDColumn(ntBar, 4, barHit->GetEarliestPos().y() / mm);  // y
        analysisManager->FillNtupleDColumn(ntBar, 5, barHit->GetEarliestPos().z() / mm);  // z

        // local position
        analysisManager->FillNtupleDColumn(ntBar, 6, barHit->GetEarliestLocalPos().x() / mm);  // x
        analysisManager->FillNtupleDColumn(ntBar, 7, barHit->GetEarliestLocalPos().y() / mm);  // y
        analysisManager->FillNtupleDColumn(ntBar, 8, barHit->GetEarliestLocalPos().z() / mm);  // z

        // latest position
        analysisManager->FillNtupleDColumn(ntBar, 9, barHit->GetLatestPos().x() / mm);  // x
        analysisManager->FillNtupleDColumn(ntBar, 10, barHit->GetLatestPos().y() / mm);  // y
        analysisManager->FillNtupleDColumn(ntBar, 11, barHit->GetLatestPos().z() / mm);  // z


        // local position
        analysisManager->FillNtupleDColumn(ntBar, 12, barHit->GetLatestLocalPos().x() / mm);  // x
        analysisManager->FillNtupleDColumn(ntBar, 13, barHit->GetLatestLocalPos().y() / mm);  // y
        analysisManager->FillNtupleDColumn(ntBar, 14, barHit->GetLatestLocalPos().z() / mm);  // z

        // times
        analysisManager->FillNtupleDColumn(ntBar, 15, barHit->GetEarliestGlobalTime() / ns);  // tG
        analysisManager->FillNtupleDColumn(ntBar, 16, barHit->GetLatestGlobalTime() / ns);  // tG

        analysisManager->FillNtupleDColumn(ntBar, 17, barHit->GetEarliestTimeA() / ns);      // tA
        analysisManager->FillNtupleDColumn(ntBar, 18, barHit->GetEarliestTimeB() / ns);      // tB

        analysisManager->FillNtupleDColumn(ntBar, 19, barHit->GetLatestTimeA() / ns);      // tA
        analysisManager->FillNtupleDColumn(ntBar, 20, barHit->GetLatestTimeB() / ns);      // tB

        analysisManager->FillNtupleDColumn(ntBar, 21, barHit->GetEdep() / MeV);
        analysisManager->FillNtupleIColumn(ntBar, 22, barHit->GetPlaneID()); // planeID
        analysisManager->FillNtupleSColumn(ntBar, 23, barHit->GetVolumeName()); // volumeName

        analysisManager->AddNtupleRow(ntBar);
    }



    // Fill QPU hits ntuple
    G4int ntQPU = fRunAction->GetNtQPUHitsId();

    for (G4int i = 0; i < qpuHC->entries(); ++i) {

        auto qpuHit = (*qpuHC)[i];

        analysisManager->FillNtupleSColumn(ntQPU, 0, qpuHit->GetParticleName());
        analysisManager->FillNtupleIColumn(ntQPU, 1, event->GetEventID());
        analysisManager->FillNtupleDColumn(ntQPU, 2, qpuHit->GetPos().x() / mm);
        analysisManager->FillNtupleDColumn(ntQPU, 3, qpuHit->GetPos().y() / mm);
        analysisManager->FillNtupleDColumn(ntQPU, 4, qpuHit->GetPos().z() / mm);
        analysisManager->FillNtupleDColumn(ntQPU, 5, qpuHit->GetEdep() / MeV);

        analysisManager->AddNtupleRow(ntQPU);

        G4cout << "QPU Hit: Particle " << qpuHit->GetParticleName()
               << ", Event ID " << event->GetEventID()
               << ", Position (" << qpuHit->GetPos().x() / mm << ", "
               << qpuHit->GetPos().y() / mm << ", "
               << qpuHit->GetPos().z() / mm << ") mm"
               << ", Energy Dep " << qpuHit->GetEdep() / MeV << " MeV"
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
