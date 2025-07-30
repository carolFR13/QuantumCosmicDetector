#ifndef QDBARHIT_HH
#define QDBARHIT_HH


#include "G4Allocator.hh"
#include "G4VHit.hh"    
#include "G4ThreeVector.hh"
#include "G4THitsCollection.hh"
#include "globals.hh"

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
        void SetPos(G4ThreeVector xyz) { fglobalPosition = xyz; }
        void SetLocalPos(G4ThreeVector xyz) { flocalPosition = xyz; }
        void SetTime1(G4double t) { fTimeAtEnd1 = t; }
        void SetTime2(G4double t) { fTimeAtEnd2 = t; }
        void SetGlobalTime(G4double t) { fTimeGlobal = t; }
        void SetVolumeName(G4String name ) { fVolumeName = name; }
        void SetPlaneID(G4int id) { fPlaneID = id; }
        void SetEventID(G4int id) { fEventID = id; }


        // Getters
        G4int GetBarID() const { return fBarID; }
        G4String GetParticleName() const { return fParticleName; }
        G4double GetEdep() const { return fEnergyDep; }
        G4ThreeVector GetPos() const { return fglobalPosition; }
        G4ThreeVector GetLocalPos() const { return flocalPosition; }
        G4double GetTime1() const { return fTimeAtEnd1; }
        G4double GetTime2() const { return fTimeAtEnd2; }
        G4double GetGlobalTime() const { return fTimeGlobal; }
        G4String GetVolumeName() const { return fVolumeName; }
        G4int GetPlaneID() const { return fPlaneID; }
        G4int GetEventID() const { return fEventID; }


    private:

        G4int fBarID = -1;
        G4int fPlaneID = -1; // ID of the plane the bar belongs to
        G4int fEventID = -1; // Event ID

        G4double fEnergyDep;
        G4double fTimeGlobal;
        G4double fTimeAtEnd1;
        G4double fTimeAtEnd2;
        G4ThreeVector fglobalPosition;
        G4ThreeVector flocalPosition;
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