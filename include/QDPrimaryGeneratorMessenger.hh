//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: PrimaryGeneratorMessenger.hh,v 1.6 2002/12/16 16:37:26 maire Exp $
// GEANT4 tag $Name: geant4-07-00-patch-01 $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef QDPRIMARYGENERATORMESSENGER_HH
#define QDPRIMARYGENERATORMESSENGER_HH

#include "G4UImessenger.hh"
#include "globals.hh"
#include "G4UIcmdWithABool.hh"
#include "QDRunAction.hh"
#include "QDEventAction.hh"
#include "G4RunManager.hh"

class QDPrimaryGeneratorCRY;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class QDPrimaryGeneratorMessenger: public G4UImessenger
{
  public:
  
    QDPrimaryGeneratorMessenger(QDPrimaryGeneratorCRY*);
   ~QDPrimaryGeneratorMessenger();
    
    void SetNewValue(G4UIcommand*, G4String);

    void SetRunAction(QDRunAction* runAction) { fRunAction = runAction; }

    
  private:

    QDPrimaryGeneratorCRY*       fCryAction;
    QDRunAction*                 fRunAction;
    G4UIdirectory*               fCryDir;
    G4UIcmdWithAString*          fFileCmd; 
    G4UIcmdWithAString*          fInputCmd;
    G4UIcmdWithoutParameter*     fUpdateCmd;
    std::string*                 fMessInput;
    G4UIcmdWithABool*            fOutputCmd;
    G4UIcmdWithABool*            fVerbosityCmd;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

