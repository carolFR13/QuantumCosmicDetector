#ifndef QDDETECTORMESSENGER_HH
#define QDDETECTORMESSENGER_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4UIcmdWithoutParameter.hh"

#include "globals.hh"
#include "G4UImessenger.hh"


class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcommand;
class QDDetectorConstruction;


class QDDetectorMessenger : public G4UImessenger {

    
    public:
        QDDetectorMessenger(QDDetectorConstruction*);
        virtual ~QDDetectorMessenger();
    
        virtual void SetNewValue(G4UIcommand*, G4String);



    private:
        G4UIdirectory* fDir;  
        G4UIcmdWithAString* fSetModeCmd;
        QDDetectorConstruction* fDetector;
        G4UIcmdWithoutParameter* fSaveLayoutCmd;

};


#endif

