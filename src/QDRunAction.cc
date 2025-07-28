#include "QDRunAction.hh"
#include "G4AnalysisManager.hh"

QDRunAction::QDRunAction(): fNtCryId(-1), fNtBarHitsId(-1) {

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root"); 
  analysisManager->SetVerboseLevel(1);


  analysisManager->SetFileName("all_data");

  // ------------ file for primary cosmic rays ----------------------

  // Add debug print to verify
  G4cout << "Debug: fCryOutputEnabled = " << fCryOutputEnabled << G4endl;


  if (fCryOutputEnabled) {
    fNtCryId = analysisManager->CreateNtuple("primaries", "cosmic rays");
    analysisManager->CreateNtupleDColumn(fNtCryId,"timeSim");
    analysisManager->CreateNtupleIColumn(fNtCryId,"eventId");
    analysisManager->CreateNtupleSColumn(fNtCryId,"pName");
    analysisManager->CreateNtupleDColumn(fNtCryId,"ke");
    analysisManager->CreateNtupleDColumn(fNtCryId,"dirX");
    analysisManager->CreateNtupleDColumn(fNtCryId,"dirY");
    analysisManager->CreateNtupleDColumn(fNtCryId,"dirZ");
    analysisManager->CreateNtupleDColumn(fNtCryId,"x");
    analysisManager->CreateNtupleDColumn(fNtCryId,"y");
    analysisManager->CreateNtupleDColumn(fNtCryId,"z");
    analysisManager->FinishNtuple(fNtCryId);

    //analysisManager->SetNtupleFileName(fNtCryId,"output/primaries"); 
  }

  // ------------ file for bar sensitive detector ----------------------

  fNtBarHitsId = analysisManager->CreateNtuple("hits","bars");
  analysisManager->CreateNtupleIColumn(fNtBarHitsId,"event");   // 0
  analysisManager->CreateNtupleIColumn(fNtBarHitsId,"barID");   // 1
  analysisManager->CreateNtupleSColumn(fNtBarHitsId,"pName");   // 2
  /* global position */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"x");       // 3
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"y");       // 4
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"z");       // 5
  /* local position */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"lx");      // 6
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"ly");      // 7
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"lz");      // 8
  /* times */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tG");      // 9
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tA");      // 10
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tB");      // 11
  /* energy deposited */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"Edep");    // 12
  analysisManager->FinishNtuple(fNtBarHitsId);

  //analysisManager->SetNtupleFileName(fNtBarHitsId,"output/bar_hits"); 


}

QDRunAction::~QDRunAction() {}

void QDRunAction::BeginOfRunAction(const G4Run*) {
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile();
}

void QDRunAction::EndOfRunAction(const G4Run*) {
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write(); // Write all ntuples to file
  analysisManager->CloseFile(); // Closes all files
}