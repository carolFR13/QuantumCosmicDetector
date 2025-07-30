#include "QDDetectorMessenger.hh"
#include "QDDetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

QDDetectorMessenger::QDDetectorMessenger(QDDetectorConstruction* det)
: fDetector(det), fDir(nullptr), fSetModeCmd(nullptr) {
    fDir = new G4UIdirectory("/detector/");
    fDir->SetGuidance("Detector construction control commands");

    fSetModeCmd = new G4UIcmdWithAString("/detector/setMode", this);
    fSetModeCmd->SetGuidance("Choose simulation mode: parameterized, internal, both or none");
    fSetModeCmd->SetCandidates("none parameterized internal both");
    fSetModeCmd->SetParameterName("Mode", false);
    fSetModeCmd->SetDefaultValue("internal");

    fSaveLayoutCmd = new G4UIcmdWithoutParameter("/detector/saveLayout", this);
    fSaveLayoutCmd->SetGuidance("Saves the detector layout to a CSV file.");

}

QDDetectorMessenger::~QDDetectorMessenger() {
    delete fSetModeCmd;
    delete fDir;
    delete fSaveLayoutCmd;
}

void QDDetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {


    if (command == fSetModeCmd) {
        G4cout << "\n=== Detector Messenger receiving mode change ===" << G4endl;
        G4cout << "Current mode before change: " 
               << (fDetector->GetSimulationMode() == SimulationMode::INTERNAL ? "INTERNAL" :
                   fDetector->GetSimulationMode() == SimulationMode::PARAMETERIZED ? "PARAMETERIZED" :
                   fDetector->GetSimulationMode() == SimulationMode::BOTH ? "BOTH" : "NONE") << G4endl;
        
        G4cout << "Attempting to set mode to: " << newValue << G4endl;

        SimulationMode currentMode = fDetector->GetSimulationMode();
        SimulationMode newMode = currentMode;
        
        if (newValue == "internal") {
            newMode = SimulationMode::INTERNAL;
        } else if (newValue == "parameterized") {
            newMode = SimulationMode::PARAMETERIZED;
        } else if (newValue == "both") {
            newMode = SimulationMode::BOTH;
        } else if (newValue == "none") {
            newMode = SimulationMode::NONE;
        } else {
            G4cerr << "Invalid mode: " << newValue << G4endl;
            return;
        }


        if (newMode != currentMode) {
            fDetector->SetSimulationMode(newMode);
            fDetector->UpdateGeometry();
        }
        
        G4cout << "Mode after change: " 
               << (fDetector->GetSimulationMode() == SimulationMode::INTERNAL ? "INTERNAL" :
                   fDetector->GetSimulationMode() == SimulationMode::PARAMETERIZED ? "PARAMETERIZED" :
                   fDetector->GetSimulationMode() == SimulationMode::BOTH ? "BOTH" : "NONE") << G4endl;
        G4cout << "NOTE: Run the following commands to apply changes:" << G4endl;
        G4cout << "/vis/scene/notifyHandlers" << G4endl;
        G4cout << "/vis/scene/add/volume" << G4endl;
        G4cout << "/vis/viewer/flush" << G4endl;
        G4cout << "=======================================\n" << G4endl;
    
    }
    else if (command == fSaveLayoutCmd) {
        fDetector->SaveGeometry();  // You'll define this method in QDDetectorConstruction
    }
    
}


