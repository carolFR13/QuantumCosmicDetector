#ifndef QDDETECTORCONSTRUCTION
#define QDDETECTORCONSTRUCTION

#include "G4VUserDetectorConstruction.hh"

#include "G4Box.hh"

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4VisAttributes.hh"
#include "G4Color.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"

#include "G4OpticalSurface.hh" 
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4VVisManager.hh"

#include "QDSensitiveQPUDetector.hh"
#include "QDSensitiveSiPMDetector.hh"
#include "QDSensitiveBarDetector.hh"
#include "QDDetectorMessenger.hh"


enum class SimulationMode {
    NONE,
    PARAMETERIZED,
    INTERNAL,
    BOTH
};

class QDDetectorConstruction : public G4VUserDetectorConstruction{

    public: 
        QDDetectorConstruction();
        virtual ~QDDetectorConstruction(); // virtual function cause its already defined in the parent class 

        virtual G4VPhysicalVolume *Construct() override;
        virtual void ConstructSDandField() override;

        void SetSimulationMode(SimulationMode mode);
        SimulationMode GetSimulationMode() const;

        void UpdateGeometry();
        void SaveGeometry();


    private:

        void CleanupGeometry();
        void DefineMaterials();
        void DefineScintillatorMaterials();
        G4VPhysicalVolume* DefineVolumes();


        void ConstructScintillatorLayer1(G4LogicalVolume* motherVolume);
        void ConstructScintillatorLayer2(G4LogicalVolume* motherVolume);
        void ConstructQPU(G4LogicalVolume* motherVolume);


        QDDetectorMessenger* fMessenger;
        SimulationMode fMode;

        G4Material* fworldMat = nullptr;  // pointer to the target  material
        G4Material* fscintillatorMat = nullptr;  // pointer to the chamber material
        G4Material* fcoatingMat = nullptr;
        G4Material* fsipmMat = nullptr;
        G4Material* fqpuMat = nullptr;

        G4LogicalVolume *flogicWorld = nullptr;  // pointer to the world logical volume
        G4LogicalVolume *flogicBar1 = nullptr;  
        G4LogicalVolume *flogicBar2 = nullptr; 
        G4LogicalVolume *flogicSiPM = nullptr;
        G4LogicalVolume *flogicQPU = nullptr;

        // Visualization attributes
        G4VisAttributes* fcoatingVisAtt = nullptr;
        G4VisAttributes* fscintillatorVisAtt = nullptr;
        G4VisAttributes* fsipmVisAtt = nullptr;
        G4VisAttributes* fqpuVisAtt = nullptr;

        G4OpticalSurface* fScintCoatingSurface = nullptr;
        G4OpticalSurface* fSiPMSurface = nullptr;

        QDSensitiveBarDetector* fSensitiveBarDetector = nullptr;
        QDSensitiveSiPMDetector* fSensitiveSiPMDetector = nullptr;
        QDSensitiveQPUDetector* fSensitiveQPUDetector = nullptr;

        G4double fcoatingThickness = 0.1 * mm; // Coating thickness
        G4bool fcheckOverlaps = false;  // option to activate checking of volumes overlaps

        std::vector<G4ThreeVector> fBarPositions;
        std::vector<G4String> fBarNames;


};


#endif