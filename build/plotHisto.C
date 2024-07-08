
// ROOT macro file for plotting example B4 histograms
//
// Can be run from ROOT session:
// root[0] .x plotHisto.C

{
  gROOT->Reset();
  gROOT->SetStyle("Plain");

  // Draw histos filled by Geant4 simulation
  //

  // Open file filled by Geant4 simulation
  TFile f("B4.root");

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 10, 10);

  // set logarithmic scale for y
  //gPad->SetLogy(1);
  TH1S* hist1 = (TH1S*)f.Get("TCount");
  hist1->Draw("HIST");
}
