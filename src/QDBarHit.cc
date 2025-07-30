#include "QDBarHit.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include <iomanip>


G4ThreadLocal G4Allocator<QDBarHit>* QDBarHitAllocator = nullptr;

G4bool QDBarHit::operator==(const QDBarHit& right) const
{
  return (this == &right) ? true : false;
}

void QDBarHit::Print() {
    G4cout << "Hit in volume: " << fVolumeName
           << " at position: " << fglobalPosition 
           << ", local position: " << flocalPosition 
           << ", global time: " << fTimeGlobal / ns << " ns"
           << ", arrival1: " << fTimeAtEnd1 / ns << " ns"
           << ", arrival2: " << fTimeAtEnd2 / ns << " ns"
           << ", energy deposited: " << fEnergyDep / MeV << " MeV"
           << ", plane ID: " << fPlaneID
           << G4endl;
}
