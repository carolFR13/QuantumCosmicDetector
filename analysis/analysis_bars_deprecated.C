#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <map>  
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <tuple>

struct GeometryEntry {
    std::string volumeName;
    double x, y, z;
    GeometryEntry(const std::string& name, double x_, double y_, double z_) 
        : volumeName(name), x(x_), y(y_), z(z_) {}
};

std::vector<GeometryEntry> read_geometry_vector(const std::string& filename) {
    std::vector<GeometryEntry> geometry;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open geometry file: " << filename << std::endl;
        return geometry;
    }

    // Skip header line
    std::getline(file, line);
    std::cout << "Skipping header: " << line << std::endl;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string volumeName;
        std::string xStr, yStr, zStr;

        if (std::getline(ss, volumeName, ',') &&
            std::getline(ss, xStr, ',') &&
            std::getline(ss, yStr, ',') &&
            std::getline(ss, zStr)) {  

            // Trim whitespace
            // volumeName.erase(0, volumeName.find_first_not_of(" \t"));
            // volumeName.erase(volumeName.find_last_not_of(" \t") + 1);
            // xStr.erase(0, xStr.find_first_not_of(" \t"));
            // xStr.erase(xStr.find_last_not_of(" \t") + 1);
            // yStr.erase(0, yStr.find_first_not_of(" \t"));
            // yStr.erase(yStr.find_last_not_of(" \t") + 1);
            // zStr.erase(0, zStr.find_first_not_of(" \t"));
            // zStr.erase(zStr.find_last_not_of(" \t") + 1);

            // Simple manual trimming (avoid problematic string methods)
            auto trim = [](std::string& s) {
                while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
                    s.erase(0, 1);
                }
                while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
                    s.pop_back();
                }
            };
            
            trim(volumeName);
            trim(xStr);
            trim(yStr);
            trim(zStr);

            try {
                double x = std::stod(xStr);
                double y = std::stod(yStr);  
                double z = std::stod(zStr);
                geometry.emplace_back(volumeName, x, y, z);
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse numbers in line: " << line << std::endl;
            }
        }
    }
    std::cout << "Loaded " << geometry.size() << " geometry entries" << std::endl;
    return geometry;
}

void initial_plot(TTree* tree, const std::string& label) {

    std::string cleanLabel = label;
    std::replace(cleanLabel.begin(), cleanLabel.end(), ' ', '_');

    // Add a timestamp to make names unique (instead of deleting)
    static int plotCounter = 0;
    plotCounter++;
    std::string uniqueLabel = cleanLabel + "_" + std::to_string(plotCounter);


    auto c1 = new TCanvas(("c1_" + uniqueLabel).c_str(), ("Local coordinates - " + cleanLabel).c_str(), 1200, 400);
    c1->Divide(3,1);
    c1->cd(1); tree->Draw("lz:lx", "", "COLZ");
    c1->cd(2); tree->Draw("lz:lx", "planeID==1", "COLZ");
    c1->cd(3); tree->Draw("lz:lx", "planeID==2", "COLZ");
    c1->Update();
    c1->Draw();
    gSystem->ProcessEvents();

    auto c2 = new TCanvas(("c2_" + uniqueLabel).c_str(), ("Global coordinates - " + cleanLabel).c_str(), 1200, 400);
    c2->Divide(3,1);
    c2->cd(1); tree->Draw("z:x", "", "COLZ");
    c2->cd(2); tree->Draw("z:x", "planeID==1", "COLZ");
    c2->cd(3); tree->Draw("z:x", "planeID==2", "COLZ");
    c2->Update();
    c2->Draw();
    gSystem->ProcessEvents();


    auto c3 = new TCanvas(("c3_" + uniqueLabel).c_str(), ("Time histograms - " + cleanLabel).c_str(), 1200, 400);
    c3->Divide(4,1);
        
    c3->cd(1); tree->Draw("tA");
    c3->cd(2); tree->Draw("tB");
    c3->cd(3); tree->Draw("tB-tA");
    c3->cd(4); tree->Draw("tB+tA");
    c3->Update();
    c3->Draw();
    gSystem->ProcessEvents();

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
    filtered->Branch("volumeName", &outVolName, "volumeName[128]/C");


    // Map to store: (eventID, volumeName) -> {entryIndex, time}
    std::map<std::pair<int, std::string>, std::pair<int, double>> bestHitMap;

    Long64_t nEntries = tree->GetEntries();
    std::cout << "Processing " << nEntries << " entries for filtering..." << std::endl;
    
    // First pass: find the earliest hit for each (event, volume) combination
    for (Long64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);

        if (i % 100000 == 0) {
            std::cout << "Processing entry " << i << "..." << std::endl;
        }

        double time = TMath::Min(tA, tB);
        std::string volName(volumeName);

        // Create key for this event-volume combination
        auto key = std::make_pair(event, volName);

        // Check if we already have a hit for this event-volume, or if this one is earlier
        if (bestHitMap.find(key) == bestHitMap.end() || time < bestHitMap[key].second) {
            bestHitMap[key] = std::make_pair(i, time);
        }
    }

    std::cout << "Found " << bestHitMap.size() << " unique (event, volume) combinations" << std::endl;
    std::cout << "This should be much smaller than " << nEntries << " original entries" << std::endl;


    // Second pass: copy the selected entries to the filtered tree
    int copiedEntries = 0;
    for (const auto& mapEntry : bestHitMap) {
        Long64_t entryIndex = mapEntry.second.first; // Get the entry index
        tree->GetEntry(entryIndex);
        
        // Copy volumeName to output buffer
        strncpy(outVolName, volumeName, 127);
        outVolName[127] = '\0';
        
        filtered->Fill();
        copiedEntries++;
        
        if (copiedEntries % 1000 == 0) {
            std::cout << "Copied " << copiedEntries << " entries..." << std::endl;
        }
    }

    std::cout << "Copied " << copiedEntries << " entries to filtered tree" << std::endl;
    std::cout << "Reduction factor: " << (double)nEntries / copiedEntries << "x" << std::endl;

    // Save to file with error checking
    TFile* tempFile = new TFile("data/filtered_data.root", "RECREATE");

    if (!tempFile || tempFile->IsZombie()) {
        std::cerr << "ERROR: Could not create filtered data file!" << std::endl;
        return filtered;
    }

    tempFile->cd();
    filtered->Write("filtered_hits");
    tempFile->Close();
    delete tempFile;

    std::cout << "Filtered tree saved to data/filtered_data.root" << std::endl;

    return filtered;

}



