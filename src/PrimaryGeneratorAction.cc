

#include "PrimaryGeneratorAction.hh"

#include "G4Box.hh"
#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace B4 {
PrimaryGeneratorAction::PrimaryGeneratorAction() {
  fParticleGun = new G4ParticleGun(0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction() { delete fParticleGun; }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent) {
  G4int n_particlePo = 4200;
  G4int n_particlePi = 2200;
  G4int n_particlePr = 1100;
  // 1 GeV 5700 1100 1100 Sum: 7900
  // 1.5 GeV 4200 2200 1100 Sum: 7500
  // 2 GeV 3100 3700 1100 Sum: 7900

  G4ParticleDefinition *po =
      G4ParticleTable::GetParticleTable()->FindParticle("e+");
  G4ParticleDefinition *pi =
      G4ParticleTable::GetParticleTable()->FindParticle("pi+");
  G4ParticleDefinition *pr =
      G4ParticleTable::GetParticleTable()->FindParticle("proton");
  fParticleGun->SetParticlePosition(G4ThreeVector(-1 * m, 0. * cm, 0. * m));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(1., 0., 0.));

  fParticleGun->SetParticleMomentum(1.5 * GeV);

  fParticleGun->SetNumberOfParticles(n_particlePo);
  fParticleGun->SetParticleDefinition(po);
  fParticleGun->GeneratePrimaryVertex(anEvent);

  fParticleGun->SetParticleDefinition(pi);
  fParticleGun->SetNumberOfParticles(n_particlePi);
  fParticleGun->GeneratePrimaryVertex(anEvent);
  fParticleGun->SetParticleDefinition(pr);
  fParticleGun->SetNumberOfParticles(n_particlePr);
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
} // namespace B4
