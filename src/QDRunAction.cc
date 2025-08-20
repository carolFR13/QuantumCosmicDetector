#include "QDRunAction.hh"
#include "G4AnalysisManager.hh"

QDRunAction::QDRunAction(): fNtCryId(-1), fNtBarHitsId(-1), fNtQPUHitsId(-1), fCryOutputEnabled(false) {

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
  fNtBarHitsId = analysisManager->CreateNtuple("barHits","bars");
  analysisManager->CreateNtupleIColumn(fNtBarHitsId,"event");            // 0
  analysisManager->CreateNtupleIColumn(fNtBarHitsId,"barID");            // 1
  analysisManager->CreateNtupleSColumn(fNtBarHitsId,"pName");            // 2
  /* earliest global position */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"x1");                // 3
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"y1");                // 4
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"z1");                // 5
  /* earliest local position */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"lx1");               // 6
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"ly1");               // 7
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"lz1");               // 8
  /* latest global position */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"x2");                // 3
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"y2");                // 4
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"z2");                // 5
  /* latest local position */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"lx2");               // 6
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"ly2");               // 7
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"lz2");               // 8
  /* times */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tG_earliest");      // 9
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tG_latest");        // 10
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tA_earliest");      // 11
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tB_earliest");      // 12
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tA_latest");        // 13
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"tB_latest");        // 14
  /* energy deposited */
  analysisManager->CreateNtupleDColumn(fNtBarHitsId,"Edep");             // 15

  analysisManager->CreateNtupleIColumn(fNtBarHitsId, "planeID");         // 16
  analysisManager->CreateNtupleSColumn(fNtBarHitsId, "volumeName");      // 17
  analysisManager->FinishNtuple(fNtBarHitsId);



  // Always create bar hits ntuple
  fNtQPUHitsId = analysisManager->CreateNtuple("qpuHits","QPU");
  analysisManager->CreateNtupleSColumn(fNtQPUHitsId,"pName");    // 0
  analysisManager->CreateNtupleIColumn(fNtQPUHitsId,"eventID");  // 1

  /*  position */
  analysisManager->CreateNtupleDColumn(fNtQPUHitsId,"x");       // 2
  analysisManager->CreateNtupleDColumn(fNtQPUHitsId,"y");       // 3
  analysisManager->CreateNtupleDColumn(fNtQPUHitsId,"z");       // 4

  /* energy deposited */
  analysisManager->CreateNtupleDColumn(fNtQPUHitsId,"Edep");    // 5

  analysisManager->FinishNtuple(fNtQPUHitsId);
    
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