TTree* process_data(TTree* filtered_tree,
                    const std::vector<GeometryEntry>& geometry,
                    double timeSmearSigma_ns = 0.5){

    std::cout << "=== Starting process_data ===" << std::endl;

    // Variables to read branches - MATCH the filtered tree exactly
    int planeID;
    double tA, tB, x, y, z;
    char volumeName[128];  // Use TString to match what's in the filtered tree
    

    // Set branch addresses
    filtered_tree->SetBranchAddress("planeID", &planeID);
    filtered_tree->SetBranchAddress("tA", &tA);
    filtered_tree->SetBranchAddress("tB", &tB);
    filtered_tree->SetBranchAddress("x", &x);
    filtered_tree->SetBranchAddress("y", &y);
    filtered_tree->SetBranchAddress("z", &z);
    filtered_tree->SetBranchAddress("volumeName", &volumeName);  // TString address

    if (filtered_tree->GetBranch("volumeName") == nullptr) {
        std::cerr << "Branch 'volumeName' not found!" << std::endl;
        return nullptr;
    }

    // Create output tree
    TTree* reco_tree = new TTree("recoTree", "Reconstructed Coordinates");
    double xReco, yReco, zReco;
    double xRecoSmeared, yRecoSmeared, zRecoSmeared;
    double xOrig, yOrig, zOrig;
    double tA_smeared, tB_smeared;
    double tA_Orig, tB_Orig;
    double sigma_x, sigma_y, sigma_z;
    int PlaneID_Orig;

    reco_tree->Branch("xReco", &xReco);
    reco_tree->Branch("yReco", &yReco);
    reco_tree->Branch("zReco", &zReco);

    reco_tree->Branch("xRecoSmeared", &xRecoSmeared);
    reco_tree->Branch("yRecoSmeared", &yRecoSmeared);
    reco_tree->Branch("zRecoSmeared", &zRecoSmeared);

    reco_tree->Branch("tA_smeared", &tA_smeared);
    reco_tree->Branch("tB_smeared", &tB_smeared);

    reco_tree->Branch("tA", &tA_Orig);
    reco_tree->Branch("tB", &tB_Orig);

    reco_tree->Branch("x", &xOrig);
    reco_tree->Branch("y", &yOrig);
    reco_tree->Branch("z", &zOrig);

    reco_tree->Branch("sigma_x", &sigma_x);
    reco_tree->Branch("sigma_y", &sigma_y);
    reco_tree->Branch("sigma_z", &sigma_z);

    reco_tree->Branch("planeID", &PlaneID_Orig);

    double vg = 299.792458 / 1.58;
    Long64_t nentries = filtered_tree->GetEntries();
    
    std::cout << "Processing " << nentries << " entries..." << std::endl;
        
    // random generator
    TRandom3 rng(42); // fixed seed for reproducibility
    
    for (Long64_t i = 0; i < nentries; ++i) {
        filtered_tree->GetEntry(i);

        if (i == 0) {  // Just print the first entry for debug
            std::cout << "First entry volumeName: '" << volumeName << "'" << std::endl;
        }
        
        std::string volNameStr(volumeName);
        
        if (volNameStr.empty()) continue;
        
        // Find geometry
        double geom_x = 0, geom_y = 0, geom_z = 0;
        bool found = false;
        
        for (const auto& entry : geometry) {
            if (entry.volumeName == volNameStr) {
                geom_x = entry.x;
                geom_y = entry.y;
                geom_z = entry.z;
                found = true;
                break;
            }
        }
        
        if (!found) continue;
        
        // Calculate reconstruction
        double deltaT = tB - tA;
        double reco_coord = deltaT * vg / 2.0;

        // Smear the times
        tA_smeared = tA + rng.Gaus(0, timeSmearSigma_ns);
        tB_smeared = tB + rng.Gaus(0, timeSmearSigma_ns);

        double deltaT_smeared = tB_smeared - tA_smeared;
        double reco_coord_smeared = deltaT_smeared * vg / 2.0;

        const double sigma_geom = 15.0 / std::sqrt(12);  // ≈ 4.33 mm
        const double sigma_time = timeSmearSigma_ns;             // ns
        const double sigma_reco = sigma_time * vg / std::sqrt(2);  

        // Set coordinates
        if (planeID == 1) {
            xReco = geom_x;
            yReco = geom_y;
            zReco = reco_coord;

            sigma_x = sigma_geom;
            sigma_y = sigma_geom;
            sigma_z = sigma_reco;

            xRecoSmeared = geom_x;
            yRecoSmeared = geom_y;
            zRecoSmeared = reco_coord_smeared;

        } else if (planeID == 2) {
            xReco = reco_coord;
            yReco = geom_y;
            zReco = geom_z;

            sigma_x = sigma_reco;
            sigma_y = sigma_geom;
            sigma_z = sigma_geom;

            xRecoSmeared = reco_coord_smeared;
            yRecoSmeared = geom_y;
            zRecoSmeared = geom_z;
        } else {
            continue;
        }

        tA_Orig = tA;
        tB_Orig = tB;

        xOrig = x;
        yOrig = y;
        zOrig = z;

        PlaneID_Orig = planeID;
        
        reco_tree->Fill();
    }
    
    std::cout << "Completed processing. Output tree has " << reco_tree->GetEntries() << " entries." << std::endl;
    return reco_tree;
}

