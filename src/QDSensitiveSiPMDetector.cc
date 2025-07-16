#include "QDSensitiveSiPMDetector.hh"



QDSensitiveSiPMDetector::QDSensitiveSiPMDetector(G4String name) : G4VSensitiveDetector(name) {

    fTotalEnergyDeposited = 0;
    collectionName.insert("sipmCollection");

}

QDSensitiveSiPMDetector::~QDSensitiveSiPMDetector(){

}


void QDSensitiveSiPMDetector::Initialize(G4HCofThisEvent *){

    fTotalEnergyDeposited = 0;
    fPhotonCount = 0;
    fSiPMCounts.clear();

    G4cout << "Starting new event" << G4endl;

}


void QDSensitiveSiPMDetector::EndOfEvent(G4HCofThisEvent *){
    
    G4cout << "--------------------" << G4endl;
    G4cout << "  Event Summary" << G4endl;
    G4cout << "  Total photons detected: " << fPhotonCount << G4endl;

    for (const auto& [SiPMID, count] : fSiPMCounts) {
        G4cout << "    PMT " << SiPMID << ": " << count << " photons" << G4endl;
    }

    G4cout << "--------------------" << G4endl;

    // Si quieres también dejar esto:
    G4cout << "Deposited energy in the SiPM: " << fTotalEnergyDeposited << " MeV" << G4endl;

}

G4bool QDSensitiveSiPMDetector::ProcessHits(G4Step *aStep, G4TouchableHistory * ){

    G4Track* track = aStep->GetTrack();

    if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
        G4ThreeVector pos = track->GetPosition();
        G4double time = track->GetGlobalTime();
        G4int copyID = aStep->GetPreStepPoint()->GetTouchable()->GetCopyNumber();

        G4cout << "[SiPM " << copyID << "] hit by photon!" << G4endl;
        G4cout << "Position: " << pos << ", Time: " << time / ns << " ns" << G4endl;

        fPhotonCount++;
        fSiPMCounts[copyID]++;

        track->SetTrackStatus(fStopAndKill);
        return true;
    }

    
    return false;
}