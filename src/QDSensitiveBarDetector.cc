#include "QDSensitiveBarDetector.hh"



QDSensitiveBarDetector::QDSensitiveBarDetector(G4String name) : G4VSensitiveDetector(name) {

    fTotalEnergyDeposited = 0;
    collectionName.insert("barCollection");

}

QDSensitiveBarDetector::~QDSensitiveBarDetector(){

}


void QDSensitiveBarDetector::Initialize(G4HCofThisEvent *){

    fTotalEnergyDeposited = 0;

}


void QDSensitiveBarDetector::EndOfEvent(G4HCofThisEvent *){

    G4cout << "Deposited energy in the bars: " << fTotalEnergyDeposited << " MeV" << G4endl;

}

G4bool QDSensitiveBarDetector::ProcessHits(G4Step *aStep, G4TouchableHistory * ){

    G4double fEnergyDeposited = aStep -> GetTotalEnergyDeposit();

    if(fEnergyDeposited > 0){

        fTotalEnergyDeposited += fEnergyDeposited;

    }

    return true;
}