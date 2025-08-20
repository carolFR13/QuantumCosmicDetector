#ifndef QDBARHIT_HH
#define QDBARHIT_HH


#include "G4Allocator.hh"
#include "G4VHit.hh"    
#include "G4ThreeVector.hh"
#include "G4THitsCollection.hh"
#include "globals.hh"


// code taken from B4 geant4 example 

class QDBarHit : public G4VHit {

    public:

        QDBarHit() = default;
        QDBarHit(const QDBarHit&) = default;
        ~QDBarHit() override = default;

        QDBarHit& operator=(const QDBarHit&) = default;
        G4bool operator==(const QDBarHit&) const;

        inline void* operator new(size_t);
        inline void operator delete(void*);

        // methods from base class
        void Draw() override {};
        void Print() override;

        // Setters
        void SetBarID(G4int id) { fBarID = id; }
        void SetParticleName(G4String name) { fParticleName = name; }
        void SetEdep(G4double de) { fEnergyDep = de; }
        void SetEarliestPos(G4ThreeVector xyz) { fglobalEarliestPosition = xyz; }
        void SetEarliestLocalPos(G4ThreeVector xyz) { flocalEarliestPosition = xyz; }
        void SetLatestPos(G4ThreeVector xyz) { fglobalLatestPosition = xyz; }
        void SetLatestLocalPos(G4ThreeVector xyz) { flocalLatestPosition = xyz; }
        void SetEarliestTimeA(G4double t) { fEarliestTimeAtEnd_A = t; }
        void SetEarliestTimeB(G4double t) { fEarliestTimeAtEnd_B = t; }
        void SetLatestTimeA(G4double t) { fLatestTimeAtEnd_A = t; }
        void SetLatestTimeB(G4double t) { fLatestTimeAtEnd_B = t; }
        void SetEarliestGlobalTime(G4double t) { fEarliestTimeGlobal = t; }
        void SetLatestGlobalTime(G4double t) { fLatestTimeGlobal = t; }
        void SetVolumeName(G4String name ) { fVolumeName = name; }
        void SetPlaneID(G4int id) { fPlaneID = id; }
        void SetEventID(G4int id) { fEventID = id; }


        // Getters
        G4int GetBarID() const { return fBarID; }
        G4String GetParticleName() const { return fParticleName; }
        G4double GetEdep() const { return fEnergyDep; }
        G4ThreeVector GetEarliestPos() const { return fglobalEarliestPosition; }
        G4ThreeVector GetEarliestLocalPos() const { return flocalEarliestPosition; }
        G4ThreeVector GetLatestPos() const { return fglobalLatestPosition; }
        G4ThreeVector GetLatestLocalPos() const { return flocalLatestPosition; }
        G4double GetEarliestTimeA() const { return fEarliestTimeAtEnd_A; }
        G4double GetEarliestTimeB() const { return fEarliestTimeAtEnd_B; }
        G4double GetLatestTimeA() const { return fLatestTimeAtEnd_A; }
        G4double GetLatestTimeB() const { return fLatestTimeAtEnd_B; }
        G4double GetEarliestGlobalTime() const { return fEarliestTimeGlobal; }
        G4double GetLatestGlobalTime() const { return fLatestTimeGlobal; }
        G4String GetVolumeName() const { return fVolumeName; }
        G4int GetPlaneID() const { return fPlaneID; }
        G4int GetEventID() const { return fEventID; }


    private:

        G4int fBarID = -1;
        G4int fPlaneID = -1; // ID of the plane the bar belongs to
        G4int fEventID = -1; // Event ID

        G4double fEnergyDep;
        G4double fEarliestTimeGlobal;
        G4double fLatestTimeGlobal;
        G4double fEarliestTimeAtEnd_A;
        G4double fEarliestTimeAtEnd_B;
        G4double fLatestTimeAtEnd_A;
        G4double fLatestTimeAtEnd_B;
        G4ThreeVector fglobalEarliestPosition;
        G4ThreeVector flocalEarliestPosition;
        G4ThreeVector fglobalLatestPosition;
        G4ThreeVector flocalLatestPosition;
        G4String fVolumeName;
        G4String fParticleName;

};

using QDBarHitsCollection = G4THitsCollection<QDBarHit>;
extern G4ThreadLocal G4Allocator<QDBarHit>* QDBarHitAllocator;

inline void* QDBarHit::operator new(size_t)
{
    if (!QDBarHitAllocator){
        QDBarHitAllocator = new G4Allocator<QDBarHit>;
    }
    void* hit;
    hit = (void*)QDBarHitAllocator->MallocSingle();
    return hit;
}

inline void QDBarHit::operator delete(void* hit)
{
    if (!QDBarHitAllocator){
        QDBarHitAllocator = new G4Allocator<QDBarHit>;
    }
    QDBarHitAllocator->FreeSingle((QDBarHit*)hit);
}

#endif