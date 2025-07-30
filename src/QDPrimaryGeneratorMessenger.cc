#include "QDPrimaryGeneratorMessenger.hh"
#include "QDPrimaryGeneratorCRY.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

QDPrimaryGeneratorMessenger::QDPrimaryGeneratorMessenger(
                                          QDPrimaryGeneratorCRY* cryGen)
  :fCryAction(cryGen)
{
  fCryDir = new G4UIdirectory("/CRY/");
  fCryDir -> SetGuidance("CRY initialization");
   
  fFileCmd = new G4UIcmdWithAString("/CRY/file",this);
  fFileCmd -> SetGuidance("This reads the CRY definition from a file");
  fFileCmd -> AvailableForStates(G4State_PreInit,G4State_Idle);
   
  fInputCmd = new G4UIcmdWithAString("/CRY/input",this);
  fInputCmd -> SetGuidance("CRY input lines");
  fInputCmd -> AvailableForStates(G4State_PreInit,G4State_Idle);

  fUpdateCmd = new G4UIcmdWithoutParameter("/CRY/update",this);
  fUpdateCmd -> SetGuidance("Update CRY definition.");
  fUpdateCmd -> SetGuidance("This command MUST be applied before \"beamOn\" ");
  fUpdateCmd -> SetGuidance("if you changed the CRY definition.");
  fUpdateCmd -> AvailableForStates(G4State_Idle);

  fOutputCmd = new G4UIcmdWithABool("/CRY/output", this);
  fOutputCmd->SetGuidance("Enable/disable particle data output to root file");
  fOutputCmd->SetParameterName("Enable", false);
  fOutputCmd->SetDefaultValue(false);

  fVerbosityCmd = new G4UIcmdWithABool("/CRY/setVerbosity", this);
  fVerbosityCmd->SetGuidance("Enable/disable debug verbosity output");
  fVerbosityCmd->SetParameterName("Verbosity", false);
  fVerbosityCmd->SetDefaultValue(false);

  fMessInput = new std::string;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

QDPrimaryGeneratorMessenger::~QDPrimaryGeneratorMessenger()
{
  delete fCryDir;
  delete fInputCmd;
  delete fUpdateCmd;
  delete fFileCmd;
  delete fOutputCmd; 
  delete fVerbosityCmd;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void QDPrimaryGeneratorMessenger::SetNewValue(
                                        G4UIcommand* command, G4String newValue)
{ 
  if( command == fInputCmd )
   { 
     fCryAction -> InputCRY();
     (*fMessInput).append(newValue);
     (*fMessInput).append(" ");
   }

  if( command == fUpdateCmd )
   { 
     fCryAction -> UpdateCRY(fMessInput); 
     *fMessInput = "";
   }

  if( command == fFileCmd )
   { 
    fCryAction -> CRYFromFile(newValue); 
   }
  
  if (command == fOutputCmd) {
      G4bool outputVal = fOutputCmd->GetNewBoolValue(newValue);

      auto runManager = G4RunManager::GetRunManager();
      fRunAction = (QDRunAction*)runManager->GetUserRunAction();

      if (fRunAction) {
        fRunAction->SetCryOutputEnabled(outputVal);
      }

      QDEventAction::SetCRYOutput(outputVal);

      G4cout << "CRY output " << (outputVal ? "enabled" : "disabled") << G4endl;

  }

  if (command == fVerbosityCmd) {
    fCryAction->SetVerbosity(fVerbosityCmd->GetNewBoolValue(newValue));
  }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

