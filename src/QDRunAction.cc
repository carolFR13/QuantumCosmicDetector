#include "QDRunAction.hh"
#include "G4AnalysisManager.hh"

QDRunAction::QDRunAction(): fNtCryId(-1), fNtBarHitsId(-1), fCryOutputEnabled(false) {

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root"); 
  analysisManager->SetVerboseLevel(1);


  analysisManager->SetFileName("all_data");

}

QDRunAction::~QDRunAction() {}


void QDRunAction::CreateNtuples() {
  if (fNtuplesCreated) return;

  auto analysisManager = G4AnalysisManager::Instance();
    
  // Only create cosmic ray ntuple if enabled
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
   
  }
    
  // Always create bar hits ntuple
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

  analysisManager->CreateNtupleIColumn(fNtBarHitsId, "planeID"); // 13
  analysisManager->CreateNtupleSColumn(fNtBarHitsId, "volumeName"); // 14
  analysisManager->FinishNtuple(fNtBarHitsId);
    
  fNtuplesCreated = true;
}

void QDRunAction::BeginOfRunAction(const G4Run*) {
  CreateNtuples();
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile();
}

void QDRunAction::EndOfRunAction(const G4Run*) {
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write(); // Write all ntuples to file
  analysisManager->CloseFile(); // Closes all files
}