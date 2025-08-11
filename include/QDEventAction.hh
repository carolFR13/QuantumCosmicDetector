#ifndef QDEVENTACTION_HH
#define QDEVENTACTION_HH

#include "G4UserEventAction.hh"
#include "QDBarHit.hh"
#include "QDQPUHit.hh"
#include "QDRunAction.hh"
#include "QDSensitiveBarDetector.hh"
#include "globals.hh"

class G4Event;

class QDEventAction : public G4UserEventAction {
       public:
        QDEventAction() = default;
        ~QDEventAction() override = default;

        virtual void BeginOfEventAction(const G4Event* event) override;
        virtual void EndOfEventAction(const G4Event* event) override;

        static void SetCRYOutput(G4bool val) { fCRYOutput = val; }
        static G4bool GetCRYOutput() { return fCRYOutput; }

       private:
        QDBarHitsCollection* GetBarHitsCollection(G4int barID, const G4Event* event) const;
        QDQPUHitsCollection* GetQPUHitsCollection(G4int qpuID, const G4Event* event) const;

        const QDRunAction* fRunAction = nullptr;  // Pointer to run action for analysis manager
        G4int fCryHCID = -1;
        G4int fBarHCID = -1;
        G4int fQPUHCID = -1;

        static G4bool fCRYOutput;
};

#endif
