#include "QDSensitiveBarDetector.hh"

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4String.hh"

G4int QDSensitiveBarDetector::fBarHCID = -1;

QDSensitiveBarDetector::QDSensitiveBarDetector(const G4String &name, const G4String &hitsCollectionName)
    : G4VSensitiveDetector(name), fHitsCollectionName(hitsCollectionName) {
    fTotalEnergyDeposited = 0;
    collectionName.insert(hitsCollectionName);
    G4cout << "Debug: Created sensitive detector with name: " << name
           << " and collection: " << hitsCollectionName << G4endl;
}

void QDSensitiveBarDetector::Initialize(G4HCofThisEvent *hce) {
    fTotalEnergyDeposited = 0;
    fHitsCollection = new QDBarHitsCollection(SensitiveDetectorName, collectionName[0]);

    // avoid local redefinition of the fBarHCID
    // G4int fBarHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    fBarHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);

    G4cout << "Debug: Initializing collection with ID: " << fBarHCID << G4endl;
    hce->AddHitsCollection(fBarHCID, fHitsCollection);
    G4cout << "Debug: Added hits collection with ID " << fBarHCID << G4endl;
}

void QDSensitiveBarDetector::EndOfEvent(G4HCofThisEvent*) {

    G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();

    for (const auto& kv : hitMap) {
        const int planeID = kv.first.first;
        const int barID   = kv.first.second;
        const Accum& a    = kv.second;

        // skip bars we never actually touched
        if (!std::isfinite(a.tG_earliest) || !std::isfinite(a.tG_latest))
            continue;

        QDBarHit* hit = new QDBarHit();
        hit->SetEventID(eventID);
        hit->SetPlaneID(planeID);
        hit->SetBarID(barID);
        hit->SetParticleName(a.particle);
        hit->SetVolumeName(a.volName);
        hit->SetEdep(a.edep_total);

        hit->SetEarliestPos(a.posEarliest);
        hit->SetEarliestLocalPos(a.localPosEarliest);

        hit->SetLatestPos(a.posLatest);
        hit->SetLatestLocalPos(a.localPosLatest);

        // store *both* global times and the corresponding tA/tB
        // (assumes you add these setters to QDBarHit)
        hit->SetEarliestGlobalTime(a.tG_earliest);
        hit->SetLatestGlobalTime(a.tG_latest);
        hit->SetEarliestTimeA(a.tA_earliest);
        hit->SetEarliestTimeB(a.tB_earliest);
        hit->SetLatestTimeA(a.tA_latest);
        hit->SetLatestTimeB(a.tB_latest);

        fHitsCollection->insert(hit);
    }
    hitMap.clear();

    if (verboseLevel > 1) {
        auto nofHits = fHitsCollection->entries();
        G4cout << G4endl << "-------->Hits Collection: in this event there are " << nofHits
               << " hits in the scintillating bars: " << G4endl;
        for (std::size_t i = 0; i < nofHits; ++i)
            (*fHitsCollection)[i]->Print();
    }
}


G4bool QDSensitiveBarDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *) {
    if (!aStep) return false;

    G4Track     *track     = aStep->GetTrack();
    G4StepPoint *prePoint  = aStep->GetPreStepPoint();
    G4StepPoint *postPoint = aStep->GetPostStepPoint();

    G4String       pName = track->GetDefinition()->GetParticleName();
    G4double       kineticE = track->GetKineticEnergy();
    G4ThreeVector  pos = prePoint->GetPosition();
    G4double       globalTime = prePoint->GetGlobalTime();
    G4String       volName = prePoint->GetTouchableHandle()->GetVolume()->GetName();
    G4int          barID = prePoint->GetTouchableHandle()->GetCopyNumber();
    G4int          eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();

    // Get energy deposit
    G4double edep = aStep->GetTotalEnergyDeposit();
    fTotalEnergyDeposited += edep;


    G4double impactCoord;
    G4double halfLength;
    G4int planeID = -1;

    // transform from global to local coordinates
    G4AffineTransform g2l = prePoint->GetTouchable()->GetHistory()->GetTopTransform();

    G4ThreeVector local = g2l.TransformPoint(pos);

    if (G4StrUtil::contains(volName, "physBar1_")) {
        impactCoord = local.z();  // bars aligned to X
        halfLength = 0.5 * 900.0;
        planeID = 1; // plane ID 1 for volume "1"
    } else if (G4StrUtil::contains(volName, "physBar2_")) {
        impactCoord = local.x();  // bars aligned to Y
        halfLength = 0.5 * 1350.0;
        planeID = 2; //plane ID 2 for volume "2"
    } else {
        return false; // not one of our bars
    }

    // CHANGE WHEN YOU HAVE THE REAL INDEX OF REFRACTION !!!
    G4double n = 1.58;
    G4double vg = CLHEP::c_light / n;

    G4double dA = halfLength - impactCoord;
    G4double dB = halfLength + impactCoord;

    G4double tA = globalTime + dA / vg;
    G4double tB = globalTime + dB / vg;

    // update accumulator for this bar (per event)
    auto& acc = hitMap[{planeID, barID}];

    // keep some context once 
    if (acc.particle.empty())
        acc.particle = pName;
    if (acc.volName.empty())
        acc.volName  = volName;

    acc.edep_total += edep;


    // earliest by global time
    if (globalTime < acc.tG_earliest) {
        acc.tG_earliest     = globalTime;
        acc.tA_earliest     = tA-globalTime;
        acc.tB_earliest     = tB-globalTime;
        acc.posEarliest     = pos;
        acc.localPosEarliest= local;
    }

    // latest by global time
    if (globalTime > acc.tG_latest) {
        acc.tG_latest       = globalTime;
        acc.tA_latest       = tA-globalTime;
        acc.tB_latest       = tB-globalTime;
        acc.posLatest       = pos;
        acc.localPosLatest  = local;
    }

    return true;
}
