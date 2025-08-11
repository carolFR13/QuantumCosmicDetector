#include "QDSensitiveQPUDetector.hh"

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4String.hh"


G4int QDSensitiveQPUDetector::fQPUHCID = -1;

QDSensitiveQPUDetector::QDSensitiveQPUDetector(const G4String &name, const G4String &hitsCollectionName)
    : G4VSensitiveDetector(name), fHitsCollectionName(hitsCollectionName) {

    fTotalEnergyDeposited = 0;
    collectionName.insert(hitsCollectionName);
    G4cout << "Debug: Created sensitive detector with name: " << name
           << " and collection: " << hitsCollectionName << G4endl;
}


void QDSensitiveQPUDetector::Initialize(G4HCofThisEvent *hce) {
    fTotalEnergyDeposited = 0;
    fHitsCollection = new QDQPUHitsCollection(SensitiveDetectorName, collectionName[0]);

    // avoid local redefinition of the fQPUHCID
    // G4int fQPUHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    fQPUHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);

    G4cout << "Debug: Initializing collection with ID: " << fQPUHCID << G4endl;
    hce->AddHitsCollection(fQPUHCID, fHitsCollection);
    G4cout << "Debug: Added hits collection with ID " << fQPUHCID << G4endl;
}


void QDSensitiveQPUDetector::EndOfEvent(G4HCofThisEvent *) {
    G4cout << "Deposited energy in the QPU: " << fTotalEnergyDeposited << G4endl;
    if (verboseLevel > 1) {
        auto nofHits = fHitsCollection->entries();
        G4cout << G4endl << "--------> Hits Collection: in this event they are " << nofHits
               << " hits in the QPU: " << G4endl;
        for (std::size_t i = 0; i < nofHits; ++i)
            (*fHitsCollection)[i]->Print();
    }
}


G4bool QDSensitiveQPUDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *) {
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
    G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();


    // Guardar en un hit o imprimir
    G4cout << "Hit event ID " << eventID
           << ", pos = " << pos << " mm"
           << ", particle = " << pName
           << G4endl;

    // create and add a new QDQPUHit
    // auto hit = (*fHitsCollection)[fHitsCollection->entries() - 1];
    QDQPUHit *hit = new QDQPUHit();
    hit->SetEventID(eventID);
    hit->SetParticleName(pName);
    hit->SetEdep(edep);
    hit->SetPos(pos);

    fHitsCollection->insert(hit);

    return true;
}
