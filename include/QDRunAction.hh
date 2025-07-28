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

  void EnableCryOutput(G4bool on) { fCryOutputEnabled = on; }
  G4bool IsCryOutputEnabled() const { return fCryOutputEnabled; }

  inline G4int GetNtCryId() const { return fNtCryId; }
  inline G4int GetNtBarHitsId() const { return fNtBarHitsId; }

private:
  void CreateNtuples();
  G4bool fCryOutputEnabled = true;
  G4int fNtCryId = -1;   // primary cosmic rays
  G4int fNtBarHitsId = -1;   // parameterized times in bars

};

#endif