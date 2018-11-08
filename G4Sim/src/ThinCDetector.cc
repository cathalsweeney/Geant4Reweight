#include "ThinCDetector.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

ThinCDetector::ThinCDetector() : G4VUserDetectorConstruction(){ }

ThinCDetector::~ThinCDetector() { }

G4VPhysicalVolume * ThinCDetector::Construct(){
  G4cout << "Constructing" << G4endl;  
  G4NistManager * nist = G4NistManager::Instance();

  G4double radius = 1.5*m, height = 50.*m;
  G4Material * graphite  = new G4Material("graphite", 6., 12.01*g/mole, 2.266*g/cm3);
  G4bool checkOverlaps = true;

  //Disk
  //
  G4Tubs * solidDisk = new G4Tubs("Disk", 0., radius, .5*cm, 0.*deg, 360.*deg);
  G4LogicalVolume * logicDisk = new G4LogicalVolume(solidDisk, graphite, "Disk");
  G4VPhysicalVolume * physDisk = new G4PVPlacement(0, G4ThreeVector(), logicDisk, "Disk", 0, false, 0, checkOverlaps);
  return physDisk;
}
