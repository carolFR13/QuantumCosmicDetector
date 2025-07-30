#ifndef QDRUNACTION_HH
#define QDRUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "G4Types.hh" 

class G4Run;

class QDRunAction : public G4UserRunAction {
public:
  QDRunAction();
  virtual ~QDRunAction();

  virtual void BeginOfRunAction(const G4Run*) override;
  virtual void EndOfRunAction(const G4Run*) override;

  void SetCryOutputEnabled(G4bool enable) { fCryOutputEnabled = enable; }
  G4bool GetCryOutputEnabled() const { return fCryOutputEnabled; }

  inline G4int GetNtCryId() const { return fNtCryId; }
  inline G4int GetNtBarHitsId() const { return fNtBarHitsId; }

  G4bool fNtuplesCreated = false; // Flag to check if ntuples are created
  void CreateNtuples();

private:

  G4bool fCryOutputEnabled = true;
  G4int fNtCryId = -1;   // primary cosmic rays
  G4int fNtBarHitsId = -1;   // parameterized times in bars

};

#endif