void plotRecoAnalysis(TTree* tree) {
    const int N = tree->GetEntries();
    std::cout << "Plotting analysis for " << N << " entries" << std::endl;

    // Variables to read from tree (one at a time)
    double x, y, z;
    double xReco, yReco, zReco;
    double xRecoSmeared, yRecoSmeared, zRecoSmeared;
    double sigma_x, sigma_y, sigma_z;
    int planeID;

    // Set branch addresses (single variables, not vectors)
    tree->SetBranchAddress("x", &x);
    tree->SetBranchAddress("y", &y);
    tree->SetBranchAddress("z", &z);
    tree->SetBranchAddress("xReco", &xReco);
    tree->SetBranchAddress("yReco", &yReco);
    tree->SetBranchAddress("zReco", &zReco);
    tree->SetBranchAddress("xRecoSmeared", &xRecoSmeared);
    tree->SetBranchAddress("yRecoSmeared", &yRecoSmeared);
    tree->SetBranchAddress("zRecoSmeared", &zRecoSmeared);
    tree->SetBranchAddress("sigma_x", &sigma_x);
    tree->SetBranchAddress("sigma_y", &sigma_y);
    tree->SetBranchAddress("sigma_z", &sigma_z);
    tree->SetBranchAddress("planeID", &planeID);

    // Vectors to store all data
    std::vector<double> vx(N), vy(N), vz(N);
    std::vector<double> vxReco(N), vyReco(N), vzReco(N);
    std::vector<double> vxRecoSmeared(N), vyRecoSmeared(N), vzRecoSmeared(N);
    std::vector<double> vsigma_x(N), vsigma_y(N), vsigma_z(N);
    std::vector<int> vplaneID(N);

    // Read all entries into vectors
    for (int i = 0; i < N; ++i) {
        tree->GetEntry(i);
        vx[i] = x; vy[i] = y; vz[i] = z;
        vxReco[i] = xReco; vyReco[i] = yReco; vzReco[i] = zReco;
        vxRecoSmeared[i] = xRecoSmeared; vyRecoSmeared[i] = yRecoSmeared; vzRecoSmeared[i] = zRecoSmeared;
        vsigma_x[i] = sigma_x; vsigma_y[i] = sigma_y; vsigma_z[i] = sigma_z;
        vplaneID[i] = planeID;
    }

    // ---------- 1. X, Y, Z subplots with Original, Reco, and Smeared data ------------

     // Create canvas with unique timestamp
    static int recoPlotCounter = 0;
    recoPlotCounter++;
    std::string recoLabel = "reco_" + std::to_string(recoPlotCounter);


    TCanvas *c_multi = new TCanvas(("c_multi_" + recoLabel).c_str(), "Original vs Reco vs Smeared", 1200, 900);
    c_multi->Divide(1, 3);

    auto draw_subplot = [&](int pad, const std::vector<double>& orig, const std::vector<double>& reco,
                            const std::vector<double>& smeared, const std::vector<double>& sigma, const std::string& label) {
        
        // Create graphs
        TGraph *gOrig = new TGraph(N);
        TGraphErrors *gReco = new TGraphErrors(N);
        TGraphErrors *gSmeared = new TGraphErrors(N);

        // Fill graphs with data
        for (int i = 0; i < N; ++i) {
            gOrig->SetPoint(i, i, orig[i]);
            gReco->SetPoint(i, i, reco[i]);
            gReco->SetPointError(i, 0, sigma[i]);  // Error bars only in Y direction
            gSmeared->SetPoint(i, i, smeared[i]);
            gSmeared->SetPointError(i, 0, sigma[i]);  // Error bars only in Y direction
        }

        // Style the graphs
        gOrig->SetMarkerStyle(20); 
        gOrig->SetMarkerColor(kBlack);
        gOrig->SetMarkerSize(0.5);
        gOrig->SetLineColor(kBlack);
        
        gReco->SetMarkerStyle(21); 
        gReco->SetMarkerColor(kBlue);
        gReco->SetLineColor(kBlue);
        gReco->SetMarkerSize(0.4);
        
        gSmeared->SetMarkerStyle(22); 
        gSmeared->SetMarkerColor(kRed);
        gSmeared->SetLineColor(kRed);
        gSmeared->SetMarkerSize(0.4);

        // Draw on the correct pad
        c_multi->cd(pad);
        
        // Set title and axis labels
        gOrig->SetTitle((label + ";Entry;Position (mm)").c_str());
        gOrig->Draw("AP");  // Draw with axes and points
        gReco->Draw("P same");  // Draw points with error bars
        gSmeared->Draw("P same");  // Draw points with error bars

        // Add legend
        auto legend = new TLegend(0.7, 0.75, 0.88, 0.88);
        legend->AddEntry(gOrig, "Original", "p");
        legend->AddEntry(gReco, "Reco", "p");
        legend->AddEntry(gSmeared, "Smeared", "p");
        legend->Draw();

        // Add grid
        gPad->SetGrid();
    };

    // Create the three subplots
    draw_subplot(1, vx, vxReco, vxRecoSmeared, vsigma_x, "X Coordinate");
    draw_subplot(2, vy, vyReco, vyRecoSmeared, vsigma_y, "Y Coordinate");
    draw_subplot(3, vz, vzReco, vzRecoSmeared, vsigma_z, "Z Coordinate");

    c_multi->Update();
    c_multi->Draw();
    gSystem->ProcessEvents();

     // ---------- 2. Difference histograms with optimal ranges ------------

    // Calculate optimal ranges from the data
    std::vector<double> deltaX_vals, deltaZ_vals;
    for (int i = 0; i < N; ++i) {
        if (vplaneID[i] == 1) {
            deltaZ_vals.push_back(vzRecoSmeared[i] - vz[i]);
        } else if (vplaneID[i] == 2) {
            deltaX_vals.push_back(vxRecoSmeared[i] - vx[i]);
        }
    }

    // Find min/max for appropriate ranges
    double minDeltaZ = *std::min_element(deltaZ_vals.begin(), deltaZ_vals.end());
    double maxDeltaZ = *std::max_element(deltaZ_vals.begin(), deltaZ_vals.end());
    double minDeltaX = *std::min_element(deltaX_vals.begin(), deltaX_vals.end());
    double maxDeltaX = *std::max_element(deltaX_vals.begin(), deltaX_vals.end());

    // Add some padding (10% on each side)
    double rangeZ = (maxDeltaZ - minDeltaZ) * 0.1;
    double rangeX = (maxDeltaX - minDeltaX) * 0.1;

    std::cout << "DeltaZ range: [" << minDeltaZ-rangeZ << ", " << maxDeltaZ+rangeZ << "]" << std::endl;
    std::cout << "DeltaX range: [" << minDeltaX-rangeX << ", " << maxDeltaX+rangeX << "]" << std::endl;

    TH1F *hDeltaX = new TH1F("hDeltaX", "xRecoSmeared - xOrig (Plane 2);#Delta x (mm);Events", 
                             50, minDeltaX-rangeX, maxDeltaX+rangeX);
    TH1F *hDeltaZ = new TH1F("hDeltaZ", "zRecoSmeared - zOrig (Plane 1);#Delta z (mm);Events", 
                             50, minDeltaZ-rangeZ, maxDeltaZ+rangeZ);

    // Fill histograms
    for (double val : deltaZ_vals) {
        hDeltaZ->Fill(val);
    }
    for (double val : deltaX_vals) {
        hDeltaX->Fill(val);
    }

    // Debug output
    std::cout << "hDeltaZ entries: " << hDeltaZ->GetEntries() << ", mean: " << hDeltaZ->GetMean() << std::endl;
    std::cout << "hDeltaX entries: " << hDeltaX->GetEntries() << ", mean: " << hDeltaX->GetMean() << std::endl;

    TCanvas *c_diff = new TCanvas(("c_diff_" + recoLabel).c_str(), "Differences by Plane", 1000, 600);
    c_diff->Divide(2, 1);

    c_diff->cd(1);
    hDeltaZ->SetLineColor(kBlue);
    hDeltaZ->SetFillColor(kBlue);
    hDeltaZ->SetFillStyle(3001);
    hDeltaZ->Draw();
    gPad->SetGrid();
    gPad->Update();

    c_diff->cd(2);
    hDeltaX->SetLineColor(kRed);
    hDeltaX->SetFillColor(kRed);
    hDeltaX->SetFillStyle(3001);
    hDeltaX->Draw();
    gPad->SetGrid();
    gPad->Update();

    c_diff->Update();
    c_diff->Draw();
    gSystem->ProcessEvents(); // Force display

    // ---------- New: Difference histograms (Reco - Original) ------------

    TH1F *hDeltaXReco = new TH1F("hDeltaXReco", "xReco - xOrig (Plane 2);#Delta x (mm);Events", 100, -300, 300);
    TH1F *hDeltaZReco = new TH1F("hDeltaZReco", "zReco - zOrig (Plane 1);#Delta z (mm);Events", 100, -200, 200);

    for (int i = 0; i < N; ++i) {
        if (vplaneID[i] == 1) {
            hDeltaZReco->Fill(vzReco[i] - vz[i]);
        } else if (vplaneID[i] == 2) {
            hDeltaXReco->Fill(vxReco[i] - vx[i]);
        }
    }

    // Debug output
    std::cout << "hDeltaZReco entries: " << hDeltaZReco->GetEntries() << ", mean: " << hDeltaZReco->GetMean() << std::endl;
    std::cout << "hDeltaXReco entries: " << hDeltaXReco->GetEntries() << ", mean: " << hDeltaXReco->GetMean() << std::endl;

    TCanvas *c_diffReco = new TCanvas("c_diffReco", "Reco - Original Differences", 1000, 600);
    c_diffReco->Divide(2, 1);

    c_diffReco->cd(1);
    hDeltaZReco->SetLineColor(kGreen + 2);
    hDeltaZReco->SetFillColor(kGreen + 2);
    hDeltaZReco->SetFillStyle(3001);
    hDeltaZReco->Draw();
    gPad->SetGrid();

    c_diffReco->cd(2);
    hDeltaXReco->SetLineColor(kOrange + 7);
    hDeltaXReco->SetFillColor(kOrange + 7);
    hDeltaXReco->SetFillStyle(3001);
    hDeltaXReco->Draw();
    gPad->SetGrid();

    c_diffReco->Update();
    c_diffReco->Draw();
    gSystem->ProcessEvents();


    // ----------- 4. Print statistics --------------
    std::cout << "\n=== RECONSTRUCTION ANALYSIS RESULTS ===" << std::endl;
    std::cout << "Plane 1 (Z reconstruction):" << std::endl;
    std::cout << "  zRecoSmeared - zOrig: RMS = " << hDeltaZ->GetRMS() << " mm" << std::endl;
    std::cout << "  Mean sigma_z: " << TMath::Mean(N, &vsigma_z[0]) << " mm" << std::endl;
    
    std::cout << "Plane 2 (X reconstruction):" << std::endl;
    std::cout << "  xRecoSmeared - xOrig: RMS = " << hDeltaX->GetRMS() << " mm" << std::endl;
    std::cout << "  Mean sigma_x: " << TMath::Mean(N, &vsigma_x[0]) << " mm" << std::endl;
    
    std::cout << "Total entries processed: " << N << std::endl;
    std::cout << "Plane 1 entries: " << hDeltaZ->GetEntries() << std::endl;
    std::cout << "Plane 2 entries: " << hDeltaX->GetEntries() << std::endl;
    std::cout << "======================================\n" << std::endl;
}

