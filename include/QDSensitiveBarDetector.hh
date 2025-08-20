#ifndef QDSENSITIVEBARDETECTOR_HH
#define QDSENSITIVEBARDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4THitsCollection.hh"
#include "G4EventManager.hh"

#include "QDBarHit.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include <map>
#include <limits>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;


class QDSensitiveBarDetector : public G4VSensitiveDetector{


    public:
        QDSensitiveBarDetector(const G4String& name, const G4String& hitsCollectionName);
        virtual ~QDSensitiveBarDetector() override = default;

        virtual void Initialize(G4HCofThisEvent* hce) override;
        virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* history) override;

        void EndOfEvent(G4HCofThisEvent* hitCollection) override;

        static G4int GetHitsCollectionID() { return fBarHCID; }


    private:

        G4double fTotalEnergyDeposited;
        QDBarHitsCollection* fHitsCollection = nullptr;
        static G4int fBarHCID;
        G4String fHitsCollectionName;

        // HC : hits collection only important if you want to do analysis
        // or reconstraction within G4 

        // handles what happens to the particle in each step 
        // when it is inside of the detector

        struct Accum {
            // global-time sentinels
            G4double tG_earliest;
            G4double tG_latest;

            // times at extremes, tied to the steps that set the sentinels
            G4double tA_earliest, tB_earliest;
            G4double tA_latest,   tB_latest;

            // optional extra info you already store
            G4ThreeVector posEarliest, posLatest;
            G4ThreeVector localPosEarliest, localPosLatest;
            G4double edep_total;
            G4String particle;
            G4String volName;

            Accum()
            : tG_earliest(std::numeric_limits<G4double>::infinity()),
            tG_latest(-std::numeric_limits<G4double>::infinity()),
            tA_earliest(0.), tB_earliest(0.),
            tA_latest(0.),   tB_latest(0.),
            edep_total(0.) {}
        };

        // key by (planeID, barID) for the current event
        std::map<std::pair<int,int>, Accum> hitMap;



};




#endif