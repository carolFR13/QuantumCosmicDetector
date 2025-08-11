#ifndef QDSENSITIVEQPUDETECTOR_HH
#define QDSENSITIVEQPUDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4THitsCollection.hh"
#include "G4EventManager.hh"

#include "QDQPUHit.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

class QDSensitiveQPUDetector : public G4VSensitiveDetector{


    public:
        QDSensitiveQPUDetector(const G4String& name, const G4String& hitsCollectionName);
        virtual ~QDSensitiveQPUDetector() override = default;

        virtual void Initialize(G4HCofThisEvent* hce) override;
        virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* history) override;

        void EndOfEvent(G4HCofThisEvent* hitCollection) override;

        static G4int GetHitsCollectionID() { return fQPUHCID; }

    private:

        G4double fTotalEnergyDeposited;
        QDQPUHitsCollection* fHitsCollection = nullptr;
        static G4int fQPUHCID;
        G4String fHitsCollectionName;

        // HC : hits collection only important if you want to do analysis
        // or reconstraction within G4 

        // handles what happens to the particle in each step 
        // when it is inside of the detector


};




#endif