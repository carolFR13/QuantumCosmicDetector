#include "QDSensitiveBarDetector.hh"

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4String.hh"

G4int QDSensitiveBarDetector::fBarHCID = -1;

QDSensitiveBarDetector::QDSensitiveBarDetector(const G4String &name, const G4String &hitsCollectionName)
    : G4VSensitiveDetector(name), fHitsCollectionName(hitsCollectionName) {
    fTotalEnergyDeposited = 0;
    collectionName.insert(hitsCollectionName);
    G4cout << "Debug: Created sensitive detector with name: " << name
           << " and collection: " << hitsCollectionName << G4endl;
}

void QDSensitiveBarDetector::Initialize(G4HCofThisEvent *hce) {
    fTotalEnergyDeposited = 0;
    fHitsCollection = new QDBarHitsCollection(SensitiveDetectorName, collectionName[0]);

    // avoid local redefinition of the fBarHCID
    // G4int fBarHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    fBarHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);

    G4cout << "Debug: Initializing collection with ID: " << fBarHCID << G4endl;
    hce->AddHitsCollection(fBarHCID, fHitsCollection);
    G4cout << "Debug: Added hits collection with ID " << fBarHCID << G4endl;
}

void QDSensitiveBarDetector::EndOfEvent(G4HCofThisEvent *) {
    G4cout << "Deposited energy in the bars: " << fTotalEnergyDeposited << G4endl;
    if (verboseLevel > 1) {
        auto nofHits = fHitsCollection->entries();
        G4cout << G4endl << "-------->Hits Collection: in this event they are " << nofHits
               << " hits in the tracker chambers: " << G4endl;
        for (std::size_t i = 0; i < nofHits; ++i)
            (*fHitsCollection)[i]->Print();
    }
}

G4bool QDSensitiveBarDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *) {
    if (!aStep) {
        G4cout << "Error: Null step pointer" << G4endl;
        return false;
    }

    G4Track *track = aStep->GetTrack();

    G4String pName = track->GetDefinition()->GetParticleName();
    G4double kineticE = track->GetKineticEnergy();
    G4StepPoint *prePoint = aStep->GetPreStepPoint();
    G4StepPoint *postPoint = aStep->GetPostStepPoint();

    // Get energy deposit
    G4double edep = aStep->GetTotalEnergyDeposit();
    fTotalEnergyDeposited += edep;
    G4cout << "  Energy deposit: " << edep / CLHEP::MeV << " MeV" << G4endl;

    G4ThreeVector pos = prePoint->GetPosition();
    G4double globalTime = prePoint->GetGlobalTime();
    G4String volName = prePoint->GetTouchableHandle()->GetVolume()->GetName();
    G4int barID = prePoint->GetTouchableHandle()->GetCopyNumber();
    G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();


    G4double impactCoord;
    G4double halfLength;
    G4int planeID = -1;

    // transform from global to local coordinates
    G4AffineTransform g2l = prePoint->GetTouchable()->GetHistory()->GetTopTransform();

    G4ThreeVector local = g2l.TransformPoint(pos);

    if (G4StrUtil::contains(volName, "1")) {
        impactCoord = local.z();  // barras alineadas en X
        halfLength = 0.5 * 900.0;
        planeID = 1; // Assuming plane ID 1 for volume "1"
    } else if (G4StrUtil::contains(volName, "2")) {
        impactCoord = local.x();  // barras alineadas en Y
        halfLength = 0.5 * 1350.0;
        planeID = 2; // Assuming plane ID 2 for volume "2"
    }



    // CHANGE WHEN YOU HAVE THE REAL INDEX OF REFRACTION !!!
    G4double n = 1.58;
    G4double vg = CLHEP::c_light / n;

    G4double d1 = halfLength - impactCoord;
    G4double d2 = halfLength + impactCoord;

    G4double t1 = globalTime + d1 / vg;
    G4double t2 = globalTime + d2 / vg;

    // Guardar en un hit o imprimir
    G4cout << "Hit in volume " << volName
           << ", event ID " << eventID
           << ", bar ID " << barID
            <<", planeID = " << planeID
           << ", pos = " << pos << " mm"
           << ", t = " << globalTime << " ns"
           << ", t_arrival1 = " << t1 << " ns"
           << ", t_arrival2 = " << t2 << " ns" 
           << ", particle = " << pName
           << G4endl;

    // create and add a new QDBarHit
    // auto hit = (*fHitsCollection)[fHitsCollection->entries() - 1];
    QDBarHit *hit = new QDBarHit();
    hit->SetEventID(eventID);
    hit->SetBarID(barID);
    hit->SetParticleName(pName);
    hit->SetEdep(edep);
    hit->SetPos(pos);
    hit->SetLocalPos(local);
    hit->SetGlobalTime(globalTime);
    hit->SetTime1(t1-globalTime);
    hit->SetTime2(t2-globalTime);
    hit->SetVolumeName(volName);
    hit->SetPlaneID(planeID);


    fHitsCollection->insert(hit);

    G4cout << "  Hit created in bar " << barID << G4endl;

    return true;
}
