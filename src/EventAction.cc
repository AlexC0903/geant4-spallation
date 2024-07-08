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
/// \file B4/B4d/src/EventAction.cc
/// \brief Implementation of the B4d::EventAction class

#include "EventAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4UnitsTable.hh"

#include "Randomize.hh"
#include <iomanip>

namespace B4d {

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4THitsMap<G4double> *
EventAction::GetHitsCollection(G4int hcID, const G4Event *event) const {
  auto hitsCollection = static_cast<G4THitsMap<G4double> *>(
      event->GetHCofThisEvent()->GetHC(hcID));

  if (!hitsCollection) {
    G4ExceptionDescription msg;
    msg << "Cannot access hitsCollection ID " << hcID;
    G4Exception("EventAction::GetHitsCollection()", "MyCode0003",
                FatalException, msg);
  }

  return hitsCollection;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double EventAction::GetSum(G4THitsMap<G4double> *hitsMap) const {
  G4double sumValue = 0.;
  for (auto it : *hitsMap->GetMap()) {
    // hitsMap->GetMap() returns the map of std::map<G4int, G4double*>
    sumValue += *(it.second);
  }
  return sumValue;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event * /*event*/) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event *event) {
  // Get hist collections IDs
  fGapTrackCounterHCID =
      G4SDManager::GetSDMpointer()->GetCollectionID("Gap/TrackCounter");
  fTargetTrackLengthHCID =
      G4SDManager::GetSDMpointer()->GetCollectionID("TargetDet/TrackLength");
  fNTrackCounterHCID =
      G4SDManager::GetSDMpointer()->GetCollectionID("NDet/TrackCounter");

  // Get sum values from hits collections
  auto gapTrackCounter = GetSum(GetHitsCollection(fGapTrackCounterHCID, event));
  auto TargetTrackLength =
      GetSum(GetHitsCollection(fTargetTrackLengthHCID, event));
  auto NTrackCounter = GetSum(GetHitsCollection(fNTrackCounterHCID, event));

  // get analysis manager
  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

  // fill histograms
  //
  analysisManager->FillH1(0, gapTrackCounter);

  // fill ntuple
  //
  analysisManager->FillNtupleDColumn(0, gapTrackCounter);
  analysisManager->FillNtupleDColumn(1, TargetTrackLength);
  analysisManager->FillNtupleDColumn(2, NTrackCounter);
  analysisManager->AddNtupleRow();

  // print per event (modulo n)
  //   //
  //   auto eventID = event->GetEventID();
  //   auto printModulo = G4RunManager::GetRunManager()->GetPrintProgress();
  //   if ((printModulo > 0) && (eventID % printModulo == 0)) {
  //     // PrintEventStatistics(absoEdep, absoTrackLength, gapEdep,
  //     gapTrackLength); G4cout << "--> End of event: " << eventID << "\n" <<
  //     G4endl;
  //   }
  // }

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
} // namespace B4d
} // namespace B4d