void analysis_bars_deprecated() {

    // Force interactive mode
    gROOT->SetBatch(kFALSE);
    gErrorIgnoreLevel = kFatal;

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

    std::cout << "=== Step 1: Original plots ===" << std::endl;
    initial_plot(tree, "Original Data");

    std::cout << "=== Step 2: Filtering hits ===" << std::endl;
    TTree* filteredTree = filter_first_hits(tree);
    std::cout << "Filtered tree has " << filteredTree->GetEntries() << " entries" << std::endl;
    f->Close(); // Close original file

    std::cout << "=== Step 3: Reopening filtered tree from file ===" << std::endl;
    TFile* filteredFile = TFile::Open("data/filtered_data.root");
    TTree* savedFilteredTree = (TTree*)filteredFile->Get("filtered_hits");
    
    if (!savedFilteredTree) {
        std::cerr << "Could not read filtered tree from file!" << std::endl;
        return;
    }
    
    std::cout << "Saved filtered tree has " << savedFilteredTree->GetEntries() << " entries" << std::endl;

    std::cout << "=== Step 4: Filtered plots ===" << std::endl;
    initial_plot(savedFilteredTree, "Filtered Data");

    std::cout << "=== Step 5: Reading geometry ===" << std::endl;
    auto geometry = read_geometry_vector("data/geometry.csv");
    if (geometry.empty()) {
        std::cerr << "Error: No geometry data loaded!" << std::endl;
        return;
    }

    double timeSmearSigma_ns = 0.1;
    std::cout << "=== Step 6: Processing data ===" << std::endl;
    TTree* reco_tree = process_data(savedFilteredTree, geometry, timeSmearSigma_ns);  // Use the file-backed tree
    std::cout << "Reco tree has " << reco_tree->GetEntries() << " entries" << std::endl;

    std::cout << "=== Step 7: Writing output ===" << std::endl;
    TFile* outFile = TFile::Open("data/reconstructed_data.root", "RECREATE");
    if (outFile && !outFile->IsZombie()) {
        reco_tree->Write();
        outFile->Close();
        std::cout << "Output saved to data/reconstructed_data.root" << std::endl;
    } else {
        std::cerr << "Failed to create output file!" << std::endl;
    }
    
    filteredFile->Close();
    
    
    std::cout << "=== Step 8: Reopening reconstructed data for plotting ===" << std::endl;
    TFile* recoFile = TFile::Open("data/reconstructed_data.root");
    if (!recoFile || recoFile->IsZombie()) {
        std::cerr << "Could not reopen reconstructed data file!" << std::endl;
        return;
    }
    
    TTree* savedRecoTree = (TTree*)recoFile->Get("recoTree");
    if (!savedRecoTree) {
        std::cerr << "Could not read reconstructed tree from file!" << std::endl;
        return;
    }
    
    std::cout << "Reopened reconstructed tree has " << savedRecoTree->GetEntries() << " entries" << std::endl;

    std::cout << "=== Step 9: Plotting reconstructed analysis ===" << std::endl;
    plotRecoAnalysis(savedRecoTree);  // ← Use the file-backed tree
    
    recoFile->Close();
    
    std::cout << "=== Analysis completed successfully! ===" << std::endl;

    // Keep ROOT alive to see the plots
    std::cout << "Plots created. Press Enter to exit..." << std::endl;
    std::cin.get();


}