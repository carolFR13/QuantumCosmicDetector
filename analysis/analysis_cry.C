#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"


void analysis_cry() {
    // Open the ROOT file that contains the CRY tree
    TFile* f = TFile::Open("data/all_data_with_qpu_complete.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return;
    }

    TTree* t = (TTree*)f->Get("primaries");
    if (!t) {
        std::cerr << "Tree 'primaries' not found in file!" << std::endl;
        return;
    }


    char pname[16];
    double ke;
    t->SetBranchAddress("pName", pname);
    t->SetBranchAddress("ke", &ke);

    TH1F* h_muons = new TH1F("h_muons", "Energy Spectrum (Muons);Energy (MeV);Counts", 100, 0, 20000);
    TH1F* h_electrons = new TH1F("h_electrons", "Energy Spectrum (Electrons);Energy (MeV);Counts", 100, 0, 11000);

    Long64_t nentries = t->GetEntries();
    for (Long64_t i = 0; i < nentries; ++i) {
        t->GetEntry(i);
        if (strcmp(pname, "mu-") == 0 || strcmp(pname, "mu+") == 0)
            h_muons->Fill(ke);
        else if (strcmp(pname, "e-") == 0 || strcmp(pname, "e+") == 0)
            h_electrons->Fill(ke);

    }

    
    //t->Print();

    // === Particle Distribution ===
    TCanvas* c1 = new TCanvas("c1", "Particle Distribution", 800, 600);
    TH1F* h_particles = new TH1F("h_particles", "Particle Distribution;Particle Type;Counts", 10, 0, 10);
    h_particles->SetLineColor(kViolet);
    h_particles->SetLineWidth(2);
    h_particles->SetFillColor(kViolet+1);
    h_particles->SetFillStyle(3004);
    t->Draw("pName>>h_particles");

    // === Energy Spectrum ===
    TCanvas* c2 = new TCanvas("c2", "Energy Spectrum", 800, 600);
    c2->SetLogy();
    TH1F* h_energy = new TH1F("h_energy", "Energy Spectrum;Energy (MeV);Counts", 100, 0, 18000);
    t->Draw("ke>>h_energy");
    h_energy->SetLineColor(kBlue);
    h_energy->SetLineWidth(2);
    h_energy->SetFillColor(kBlue-10);
    h_energy->SetFillStyle(3004);
    h_energy->Draw();

    // === Muon Energy Spectrum ===
    TCanvas* c3 = new TCanvas("c3", "Energy Spectrum - Muons", 800, 600);
    c3->SetLogy();
    //TH1F* h_muons = new TH1F("h_muons", "Energy Spectrum (Muons);Energy (MeV);Counts", 100, 0, 20000);
    //t->Draw("ke>>h_muons", "strcmp(pName, \"mu+\") == 0 || strcmp(pName, \"mu-\") == 0");
    h_muons->SetLineColor(kRed);
    h_muons->SetLineWidth(2);
    h_muons->SetFillColor(kRed-10);
    h_muons->SetFillStyle(3004);
    h_muons->Draw();

    // === Electron Energy Spectrum ===
    TCanvas* c4 = new TCanvas("c4", "Energy Spectrum - Electrons", 800, 600);
    c4->SetLogy();
    //TH1F* h_electrons = new TH1F("h_electrons", "Energy Spectrum (Electrons);Energy (MeV);Counts", 100, 0, 11000);
    //t->Draw("ke>>h_electrons", "strcmp(pName, \"e+\") == 0 || strcmp(pName, \"e-\") == 0");
    h_electrons->SetLineColor(kOrange);
    h_electrons->SetLineWidth(2);
    h_electrons->SetFillColor(kOrange-2);
    h_electrons->SetFillStyle(3004);
    h_electrons->Draw();

    // === Incidence Plane ===
    TCanvas* c5 = new TCanvas("c5", "Incidence Plane", 800, 700);
    TH2F* h2_incidence = new TH2F("h2_incidence", "Incidence Plane;x (mm);z (mm)", 100, -1500, 1500, 100, -1500, 1500);
    t->Draw("z:x >> h2_incidence", "", "COLZ");

    // === Theta Distribution ===
    TCanvas* c6 = new TCanvas("c6", "Theta Distribution", 800, 600);
    TH1F* h_theta = new TH1F("h_theta", "Theta Distribution;#theta (#circ);Counts", 90, 0, 180);
    t->Draw("acos(dirY/sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ))*180/pi >> h_theta");
    h_theta->SetLineColor(kGreen+2);
    h_theta->SetLineWidth(2);

    // === Phi Distribution ===
    TCanvas* c7 = new TCanvas("c7", "Phi Distribution", 800, 600);
    TH1F* h_phi = new TH1F("h_phi", "Phi Distribution;#varphi (#circ);Counts", 90, -180, 180);
    t->Draw("atan2(dirX, dirZ)*180/pi >> h_phi");
    h_phi->SetLineColor(kRed+2);
    h_phi->SetLineWidth(2);

}


