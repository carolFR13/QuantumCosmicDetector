// Include headers for this class and its messenger
#include "QDPrimaryGeneratorCRY.hh"
#include "QDPrimaryGeneratorMessenger.hh"

// Geant4 headers
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include <iostream> 
#include "G4ParticleTable.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4AnalysisManager.hh"

// Particles used in cosmic rays
#include "G4Gamma.hh"
#include "G4Neutron.hh"
#include "G4Electron.hh"
#include "G4Proton.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
//#include "G4Kaon.hh"
//#include "G4Pion.hh"

// CRY library headers
#include "CRYGenerator.h"
#include "CRYParticle.h"
#include "CRYSetup.h"
#include "CRYUtils.h"

// Custom random number interface
#include "RNGWrapper.hh"
#include "Randomize.hh"

#include "QDRunAction.hh"
#include "G4RunManager.hh"



// Primary particle generator class that reads a setup file with 
// particle shower definitions; initializes a CRY generator to
// produce cosmic-ray secondaries (e.g., muons); and uses a
// G4ParticleGun to convert these particles into Geant4 primary 
// vertices. The class also applies spatial and temporal transformations 
// to the particles and automatically reinitializes CRY to prevent 
// floating-point precision loss due to long simulation times.



// Constructor
QDPrimaryGeneratorCRY::QDPrimaryGeneratorCRY() : G4VUserPrimaryGeneratorAction(),
particleGun(new G4ParticleGun()),                        // initializes particle gun
particleTable(G4ParticleTable::GetParticleTable()),      // gets the particle table
InputState(-1),                                          // state to check CRY input setup ! Important: starts at -1, means not initialized
generator(nullptr),                                      // pointer to the CRY generator
fInitialized(false),
fOutputEnabled(false),
fVerbosityEnabled(true) 
{ 
  gunMessenger = new QDPrimaryGeneratorMessenger(this);  // connects the messenger
  global_simulation_time = 0;                            // initialize global time

}

void QDPrimaryGeneratorCRY::SetOutputEnabled(G4bool enable) {
    fOutputEnabled = enable;

    auto* runAction = dynamic_cast<const QDRunAction*>(G4RunManager::GetRunManager()->GetUserRunAction());
   
    if (runAction){
      auto* ra = const_cast<QDRunAction*>(runAction);
      ra->SetCryOutputEnabled(enable);
    }
}

void QDPrimaryGeneratorCRY::SetVerbosity(G4bool enable) {
    fVerbosityEnabled = enable;
}

void QDPrimaryGeneratorCRY::SetPlane(const G4String& plane) {
    fPlane = plane;
}


// Initialize CRY and check world geometry
void QDPrimaryGeneratorCRY::Initialize()
{
  // Default setup first - with minimum valid subbox size
  std::string defaultSetup = "returnNeutrons 1\n"
  "returnProtons 1\n"
  "returnGammas 1\n"
  "returnMuons 1\n"
  "returnElectrons 1\n"
  "subboxLength 3\n"  // Using 3m as minimum valid size
  "latitude 60\n"
  "altitude 0\n";
        
  CRY_setup_string = defaultSetup;

  if(CRYFromFile("default.cry.setup")) {    
    // If file reading fails, use default setup
    CRYSetup *setup = new CRYSetup(CRY_setup_string, CRY_DATA_DIR);
    generator = new CRYGenerator(setup);
    delete setup;
  } 
    
  // Find the world logical volume to get its size
    
  G4LogicalVolume *worldLV = G4LogicalVolumeStore::GetInstance()->GetVolume("logicWorld");
  if(!worldLV) {

    G4String* str = new G4String("Couldn't find the world logical volume.");
    G4Exception("QDPrimaryGeneratorCRY", "2", FatalException, *str);

  } else if(worldLV->GetSolid()->GetEntityType()!="G4Box") {

    G4String* str = new G4String("The world volume must be of solid type G4Box");
    G4Exception("QDPrimaryGeneratorCRY", "3", FatalException, *str);

  } else {
    // Save the world size to inform CRY about sub-box size
    G4Box* worldBox = (G4Box*)(worldLV->GetSolid());
    
    world_extent = worldBox->GetXHalfLength();  // Get full length
    world_z_max = worldBox->GetZHalfLength();

    G4String cmd = "subboxLength " + std::to_string(world_extent/m);
    UpdateCRY(&cmd);

    // Mark as initialized after successful setup
    fInitialized = true;
  }
}


// Destructor
QDPrimaryGeneratorCRY::~QDPrimaryGeneratorCRY(){
    delete particleGun;
    delete gunMessenger;

}

// Generate primary particles. 
// Using primary vertex instead of particle gun to 
// assign a time to the primary particle available
// for all the simulation


