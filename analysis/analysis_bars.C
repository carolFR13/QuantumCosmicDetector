#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"


void analysis_bars() {

    // Open the ROOT file that contains the hits tree
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

    
    // tree->Print();

    auto c1 = new TCanvas("c1", "Local coordinates", 800, 600);
    c1->Divide(3,1);

    c1->cd(1);
    tree->Draw("lz:lx", "", "COLZ");

    c1->cd(2);
    tree->Draw("lz:lx", "planeID==1", "COLZ"); 

    c1->cd(3);
    tree->Draw("lz:lx", "planeID==2", "COLZ"); 

    auto c2 = new TCanvas("c2", "Global coordinates", 800, 600);
    c2->Divide(3,1);

    c2->cd(1);
    tree->Draw("z:x", "", "COLZ");

    c2->cd(2);
    tree->Draw("z:x", "planeID==1", "COLZ");

    c2->cd(3);
    tree->Draw("z:x", "planeID==2", "COLZ"); 



    tree->SetAlias("dt", "tA - tB");

    auto c3 = new TCanvas("c3", "Time histograms", 1200, 400);
    c3->Divide(3,1);

    c3->cd(1);
    tree->Draw("tA>>hT1(100,0,10)");

    c3->cd(2);
    tree->Draw("tB>>hT2(100,0,10)");

    c3->cd(3);
    tree->Draw("dt>>hDeltaT(100,-10,10)");

}