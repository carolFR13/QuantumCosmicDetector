#include <iostream>

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4AnalysisManager.hh"

#include "QDPhysicsList.hh"
#include "QDDetectorConstruction.hh"
#include "QDActionInitialization.hh"

int main(int argc, char** argv){

    #ifdef G4MULTITHREADED 
        G4MTRunManager *runManager = new G4MTRunManager;
    #else
        G4RunManager *runManager = new G4RunManager;
    #endif

    // Detector Construction
    runManager -> SetUserInitialization(new QDDetectorConstruction());

    // Physics list
    runManager -> SetUserInitialization(new QDPhysicsList());

    // Action Initialization
    runManager -> SetUserInitialization(new QDActionInitialization());

    // Initialize G4 kernel
    runManager -> Initialize();

    // user interface
    G4UIExecutive *ui = 0;

    // if no arguments are provided, start the UI session
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if(ui){
        // If we are in interactive mode, we can use the UI
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui -> SessionStart();
    } else {
        // If we are in batch mode, we can execute a macro file
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }


    delete ui;
    delete visManager;
    delete runManager;

    return 0;

}