void QDPrimaryGeneratorCRY::GeneratePrimaries(G4Event* event)
{

  if (fVerbosityEnabled) {
    G4cout << "\n=== Starting Event " << event->GetEventID() << " ===" << G4endl;
    G4cout << "InputState: " << InputState << G4endl;
    G4cout << "Generator initialized: " << (generator != nullptr) << G4endl;
  }
    
  // If not initialized, try to initialize again
  if (InputState != 0) {
    G4cout << "Trying to initialize from file..." << G4endl;
    if(CRYFromFile("default.cry.setup")) {
      G4String str("CRY library was not successfully initialized");
      G4Exception("PrimaryGeneratorActionCRY", "4",RunMustBeAborted, str);
      return;
    }
    G4cout << "CRY initialized successfully" << G4endl;
    if (!fInitialized) Initialize();
  }

  G4int nGenerated(0);

  while(!nGenerated) { // loop to make sure at least one particle is generated per event

    particles.clear();
    generator->genEvent(&particles); // CRY generates particles for this event

    // the time (*particle)->t() is the global time, i.e. the total time elapsed in the CRY simulation
    // the rate is around 1/0.01 sec = 1e-01 ms⁻¹ (but depends on the size of the world)
    // I set the time of the track to the local time, ie. the time since the generation of the primary
    // cosmic ray in CRY. Hopefully we will not loose the time correlation due to precision.
    // CRY does use double precision, so 52 bits for the number ~ 1E15, so we should be able to run
    // about 1e6 s before ns precision is lost. What I can do is reinitilize CRY when timeSimulated is
    // too large.
    
    G4double timeSimulated = generator->timeSimulated();

    for(auto particle=particles.begin();particle!=particles.end();particle++) {
      
      if(G4ParticleDefinition* particleDef = CryParticleDef((*particle)->id(),(*particle)->charge())) {

        //particleGun->SetParticleDefinition(particleDef);
        //particleGun->SetParticleEnergy((*particle)->ke()*MeV); // kinetic energy

        // Transform coordinates so particles come from above (changing to g4 coordinate system):
        // Original (x,y,z) becomes (x,z,y)

        G4ThreeVector pos;
        G4ThreeVector dir;

        fPlane = GetPlane();

        if (fPlane == "aboveDetector") {

          pos.set((*particle)->x()*meter,         // x stays as x
                    0.01*meter,             // y at top of world
                    (*particle)->y()*meter);      // y becomes z

          dir.set((*particle)->u(),                  // x component stays as x
                   -std::abs((*particle)->w()),      // z component becomes y
                    (*particle)->v());               // y component becomes z
        }
        else if (fPlane == "qpu") {

            pos.set(0, 
                    0.5*meter,
                    0);

            dir.set((*particle)->u(), 
                      -std::abs((*particle)->w()), 
                      (*particle)->v());
        }
        else if (fPlane == "topWorld") {

            pos.set((*particle)->x()*meter, 
                      0.5*world_extent, 
                      (*particle)->y()*meter);

            dir.set((*particle)->u(), 
                    -std::abs((*particle)->w()), 
                    (*particle)->v());
        }
        else {
          G4String msg = "Unknown plane: " + fPlane;
          G4Exception("QDPrimaryGeneratorCRY::GeneratePrimaries", "InvalidPlane", FatalException, msg);
          return;
        }     

        dir = dir.unit();

        G4String pName = CRYUtils::partName((*particle)->id());


        if (fVerbosityEnabled) {
          G4cout << "Original position: ("
          << (*particle)->x() << ","
          << (*particle)->y() << ","
          << (*particle)->z() << ")" << G4endl;
          G4cout << "Transformed position: " << pos << G4endl;
          G4cout << "Transformed direction: " << dir << G4endl;
        }

        auto primary = new G4PrimaryParticle(particleDef,
                                             dir.x() * (*particle)->ke() * MeV,
                                             dir.y() * (*particle)->ke() * MeV,
                                             dir.z() * (*particle)->ke() * MeV);
        primary->SetKineticEnergy((*particle)->ke() * MeV);

        // Create vertex and assign time directly from CRY
        auto vertex = new G4PrimaryVertex(pos, (*particle)->t() * second);  // CRY time

        vertex->SetPrimary(primary);
        event->AddPrimaryVertex(vertex);
        // const auto* runAction = dynamic_cast<const QDRunAction*>(G4RunManager::GetRunManager()->GetUserRunAction());


        // changing fillntuples to G4EventAction to deal with several files
        // option in below works well if only CRY output is needed

        // if (fOutputEnabled) {
        //   auto analysisManager = G4AnalysisManager::Instance();
        //   G4int ntId = runAction->GetNtCryId();

        //   analysisManager->FillNtupleDColumn(0, timeSimulated);
        //   analysisManager->FillNtupleIColumn(1, event->GetEventID());
        //   analysisManager->FillNtupleSColumn(2, pName);
        //   analysisManager->FillNtupleDColumn(3, (*particle)->ke());
        //   analysisManager->FillNtupleDColumn(4, dir.x());
        //   analysisManager->FillNtupleDColumn(5, dir.y());
        //   analysisManager->FillNtupleDColumn(6, dir.z());
        //   analysisManager->FillNtupleDColumn(7, pos.x() / mm); // o usa directamente m
        //   analysisManager->FillNtupleDColumn(8, pos.y() / mm);
        //   analysisManager->FillNtupleDColumn(9, pos.z() / mm);

        //   analysisManager->AddNtupleRow(ntId);
        // }

        // particleGun->SetParticleMomentumDirection(dir);
        // particleGun->SetParticlePosition(pos);
        // particleGun->SetParticleTime(((*particle)->t()-timeSimulated)*second); // relative time
        // particleGun->GeneratePrimaryVertex(event);

        ++nGenerated;
      }
      
      //....debug output 
      G4String pName = CRYUtils::partName((*particle)->id()); 

      if (fVerbosityEnabled) {
        G4cout << "  "          << pName << " "
            << "charge="      << (*particle)->charge() << " "
            //<< setprecision(4)
            << "energy (MeV)=" << (*particle)->ke()*MeV << " "
            << "time (sec)=" << (*particle)->t() << " "
            << "pos (m)"
            << G4ThreeVector((*particle)->x(), (*particle)->y(), (*particle)->z())
            << " " << "direction cosines "
            << G4ThreeVector((*particle)->u(), (*particle)->v(), (*particle)->w())
            << "----------------------------------------------------------" << G4endl;
      }

      delete (*particle); // clean up

    }
  }
  
  if(generator->timeSimulated()>1.e5) { //reinit the CRY generator to not loose time correlations
    global_simulation_time += generator->timeSimulated();
    std::string dummy("");
    UpdateCRY(&dummy);
  }

  // fill
  // if (fOutputEnabled) {
  //     auto analysisManager = G4AnalysisManager::Instance();
  //     analysisManager->AddNtupleRow();
  // }
}

