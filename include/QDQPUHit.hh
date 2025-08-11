#ifndef QDQPUHIT_HH
#define QDQPUHIT_HH


#include "G4Allocator.hh"
#include "G4VHit.hh"    
#include "G4ThreeVector.hh"
#include "G4THitsCollection.hh"
#include "globals.hh"

// code taken from B4 geant4 example 

class QDQPUHit : public G4VHit {

    public:

        QDQPUHit() = default;
        QDQPUHit(const QDQPUHit&) = default;
        ~QDQPUHit() override = default;

        QDQPUHit& operator=(const QDQPUHit&) = default;
        G4bool operator==(const QDQPUHit&) const;

        inline void* operator new(size_t);
        inline void operator delete(void*);

        // methods from base class
        void Draw() override {};
        void Print() override;

        // Setters
        void SetParticleName(G4String name) { fParticleName = name; }
        void SetEdep(G4double de) { fEnergyDep = de; }
        void SetPos(G4ThreeVector xyz) { fPosition = xyz; }
        void SetEventID(G4int id) { fEventID = id; }


        // Getters
        G4String GetParticleName() const { return fParticleName; }
        G4double GetEdep() const { return fEnergyDep; }
        G4ThreeVector GetPos() const { return fPosition; }
        G4int GetEventID() const { return fEventID; }


    private:

        G4int fEventID = -1; // Event ID

        G4double fEnergyDep;
        G4ThreeVector fPosition;
        G4String fParticleName;

};

using QDQPUHitsCollection = G4THitsCollection<QDQPUHit>;
extern G4ThreadLocal G4Allocator<QDQPUHit>* QDQPUHitAllocator;

inline void* QDQPUHit::operator new(size_t)
{
    if (!QDQPUHitAllocator){
        QDQPUHitAllocator = new G4Allocator<QDQPUHit>;
    }
    void* hit;
    hit = (void*)QDQPUHitAllocator->MallocSingle();
    return hit;
}

inline void QDQPUHit::operator delete(void* hit)
{
    if (!QDQPUHitAllocator){
        QDQPUHitAllocator = new G4Allocator<QDQPUHit>;
    }
    QDQPUHitAllocator->FreeSingle((QDQPUHit*)hit);
}

#endif