//
/// \file PrimaryGeneratorActionCRY.hh
/// \brief Definition of the PrimaryGeneratorActionCRY class

#ifndef QDPRIMARYGENERATORCRY_HH
#define QDPRIMARYGENERATORCRY_HH 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "CRYParticle.h"

class G4Event;
class G4ParticleGun;
class G4ParticleDefinition;
class QDPrimaryGeneratorMessenger;
class CRYGenerator;
class G4ParticleTable;
//class CRYParticle;

/// The first primary generator action class. It is of the type G4VUserPrimaryGeneratorAction

class QDPrimaryGeneratorCRY : public G4VUserPrimaryGeneratorAction
{
  public:
    QDPrimaryGeneratorCRY();    
    virtual ~QDPrimaryGeneratorCRY();
    virtual void GeneratePrimaries(G4Event*); 

    void Initialize();
    bool IsInitialized() const { return fInitialized; }

    void InputCRY();
    void UpdateCRY(std::string* MessInput);
    G4int CRYFromFile(G4String newValue);
    G4ParticleDefinition* CryParticleDef(CRYParticle::CRYId, int charge);
    //G4ParticleDefinition* CryParticleDef(enum id);

    void SetOutputEnabled(G4bool enable);
    G4bool IsOutputEnabled() const { return fOutputEnabled; }

    void SetVerbosity(G4bool enable);
    G4bool GetVerbosity() const { return fVerbosityEnabled; }

    void SetPlane(const G4String& plane);
    G4String GetPlane() const { return fPlane; }

  private:
    G4ParticleGun* particleGun;
    G4ParticleTable* particleTable;
    G4int InputState;
    CRYGenerator* generator;

    QDPrimaryGeneratorMessenger* gunMessenger;
    std::vector<CRYParticle*> particles; // vector of generated particles

    G4double world_extent;
    G4double world_z_max;

    std::string CRY_setup_string;
    G4double global_simulation_time;

    G4bool fInitialized;

    G4bool fOutputEnabled;

    G4bool fVerbosityEnabled;

    G4String fPlane = "topWorld"; // default
};

#endif