// Set input state to "awaiting input"
void QDPrimaryGeneratorCRY::InputCRY()
{
  InputState = 1;
}

// Reconfigure the CRY generator with new setup string
void QDPrimaryGeneratorCRY::UpdateCRY(std::string* MessInput)
{
  CRY_setup_string.append(*MessInput);
  CRYSetup *setup = new CRYSetup(CRY_setup_string,CRY_DATA_DIR);

  if(generator) delete generator;
  generator = new CRYGenerator(setup);

  // set random number generator
  RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(),&CLHEP::HepRandomEngine::flat);
  setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
  InputState=0;

  delete setup;
}

// Read CRY setup from file
G4int QDPrimaryGeneratorCRY::CRYFromFile(G4String newValue)
{
  // Read the cry input file
  std::ifstream inputFile;
  inputFile.open(newValue,std::ios::in);
  char buffer[1000];

  if(inputFile.fail()) {
    G4cout << "Failed to open input file " << newValue << G4endl;
    G4cout << "Make sure to define the cry library on the command line" << G4endl;
    InputState=-1;
    return InputState;
  }

  CRY_setup_string.clear();
  while ( !inputFile.getline(buffer,1000).eof()) {
    CRY_setup_string.append(buffer);
    CRY_setup_string.append(" ");
  }
  inputFile.close();

  CRYSetup *setup = new CRYSetup(CRY_setup_string,CRY_DATA_DIR);

  if(generator) delete generator;
  generator = new CRYGenerator(setup);

  // set random number generator
  RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(),&CLHEP::HepRandomEngine::flat);
  setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
  InputState=0;

  delete setup;

  return InputState;
}

// Convert CRY particle types to Geant4 particles
G4ParticleDefinition* QDPrimaryGeneratorCRY::CryParticleDef(CRYParticle::CRYId id, int charge)
{
  switch(id)
  {
    case CRYParticle::Neutron: return G4Neutron::Neutron();
    case CRYParticle::Proton: return G4Proton::Proton();
    case CRYParticle::Electron: return G4Electron::Electron();
    case CRYParticle::Gamma: return G4Gamma::Gamma();
    case CRYParticle::Muon:
      switch(charge)
      {
        case -1: return G4MuonMinus::MuonMinus();
        case +1: return G4MuonPlus::MuonPlus();
      }
      return nullptr;

    case CRYParticle::Pion: return nullptr; // not supported
    case CRYParticle::Kaon: return nullptr; // not supported
  }

  return nullptr;
}