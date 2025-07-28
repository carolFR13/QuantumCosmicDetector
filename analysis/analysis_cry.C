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
    TFile* f = TFile::Open("all_data.root");
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
    TH1F* h_theta = new TH1F("h_theta", "Theta Distribution;Theta (deg);Counts", 90, 0, 180);
    t->Draw("acos(dirY/sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ))*180/pi >> h_theta");
    h_theta->SetLineColor(kGreen+2);
    h_theta->SetLineWidth(2);

    // === Phi Distribution ===
    TCanvas* c7 = new TCanvas("c7", "Phi Distribution", 800, 600);
    TH1F* h_phi = new TH1F("h_phi", "Phi Distribution;Phi (deg);Counts", 90, -180, 180);
    t->Draw("atan2(dirX, dirZ)*180/pi >> h_phi");
    h_phi->SetLineColor(kRed+2);
    h_phi->SetLineWidth(2);

}




// DEPRECATED MACRO. VALID FOR CSV OUTPUT FILE

// void analysis_cry() {

//     std::ifstream file("cry_output_nt_CRY.csv");

//     if (!file.is_open()) {
//         std::cerr << "Error opening file!" << std::endl;
//         return;
//     }

//     TFile* output = new TFile("output.root", "RECREATE");
//     TTree* tree = new TTree("CRY", "CRY data");

//     double simulationTime, energy, px, py, pz, x, y, z;
//     int eventID;
//     char pname[20];

//     tree->Branch("simulationTime", &simulationTime);
//     tree->Branch("eventID", &eventID);
//     tree->Branch("particleName", &pname, "particleName/C");
//     tree->Branch("energy", &energy);
//     tree->Branch("px", &px);
//     tree->Branch("py", &py);
//     tree->Branch("pz", &pz);
//     tree->Branch("x", &x);
//     tree->Branch("y", &y);
//     tree->Branch("z", &z);

//     std::string line;
//     while (std::getline(file, line)) {
//         if (line.empty() || line[0] == '#') continue;  // saltar líneas de comentario

//         std::stringstream ss(line);
//         std::string item;

//         std::getline(ss, item, ','); simulationTime = std::stod(item);
//         std::getline(ss, item, ','); eventID = std::stoi(item);
//         std::getline(ss, item, ','); strncpy(pname, item.c_str(), 19); pname[19] = '\0';
//         std::getline(ss, item, ','); energy = std::stod(item);
//         std::getline(ss, item, ','); px = std::stod(item);
//         std::getline(ss, item, ','); py = std::stod(item);
//         std::getline(ss, item, ','); pz = std::stod(item);
//         std::getline(ss, item, ','); x = std::stod(item);
//         std::getline(ss, item, ','); y = std::stod(item);
//         std::getline(ss, item, ','); z = std::stod(item);

//         tree->Fill();

//     }

//     tree->Write();
//     output->Close();
//     file.close();

//     TFile* f = TFile::Open("output.root");
//     TTree* t = (TTree*)f->Get("CRY");

//     // Delete existing canvases if they exist
//     if (gROOT->GetListOfCanvases()->FindObject("c1"))
//         delete gROOT->GetListOfCanvases()->FindObject("c1");
//     if (gROOT->GetListOfCanvases()->FindObject("c2"))
//         delete gROOT->GetListOfCanvases()->FindObject("c2");


//     TCanvas* c1 = new TCanvas("c1", "Particle Distribution", 800, 600);
//     TH1F* h_particles = new TH1F("h_particles", "Particle Distribution;Particle Type;Counts", 10, 0, 10);
//     h_particles->SetLineColor(kViolet);
//     h_particles->SetLineWidth(2);
//     h_particles->SetFillColor(kViolet+1);
//     h_particles->SetFillStyle(3004);
//     t->Draw("particleName>>h_particles");


//     // Create canvas for energy spectrum
//     TCanvas* c2 = new TCanvas("c2", "Energy Spectrum", 800, 600);
//     c2->SetLogy(); // Set log scale on y-axis

//     // Create and fill histogram
//     TH1F* h_energy = new TH1F("h_energy", "Energy Spectrum;Energy (MeV);Counts", 100, 0, 18000);
//     t->Draw("energy>>h_energy");
    
//     // Customize histogram
//     h_energy->SetLineColor(kBlue);
//     h_energy->SetLineWidth(2);
//     h_energy->SetFillColor(kBlue-10);
//     h_energy->SetFillStyle(3004);
    
//     // Draw 
//     h_energy->Draw();

//     // Create canvas for energy spectrum of muons
//     TCanvas* c3 = new TCanvas("c3", "Energy Spectrum - Muons", 800, 600);
//     c3->SetLogy(); 

//     // Create histogram for muons only
//     TH1F* h_muons = new TH1F("h_muons", "Energy Spectrum (Muons);Energy (MeV);Counts", 100, 0, 20000);
//     t->Draw("energy>>h_muons", "particleName == \"muon\"");

//     // Customize histogram
//     h_muons->SetLineColor(kRed);
//     h_muons->SetLineWidth(2);
//     h_muons->SetFillColor(kRed-10);
//     h_muons->SetFillStyle(3004);

//     // Draw and save
//     h_muons->Draw();

//     // Create canvas for energy spectrum of electrons
//     TCanvas* c4 = new TCanvas("c4", "Energy Spectrum - Electrons", 800, 600);
//     c4->SetLogy(); 

//     // Create histogram for muons only
//     TH1F* h_electrons = new TH1F("h_electrons", "Energy Spectrum (Electrons);Energy (MeV);Counts", 100, 0, 11000);
//     t->Draw("energy>>h_electrons", "particleName == \"electron\"");

//     // Customize histogram
//     h_electrons->SetLineColor(kOrange);
//     h_electrons->SetLineWidth(2);
//     h_electrons->SetFillColor(kOrange-2);
//     h_electrons->SetFillStyle(3004);

//     // Draw and save
//     h_electrons->Draw();

//     TCanvas* c5 = new TCanvas("c5", "Incidence Plane", 800, 700);
//     TH2F* h2_incidence = new TH2F("h2_incidence", "Incidence Plane;x (mm);z (mm)", 100, -1500, 1500, 100, -1500, 1500);

//     // fill the 2D histogram
//     t->Draw("z:x >> h2_incidence", "", "COLZ");

//     TCanvas* c6 = new TCanvas("c6", "Theta Distribution", 800, 600);
//     TH1F* h_theta = new TH1F("h_theta", "Theta Distribution;Theta (deg);Counts", 90, 0, 180);

//     // theta = acos(pz / sqrt(px^2 + py^2 + pz^2)) * 180/pi
//     t->Draw("acos(py/sqrt(px*px + py*py + pz*pz))*180/pi >> h_theta");

//     h_theta->SetLineColor(kGreen+2);
//     h_theta->SetLineWidth(2);



//     TCanvas* c7 = new TCanvas("c7", "Phi Distribution", 800, 600);
//     TH1F* h_phi = new TH1F("h_phi", "Phi Distribution;Phi (deg);Counts", 90, -180, 180);

//     // phi = atan2(py, px) * 180/pi
//     t->Draw("atan2(px, pz)*180/pi >> h_phi");

//     h_phi->SetLineColor(kRed+2);
//     h_phi->SetLineWidth(2);



//     // Save canvases
//     // c1->SaveAs("particle_distribution.png");
//     // c2->SaveAs("energy_spectrum.png");
//     // c3->SaveAs("energy_spectrum_muons.png");
//     // c5->SaveAs("incidence_plane.png");
//     // c6->SaveAs("theta_distribution.png");
//     // c7->SaveAs("phi_distribution.png");



// }