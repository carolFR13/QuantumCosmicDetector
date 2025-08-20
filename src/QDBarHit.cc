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
           << ", event ID: " << fEventID
           << ", bar ID: " << fBarID
           << ", plane ID: " << fPlaneID
           << ", earliest arrival at A: " << fEarliestTimeAtEnd_A / ns << " ns"
           << ", earliest arrival at B: " << fEarliestTimeAtEnd_B / ns << " ns"
           << ", latest arrival at A: " << fLatestTimeAtEnd_A / ns << " ns"
           << ", latest arrival at B: " << fLatestTimeAtEnd_B / ns << " ns"
           << ", energy deposited: " << fEnergyDep / MeV << " MeV"
           << ", particle: " << fParticleName
           << G4endl;
}
