#include "QDQPUHit.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include <iomanip>


G4ThreadLocal G4Allocator<QDQPUHit>* QDQPUHitAllocator = nullptr;

G4bool QDQPUHit::operator==(const QDQPUHit& right) const
{
  return (this == &right) ? true : false;
}

void QDQPUHit::Print() {
    G4cout << "Hit at position: " << fPosition 
           << ", energy deposited: " << fEnergyDep / MeV << " MeV"
           << ", event ID: " << fEventID
           << G4endl;
}
