#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"
#include <iostream>
#include <map>  

void initial_plot(TTree* tree, const std::string& label) {

    std::string cleanLabel = label;
    std::replace(cleanLabel.begin(), cleanLabel.end(), ' ', '_');

    auto c1 = new TCanvas(("c1_" + cleanLabel).c_str(), ("Local coordinates - " + cleanLabel).c_str(), 800, 600);
    c1->Divide(3,1);
    c1->cd(1); tree->Draw("lz:lx", "", "COLZ");
    c1->cd(2); tree->Draw("lz:lx", "planeID==1", "COLZ");
    c1->cd(3); tree->Draw("lz:lx", "planeID==2", "COLZ");

    auto c2 = new TCanvas(("c2_" + cleanLabel).c_str(), ("Global coordinates - " + cleanLabel).c_str(), 800, 600);
    c2->Divide(3,1);
    c2->cd(1); tree->Draw("z:x", "", "COLZ");
    c2->cd(2); tree->Draw("z:x", "planeID==1", "COLZ");
    c2->cd(3); tree->Draw("z:x", "planeID==2", "COLZ");

    auto c3 = new TCanvas(("c3_" + cleanLabel).c_str(), ("Time histograms - " + cleanLabel).c_str(), 1200, 400);
    c3->Divide(3,1);
    // Use the clean label (no spaces) for histogram names
    c3->cd(1); tree->Draw(Form("tA>>hT1_%s(100,0,10)", cleanLabel.c_str()));
    c3->cd(2); tree->Draw(Form("tB>>hT2_%s(100,0,10)", cleanLabel.c_str()));
    c3->cd(3); tree->Draw(Form("tA-tB>>hDeltaT_%s(100,-10,10)", cleanLabel.c_str()));


}

TTree* filter_first_hits(TTree* tree) {
    // Variables to read input
    Int_t event;
    char volumeName[128];  
    Double_t tA, tB, tG, x, y, z, lx, ly, lz, Edep;
    Int_t barID, planeID;
    char pName[128];

    // Set branches
    tree->SetBranchAddress("event", &event);
    tree->SetBranchAddress("barID", &barID);
    tree->SetBranchAddress("pName", &pName);
    tree->SetBranchAddress("x", &x);
    tree->SetBranchAddress("y", &y);
    tree->SetBranchAddress("z", &z);
    tree->SetBranchAddress("lx", &lx);
    tree->SetBranchAddress("ly", &ly);
    tree->SetBranchAddress("lz", &lz);
    tree->SetBranchAddress("tG", &tG);
    tree->SetBranchAddress("tA", &tA);
    tree->SetBranchAddress("tB", &tB);
    tree->SetBranchAddress("Edep", &Edep);
    tree->SetBranchAddress("planeID", &planeID);
    tree->SetBranchAddress("volumeName", &volumeName);

    // Create output tree
    TTree* filtered = new TTree("filtered_hits", "First hits per bar per event");

    // Output variables
    char outVolName[128];
    filtered->Branch("event", &event, "event/I");
    filtered->Branch("barID", &barID, "barID/I");
    filtered->Branch("pName", &pName, "pName[128]/C");
    filtered->Branch("x", &x, "x/D");
    filtered->Branch("y", &y, "y/D");
    filtered->Branch("z", &z, "z/D");
    filtered->Branch("lx", &lx, "lx/D");
    filtered->Branch("ly", &ly, "ly/D");
    filtered->Branch("lz", &lz, "lz/D");
    filtered->Branch("tG", &tG, "tG/D");
    filtered->Branch("tA", &tA, "tA/D");
    filtered->Branch("tB", &tB, "tB/D");
    filtered->Branch("Edep", &Edep, "Edep/D");
    filtered->Branch("planeID", &planeID, "planeID/I");
    filtered->Branch("volumeName", &outVolName,  "volumeName[128]/C");


    std::map<std::pair<int, std::string>, int> bestHitIndex;
    std::map<std::pair<int, std::string>, double> bestTime;

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);

        double time = TMath::Min(tA, tB);
        auto key = std::make_pair(event, std::string(volumeName));

        if (bestHitIndex.find(key) == bestHitIndex.end() || time < bestTime[key]) {
            bestHitIndex[key] = i;
            bestTime[key] = time;
        }
    }

    // Segundo loop
    for (const auto& pair : bestHitIndex) {
        tree->GetEntry(pair.second);
        std::strncpy(outVolName, volumeName, sizeof(outVolName));
        filtered->Fill();
    }

    return filtered;
}

void analysis_bars() {

    TFile* f = TFile::Open("data/all_data_4.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return;
    }

    TTree* tree = (TTree*)f->Get("hits");
    
    if (!tree) {
        std::cerr << "Tree 'hits' not found in file!" << std::endl;
        return;
    }

    // Plot original data
    initial_plot(tree, "Original Data");

    // Filtered data: only the first hit per bar per event
    TTree* filteredTree = filter_first_hits(tree);

    // Plot filtered data
    initial_plot(filteredTree, "Filtered Data");
 }
