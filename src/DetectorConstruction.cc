//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B4/B4d/src/DetectorConstruction.cc
/// \brief Implementation of the B4d::DetectorConstruction class

#include "DetectorConstruction.hh"
#include "G4AutoDelete.hh"
#include "G4Box.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"

#include "G4Colour.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSTrackLength.hh"
#include "G4PsTrackCounter.hh"
#include "G4SDChargedFilter.hh"
#include "G4SDManager.hh"
#include "G4SDParticleFilter.hh"
#include "G4Sphere.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4VisAttributes.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include <vector>

namespace B4d {

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::Construct() {
  // Define materials
  DefineMaterials();

  // Define volumes
  return DefineVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials() {
  // Lead material defined using NIST Manager
  auto nistManager = G4NistManager::Instance();
  nistManager->FindOrBuildMaterial("G4_Pb");

  G4double a; // mass of a mole;
  G4double z; // z=mean number of protons;
  G4double density;

  // Vacuum
  new G4Material("Galactic", z = 1., a = 1.01 * g / mole,
                 density = universe_mean_density, kStateGas, 2.73 * kelvin,
                 3.e-18 * pascal);

  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::DefineVolumes() {
  // Geometry parameters
  G4double detHeight = 21 * cm;
  G4double detDiameter = 22.86 * cm;

  // Get materials
  G4Material *gapMaterial = G4Material::GetMaterial("Galactic");
  G4Material *targetMaterial = G4Material::GetMaterial("G4_Pb");
  //
  // World
  //
  auto worldS = new G4Box("World",                 // its name
                          10 * m, 10 * m, 10 * m); // its size

  auto worldLV = new G4LogicalVolume(worldS,      // its solid
                                     gapMaterial, // its material
                                     "World");    // its name

  auto worldPV = new G4PVPlacement(nullptr,         // no rotation
                                   G4ThreeVector(), // at (0,0,0)
                                   worldLV,         // its logical volume
                                   "World",         // its name
                                   nullptr,         // its mother  volume
                                   false,           // no boolean operation
                                   0,               // copy number
                                   fCheckOverlaps); // checking overlaps

  // Define dimensions for the outer box
  G4double outerBoxXHalfLength = 2.0 * m;
  G4double outerBoxYHalfLength = 2.0 * m;
  G4double outerBoxZHalfLength = 2.0 * m;

  // Define dimensions for the inner box (cavity)
  G4double innerBoxXHalfLength = 1.8 * m;
  G4double innerBoxYHalfLength = 1.8 * m;
  G4double innerBoxZHalfLength = 1.8 * m;

  // Create the outer box solid
  G4Box *outerBoxSolid = new G4Box("OuterBox", outerBoxXHalfLength,
                                   outerBoxYHalfLength, outerBoxZHalfLength);

  // Create the inner box solid
  G4Box *innerBoxSolid = new G4Box("InnerBox", innerBoxXHalfLength,
                                   innerBoxYHalfLength, innerBoxZHalfLength);
  G4SubtractionSolid *hollowBoxSolid =
      new G4SubtractionSolid("HollowBox", outerBoxSolid, innerBoxSolid);

  G4LogicalVolume *NDetLV =
      new G4LogicalVolume(hollowBoxSolid, gapMaterial, "NDetLV");

  new G4PVPlacement(nullptr,                            // no rotation
                    G4ThreeVector(0 * m, 0 * m, 0 * m), // at (0,0,0)
                    NDetLV,                             // its logical volume
                    "NDet",                             // its name
                    worldLV,                            // its mother  volume
                    false,                              // no boolean operation
                    0,                                  // copy number
                    fCheckOverlaps);
  // Target
  // // auto TargetS = new G4Sphere("Target", 0., 5 * cm, 0., twopi, 0., pi);
  auto TargetS = new G4Box("Target",                   // its name
                           10 * cm, 10 * cm, 10 * cm); // its size

  G4RotationMatrix *Rotation = new G4RotationMatrix();
  Rotation->rotateX(90 * deg);
  Rotation->rotateY(0 * deg);
  Rotation->rotateZ(0 * deg);
  // auto TargetS = new G4Tubs("Target", 0., 5 * cm, 5 * cm, 0., twopi);

  auto TargetLV = new G4LogicalVolume(TargetS,        // its solid
                                      targetMaterial, // its material
                                      "Target");      // its name

  auto TargetPV = new G4PVPlacement(Rotation,        // no rotation
                                    G4ThreeVector(), // at (0,0,0)
                                    TargetLV,        // its logical volume
                                    "Target",        // its name
                                    worldLV,         // its mother  volume
                                    false,           // no boolean operation
                                    0,               // copy number
                                    fCheckOverlaps); // checking overlaps

  auto TargetDetLV = new G4LogicalVolume(TargetS,     // its solid
                                         gapMaterial, // its material
                                         "TargetDetLV");

  new G4PVPlacement(Rotation,        // no rotation
                    G4ThreeVector(), //  its position
                    TargetDetLV,     // its logical volume
                    "TargetDet",     // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // auto TargetPV = new G4PVPlacement(nullptr,         // no rotation
  //                                   G4ThreeVector(), // at (0,0,0)
  //                                   TargetLV,        // its logical volume
  //                                   "Target",        // its name
  //                                   worldLV,         // its mother  volume
  //                                   false,           // no boolean operation
  //                                   0,               // copy number
  //                                   fCheckOverlaps); // checking overlaps

  //
  // Gap
  //

  auto gapS = new G4Tubs("Gap", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  auto gapLV = new G4LogicalVolume(gapS,        // its solid
                                   gapMaterial, // its material
                                   "gapLV");    // its name

  new G4PVPlacement(Rotation,                               // no rotation
                    G4ThreeVector(0. * cm, 0., -80.5 * cm), //  its position
                    gapLV,           // its logical volume
                    "Gap",           // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps
  // --------------- det2
  auto gapS2 =
      new G4Tubs("Gap2", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV2 = new G4LogicalVolume(gapS2,       // its solid
                                    gapMaterial, // its material
                                    "gapLV2");   // its name

  new G4PVPlacement(Rotation,                                  // no rotation
                    G4ThreeVector(40.25 * cm, 0, -69.72 * cm), //  its position
                    gapLV2,          // its logical volume
                    "Gap2",          // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // -----------------
  // // --------------- det3
  auto gapS3 =
      new G4Tubs("Gap3", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV3 = new G4LogicalVolume(gapS3,       // its solid
                                    gapMaterial, // its material
                                    "gapLV3");   // its name

  new G4PVPlacement(Rotation,                                  // no rotation
                    G4ThreeVector(69.72 * cm, 0, -40.25 * cm), //  its position
                    gapLV3,          // its logical volume
                    "Gap3",          // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // -----------------
  // // --------------- det4
  auto gapS4 =
      new G4Tubs("Gap4", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV4 = new G4LogicalVolume(gapS4,       // its solid
                                    gapMaterial, // its material
                                    "gapLV4");   // its name

  new G4PVPlacement(Rotation,                           // no rotation
                    G4ThreeVector(80.5 * cm, 0, 0 * m), //  its position
                    gapLV4,                             // its logical volume
                    "Gap4",                             // its name
                    worldLV,                            // its mother  volume
                    false,                              // no boolean operation
                    0,                                  // copy number
                    fCheckOverlaps);                    // checking overlaps

  // -----------------
  // // --------------- det5
  auto gapS5 =
      new G4Tubs("Gap5", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV5 = new G4LogicalVolume(gapS5,       // its solid
                                    gapMaterial, // its material
                                    "gapLV5");   // its name

  new G4PVPlacement(Rotation,                                 // no rotation
                    G4ThreeVector(69.72 * cm, 0, 40.25 * cm), //  its position
                    gapLV5,          // its logical volume
                    "Gap5",          // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // -----------------
  // // --------------- det6
  auto gapS6 =
      new G4Tubs("Gap6", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV6 = new G4LogicalVolume(gapS6,       // its solid
                                    gapMaterial, // its material
                                    "gapLV6");   // its name

  new G4PVPlacement(Rotation,                                 // no rotation
                    G4ThreeVector(40.25 * cm, 0, 69.72 * cm), //  its position
                    gapLV6,          // its logical volume
                    "Gap6",          // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // -----------------
  // // --------------- det7
  auto gapS7 =
      new G4Tubs("Gap7", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV7 = new G4LogicalVolume(gapS7,       // its solid
                                    gapMaterial, // its material
                                    "gapLV7");   // its name

  new G4PVPlacement(Rotation,                           // no rotation
                    G4ThreeVector(0 * m, 0, 80.5 * cm), //  its position
                    gapLV7,                             // its logical volume
                    "Gap7",                             // its name
                    worldLV,                            // its mother  volume
                    false,                              // no boolean operation
                    0,                                  // copy number
                    fCheckOverlaps);                    // checking overlaps

  // -----------------
  // // --------------- det8
  auto gapS8 =
      new G4Tubs("Gap8", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV8 = new G4LogicalVolume(gapS8,       // its solid
                                    gapMaterial, // its material
                                    "gapLV8");   // its name

  new G4PVPlacement(Rotation,                                  // no rotation
                    G4ThreeVector(-40.25 * cm, 0, 69.72 * cm), //  its position
                    gapLV8,          // its logical volume
                    "Gap8",          // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // -----------------
  // // --------------- det9
  auto gapS9 =
      new G4Tubs("Gap9", 0., detDiameter / 2, detHeight / 2, 0., twopi);

  // auto gapS = new G4Box("Gap", // its name
  //              15 * cm, 15 * cm, 20 * cm); // its
  //              size

  auto gapLV9 = new G4LogicalVolume(gapS9,       // its solid
                                    gapMaterial, // its material
                                    "gapLV9");   // its name

  new G4PVPlacement(Rotation,                                   // no rotation
                    G4ThreeVector(-40.25 * cm, 0, -69.72 * cm), //  its position
                    gapLV9,          // its logical volume
                    "Gap9",          // its name
                    worldLV,         // its mother  volume
                    false,           // no boolean operation
                    0,               // copy number
                    fCheckOverlaps); // checking overlaps

  // -----------------

  // Visualization attributes
  //

  worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());
  G4VisAttributes visAttributes;
  visAttributes.SetForceSolid(true);
  visAttributes.SetColor(G4Color::White());
  G4VisAttributes visAttributesS;
  visAttributesS.SetForceSolid(true);
  visAttributesS.SetColor(G4Color::Red());

  gapLV->SetVisAttributes(visAttributes);   // 180 degrees 2321
  gapLV2->SetVisAttributes(visAttributesS); // 150 2313
  gapLV3->SetVisAttributes(visAttributes);  // 120 2354
  gapLV4->SetVisAttributes(visAttributes);  // 90 2557
  gapLV5->SetVisAttributes(visAttributes);  // 60 2311
  gapLV6->SetVisAttributes(visAttributes);  // 30 2231
  gapLV7->SetVisAttributes(visAttributes);  // 0 degrees 2186
  gapLV8->SetVisAttributes(visAttributesS); // 330 2150
  gapLV9->SetVisAttributes(visAttributes);  // 210 Degrees 2178
  TargetLV->SetVisAttributes(visAttributes);

  //
  // Always return the physical World
  //
  return worldPV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField() {
  G4SDManager::GetSDMpointer()->SetVerboseLevel(1);
  //
  // Scorers
  //

  // primitive ->SetFilter(charged);
  // absDetector->RegisterPrimitive(primitive);

  // declare Gap as a MultiFunctionalDetector scorer

  G4MultiFunctionalDetector *TargetDet =
      new G4MultiFunctionalDetector("TargetDet");
  G4MultiFunctionalDetector *NDet = new G4MultiFunctionalDetector("NDet");

  G4MultiFunctionalDetector *gapDetector = new G4MultiFunctionalDetector("Gap");
  G4MultiFunctionalDetector *gapDetector2 =
      new G4MultiFunctionalDetector("Gap2");
  G4MultiFunctionalDetector *gapDetector3 =
      new G4MultiFunctionalDetector("Gap3");
  G4MultiFunctionalDetector *gapDetector4 =
      new G4MultiFunctionalDetector("Gap4");
  G4MultiFunctionalDetector *gapDetector5 =
      new G4MultiFunctionalDetector("Gap5");
  G4MultiFunctionalDetector *gapDetector6 =
      new G4MultiFunctionalDetector("Gap6");
  G4MultiFunctionalDetector *gapDetector7 =
      new G4MultiFunctionalDetector("Gap7");
  G4MultiFunctionalDetector *gapDetector8 =
      new G4MultiFunctionalDetector("Gap8");
  G4MultiFunctionalDetector *gapDetector9 =
      new G4MultiFunctionalDetector("Gap9");

  G4SDManager::GetSDMpointer()->AddNewDetector(TargetDet);
  G4SDManager::GetSDMpointer()->AddNewDetector(NDet);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector2);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector3);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector4);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector5);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector6);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector7);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector8);
  G4SDManager::GetSDMpointer()->AddNewDetector(gapDetector9);

  G4VPrimitiveScorer *primitive;
  G4PSTrackCounter *scorerN =
      new G4PSTrackCounter("TrackCounter", fCurrent_InOut);
  primitive = scorerN;
  G4SDParticleFilter *neutronFilter =
      new G4SDParticleFilter("neutronFilter", "neutron");
  NDet->SetFilter(neutronFilter);
  NDet->RegisterPrimitive(primitive);

  G4PSTrackLength *scorerT = new G4PSTrackLength("TrackLength");
  primitive = scorerT;
  // G4SDParticleFilter *particleFilter =
  //     new G4SDParticleFilter("protonFilter", "proton");
  auto charged = new G4SDChargedFilter("chargedFilter");
  primitive->SetFilter(charged);
  TargetDet->RegisterPrimitive(primitive);
  // ------------------------ 1
  G4PSTrackCounter *scorer = new G4PSTrackCounter("TrackCounter", fCurrent_In);
  primitive = scorer;

  gapDetector->SetFilter(neutronFilter);
  gapDetector->RegisterPrimitive(primitive);

  // ------------------------

  // ------------------------ 2
  G4PSTrackCounter *scorer2 =
      new G4PSTrackCounter("TrackCounter2", fCurrent_In);
  primitive = scorer2;

  gapDetector2->SetFilter(neutronFilter);
  gapDetector2->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 3
  G4PSTrackCounter *scorer3 =
      new G4PSTrackCounter("TrackCounter3", fCurrent_In);
  primitive = scorer3;

  gapDetector3->SetFilter(neutronFilter);
  gapDetector3->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 4
  G4PSTrackCounter *scorer4 =
      new G4PSTrackCounter("TrackCounter4", fCurrent_In);
  primitive = scorer4;

  gapDetector4->SetFilter(neutronFilter);
  gapDetector4->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 5
  G4PSTrackCounter *scorer5 =
      new G4PSTrackCounter("TrackCounter5", fCurrent_In);
  primitive = scorer5;
  gapDetector5->SetFilter(neutronFilter);
  gapDetector5->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 6
  G4PSTrackCounter *scorer6 =
      new G4PSTrackCounter("TrackCounter6", fCurrent_In);
  primitive = scorer6;

  gapDetector6->SetFilter(neutronFilter);
  gapDetector6->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 7
  G4PSTrackCounter *scorer7 =
      new G4PSTrackCounter("TrackCounter7", fCurrent_In);
  primitive = scorer7;
  gapDetector7->SetFilter(neutronFilter);
  gapDetector7->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 8
  G4PSTrackCounter *scorer8 =
      new G4PSTrackCounter("TrackCounter8", fCurrent_In);
  primitive = scorer8;
  gapDetector8->SetFilter(neutronFilter);
  gapDetector8->RegisterPrimitive(primitive);

  // ------------------------
  //
  // ------------------------ 9
  G4PSTrackCounter *scorer9 =
      new G4PSTrackCounter("TrackCounter9", fCurrent_In);
  primitive = scorer9;

  gapDetector9->SetFilter(neutronFilter);
  gapDetector9->RegisterPrimitive(primitive);

  // ------------------------

  SetSensitiveDetector("TargetDetLV", TargetDet);
  SetSensitiveDetector("NDetLV", NDet);
  SetSensitiveDetector("gapLV", gapDetector);
  SetSensitiveDetector("gapLV2", gapDetector2);
  SetSensitiveDetector("gapLV3", gapDetector3);
  SetSensitiveDetector("gapLV4", gapDetector4);
  SetSensitiveDetector("gapLV5", gapDetector5);
  SetSensitiveDetector("gapLV6", gapDetector6);
  SetSensitiveDetector("gapLV7", gapDetector7);
  SetSensitiveDetector("gapLV8", gapDetector8);
  SetSensitiveDetector("gapLV9", gapDetector9);

  //
  // Magnetic field
  //
  // Create global magnetic field messenger.
  // Uniform magnetic field is then created automatically if
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

} // namespace B4d
