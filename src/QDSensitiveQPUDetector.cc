#include "QDSensitiveQPUDetector.hh"



QDSensitiveQPUDetector::QDSensitiveQPUDetector(G4String name) : G4VSensitiveDetector(name) {

    fTotalEnergyDeposited = 0;
    collectionName.insert("qpuCollection");

}

QDSensitiveQPUDetector::~QDSensitiveQPUDetector(){

}


void QDSensitiveQPUDetector::Initialize(G4HCofThisEvent *){

    fTotalEnergyDeposited = 0;

}


void QDSensitiveQPUDetector::EndOfEvent(G4HCofThisEvent *){

    G4cout << "Deposited energy in the QPU: " << fTotalEnergyDeposited << G4endl;

}

G4bool QDSensitiveQPUDetector::ProcessHits(G4Step *aStep, G4TouchableHistory * ){

    G4double fEnergyDeposited = aStep -> GetTotalEnergyDeposit();

    if(fEnergyDeposited > 0){

        fTotalEnergyDeposited += fEnergyDeposited;
    }

    return true;
}