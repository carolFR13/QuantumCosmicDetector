#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TRandom3.h>
#include <TMath.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TText.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>

struct BarData {
    std::vector<int> eventID;
    std::vector<int> barID;
    std::vector<std::string> pName;

    std::vector<double> x1, y1, z1;
    std::vector<double> lx1, ly1, lz1;
    std::vector<double> x2, y2, z2;
    std::vector<double> lx2, ly2, lz2;

    std::vector<double> tG_earliest, tG_latest;
    std::vector<double> tA_earliest, tB_earliest;
    std::vector<double> tA_latest, tB_latest;

    std::vector<double> edep;
    std::vector<int> planeID;
    std::vector<std::string> volumeName;
};
struct RecoData {
    int eventID;
    int planeID, barID;
    double xReco, yReco, zReco;
    double xRecoSmeared, yRecoSmeared, zRecoSmeared;
    double tAOrig, tBOrig;
    double tA_smeared, tB_smeared;
    double sigma_x, sigma_y, sigma_z;
    double xOrig, yOrig, zOrig;
};
struct FitResults {
    double timeSmearSigma;
    double sigma_tA_p1, sigma_tA_p1_err;
    double sigma_tB_p1, sigma_tB_p1_err;
    double sigma_tA_p2, sigma_tA_p2_err;
    double sigma_tB_p2, sigma_tB_p2_err;
    double sigma_zP1, sigma_zP1_err;
    double sigma_zP2, sigma_zP2_err;
    int plane1_count, plane2_count;
};
struct GeometryEntry {
    std::string volumeName;
    double x, y, z;
    GeometryEntry(const std::string& name, double x_, double y_, double z_) 
        : volumeName(name), x(x_), y(y_), z(z_) {}
};
struct EventHitPattern {
    int eventID;
    int plane1_hits;
    int plane2_hits;
    std::vector<RecoData> plane1_data;
    std::vector<RecoData> plane2_data;
    
    // Classification of hit pattern
    enum PatternType {
        SINGLE_SINGLE,    // 1 hit in plane 1, 1 hit in plane 2
        SINGLE_DOUBLE,    // 1 hit in one plane, 2 hits in the other
        DOUBLE_DOUBLE,    // 2 hits in plane 1, 2 hits in plane 2
        OTHER             // Any other pattern
    };
    
    PatternType pattern_type;
    
    EventHitPattern() : eventID(-1), plane1_hits(0), plane2_hits(0), pattern_type(OTHER) {}
};
struct HitPatternStatistics {
    int total_events;
    int single_single_events;    // 1-1 pattern
    int single_double_events;    // 1-2 or 2-1 pattern
    int double_double_events;    // 2-2 pattern
    int other_pattern_events;    // any other pattern
    
    // Detailed breakdown
    int events_1p1_1p2;  // 1 hit plane1, 1 hit plane2
    int events_1p1_2p2;  // 1 hit plane1, 2 hits plane2
    int events_2p1_1p2;  // 2 hits plane1, 1 hit plane2
    int events_2p1_2p2;  // 2 hits plane1, 2 hits plane2
    
    HitPatternStatistics() : total_events(0), single_single_events(0), 
                           single_double_events(0), double_double_events(0), 
                           other_pattern_events(0), events_1p1_1p2(0), 
                           events_1p1_2p2(0), events_2p1_1p2(0), events_2p1_2p2(0) {}
};
struct TrackPoint {
    int eventID;
    int planeID;
    double x, y, z;
    double sigma_x, sigma_y, sigma_z;
    int num_hits_used;
    std::vector<int> barIDs_used;
    bool valid;
    
    TrackPoint() : eventID(-1), planeID(-1), x(0), y(0), z(0), 
                   sigma_x(0), sigma_y(0), sigma_z(0), num_hits_used(0), valid(false) {}
};
struct EventTrack {
    int eventID;
    TrackPoint plane1_point;
    TrackPoint plane2_point;
    bool has_plane1;
    bool has_plane2;
    bool valid_track;
    
    EventTrack() : eventID(-1), has_plane1(false), has_plane2(false), valid_track(false) {}
};
struct PredictedImpact {
    int eventID;
    double x_pred, y_pred, z_pred;
    double sigma_x_pred, sigma_y_pred, sigma_z_pred;
    bool valid_prediction;
    
    // Store the input track points for reference
    TrackPoint plane1_point;
    TrackPoint plane2_point;
    
    PredictedImpact() : eventID(-1), x_pred(0), y_pred(500.0), z_pred(0), 
                       sigma_x_pred(0), sigma_y_pred(0), sigma_z_pred(0), 
                       valid_prediction(false) {}
};

struct ImpactFitResults {
    double timeSmearSigma;
    double mean_x_pred, sigma_x_pred, sigma_x_pred_err;
    double mean_z_pred, sigma_z_pred, sigma_z_pred_err;
    double chi2_x_ndf, chi2_z_ndf;
    int n_valid_predictions;
    
    // Statistical means with proper uncertainties
    double statistical_mean_x, statistical_error_x;
    double statistical_mean_z, statistical_error_z;
    
    ImpactFitResults() : timeSmearSigma(0), mean_x_pred(0), sigma_x_pred(0), sigma_x_pred_err(0),
                        mean_z_pred(0), sigma_z_pred(0), sigma_z_pred_err(0),
                        chi2_x_ndf(0), chi2_z_ndf(0), n_valid_predictions(0),
                        statistical_mean_x(0), statistical_error_x(0),
                        statistical_mean_z(0), statistical_error_z(0) {}
};

void SetHistogramStyle(TH1* hist, Color_t fillColor, Color_t lineColor = kBlack, 
                      const std::string& xTitle = "", const std::string& yTitle = "Events",
                      float alpha = 0.3) {
    // Fill and line colors
    hist->SetFillColor(fillColor);
    hist->SetFillColorAlpha(fillColor, alpha); 
    hist->SetLineColor(lineColor);
    hist->SetLineWidth(2);
    
    // Axis titles
    if (!xTitle.empty()) hist->GetXaxis()->SetTitle(xTitle.c_str());
    if (!yTitle.empty()) hist->GetYaxis()->SetTitle(yTitle.c_str());
    
    // Axis label and title sizes
    hist->GetXaxis()->SetTitleSize(0.045);
    hist->GetYaxis()->SetTitleSize(0.045);
    hist->GetXaxis()->SetLabelSize(0.04);
    hist->GetYaxis()->SetLabelSize(0.04);
    
    // Axis title offsets
    hist->GetXaxis()->SetTitleOffset(1.1);
    hist->GetYaxis()->SetTitleOffset(1.2);
    
    // Make axis lines darker
    hist->GetXaxis()->SetAxisColor(kBlack);
    hist->GetYaxis()->SetAxisColor(kBlack);
    hist->GetXaxis()->SetLabelColor(kBlack);
    hist->GetYaxis()->SetLabelColor(kBlack);
    hist->GetXaxis()->SetTitleColor(kBlack);
    hist->GetYaxis()->SetTitleColor(kBlack);
}

void SetCanvasStyle(TVirtualPad* pad) {
    pad->SetLeftMargin(0.12);
    pad->SetBottomMargin(0.12);
    pad->SetTopMargin(0.08);
    pad->SetRightMargin(0.05);
    pad->SetFrameLineColor(kBlack);
}

void SetCanvasStyle(TCanvas* canvas) {
    SetCanvasStyle((TVirtualPad*)canvas);
}

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

bool LoadBarHitsData(const std::string& filename, BarData& data) {
    TFile file(filename.c_str(), "READ");
    if (file.IsZombie()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    TTree* tree = nullptr;
    file.GetObject("barHits", tree);
    if (!tree) {
        std::cerr << "TTree 'barHits' not found in file!" << std::endl;
        return false;
    }

    // Temporary variables for reading
    int evtID, barID, planeID;
    double ePosX, ePosY, ePosZ;
    double eLocalX, eLocalY, eLocalZ;
    double lPosX, lPosY, lPosZ;
    double lLocalX, lLocalY, lLocalZ;
    double eGTime, lGTime;
    double eTimeA, eTimeB, lTimeA, lTimeB;
    double energyDep;
    char pName[16];
    char volumeName[20];

    tree->SetBranchAddress("event", &evtID);
    tree->SetBranchAddress("barID", &barID);
    tree->SetBranchAddress("pName", &pName);

    tree->SetBranchAddress("x1", &ePosX);
    tree->SetBranchAddress("y1", &ePosY);
    tree->SetBranchAddress("z1", &ePosZ);

    tree->SetBranchAddress("lx1", &eLocalX);
    tree->SetBranchAddress("ly1", &eLocalY);
    tree->SetBranchAddress("lz1", &eLocalZ);

    tree->SetBranchAddress("x2", &lPosX);
    tree->SetBranchAddress("y2", &lPosY);
    tree->SetBranchAddress("z2", &lPosZ);

    tree->SetBranchAddress("lx2", &lLocalX);
    tree->SetBranchAddress("ly2", &lLocalY);
    tree->SetBranchAddress("lz2", &lLocalZ);

    tree->SetBranchAddress("tG_earliest", &eGTime);
    tree->SetBranchAddress("tG_latest", &lGTime);

    tree->SetBranchAddress("tA_earliest", &eTimeA);
    tree->SetBranchAddress("tB_earliest", &eTimeB);

    tree->SetBranchAddress("tA_latest", &lTimeA);
    tree->SetBranchAddress("tB_latest", &lTimeB);

    tree->SetBranchAddress("Edep", &energyDep);
    tree->SetBranchAddress("planeID", &planeID);
    tree->SetBranchAddress("volumeName", &volumeName);

    // Loop through all entries
    Long64_t nEntries = tree->GetEntries();
    data.eventID.reserve(nEntries);
    data.barID.reserve(nEntries);
    data.pName.reserve(nEntries);
    data.x1.reserve(nEntries);
    data.y1.reserve(nEntries);
    data.z1.reserve(nEntries);
    data.lx1.reserve(nEntries);
    data.ly1.reserve(nEntries);
    data.lz1.reserve(nEntries);
    data.x2.reserve(nEntries);
    data.y2.reserve(nEntries);
    data.z2.reserve(nEntries);
    data.lx2.reserve(nEntries);
    data.ly2.reserve(nEntries);
    data.lz2.reserve(nEntries);
    data.tG_earliest.reserve(nEntries);
    data.tG_latest.reserve(nEntries);
    data.tA_earliest.reserve(nEntries);
    data.tB_earliest.reserve(nEntries);
    data.tA_latest.reserve(nEntries);
    data.tB_latest.reserve(nEntries);
    data.edep.reserve(nEntries);
    data.planeID.reserve(nEntries);
    data.volumeName.reserve(nEntries);

    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        data.eventID.push_back(evtID);
        data.barID.push_back(barID);
        data.pName.push_back(pName);

        data.x1.push_back(ePosX);
        data.y1.push_back(ePosY);
        data.z1.push_back(ePosZ);

        data.lx1.push_back(eLocalX);
        data.ly1.push_back(eLocalY);
        data.lz1.push_back(eLocalZ);

        data.x2.push_back(lPosX);
        data.y2.push_back(lPosY);
        data.z2.push_back(lPosZ);

        data.lx2.push_back(lLocalX);
        data.ly2.push_back(lLocalY);
        data.lz2.push_back(lLocalZ);

        data.tG_earliest.push_back(eGTime);
        data.tG_latest.push_back(lGTime);

        data.tA_earliest.push_back(eTimeA);
        data.tB_earliest.push_back(eTimeB);

        data.tA_latest.push_back(lTimeA);
        data.tB_latest.push_back(lTimeB);

        data.edep.push_back(energyDep);
        data.planeID.push_back(planeID);
        data.volumeName.push_back(volumeName);
    }

    return true;
}

void PlotParticleHistogram(const BarData& data) {
    // First count all particles
    std::map<std::string, int> counts;
    for (const auto& name : data.pName) {
        counts[name]++;
    }
    
    // Filter out particles with less than 10 entries
    std::map<std::string, int> filtered_counts;
    for (const auto& kv : counts) {
        if (kv.second >= 380) {
            filtered_counts[kv.first] = kv.second;
        }
    }
    
    if (filtered_counts.empty()) {
        std::cout << "No particle types with >= 10 entries found!" << std::endl;
        return;
    }
    
    // Create histogram with filtered entries
    TH1F *hParticles = new TH1F("hParticles", "Particle Distribution;Particle Type;Count", 
                                filtered_counts.size(), 0, filtered_counts.size());
    
    // Fill histogram with filtered data
    int bin = 1;
    for (const auto& kv : filtered_counts) {
        hParticles->SetBinContent(bin, kv.second);
        hParticles->GetXaxis()->SetBinLabel(bin, kv.first.c_str());
        bin++;
    }
    
    // Apply consistent styling
    SetHistogramStyle(hParticles, kOrange-2, kOrange+2, "Particle Type", "Number of Entries");
    
    // Create canvas
    TCanvas *c1 = new TCanvas("cParticles", "Particle Distribution", 1000, 600);
    SetCanvasStyle(c1);
    c1->SetBottomMargin(0.15); // More space for horizontal labels
    
    hParticles->Draw("hist");
    
    // Set horizontal labels (this is the key change)
    hParticles->LabelsOption("h", "X"); // "h" for horizontal instead of "v" for vertical
    
    // Adjust label size for better readability
    hParticles->GetXaxis()->SetLabelSize(0.04);
    hParticles->GetXaxis()->SetTitleSize(0.045);
    
    // Update canvas
    c1->Update();
    gSystem->ProcessEvents();
    
}

void PlotMeanGlobalPositions(const BarData& data, bool muonsOnly=false) {
    TString suffix = muonsOnly ? "_muons" : "_all";
    TString titleSuffix = muonsOnly ? " (only muons)" : " (all particles)";

    TH1F *hx = new TH1F("hx"+suffix,"Global coordinates"+titleSuffix+";xm (mm);Entries",100,-800,800);
    TH1F *hy = new TH1F("hy"+suffix,"Global coordinates"+titleSuffix+";ym (mm);Entries",50,-600,100);
    TH1F *hz = new TH1F("hz"+suffix,"Global coordinates"+titleSuffix+";zm (mm);Entries",100,-800,800);

    TH1F *hlx = new TH1F("hlx"+suffix,"Local coordinates"+titleSuffix+";lxm (mm);Entries",100,-800,800);
    TH1F *hly = new TH1F("hly"+suffix,"Local coordinates"+titleSuffix+";lym (mm);Entries",50,-20,20);
    TH1F *hlz = new TH1F("hlz"+suffix,"Local coordinates"+titleSuffix+";lzm (mm);Entries",100,-600,600);

    // Apply consistent styling
    SetHistogramStyle(hx, kGreen+1, kGreen-12, "x (mm)", "Number of Entries");
    SetHistogramStyle(hy, kRed+1, kRed-12, "y (mm)", "Number of Entries");
    SetHistogramStyle(hz, kBlue, kBlue+2, "z (mm)", "Number of Entries");

    SetHistogramStyle(hlx, kOrange+1, kOrange-8, "x_{local} (mm)", "Number of Entries");
    SetHistogramStyle(hly, kMagenta+1, kMagenta-8, "y_{local} (mm)", "Number of Entries");
    SetHistogramStyle(hlz, kCyan+1, kCyan-8, "z_{local} (mm)", "Number of Entries");


    size_t n = data.x1.size();
    int diffCount = 0;

    for (size_t i = 0; i < n; i++) {
        if (muonsOnly && !(data.pName[i] == "mu-" || data.pName[i] == "mu+"))
            continue;

        double xm = 0.5 * (data.x1[i] + data.x2[i]);
        double ym = 0.5 * (data.y1[i] + data.y2[i]);
        double zm = 0.5 * (data.z1[i] + data.z2[i]);

        double lxm = 0.5 * (data.lx1[i] + data.lx2[i]);
        double lym = 0.5 * (data.ly1[i] + data.ly2[i]);
        double lzm = 0.5 * (data.lz1[i] + data.lz2[i]);

        hx->Fill(xm);
        hy->Fill(ym);
        hz->Fill(zm);

        hlx->Fill(lxm);
        hly->Fill(lym);
        hlz->Fill(lzm);

        if (data.x1[i] != data.x2[i] ||
            data.y1[i] != data.y2[i] ||
            data.z1[i] != data.z2[i])
            diffCount++;
    }

    std::cout << "Entries with differences between earliest and latest coordinates" << titleSuffix << ": "
              << diffCount << " out of " << n << " (" << 100.0 * diffCount / n << "%)" << std::endl;

    TCanvas *c2 = new TCanvas("c2"+suffix, "Mean Global Positions"+titleSuffix, 1200, 400);
    c2->Divide(3, 1);
    c2->cd(1); SetCanvasStyle(gPad); hx->Draw(); 
    c2->cd(2); SetCanvasStyle(gPad); hy->Draw(); 
    c2->cd(3); SetCanvasStyle(gPad); hz->Draw(); 
    c2->Update();
    gSystem->ProcessEvents();

    TCanvas *c3 = new TCanvas("c3"+suffix, "Mean Local Positions"+titleSuffix, 1200, 400);
    c3->Divide(3, 1);
    c3->cd(1); SetCanvasStyle(gPad); hlx->Draw(); 
    c3->cd(2); SetCanvasStyle(gPad); hly->Draw(); 
    c3->cd(3); SetCanvasStyle(gPad); hlz->Draw(); 
    c3->Update();
    gSystem->ProcessEvents();
}

void PlotColormaps(const BarData &data, bool muonsOnly=false) {

    TString suffix = muonsOnly ? "_muons" : "_all";
    TString titleSuffix = muonsOnly ? " (only muons)" : " (all particles)";

    TH2F *h_all = new TH2F("h_all"+suffix,"All planes"+titleSuffix+";x_{m};z_{m}",100,-750,750,100,-750,750);
    TH2F *h_p1  = new TH2F("h_p1"+suffix,"Plane 1"+titleSuffix+";x_{m};z_{m}",100,-500,500,100,-500,500);
    TH2F *h_p2  = new TH2F("h_p2"+suffix,"Plane 2"+titleSuffix+";x_{m};z_{m}",100,-750,750,100,-750,750);

    TH2F *hLocal_all = new TH2F("hLocal_all"+suffix,"All planes"+titleSuffix+";x_{m,local};z_{m,local}",100,-750,750,100,-750,750);
    TH2F *hLocal_p1  = new TH2F("hLocal_p1"+suffix,"Plane 1"+titleSuffix+";x_{m,local};z_{m,local}",100,-8,8,100,-500,500);
    TH2F *hLocal_p2  = new TH2F("hLocal_p2"+suffix,"Plane 2"+titleSuffix+";x_{m,local};z_{m,local}",100,-750,750,100,-8,8);

    // Set consistent axis styling for 2D plots
    auto set2DStyle = [](TH2F* hist) {
        hist->GetXaxis()->SetTitleSize(0.045);
        hist->GetYaxis()->SetTitleSize(0.045);
        hist->GetXaxis()->SetLabelSize(0.04);
        hist->GetYaxis()->SetLabelSize(0.04);
        hist->GetXaxis()->SetTitleOffset(1.1);
        hist->GetYaxis()->SetTitleOffset(1.2);
    };

    set2DStyle(h_all); set2DStyle(h_p1); set2DStyle(h_p2);
    set2DStyle(hLocal_all); set2DStyle(hLocal_p1); set2DStyle(hLocal_p2);

    size_t n = data.x1.size();
    int processedCount = 0;

    for(size_t i=0; i<n; i++) {
        // Filter if only muons
        if (muonsOnly && !(data.pName[i] == "mu-" || data.pName[i] == "mu+"))
            continue;

        processedCount++;

        Double_t xm = 0.5 * (data.x1[i] + data.x2[i]);
        Double_t ym = 0.5 * (data.y1[i] + data.y2[i]);
        Double_t zm = 0.5 * (data.z1[i] + data.z2[i]);

        Double_t lxm = 0.5 * (data.lx1[i] + data.lx2[i]);
        Double_t lym = 0.5 * (data.ly1[i] + data.ly2[i]);
        Double_t lzm = 0.5 * (data.lz1[i] + data.lz2[i]);

        h_all->Fill(xm, zm);
        hLocal_all->Fill(lxm, lzm);

        if(data.planeID[i] == 1) {
            h_p1->Fill(xm, zm);
            hLocal_p1->Fill(lxm, lzm);
        }
        else if(data.planeID[i] == 2) {
            h_p2->Fill(xm, zm);
            hLocal_p2->Fill(lxm, lzm);
        }
    }

    // Global coordinates plot
    TCanvas *c4 = new TCanvas("c4"+suffix, "Colormaps - Global Coordinates"+titleSuffix,1500,500);
    c4->Divide(3,1);
    c4->cd(1); SetCanvasStyle(gPad); h_all->Draw("COLZ");
    c4->cd(2); SetCanvasStyle(gPad); h_p1->Draw("COLZ");
    c4->cd(3); SetCanvasStyle(gPad); h_p2->Draw("COLZ");
    c4->Update();
    gSystem->ProcessEvents();

    // Local coordinates plot
    TCanvas *c5 = new TCanvas("c5"+suffix, "Colormaps - Local Coordinates"+titleSuffix,1500,500);
    c5->Divide(3,1);
    c5->cd(1); SetCanvasStyle(gPad); hLocal_all->Draw("COLZ");
    c5->cd(2); SetCanvasStyle(gPad); hLocal_p1->Draw("COLZ");
    c5->cd(3); SetCanvasStyle(gPad); hLocal_p2->Draw("COLZ");
    c5->Update();
    gSystem->ProcessEvents();
}

void PlotTimeDistributions(const BarData &data, bool muonsOnly=false) {
    TString suffix = muonsOnly ? "_muons" : "_all";
    TString titleSuffix = muonsOnly ? " (only muons)" : " (all particles)";

    // Histograms for earliest
    // Create histograms with proper titles and axis labels
    TH1F *h_tA1 = new TH1F("h_tA1"+suffix, "Earliest Time A"+titleSuffix, 100, -0.5, 8);
    TH1F *h_tB1 = new TH1F("h_tB1"+suffix, "Earliest Time B"+titleSuffix, 100, -0.5, 8);
    TH1F *h_tA1mB1 = new TH1F("h_tA1mB1"+suffix, "Time Difference (A-B) Earliest"+titleSuffix, 100, -8, 8);
    TH1F *h_tA1pB1 = new TH1F("h_tA1pB1"+suffix, "Time Sum (A+B) Earliest"+titleSuffix, 100, 4, 8);

    TH1F *h_tA2 = new TH1F("h_tA2"+suffix, "Latest Time A"+titleSuffix, 100, -1, 8);
    TH1F *h_tB2 = new TH1F("h_tB2"+suffix, "Latest Time B"+titleSuffix, 100, -1, 8);
    TH1F *h_tA2mB2 = new TH1F("h_tA2mB2"+suffix, "Time Difference (A-B) Latest"+titleSuffix, 100, -8, 8);
    TH1F *h_tA2pB2 = new TH1F("h_tA2pB2"+suffix, "Time Sum (A+B) Latest"+titleSuffix, 100, 4, 8);

    TH1F *h_dA = new TH1F("h_dA"+suffix, "Time A Difference (Latest-Earliest)"+titleSuffix, 100, -0.1, 0.1);
    TH1F *h_dB = new TH1F("h_dB"+suffix, "Time B Difference (Latest-Earliest)"+titleSuffix, 100, -0.1, 0.1);
    TH1F *h_dAbsDiff = new TH1F("h_dAbsDiff"+suffix, "Abs Difference Change"+titleSuffix, 100, -0.1, 0.2);
    TH1F *h_dAbsSum = new TH1F("h_dAbsSum"+suffix, "Abs Sum Change"+titleSuffix, 100, -0.05, 0.05);

    // Apply consistent styling with alpha transparency
    SetHistogramStyle(h_tA1, kBlue, kBlue+2, "t_{A} (ns)", "Number of Entries");
    SetHistogramStyle(h_tB1, kRed, kRed+2, "t_{B} (ns)", "Number of Entries");
    SetHistogramStyle(h_tA1mB1, kGreen, kGreen+2, "t_{A} - t_{B} (ns)", "Number of Entries");
    SetHistogramStyle(h_tA1pB1, kMagenta, kMagenta+2, "t_{A} + t_{B} (ns)", "Number of Entries");

    SetHistogramStyle(h_tA2, kBlue, kBlue+2, "t_{A} (ns)", "Number of Entries");
    SetHistogramStyle(h_tB2, kRed, kRed+2, "Time B (ns)", "Number of Entries");
    SetHistogramStyle(h_tA2mB2, kGreen, kGreen+2, "t_{A} - t_{B} (ns)", "Number of Entries");
    SetHistogramStyle(h_tA2pB2, kMagenta, kMagenta+2, "t_{A} + t_{B} (ns)", "Number of Entries");

    SetHistogramStyle(h_dA, kOrange, kOrange+2, "#Delta t_{A} (ns)", "Number of Entries");
    SetHistogramStyle(h_dB, kCyan, kCyan+2, "#Delta t_{B} (ns)", "Number of Entries");
    SetHistogramStyle(h_dAbsDiff, kViolet, kViolet+2, "#Delta |t_{A}-t_{B}| (ns)", "Number of Entries");
    SetHistogramStyle(h_dAbsSum, kSpring, kSpring+2, "#Delta |t_{A}+t_{B}| (ns)", "Number of Entries");

    // Loop over events
    for (size_t i = 0; i < data.tA_earliest.size(); ++i) {
        if (muonsOnly && !(data.pName[i] == "mu-" || data.pName[i] == "mu+")) continue;

        double tA1 = data.tA_earliest[i];
        double tB1 = data.tB_earliest[i];
        double tA2 = data.tA_latest[i];
        double tB2 = data.tB_latest[i];

        // Fill earliest
        h_tA1->Fill(tA1);
        h_tB1->Fill(tB1);
        h_tA1mB1->Fill(tA1 - tB1);
        h_tA1pB1->Fill(tA1 + tB1);

        // Fill latest
        h_tA2->Fill(tA2);
        h_tB2->Fill(tB2);
        h_tA2mB2->Fill(tA2 - tB2);
        h_tA2pB2->Fill(tA2 + tB2);

        // Fill comparisons
        h_dA->Fill(tA2 - tA1);
        h_dB->Fill(tB2 - tB1);
        h_dAbsDiff->Fill(fabs(tA2 - tB2) - fabs(tA1 - tB1));
        h_dAbsSum->Fill(fabs(tA2 + tB2) - fabs(tA1 + tB1));
    }

    // Draw earliest
    TCanvas *cEarliest = new TCanvas("cEarliest"+suffix, "Earliest Times"+titleSuffix, 1200, 500);
    cEarliest->Divide(4,1);
    cEarliest->cd(1); h_tA1->Draw();
    cEarliest->cd(2); h_tB1->Draw();
    cEarliest->cd(3); h_tA1mB1->Draw();
    cEarliest->cd(4); h_tA1pB1->Draw();

    // Draw latest
    TCanvas *cLatest = new TCanvas("cLatest"+suffix, "Latest Times"+titleSuffix, 1200, 500);
    cLatest->Divide(4,1);
    cLatest->cd(1); h_tA2->Draw();
    cLatest->cd(2); h_tB2->Draw();
    cLatest->cd(3); h_tA2mB2->Draw();
    cLatest->cd(4); h_tA2pB2->Draw();

    // Draw comparisons
    TCanvas *cCompare = new TCanvas("cCompare"+suffix, "Time Comparisons"+titleSuffix, 1200, 500);
    cCompare->Divide(4,1);
    cCompare->cd(1); h_dA->Draw();
    cCompare->cd(2); h_dB->Draw();
    cCompare->cd(3); h_dAbsDiff->Draw();
    cCompare->cd(4); h_dAbsSum->Draw();
}

void ProcessData(const BarData &data, std::vector<RecoData> &reco_results, const std::vector<GeometryEntry>& geometry,
                    double timeSmearSigma_ns = 0.5, bool muonsOnly = false, bool earliestImpact = true){

    double vg = 299.792458 / 1.58;
    // random generator
    TRandom3 rng(42); // fixed seed for reproducibility

    size_t n = data.x1.size();
    reco_results.clear();
    reco_results.reserve(n);

    for (size_t i = 0; i < n; i++) {
        if (muonsOnly && !(data.pName[i] == "mu-" || data.pName[i] == "mu+"))
            continue;

        if (i == 0) {  // Just print the first entry for debug
            std::cout << "First entry volumeName: '" << data.volumeName[i] << "'" << std::endl;  // ← Fix: access element [i]
        }
        
        std::string volNameStr(data.volumeName[i]);
        
        if (volNameStr.empty()) continue;

        double tA, tB;
        double x_hit, y_hit, z_hit;
        if (earliestImpact == true) {
            tA = data.tA_earliest[i];
            tB = data.tB_earliest[i];
            x_hit = data.x1[i];
            y_hit = data.y1[i];
            z_hit = data.z1[i];
        } else {
            tA = data.tA_latest[i];
            tB = data.tB_latest[i];
            x_hit = data.x2[i];
            y_hit = data.y2[i];
            z_hit = data.z2[i];
        }

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
        double tA_smeared = tA + rng.Gaus(0, timeSmearSigma_ns);
        double tB_smeared = tB + rng.Gaus(0, timeSmearSigma_ns);

        double deltaT_smeared = tB_smeared - tA_smeared;
        double reco_coord_smeared = deltaT_smeared * vg / 2.0;

        const double sigma_geom = 15.0 / std::sqrt(12);  // ≈ 4.33 mm
        const double sigma_time = timeSmearSigma_ns;             // ns
        const double sigma_reco = sigma_time * vg / std::sqrt(2);  

        // Create new RecoData entry
        RecoData reco_entry;  // ← Fix: create new object instead of modifying const reference

        // Set coordinates
        if (data.planeID[i] == 1) {
            reco_entry.xReco = geom_x;
            reco_entry.yReco = geom_y;
            reco_entry.zReco = reco_coord;

            reco_entry.sigma_x = sigma_geom;
            reco_entry.sigma_y = sigma_geom;
            reco_entry.sigma_z = sigma_reco;

            reco_entry.xRecoSmeared = geom_x;
            reco_entry.yRecoSmeared = geom_y;
            reco_entry.zRecoSmeared = reco_coord_smeared;

        } else if (data.planeID[i] == 2) {
            reco_entry.xReco = reco_coord;
            reco_entry.yReco = geom_y;
            reco_entry.zReco = geom_z;

            reco_entry.sigma_x = sigma_reco;
            reco_entry.sigma_y = sigma_geom;
            reco_entry.sigma_z = sigma_geom;

            reco_entry.xRecoSmeared = reco_coord_smeared;
            reco_entry.yRecoSmeared = geom_y;
            reco_entry.zRecoSmeared = geom_z;
        } else {
            continue;
        }

        reco_entry.tAOrig = tA;
        reco_entry.tBOrig = tB;
        reco_entry.tA_smeared = tA_smeared;
        reco_entry.tB_smeared = tB_smeared;

        reco_entry.xOrig = x_hit;
        reco_entry.yOrig = y_hit;
        reco_entry.zOrig = z_hit;

        reco_entry.planeID = data.planeID[i];
        reco_entry.barID = data.barID[i];
        reco_entry.eventID = data.eventID[i];

        reco_results.push_back(reco_entry);  // ← Fix: add to vector
    }

    std::cout << "Processed " << reco_results.size() << " reconstructed hits" << std::endl;
}

void PlotRecoResults(const std::vector<RecoData>& reco_results, double timeSmearSigma_ns) {
    if (reco_results.empty()) {
        std::cerr << "Error: No reconstruction results to plot!" << std::endl;
        return;
    }

    std::cout << "Plotting reconstruction results for " << reco_results.size() << " entries" << std::endl;

    // === 1a. Time distributions - Plane 1 ===
    TH1F *h_tAOrig_p1 = new TH1F("h_tAOrig_p1", "Plane 1 - Original tA;tA (ns);Entries", 100, -1, 6);
    TH1F *h_tASmeared_p1 = new TH1F("h_tASmeared_p1", "Plane 1 - Smeared tA;tA (ns);Entries", 100, -1, 6);
    TH1F *h_tBOrig_p1 = new TH1F("h_tBOrig_p1", "Plane 1 - Original tB;tB (ns);Entries", 100, -1, 6);
    TH1F *h_tBSmeared_p1 = new TH1F("h_tBSmeared_p1", "Plane 1 - Smeared tB;tB (ns);Entries", 100, -1, 6);

    // === 1b. Time distributions - Plane 2 ===
    TH1F *h_tAOrig_p2 = new TH1F("h_tAOrig_p2", "Plane 2 - Original tA;tA (ns);Entries", 100, -1, 8);
    TH1F *h_tASmeared_p2 = new TH1F("h_tASmeared_p2", "Plane 2 - Smeared tA;tA (ns);Entries", 100, -1, 8);
    TH1F *h_tBOrig_p2 = new TH1F("h_tBOrig_p2", "Plane 2 - Original tB;tB (ns);Entries", 100, -1, 8);
    TH1F *h_tBSmeared_p2 = new TH1F("h_tBSmeared_p2", "Plane 2 - Smeared tB;tB (ns);Entries", 100, -1, 8);

    // Set colors for Plane 1 time histograms
    h_tAOrig_p1->SetLineColor(kBlue); h_tAOrig_p1->SetFillColor(kBlue);
    h_tAOrig_p1->SetFillColorAlpha(kBlue, 0.3);
    h_tASmeared_p1->SetLineColor(kRed); h_tASmeared_p1->SetFillColor(kRed);
    h_tASmeared_p1->SetFillColorAlpha(kRed, 0.3);
    h_tBOrig_p1->SetLineColor(kBlue); h_tBOrig_p1->SetFillColor(kBlue);
    h_tBOrig_p1->SetFillColorAlpha(kBlue, 0.3);
    h_tBSmeared_p1->SetLineColor(kRed); h_tBSmeared_p1->SetFillColor(kRed);
    h_tBSmeared_p1->SetFillColorAlpha(kRed, 0.3);

    // Set colors for Plane 2 time histograms
    h_tAOrig_p2->SetLineColor(kBlue); h_tAOrig_p2->SetFillColor(kBlue);
    h_tAOrig_p2->SetFillColorAlpha(kBlue, 0.3);
    h_tASmeared_p2->SetLineColor(kRed); h_tASmeared_p2->SetFillColor(kRed);
    h_tASmeared_p2->SetFillColorAlpha(kRed, 0.3);
    h_tBOrig_p2->SetLineColor(kBlue); h_tBOrig_p2->SetFillColor(kBlue);
    h_tBOrig_p2->SetFillColorAlpha(kBlue, 0.3);
    h_tBSmeared_p2->SetLineColor(kRed); h_tBSmeared_p2->SetFillColor(kRed);
    h_tBSmeared_p2->SetFillColorAlpha(kRed, 0.3);

    // === 2. Plane 1 reconstruction residuals ===
    TH1F *h_xReco_p1 = new TH1F("h_xReco_p1", "Plane 1: xReco - xOrig;#Delta x (mm);Entries", 100, -10, 10);
    TH1F *h_yReco_p1 = new TH1F("h_yReco_p1", "Plane 1: yReco - yOrig;#Delta y (mm);Entries", 100, -10, 10);
    TH1F *h_zReco_p1 = new TH1F("h_zReco_p1", "Plane 1: zReco - zOrig;#Delta z (mm);Entries", 100, -0.5, 0.5);

    // === 3. Plane 2 reconstruction residuals ===
    TH1F *h_xReco_p2 = new TH1F("h_xReco_p2", "Plane 2: xReco - xOrig;#Delta x (mm);Entries", 100, -0.5, 0.5);
    TH1F *h_yReco_p2 = new TH1F("h_yReco_p2", "Plane 2: yReco - yOrig;#Delta y (mm);Entries", 100, -10, 10);
    TH1F *h_zReco_p2 = new TH1F("h_zReco_p2", "Plane 2: zReco - zOrig;#Delta z (mm);Entries", 100, -10, 10);

    // === 4. Plane 1 smeared reconstruction residuals ===
    TH1F *h_xRecoSmear_p1 = new TH1F("h_xRecoSmear_p1", "Plane 1: xRecoSmeared - xOrig;#Delta x (mm);Entries", 100, -10, 10);
    TH1F *h_yRecoSmear_p1 = new TH1F("h_yRecoSmear_p1", "Plane 1: yRecoSmeared - yOrig;#Delta y (mm);Entries", 100, -10, 10);
    TH1F *h_zRecoSmear_p1 = new TH1F("h_zRecoSmear_p1", "Plane 1: zRecoSmeared - zOrig;#Delta z (mm);Entries", 100, -60, 60);

    // === 5. Plane 2 smeared reconstruction residuals ===
    TH1F *h_xRecoSmear_p2 = new TH1F("h_xRecoSmear_p2", "Plane 2: xRecoSmeared - xOrig;#Delta x (mm);Entries", 100, -60, 60);
    TH1F *h_yRecoSmear_p2 = new TH1F("h_yRecoSmear_p2", "Plane 2: yRecoSmeared - yOrig;#Delta y (mm);Entries", 100, -10, 10);
    TH1F *h_zRecoSmear_p2 = new TH1F("h_zRecoSmear_p2", "Plane 2: zRecoSmeared - zOrig;#Delta z (mm);Entries", 100, -10, 10);

    // Set colors for reconstruction histograms
    h_xReco_p1->SetLineColor(kGreen+2); h_xReco_p1->SetFillColor(kGreen);
    h_xReco_p1->SetFillColorAlpha(kGreen, 0.3);
    h_yReco_p1->SetLineColor(kGreen+2); h_yReco_p1->SetFillColor(kGreen);
    h_yReco_p1->SetFillColorAlpha(kGreen, 0.3);
    h_zReco_p1->SetLineColor(kGreen+2); h_zReco_p1->SetFillColor(kGreen);
    h_zReco_p1->SetFillColorAlpha(kGreen, 0.3);

    h_xReco_p2->SetLineColor(kMagenta+2); h_xReco_p2->SetFillColor(kMagenta);
    h_xReco_p2->SetFillColorAlpha(kMagenta, 0.3);
    h_yReco_p2->SetLineColor(kMagenta+2); h_yReco_p2->SetFillColor(kMagenta);
    h_yReco_p2->SetFillColorAlpha(kMagenta, 0.3);
    h_zReco_p2->SetLineColor(kMagenta+2); h_zReco_p2->SetFillColor(kMagenta);
    h_zReco_p2->SetFillColorAlpha(kMagenta, 0.3);

    h_xRecoSmear_p1->SetLineColor(kOrange+2); h_xRecoSmear_p1->SetFillColor(kOrange);
    h_xRecoSmear_p1->SetFillColorAlpha(kOrange, 0.3);
    h_yRecoSmear_p1->SetLineColor(kOrange+2); h_yRecoSmear_p1->SetFillColor(kOrange);
    h_yRecoSmear_p1->SetFillColorAlpha(kOrange, 0.3);
    h_zRecoSmear_p1->SetLineColor(kOrange+2); h_zRecoSmear_p1->SetFillColor(kOrange);
    h_zRecoSmear_p1->SetFillColorAlpha(kOrange, 0.3);

    h_xRecoSmear_p2->SetLineColor(kCyan+2); h_xRecoSmear_p2->SetFillColor(kCyan);
    h_xRecoSmear_p2->SetFillColorAlpha(kCyan, 0.3);
    h_yRecoSmear_p2->SetLineColor(kCyan+2); h_yRecoSmear_p2->SetFillColor(kCyan);
    h_yRecoSmear_p2->SetFillColorAlpha(kCyan, 0.3);
    h_zRecoSmear_p2->SetLineColor(kCyan+2); h_zRecoSmear_p2->SetFillColor(kCyan);
    h_zRecoSmear_p2->SetFillColorAlpha(kCyan, 0.3);

    // Counters for statistics
    int plane1_count = 0, plane2_count = 0;

    // Fill histograms
    for (const auto& entry : reco_results) {
        // Calculate residuals
        double dx_reco = entry.xReco - entry.xOrig;
        double dy_reco = entry.yReco - entry.yOrig;
        double dz_reco = entry.zReco - entry.zOrig;

        double dx_smear = entry.xRecoSmeared - entry.xOrig;
        double dy_smear = entry.yRecoSmeared - entry.yOrig;
        double dz_smear = entry.zRecoSmeared - entry.zOrig;

        // Fill based on plane
        if (entry.planeID == 1) {
            plane1_count++;
            
            // Fill time histograms for Plane 1
            h_tAOrig_p1->Fill(entry.tAOrig);
            h_tASmeared_p1->Fill(entry.tA_smeared);
            h_tBOrig_p1->Fill(entry.tBOrig);
            h_tBSmeared_p1->Fill(entry.tB_smeared);
            
            // Fill reconstruction residuals for Plane 1
            h_xReco_p1->Fill(dx_reco);
            h_yReco_p1->Fill(dy_reco);
            h_zReco_p1->Fill(dz_reco);

            h_xRecoSmear_p1->Fill(dx_smear);
            h_yRecoSmear_p1->Fill(dy_smear);
            h_zRecoSmear_p1->Fill(dz_smear);
            
        } else if (entry.planeID == 2) {
            plane2_count++;
            
            // Fill time histograms for Plane 2
            h_tAOrig_p2->Fill(entry.tAOrig);
            h_tASmeared_p2->Fill(entry.tA_smeared);
            h_tBOrig_p2->Fill(entry.tBOrig);
            h_tBSmeared_p2->Fill(entry.tB_smeared);
            
            // Fill reconstruction residuals for Plane 2
            h_xReco_p2->Fill(dx_reco);
            h_yReco_p2->Fill(dy_reco);
            h_zReco_p2->Fill(dz_reco);

            h_xRecoSmear_p2->Fill(dx_smear);
            h_yRecoSmear_p2->Fill(dy_smear);
            h_zRecoSmear_p2->Fill(dz_smear);
        }
    }

    std::cout << "Plane 1 entries: " << plane1_count << ", Plane 2 entries: " << plane2_count << std::endl;

    // === Canvas 1a: Time distributions - Plane 1 ===
    TCanvas *cTimes_p1 = new TCanvas("cTimes_p1", Form("Plane 1 - Time Distributions (σ = %.1f ns)", timeSmearSigma_ns), 1200, 800);
    cTimes_p1->Divide(2, 2);
    cTimes_p1->cd(1); h_tAOrig_p1->Draw("hist"); gPad->Update();
    cTimes_p1->cd(2); h_tASmeared_p1->Draw("hist"); gPad->Update();
    cTimes_p1->cd(3); h_tBOrig_p1->Draw("hist"); gPad->Update();
    cTimes_p1->cd(4); h_tBSmeared_p1->Draw("hist"); gPad->Update();
    cTimes_p1->Update();
    gSystem->ProcessEvents();

    // === Canvas 1b: Time distributions - Plane 2 ===
    TCanvas *cTimes_p2 = new TCanvas("cTimes_p2", Form("Plane 2 - Time Distributions (σ = %.1f ns)", timeSmearSigma_ns), 1200, 800);
    cTimes_p2->Divide(2, 2);
    cTimes_p2->cd(1); h_tAOrig_p2->Draw("hist"); gPad->Update();
    cTimes_p2->cd(2); h_tASmeared_p2->Draw("hist"); gPad->Update();
    cTimes_p2->cd(3); h_tBOrig_p2->Draw("hist"); gPad->Update();
    cTimes_p2->cd(4); h_tBSmeared_p2->Draw("hist"); gPad->Update();
    cTimes_p2->Update();
    gSystem->ProcessEvents();

    // === Canvas 2: Plane 1 reconstruction residuals ===
    TCanvas *cReco_p1 = new TCanvas("cReco_p1", "Plane 1: Reconstruction Residuals", 1200, 400);
    cReco_p1->Divide(3, 1);
    cReco_p1->cd(1); h_xReco_p1->Draw("hist"); gPad->Update();
    cReco_p1->cd(2); h_yReco_p1->Draw("hist"); gPad->Update();
    cReco_p1->cd(3); h_zReco_p1->Draw("hist"); gPad->Update();
    cReco_p1->Update();
    gSystem->ProcessEvents();

    // === Canvas 3: Plane 2 reconstruction residuals ===
    TCanvas *cReco_p2 = new TCanvas("cReco_p2", "Plane 2: Reconstruction Residuals", 1200, 400);
    cReco_p2->Divide(3, 1);
    cReco_p2->cd(1); h_xReco_p2->Draw("hist"); gPad->Update();
    cReco_p2->cd(2); h_yReco_p2->Draw("hist"); gPad->Update();
    cReco_p2->cd(3); h_zReco_p2->Draw("hist"); gPad->Update();
    cReco_p2->Update();
    gSystem->ProcessEvents();

    // === Canvas 4: Plane 1 smeared reconstruction residuals ===
    TCanvas *cSmear_p1 = new TCanvas("cSmear_p1", "Plane 1: Smeared Reconstruction Residuals", 1200, 400);
    cSmear_p1->Divide(3, 1);
    cSmear_p1->cd(1); h_xRecoSmear_p1->Draw("hist"); gPad->Update();
    cSmear_p1->cd(2); h_yRecoSmear_p1->Draw("hist"); gPad->Update();
    cSmear_p1->cd(3); h_zRecoSmear_p1->Draw("hist"); gPad->Update();
    cSmear_p1->Update();
    gSystem->ProcessEvents();

    // === Canvas 5: Plane 2 smeared reconstruction residuals ===
    TCanvas *cSmear_p2 = new TCanvas("cSmear_p2", "Plane 2: Smeared Reconstruction Residuals", 1200, 400);
    cSmear_p2->Divide(3, 1);
    cSmear_p2->cd(1); h_xRecoSmear_p2->Draw("hist"); gPad->Update();
    cSmear_p2->cd(2); h_yRecoSmear_p2->Draw("hist"); gPad->Update();
    cSmear_p2->cd(3); h_zRecoSmear_p2->Draw("hist"); gPad->Update();
    cSmear_p2->Update();
    gSystem->ProcessEvents();

    // Print statistics
    std::cout << "=== Reconstruction Statistics ===" << std::endl;
    std::cout << "Time smearing sigma: " << timeSmearSigma_ns << " ns" << std::endl;
    
    std::cout << "Plane 1 (" << plane1_count << " entries):" << std::endl;
    std::cout << "  Reconstruction RMS - X: " << h_xReco_p1->GetRMS() << " mm, Y: " << h_yReco_p1->GetRMS() << " mm, Z: " << h_zReco_p1->GetRMS() << " mm" << std::endl;
    std::cout << "  Smeared RMS - X: " << h_xRecoSmear_p1->GetRMS() << " mm, Y: " << h_yRecoSmear_p1->GetRMS() << " mm, Z: " << h_zRecoSmear_p1->GetRMS() << " mm" << std::endl;

    std::cout << "Plane 2 (" << plane2_count << " entries):" << std::endl;
    std::cout << "  Reconstruction RMS - X: " << h_xReco_p2->GetRMS() << " mm, Y: " << h_yReco_p2->GetRMS() << " mm, Z: " << h_zReco_p2->GetRMS() << " mm" << std::endl;
    std::cout << "  Smeared RMS - X: " << h_xRecoSmear_p2->GetRMS() << " mm, Y: " << h_yRecoSmear_p2->GetRMS() << " mm, Z: " << h_zRecoSmear_p2->GetRMS() << " mm" << std::endl;

    std::cout << "=== Reconstruction plots created successfully ===" << std::endl;
    std::cout << "Created 6 canvases: cTimes_p1, cTimes_p2, cReco_p1, cReco_p2, cSmear_p1, cSmear_p2" << std::endl;
}

// uncertainty analysis
FitResults FitGaussianAndExtractSigma(const std::vector<RecoData>& reco_results, double timeSmearSigma_ns, bool verbose = false, bool showPlots = false) {
    FitResults results;
    results.timeSmearSigma = timeSmearSigma_ns;
    
    if (reco_results.empty()) {
        if (verbose) std::cerr << "Error: No reconstruction results to fit!" << std::endl;
        return results;
    }

    if (verbose) std::cout << "=== Fitting Gaussians for σ = " << timeSmearSigma_ns << " ns ===" << std::endl;

    // === Create histograms for time differences only ===
    double diff_range = 5.0 * timeSmearSigma_ns; // Range based on input smearing
    TH1F *h_tA_p1 = new TH1F("h_tA_p1_fit", "Plane 1: tA_{smeared} - tA_{orig};#Delta tA (ns);Entries", 100, -diff_range, diff_range);
    TH1F *h_tB_p1 = new TH1F("h_tB_p1_fit", "Plane 1: tB_{smeared} - tB_{orig};#Delta tB (ns);Entries", 100, -diff_range, diff_range);
    TH1F *h_tA_p2 = new TH1F("h_tA_p2_fit", "Plane 2: tA_{smeared} - tA_{orig};#Delta tA (ns);Entries", 100, -diff_range, diff_range);
    TH1F *h_tB_p2 = new TH1F("h_tB_p2_fit", "Plane 2: tB_{smeared} - tB_{orig};#Delta tB (ns);Entries", 100, -diff_range, diff_range);

    double vg = 299.792458 / 1.58; // mm/ns
    double expected_reco_sigma = timeSmearSigma_ns * vg / std::sqrt(2);
    double diff_spatial_range = 5.0 * expected_reco_sigma;
    // Reconstruction residual histograms (keep these for the fits)
    TH1F *h_zRecoSmear_p1 = new TH1F("h_zRecoSmear_p1_fit", "Plane 1: zRecoSmeared - zOrig;#Delta z (mm);Entries", 100, -diff_spatial_range, diff_spatial_range);
    TH1F *h_xRecoSmear_p2 = new TH1F("h_xRecoSmear_p2_fit", "Plane 2: xRecoSmeared - xOrig;#Delta x (mm);Entries", 100, -diff_spatial_range, diff_spatial_range);

    // Set colors for histograms (only if plotting)
    if (showPlots) {
        h_tA_p1->SetLineColor(kBlue); h_tA_p1->SetFillColorAlpha(kBlue, 0.3);
        h_tB_p1->SetLineColor(kRed); h_tB_p1->SetFillColorAlpha(kRed, 0.3);
        h_tA_p2->SetLineColor(kGreen); h_tA_p2->SetFillColorAlpha(kGreen, 0.3);
        h_tB_p2->SetLineColor(kMagenta); h_tB_p2->SetFillColorAlpha(kMagenta, 0.3);
        h_zRecoSmear_p1->SetLineColor(kOrange); h_zRecoSmear_p1->SetFillColorAlpha(kOrange, 0.3);
        h_xRecoSmear_p2->SetLineColor(kCyan); h_xRecoSmear_p2->SetFillColorAlpha(kCyan, 0.3);
    }

    // Counters for plane separation
    int plane1_count = 0, plane2_count = 0;

    // Fill histograms
    for (const auto& entry : reco_results) {
        if (entry.planeID == 1) {
            plane1_count++;
            // Fill time differences for Plane 1
            double dtA = entry.tA_smeared - entry.tAOrig;
            double dtB = entry.tB_smeared - entry.tBOrig;
            h_tA_p1->Fill(dtA);
            h_tB_p1->Fill(dtB);
            
            // Fill reconstruction residual
            double dz_smear = entry.zRecoSmeared - entry.zOrig;
            h_zRecoSmear_p1->Fill(dz_smear);
            
        } else if (entry.planeID == 2) {
            plane2_count++;
            // Fill time differences for Plane 2
            double dtA = entry.tA_smeared - entry.tAOrig;
            double dtB = entry.tB_smeared - entry.tBOrig;
            h_tA_p2->Fill(dtA);
            h_tB_p2->Fill(dtB);
            
            // Fill reconstruction residual
            double dx_smear = entry.xRecoSmeared - entry.xOrig;
            h_xRecoSmear_p2->Fill(dx_smear);
        }
    }

    results.plane1_count = plane1_count;
    results.plane2_count = plane2_count;

    if (verbose) {
        std::cout << "Filled histograms: Plane 1 (" << plane1_count << " entries), Plane 2 (" << plane2_count << " entries)" << std::endl;
    }

    // === Function to fit Gaussian and extract parameters ===
    auto fitGaussian = [verbose, showPlots](TH1F* hist, const std::string& name) -> std::pair<double, double> {
        if (hist->GetEntries() < 10) {
            if (verbose) std::cout << "Warning: " << name << " has too few entries for fitting" << std::endl;
            return {0.0, 0.0};
        }

        // Define fit range around the peak
        double mean_estimate = hist->GetMean();
        double rms_estimate = hist->GetRMS();
        double fit_min = mean_estimate - 3 * rms_estimate;
        double fit_max = mean_estimate + 3 * rms_estimate;

        // Create and perform Gaussian fit
        TF1 *gaus = new TF1(("gaus_" + name).c_str(), "gaus", fit_min, fit_max);
        gaus->SetParameters(hist->GetMaximum(), mean_estimate, rms_estimate);
        
        // Set fit function style for visibility
        gaus->SetLineColor(kRed);
        gaus->SetLineWidth(2);
        gaus->SetLineStyle(1);

        hist->Fit(gaus, "RQ0"); // R=range, Q=quiet, 0=don't draw
        
        double fitted_mean = gaus->GetParameter(1);
        double fitted_sigma = gaus->GetParameter(2);
        double fitted_sigma_error = gaus->GetParError(2);
        double chi2_ndf = gaus->GetChisquare() / gaus->GetNDF();

        if (verbose) {
            std::cout << name << ":" << std::endl;
            std::cout << "  Entries:        " << hist->GetEntries() << std::endl;
            std::cout << "  Histogram Mean: " << hist->GetMean() << " ± " << hist->GetMeanError() << std::endl;
            std::cout << "  Histogram RMS:  " << hist->GetRMS() << " ± " << hist->GetRMSError() << std::endl;
            std::cout << "  Fitted Mean:    " << fitted_mean << " ± " << gaus->GetParError(1) << std::endl;
            std::cout << "  Fitted Sigma:   " << fitted_sigma << " ± " << fitted_sigma_error << std::endl;
            std::cout << "  Fitted Amp:     " << gaus->GetParameter(0) << " ± " << gaus->GetParError(0) << std::endl;
            std::cout << "  Chi²/NDF:       " << chi2_ndf << std::endl;
            std::cout << "  Fit Range:      [" << fit_min << ", " << fit_max << "]" << std::endl;
            std::cout << std::endl;
        }

        return {fitted_sigma, fitted_sigma_error};
    };

    // === Function to draw histogram with fit ===
    auto drawHistWithFit = [](TH1F* hist, const std::string& name) {
        hist->Draw("hist");
        
        // Get the fitted function and draw it
        TF1 *func = hist->GetFunction(("gaus_" + name).c_str());
        if (func) {
            func->SetLineColor(kRed);
            func->SetLineWidth(3);
            func->Draw("same");
        }
        gPad->Update();
    };

    // Perform fits for all histograms (even if not plotting)
    auto [s1, e1] = fitGaussian(h_tA_p1, "Plane_1_tA_Difference");
    results.sigma_tA_p1 = s1; results.sigma_tA_p1_err = e1;

    auto [s2, e2] = fitGaussian(h_tB_p1, "Plane_1_tB_Difference");
    results.sigma_tB_p1 = s2; results.sigma_tB_p1_err = e2;

    auto [s3, e3] = fitGaussian(h_tA_p2, "Plane_2_tA_Difference");
    results.sigma_tA_p2 = s3; results.sigma_tA_p2_err = e3;

    auto [s4, e4] = fitGaussian(h_tB_p2, "Plane_2_tB_Difference");
    results.sigma_tB_p2 = s4; results.sigma_tB_p2_err = e4;

    auto [s5, e5] = fitGaussian(h_zRecoSmear_p1, "Plane_1_Z_Reco_Residual");
    results.sigma_zP1 = s5; results.sigma_zP1_err = e5;

    auto [s6, e6] = fitGaussian(h_xRecoSmear_p2, "Plane_2_X_Reco_Residual");
    results.sigma_zP2 = s6; results.sigma_zP2_err = e6;

    // Create plots only if requested
    if (showPlots) {
         // === Create canvas for Plane 1 time differences ===
        TCanvas *cTimeDiffs_P1 = new TCanvas("cTimeDiffs_P1", Form("Plane 1 - Time Differences (σ = %.1f ns)", timeSmearSigma_ns), 1200, 400);
        cTimeDiffs_P1->SetBit(kCanDelete, kFALSE);
        cTimeDiffs_P1->Divide(2, 1);

        cTimeDiffs_P1->cd(1); 
        drawHistWithFit(h_tA_p1, "Plane_1_tA_Difference");

        cTimeDiffs_P1->cd(2); 
        drawHistWithFit(h_tB_p1, "Plane_1_tB_Difference");

        cTimeDiffs_P1->Update();
        gSystem->ProcessEvents();

        // === Create canvas for Plane 2 time differences ===
        TCanvas *cTimeDiffs_P2 = new TCanvas("cTimeDiffs_P2", Form("Plane 2 - Time Differences (σ = %.1f ns)", timeSmearSigma_ns), 1200, 400);
        cTimeDiffs_P2->SetBit(kCanDelete, kFALSE);
        cTimeDiffs_P2->Divide(2, 1);

        cTimeDiffs_P2->cd(1); 
        drawHistWithFit(h_tA_p2, "Plane_2_tA_Difference");

        cTimeDiffs_P2->cd(2); 
        drawHistWithFit(h_tB_p2, "Plane_2_tB_Difference");

        cTimeDiffs_P2->Update();
        gSystem->ProcessEvents();

        // === Create canvas for reconstruction residuals ===
        TCanvas *cRecoFits = new TCanvas("cRecoFits", "Reconstruction Residuals", 1200, 400);
        cRecoFits->SetBit(kCanDelete, kFALSE);
        cRecoFits->Divide(2, 1);

        cRecoFits->cd(1); 
        drawHistWithFit(h_zRecoSmear_p1, "Plane_1_Z_Reco_Residual");

        cRecoFits->cd(2); 
        drawHistWithFit(h_xRecoSmear_p2, "Plane_2_X_Reco_Residual");

        cRecoFits->Update();
        gSystem->ProcessEvents();
    }
    // Print summary if verbose
    if (verbose) {
        std::cout << "========================================" << std::endl;
        std::cout << "=== TIME DIFFERENCE ANALYSIS ===" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Input time smearing: " << timeSmearSigma_ns << " ns" << std::endl;
        std::cout << "Plane 1 entries: " << plane1_count << ", Plane 2 entries: " << plane2_count << std::endl;
        std::cout << std::endl;

        std::cout << "PLANE 1 - Time Difference Sigmas (Smeared - Original):" << std::endl;
        std::cout << "  tA Difference: " << results.sigma_tA_p1 << " ± " << results.sigma_tA_p1_err << " ns (Expected: " << timeSmearSigma_ns << " ns)" << std::endl;
        std::cout << "  tB Difference: " << results.sigma_tB_p1 << " ± " << results.sigma_tB_p1_err << " ns (Expected: " << timeSmearSigma_ns << " ns)" << std::endl;
     std::cout << std::endl;

        std::cout << "PLANE 2 - Time Difference Sigmas (Smeared - Original):" << std::endl;
        std::cout << "  tA Difference: " << results.sigma_tA_p2 << " ± " << results.sigma_tA_p2_err << " ns (Expected: " << timeSmearSigma_ns << " ns)" << std::endl;
        std::cout << "  tB Difference: " << results.sigma_tB_p2 << " ± " << results.sigma_tB_p2_err << " ns (Expected: " << timeSmearSigma_ns << " ns)" << std::endl;      std::cout << std::endl;


        // === Calculate theoretical expectations ===
        std::cout << "Reconstruction Resolution Sigmas:" << std::endl;
        std::cout << "  Plane 1 (Z): " << results.sigma_zP1 << " ± " << results.sigma_zP1_err << " mm (Expected: " << expected_reco_sigma << " mm)" << std::endl;
        std::cout << "  Plane 2 (X): " << results.sigma_zP2 << " ± " << results.sigma_zP2_err << " mm (Expected: " << expected_reco_sigma << " mm)" << std::endl;
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
    }

    // Clean up histograms if not plotting
    if (!showPlots) {
        delete h_tA_p1; delete h_tB_p1; delete h_tA_p2; delete h_tB_p2;
        delete h_zRecoSmear_p1; delete h_xRecoSmear_p2;
    }

    return results;
}

void PlotTimeSmearingResults(const std::vector<FitResults>& vectorResults) {
    if (vectorResults.empty()) {
        std::cerr << "Error: No results to plot!" << std::endl;
        return;
    }

    std::cout << "Plotting time smearing results for " << vectorResults.size() << " data points" << std::endl;

    // Extract data for plotting
    std::vector<double> timeSmearValues;
    std::vector<double> sigmaZ_p1, sigmaZ_p1_err;
    std::vector<double> sigmaX_p2, sigmaX_p2_err;
    std::vector<double> theoretical_sigma;

    double vg = 299.792458 / 1.58; // mm/ns - speed of light in material

    for (const auto& result : vectorResults) {
        timeSmearValues.push_back(result.timeSmearSigma);
        sigmaZ_p1.push_back(result.sigma_zP1);
        sigmaZ_p1_err.push_back(result.sigma_zP1_err);
        sigmaX_p2.push_back(result.sigma_zP2); // Note: this is actually sigmaX for plane 2
        sigmaX_p2_err.push_back(result.sigma_zP2_err);
        
        // Theoretical expectation: σ_reco = σ_time * vg / √2
        double theory = result.timeSmearSigma * vg / std::sqrt(2);
        theoretical_sigma.push_back(theory);
    }

    // === CALCULATE INTERSECTION POINT ===
    // For σ_spatial = 22 mm and theoretical curve σ_spatial = σ_t × v_g / √2
    // Solve: 22 = σ_t × v_g / √2
    // Therefore: σ_t = 22 × √2 / v_g
    double target_spatial_sigma = 22.0; // mm
    double intersection_sigma_t = target_spatial_sigma * std::sqrt(2) / vg;
    
    std::cout << "\n=== INTERSECTION ANALYSIS ===" << std::endl;
    std::cout << "Target spatial resolution: " << target_spatial_sigma << " mm" << std::endl;
    std::cout << "Speed of light in material: " << vg << " mm/ns" << std::endl;
    std::cout << "Theoretical formula: σ_spatial = σ_t × " << vg << " / √2" << std::endl;
    std::cout << "**Intersection point: σ_t = " << intersection_sigma_t << " ns**" << std::endl;
    std::cout << "Verification: σ_spatial = " << intersection_sigma_t * vg / std::sqrt(2) << " mm" << std::endl;
    std::cout << "=============================" << std::endl;

    // Convert to arrays for TGraphErrors
    int n = timeSmearValues.size();
    double* x_vals = new double[n];
    double* x_errs = new double[n];
    double* y_vals_p1 = new double[n];
    double* y_errs_p1 = new double[n];
    double* y_vals_p2 = new double[n];
    double* y_errs_p2 = new double[n];
    double* y_theory = new double[n];
    double* y_theory_errs = new double[n];

    for (int i = 0; i < n; i++) {
        x_vals[i] = timeSmearValues[i];
        x_errs[i] = 0.0; // No error on time smearing values (they're input parameters)
        
        y_vals_p1[i] = sigmaZ_p1[i];
        y_errs_p1[i] = sigmaZ_p1_err[i];
        
        y_vals_p2[i] = sigmaX_p2[i];
        y_errs_p2[i] = sigmaX_p2_err[i];
        
        y_theory[i] = theoretical_sigma[i];
        y_theory_errs[i] = 0.0; // No error on theoretical values
    }

    // Create TGraphErrors objects
    TGraphErrors *gr_p1 = new TGraphErrors(n, x_vals, y_vals_p1, x_errs, y_errs_p1);
    TGraphErrors *gr_p2 = new TGraphErrors(n, x_vals, y_vals_p2, x_errs, y_errs_p2);
    TGraphErrors *gr_theory = new TGraphErrors(n, x_vals, y_theory, x_errs, y_theory_errs);

    // Style the graphs
    gr_p1->SetMarkerStyle(20);
    gr_p1->SetMarkerColor(kBlue);
    gr_p1->SetLineColor(kBlue);
    gr_p1->SetMarkerSize(1.2);

    gr_p2->SetMarkerStyle(21);
    gr_p2->SetMarkerColor(kRed);
    gr_p2->SetLineColor(kRed);
    gr_p2->SetMarkerSize(1.2);

    gr_theory->SetMarkerStyle(0); // No markers for theory line
    gr_theory->SetMarkerColor(kGreen+2);
    gr_theory->SetLineColor(kGreen+2);
    gr_theory->SetLineStyle(2);
    gr_theory->SetLineWidth(3);

    // Find X-axis range for horizontal line
    double x_min = *std::min_element(timeSmearValues.begin(), timeSmearValues.end());
    double x_max = *std::max_element(timeSmearValues.begin(), timeSmearValues.end());
    
    // Extend range to show intersection point if it's outside current range
    x_min = std::min(x_min, intersection_sigma_t * 0.8);
    x_max = std::max(x_max, intersection_sigma_t * 1.2);
    
    // Create horizontal line at 22 mm
    TLine *horizontal_line = new TLine(x_min, target_spatial_sigma, x_max, target_spatial_sigma);
    horizontal_line->SetLineColor(kOrange);
    horizontal_line->SetLineWidth(3);
    horizontal_line->SetLineStyle(9); // Dashed line style

    // === Linear Scale Canvas ===
    TCanvas *cLinear = new TCanvas("cSmearStudy_Linear", "Spatial Resolution vs Time Smearing (Linear Scale)", 1000, 700);
    cLinear->SetBit(kCanDelete, kFALSE);
    cLinear->SetLeftMargin(0.12);
    cLinear->SetBottomMargin(0.12);
    cLinear->SetGrid();

    // Draw plane 1 first to set the axes
    gr_p1->SetTitle("Spatial Resolution vs Time Resolution (Linear);#sigma_{t} (ns);#sigma_{spatial} (mm)");
    gr_p1->Draw("APE");
    gr_p1->GetXaxis()->SetTitleSize(0.045);
    gr_p1->GetYaxis()->SetTitleSize(0.045);
    gr_p1->GetXaxis()->SetLabelSize(0.04);
    gr_p1->GetYaxis()->SetLabelSize(0.04);

    // Set axis ranges to show intersection clearly
    gr_p1->GetXaxis()->SetRangeUser(x_min, x_max);
    gr_p1->GetYaxis()->SetRangeUser(0, target_spatial_sigma * 1.3);

    // Draw plane 2 and theory
    gr_p2->Draw("PE same");
    gr_theory->Draw("L same");

    // Draw intersection lines
    horizontal_line->Draw("same");

    // Add legend
    TLegend *legLinear = new TLegend(0.15, 0.60, 0.60, 0.85);
    legLinear->AddEntry(gr_p1, "Plane 1: #sigma_{Z}", "pe");
    legLinear->AddEntry(gr_p2, "Plane 2: #sigma_{X}", "pe");
    legLinear->AddEntry(gr_theory, "Theory: #sigma_{t} v_{g} / #sqrt{2}", "l");
    legLinear->AddEntry(horizontal_line, Form("%.0f mm target", target_spatial_sigma), "l");
    legLinear->SetTextSize(0.030);
    legLinear->SetBorderSize(1);
    legLinear->SetFillColor(kWhite);
    legLinear->Draw();

    cLinear->Update();
    gSystem->ProcessEvents();

    // === Log-Log Scale Canvas ===
    TCanvas *cLogLog = new TCanvas("cSmearStudy_LogLog", "Spatial Resolution vs Time Smearing (Log-Log Scale)", 1000, 700);
    cLogLog->SetBit(kCanDelete, kFALSE);
    cLogLog->SetLeftMargin(0.12);
    cLogLog->SetBottomMargin(0.12);
    cLogLog->SetGrid();
    cLogLog->SetLogx(); // Log scale on X-axis
    cLogLog->SetLogy(); // Log scale on Y-axis

    // Clone graphs for log plot
    TGraphErrors *gr_p1_log = (TGraphErrors*)gr_p1->Clone("gr_p1_log");
    TGraphErrors *gr_p2_log = (TGraphErrors*)gr_p2->Clone("gr_p2_log");
    TGraphErrors *gr_theory_log = (TGraphErrors*)gr_theory->Clone("gr_theory_log");


    gr_p1_log->SetTitle("Spatial Resolution vs Time Resolution; #sigma_{t} (ns);#sigma_{spatial} (mm)");
    gr_p1_log->Draw("APE");
    gr_p1_log->GetXaxis()->SetTitleSize(0.045);
    gr_p1_log->GetYaxis()->SetTitleSize(0.045);
    gr_p1_log->GetXaxis()->SetLabelSize(0.04);
    gr_p1_log->GetYaxis()->SetLabelSize(0.04);

    gr_p2_log->Draw("PE same");
    gr_theory_log->Draw("L same");


    // Add legend for log plot
    TLegend *legLog = new TLegend(0.15, 0.65, 0.60, 0.85);
    legLog->AddEntry(gr_p1_log, "Plane 1: #sigma_{z}", "pe");
    legLog->AddEntry(gr_p2_log, "Plane 2: #sigma_{x}", "pe");
    legLog->AddEntry(gr_theory_log, "Theory: #sigma_{t} v_{g} / #sqrt{2}", "l");
    legLog->SetTextSize(0.035);
    legLog->SetBorderSize(1);
    legLog->SetFillColor(kWhite);
    legLog->Draw();

    cLogLog->Update();
    gSystem->ProcessEvents();

    // === Semi-Log Scale Canvas (Log X, Linear Y) ===
    TCanvas *cSemiLog = new TCanvas("cSmearStudy_SemiLog", "Spatial Resolution vs Time Smearing (Semi-Log Scale)", 1000, 700);
    cSemiLog->SetBit(kCanDelete, kFALSE);
    cSemiLog->SetLeftMargin(0.12);
    cSemiLog->SetBottomMargin(0.12);
    cSemiLog->SetGrid();
    cSemiLog->SetLogx(); // Log scale on X-axis only

    // Clone graphs for semi-log plot
    TGraphErrors *gr_p1_semi = (TGraphErrors*)gr_p1->Clone("gr_p1_semi");
    TGraphErrors *gr_p2_semi = (TGraphErrors*)gr_p2->Clone("gr_p2_semi");
    TGraphErrors *gr_theory_semi = (TGraphErrors*)gr_theory->Clone("gr_theory_semi");

    // Create lines for semi-log plot
    TLine *horizontal_line_semi = new TLine(x_min, target_spatial_sigma, x_max, target_spatial_sigma);
    horizontal_line_semi->SetLineColor(kOrange);
    horizontal_line_semi->SetLineWidth(3);
    horizontal_line_semi->SetLineStyle(9);


    gr_p1_semi->SetTitle("Spatial Resolution vs Time Smearing (Semi-Log); #sigma_{t} (ns); #sigma_{spatial} (mm)");
    gr_p1_semi->Draw("APE");
    gr_p1_semi->GetXaxis()->SetTitleSize(0.045);
    gr_p1_semi->GetYaxis()->SetTitleSize(0.045);
    gr_p1_semi->GetXaxis()->SetLabelSize(0.04);
    gr_p1_semi->GetYaxis()->SetLabelSize(0.04);

    gr_p2_semi->Draw("PE same");
    gr_theory_semi->Draw("L same");
    horizontal_line_semi->Draw("same");

    // Add legend for semi-log plot
    TLegend *legSemi = new TLegend(0.15, 0.65, 0.60, 0.85);
    legSemi->AddEntry(gr_p1_semi, "Plane 1: #sigma_{Z}", "pe");
    legSemi->AddEntry(gr_p2_semi, "Plane 2: #sigma_{X}", "pe");
    legSemi->AddEntry(gr_theory_semi, "Theory: #sigma_{t} * v_{g} / #sqrt{2}", "l");
    legSemi->SetTextSize(0.035);
    legSemi->SetBorderSize(1);
    legSemi->SetFillColor(kWhite);
    legSemi->Draw();

    cSemiLog->Update();
    gSystem->ProcessEvents();

    // Print numerical results including intersection
    std::cout << "========================================" << std::endl;
    std::cout << "    TIME SMEARING STUDY RESULTS    " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Speed of light in material: " << vg << " mm/ns" << std::endl;
    std::cout << "Theoretical formula: σ_spatial = σ_time × " << vg << " / √2 = σ_time × " << vg/std::sqrt(2) << std::endl;
    std::cout << "**INTERSECTION: For σ_spatial = " << target_spatial_sigma << " mm → σ_t = " << intersection_sigma_t << " ns**" << std::endl;
    std::cout << std::endl;

    std::cout << std::setw(12) << "σ_time (ns)" 
              << std::setw(15) << "σ_Z P1 (mm)" 
              << std::setw(15) << "σ_X P2 (mm)" 
              << std::setw(15) << "Theory (mm)" 
              << std::setw(12) << "Ratio P1" 
              << std::setw(12) << "Ratio P2" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    for (int i = 0; i < n; i++) {
        double ratio_p1 = (theoretical_sigma[i] > 0) ? sigmaZ_p1[i] / theoretical_sigma[i] : 0;
        double ratio_p2 = (theoretical_sigma[i] > 0) ? sigmaX_p2[i] / theoretical_sigma[i] : 0;
        
        std::cout << std::setw(12) << std::fixed << std::setprecision(3) << timeSmearValues[i]
                  << std::setw(15) << std::setprecision(2) << sigmaZ_p1[i] << "±" << sigmaZ_p1_err[i]
                  << std::setw(15) << std::setprecision(2) << sigmaX_p2[i] << "±" << sigmaX_p2_err[i]
                  << std::setw(15) << std::setprecision(2) << theoretical_sigma[i]
                  << std::setw(12) << std::setprecision(3) << ratio_p1
                  << std::setw(12) << std::setprecision(3) << ratio_p2 << std::endl;
    }

    std::cout << std::endl;
    std::cout << "** TARGET RESOLUTION REQUIREMENT **" << std::endl;
    std::cout << "Required spatial resolution: " << target_spatial_sigma << " mm" << std::endl;
    std::cout << "Required time resolution: " << intersection_sigma_t << " ns" << std::endl;
    std::cout << "========================================" << std::endl;

    // Clean up arrays
    delete[] x_vals;
    delete[] x_errs;
    delete[] y_vals_p1;
    delete[] y_errs_p1;
    delete[] y_vals_p2;
    delete[] y_errs_p2;
    delete[] y_theory;
    delete[] y_theory_errs;
}
std::vector<EventHitPattern> AnalyzeEventHitPatterns(const std::vector<RecoData>& reco_results, bool verbose = false) {
    if (reco_results.empty()) {
        std::cerr << "Error: No reconstruction results to analyze!" << std::endl;
        return {};
    }
    
    std::cout << "Analyzing Event Hit Patterns... " << std::endl;
    std::cout << "Total reconstruction entries: " << reco_results.size() << std::endl;
    
    // Group data by eventID
    std::map<int, std::vector<RecoData>> eventGroups;
    
    for (const auto& reco : reco_results) {
        eventGroups[reco.eventID].push_back(reco);
    }
    
    std::cout << "Found " << eventGroups.size() << " unique events" << std::endl;
    
    std::vector<EventHitPattern> eventPatterns;
    eventPatterns.reserve(eventGroups.size());
    
    for (const auto& [eventID, recoList] : eventGroups) {
        EventHitPattern pattern;
        pattern.eventID = eventID;
        
        // Separate hits by plane
        for (const auto& reco : recoList) {
            if (reco.planeID == 1) {
                pattern.plane1_data.push_back(reco);
            } else if (reco.planeID == 2) {
                pattern.plane2_data.push_back(reco);
            }
        }
        
        pattern.plane1_hits = pattern.plane1_data.size();
        pattern.plane2_hits = pattern.plane2_data.size();
        
        // Classify the pattern
        if (pattern.plane1_hits == 1 && pattern.plane2_hits == 1) {
            pattern.pattern_type = EventHitPattern::SINGLE_SINGLE;
        } else if ((pattern.plane1_hits == 1 && pattern.plane2_hits == 2) || 
                   (pattern.plane1_hits == 2 && pattern.plane2_hits == 1)) {
            pattern.pattern_type = EventHitPattern::SINGLE_DOUBLE;
        } else if (pattern.plane1_hits == 2 && pattern.plane2_hits == 2) {
            pattern.pattern_type = EventHitPattern::DOUBLE_DOUBLE;
        } else {
            pattern.pattern_type = EventHitPattern::OTHER;
        }
        
        eventPatterns.push_back(pattern);
    }
    
    return eventPatterns;
}

void AnalyzeDetailedHitPatterns(const std::vector<EventHitPattern>& eventPatterns) {
    
    // Create a map to count all possible combinations
    std::map<std::pair<int, int>, int> patternCounts;
    std::map<std::pair<int, int>, std::vector<int>> patternExamples; // Store example event IDs
    
    int maxHitsP1 = 0, maxHitsP2 = 0;
    
    for (const auto& pattern : eventPatterns) {
        std::pair<int, int> hitPattern = {pattern.plane1_hits, pattern.plane2_hits};
        patternCounts[hitPattern]++;
        
        // Store first few examples for each pattern
        if (patternExamples[hitPattern].size() < 3) {
            patternExamples[hitPattern].push_back(pattern.eventID);
        }
        
        // Track maximum hits
        if (pattern.plane1_hits > maxHitsP1) maxHitsP1 = pattern.plane1_hits;
        if (pattern.plane2_hits > maxHitsP2) maxHitsP2 = pattern.plane2_hits;
    }
    
    std::cout << "Maximum hits observed: Plane 1 = " << maxHitsP1 << ", Plane 2 = " << maxHitsP2 << std::endl;
    std::cout << std::endl;
    
    // Calculate total for percentages
    int total = eventPatterns.size();
    
    std::cout << "Complete hit pattern breakdown:" << std::endl;
    std::cout << std::setw(8) << "Plane1" << std::setw(8) << "Plane2" << std::setw(12) << "Count" 
              << std::setw(10) << "Percent" << std::setw(20) << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    // Sort patterns by frequency (descending)
    std::vector<std::pair<std::pair<int, int>, int>> sortedPatterns(patternCounts.begin(), patternCounts.end());
    std::sort(sortedPatterns.begin(), sortedPatterns.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (const auto& [pattern, count] : sortedPatterns) {
        double percentage = 100.0 * count / total;
        
        std::cout << std::setw(8) << pattern.first << std::setw(8) << pattern.second 
                  << std::setw(12) << count << std::setw(9) << std::fixed << std::setprecision(2) << percentage << "%";
        
        std::cout << std::endl;
    }
    
    std::cout << std::endl;
    
    // Special analysis for high-multiplicity events
    std::cout << "High multiplicity events:" << std::endl;

    int highMultEvents = 0;
    std::vector<int> exampleHighMult;
    
    for (const auto& pattern : eventPatterns) {
        if (pattern.plane1_hits >= 3 || pattern.plane2_hits >= 3) {
            highMultEvents++;
            if (exampleHighMult.size() < 10) {
                exampleHighMult.push_back(pattern.eventID);
            }
        }
    }
    
    std::cout << "Events with ≥3 hits in any plane: " << highMultEvents 
              << " (" << 100.0 * highMultEvents / total << "%)" << std::endl;
    
    
    // Events with 0 hits in one plane
    std::cout << "\nSingle plane events" << std::endl;
    
    int plane1Only = 0, plane2Only = 0;
    for (const auto& [pattern, count] : patternCounts) {
        if (pattern.first > 0 && pattern.second == 0) {
            plane1Only += count;
        } else if (pattern.first == 0 && pattern.second > 0) {
            plane2Only += count;
        }
    }
    
    std::cout << "Events with hits only in Plane 1: " << plane1Only 
              << " (" << 100.0 * plane1Only / total << "%)" << std::endl;
    std::cout << "Events with hits only in Plane 2: " << plane2Only 
              << " (" << 100.0 * plane2Only / total << "%)" << std::endl;
    
    // Create a 2D histogram to visualize hit patterns
    TCanvas *cHitMatrix = new TCanvas("cHitMatrix", "Hit Pattern Matrix", 800, 600);
    SetCanvasStyle(cHitMatrix);
    
    int maxDim = std::max(maxHitsP1, maxHitsP2) + 1;
    maxDim = std::max(maxDim, 6); // At least 6x6 for good visualization

    TH2I *hHitMatrix = new TH2I("hHitMatrix", "Hit Pattern Matrix;Plane 1 Hits;Plane 2 Hits;Events", 
                                maxDim, -0.5, maxDim - 0.5, maxDim, -0.5, maxDim - 0.5);
    
    for (const auto& [pattern, count] : patternCounts) {
        hHitMatrix->SetBinContent(pattern.first + 1, pattern.second + 1, count);
    }
    
    hHitMatrix->SetStats(kFALSE); // Remove statistics box
    
    // Set axis properties
    hHitMatrix->GetXaxis()->SetTitleSize(0.045);
    hHitMatrix->GetYaxis()->SetTitleSize(0.045);
    hHitMatrix->GetZaxis()->SetTitleSize(0.040);
    hHitMatrix->GetXaxis()->SetLabelSize(0.04);
    hHitMatrix->GetYaxis()->SetLabelSize(0.04);
    hHitMatrix->GetZaxis()->SetLabelSize(0.035);
    
    hHitMatrix->GetXaxis()->SetTitleOffset(1.2);
    hHitMatrix->GetYaxis()->SetTitleOffset(1.3);
    hHitMatrix->GetZaxis()->SetTitleOffset(1.3);
    
    // Set integer labels for axes
    hHitMatrix->GetXaxis()->SetNdivisions(maxDim, kFALSE);
    hHitMatrix->GetYaxis()->SetNdivisions(maxDim, kFALSE);

    hHitMatrix->GetXaxis()->SetNdivisions(maxDim, kFALSE);
    hHitMatrix->GetYaxis()->SetNdivisions(maxDim, kFALSE);
    
    for (int i = 1; i <= maxDim; i++) {
        hHitMatrix->GetXaxis()->SetBinLabel(i, Form("%d", i-1));
        hHitMatrix->GetYaxis()->SetBinLabel(i, Form("%d", i-1));
    }

    hHitMatrix->GetXaxis()->CenterLabels(kTRUE);
    hHitMatrix->GetYaxis()->CenterLabels(kTRUE);
    
    // better color palette
    gStyle->SetPalette(kViridis); 
    
    // Draw with enhanced options
    hHitMatrix->Draw("COLZ");
    
    // Add grid lines for better readability
    gPad->SetGridx(1);
    gPad->SetGridy(1);
    gStyle->SetGridColor(kGray);  // Set grid color through global style
    gStyle->SetGridStyle(1);      // Solid grid lines
    gStyle->SetGridWidth(1);
    
    // Add text annotations for significant patterns
    TLatex latex;
    latex.SetTextAlign(22); // Center alignment
    latex.SetTextSize(0.03);
    latex.SetTextColor(kWhite);
    latex.SetTextFont(42); // Helvetica
    
    
    for (const auto& [pattern, count] : patternCounts) {
            if (count > 0) {
                double percentage = 100.0 * count / total;
                
                // Only show text for cells with significant counts or important patterns
                bool showText = false;
                
                if (count > total * 0.01) { // More than 1% of events
                    showText = true;
                } else if ((pattern.first <= 2 && pattern.second <= 2) && count > 0) {
                    // Always show common patterns (≤2 hits per plane)
                    showText = true;
                }
                
                if (showText) {
                    std::string text;
                    if (percentage >= 1.0) {
                        text = Form("%.1f%%", percentage);
                    } else if (percentage >= 0.1) {
                        text = Form("%.2f%%", percentage);
                    } else {
                        text = Form("%.3f%%", percentage);
                    }
                    
                    // Choose text color based on background
                    Color_t textColor = kWhite;
                    latex.SetTextColor(textColor);
                    
                    latex.DrawLatex(pattern.first, pattern.second, text.c_str());
                }
            }
        }
    
    // Add a title with total events
    TPaveText *title = new TPaveText(0.1, 0.92, 0.9, 0.98, "NDC");
    title->SetBorderSize(0);
    title->SetFillColor(kWhite);
    title->SetTextAlign(22);
    title->SetTextSize(0.035);
    title->SetTextFont(42);
    title->AddText(Form("Hit Pattern Distribution (%d total events)", total));
    title->Draw();
    
    // Add a legend explaining the most common patterns
    TPaveText *legend = new TPaveText(0.65, 0.02, 0.98, 0.25, "NDC");
    legend->SetBorderSize(1);
    legend->SetFillColor(kWhite);
    legend->SetFillStyle(1001);
    legend->SetTextAlign(12); // Left alignment
    legend->SetTextSize(0.025);
    legend->SetTextFont(42);
    legend->AddText("Common Patterns:");
    legend->AddText("(1,1): Single-Single");
    legend->AddText("(1,2), (2,1): Single-Double");
    legend->AddText("(2,2): Double-Double");
    legend->Draw();
    
    // Add frame around the plot
    cHitMatrix->SetFrameLineWidth(2);
    cHitMatrix->SetFrameLineColor(kBlack);
    
    cHitMatrix->Update();
    gSystem->ProcessEvents();
    
}

HitPatternStatistics CalculateHitPatternStatistics(const std::vector<EventHitPattern>& eventPatterns) {
    HitPatternStatistics stats;
    stats.total_events = eventPatterns.size();
    
    for (const auto& pattern : eventPatterns) {
        switch (pattern.pattern_type) {
            case EventHitPattern::SINGLE_SINGLE:
                stats.single_single_events++;
                stats.events_1p1_1p2++;
                break;
                
            case EventHitPattern::SINGLE_DOUBLE:
                stats.single_double_events++;
                if (pattern.plane1_hits == 1 && pattern.plane2_hits == 2) {
                    stats.events_1p1_2p2++;
                } else if (pattern.plane1_hits == 2 && pattern.plane2_hits == 1) {
                    stats.events_2p1_1p2++;
                }
                break;
                
            case EventHitPattern::DOUBLE_DOUBLE:
                stats.double_double_events++;
                stats.events_2p1_2p2++;
                break;
                
            case EventHitPattern::OTHER:
                stats.other_pattern_events++;
                break;
        }
    }
    
    return stats;
}

void PlotHitPatternStatistics(const HitPatternStatistics& stats) {
    // Calculate percentages
    auto percentage = [&](int count) -> double {
        return stats.total_events > 0 ? 100.0 * count / stats.total_events : 0.0;
    };
    
    std::cout << "Pattern breakdown:" << std::endl;
    std::cout << "  1 hit plane1, 1 hit plane2: Single-Single (1-1):  " << stats.events_1p1_1p2 
              << " (" << percentage(stats.events_1p1_1p2) << "%)" << std::endl;
    std::cout << "  1 hit plane1, 2 hits plane2: Single-Double (1-2):  " << stats.events_1p1_2p2 
              << " (" << percentage(stats.events_1p1_2p2) << "%)" << std::endl;
    std::cout << "  2 hits plane1, 1 hit plane2: Double-Single (2-1):  " << stats.events_2p1_1p2 
              << " (" << percentage(stats.events_2p1_1p2) << "%)" << std::endl;
    std::cout << "  2 hits plane1, 2 hits plane2: Double-Double (2-2):  " << stats.events_2p1_2p2 
              << " (" << percentage(stats.events_2p1_2p2) << "%)" << std::endl;
    std::cout << "  Other patterns:               " << stats.other_pattern_events 
              << " (" << percentage(stats.other_pattern_events) << "%)" << std::endl;
    std::cout << std::endl;
    
    // Create histogram
    TCanvas *cPatterns = new TCanvas("cPatterns", "Hit Pattern Distribution", 1000, 600);
    cPatterns->SetLeftMargin(0.12);
    cPatterns->SetBottomMargin(0.15);
    
    // Create histogram with pattern categories
    TH1F *hPatterns = new TH1F("hPatterns", "Event Hit Pattern Distribution;Pattern Type;Number of Events", 5, 0, 5);
    
    hPatterns->GetXaxis()->SetBinLabel(1, "1P1-1P2");
    hPatterns->GetXaxis()->SetBinLabel(2, "1P1-2P2");
    hPatterns->GetXaxis()->SetBinLabel(3, "2P1-1P2");
    hPatterns->GetXaxis()->SetBinLabel(4, "2P1-2P2");
    hPatterns->GetXaxis()->SetBinLabel(5, "Other");
    
    hPatterns->SetBinContent(1, stats.events_1p1_1p2);
    hPatterns->SetBinContent(2, stats.events_1p1_2p2);
    hPatterns->SetBinContent(3, stats.events_2p1_1p2);
    hPatterns->SetBinContent(4, stats.events_2p1_2p2);
    hPatterns->SetBinContent(5, stats.other_pattern_events);
    
    // Apply consistent styling with alpha transparency
    SetHistogramStyle(hPatterns, kBlue+1, kBlue-8, "Pattern Type", "Number of Events");
    
    
    hPatterns->Draw("hist");
    hPatterns->GetXaxis()->SetLabelSize(0.045);
    hPatterns->GetYaxis()->SetLabelSize(0.04);
    hPatterns->GetXaxis()->SetTitleSize(0.045);
    hPatterns->GetYaxis()->SetTitleSize(0.045);
    
    // Add text with percentages on top of bars
    for (int i = 1; i <= 5; i++) {
        double content = hPatterns->GetBinContent(i);
        if (content > 0) {
            TText *text = new TText(i-0.5, content + stats.total_events * 0.02, 
                                   Form("%.1f%%", percentage(content)));
            text->SetTextAlign(22);
            text->SetTextSize(0.035);
            text->Draw();
        }
    }

    cPatterns->Update();
    gSystem->ProcessEvents();
}

void PlotBarIDDistribution(const std::vector<EventHitPattern>& eventPatterns) {

    // Track bar IDs for each pattern type
    std::map<int, int> barID_plane1_counts;
    std::map<int, int> barID_plane2_counts;
    
    for (const auto& pattern : eventPatterns) {
        // Count bar IDs in plane 1
        for (const auto& hit : pattern.plane1_data) {
            barID_plane1_counts[hit.barID]++;
        }
        
        // Count bar IDs in plane 2
        for (const auto& hit : pattern.plane2_data) {
            barID_plane2_counts[hit.barID]++;
        }
    }

    
    // Create bar ID distribution plots
    TCanvas *cBarIDs = new TCanvas("cBarIDs", "Bar ID Distribution", 1200, 500);
    cBarIDs->Divide(2, 1);
    
    // Plane 1 bar distribution
    cBarIDs->cd(1);
    SetCanvasStyle(gPad);
    
    int max_barID_p1 = barID_plane1_counts.empty() ? 1 : barID_plane1_counts.rbegin()->first;
    TH1I *hBarP1 = new TH1I("hBarP1", "Plane 1;Bar ID;Hit Count", 
                            max_barID_p1 + 1, -0.5, max_barID_p1 + 0.5);
    
    for (const auto& [barID, count] : barID_plane1_counts) {
        hBarP1->SetBinContent(barID + 1, count);
    }

    // Apply consistent styling
    SetHistogramStyle(hBarP1, kBlue+1, kBlue-8, "Bar ID", "Hit Count");
    hBarP1->Draw("hist");
    
    // Plane 2 bar distribution
    cBarIDs->cd(2);
    SetCanvasStyle(gPad);

    int max_barID_p2 = barID_plane2_counts.empty() ? 1 : barID_plane2_counts.rbegin()->first;
    TH1I *hBarP2 = new TH1I("hBarP2", "Plane 2;Bar ID;Hit Count", 
                            max_barID_p2 + 1, -0.5, max_barID_p2 + 0.5);
    
    for (const auto& [barID, count] : barID_plane2_counts) {
        hBarP2->SetBinContent(barID + 1, count);
    }
 // Apply consistent styling  
    SetHistogramStyle(hBarP2, kRed+1, kRed-8, "Bar ID", "Hit Count");
    hBarP2->Draw("hist");
    
    cBarIDs->Update();
    gSystem->ProcessEvents();
}

std::vector<EventTrack> ReconstructEventTracks(const std::vector<EventHitPattern>& eventPatterns, bool verbose = false) {
    std::cout << "\nReconstructing event tracks..." << std::endl;
    std::cout << "Processing " << eventPatterns.size() << " events" << std::endl;
    
    std::vector<EventTrack> tracks;
    tracks.reserve(eventPatterns.size());
    
    int valid_events = 0;
    int discarded_events = 0;
    
    // Statistics for different cases
    int events_1p1_1p2 = 0;
    int events_1p1_2p2 = 0;
    int events_2p1_1p2 = 0;
    int events_2p1_2p2 = 0;
    
    for (const auto& pattern : eventPatterns) {
        EventTrack track;
        track.eventID = pattern.eventID;
        
        bool valid_pattern = false;
        
        // Check if this is a valid pattern (1-1, 1-2, 2-1, 2-2)
        if ((pattern.plane1_hits == 1 || pattern.plane1_hits == 2) && 
            (pattern.plane2_hits == 1 || pattern.plane2_hits == 2)) {
            valid_pattern = true;
        }
        
        if (!valid_pattern) {
            discarded_events++;
            continue;
        }
        
        // Process Plane 1 if we have hits
        if (pattern.plane1_hits > 0) {
            track.plane1_point.eventID = pattern.eventID;
            track.plane1_point.planeID = 1;
            track.plane1_point.num_hits_used = pattern.plane1_hits;
            track.has_plane1 = true;
            
            // Store bar IDs used
            for (const auto& hit : pattern.plane1_data) {
                track.plane1_point.barIDs_used.push_back(hit.barID);
            }
            
            if (pattern.plane1_hits == 1) {
                // Single hit: keep original reconstruction
                const auto& hit = pattern.plane1_data[0];
                track.plane1_point.x = hit.xRecoSmeared;
                track.plane1_point.y = 0.0;  // Fixed at 0
                track.plane1_point.z = hit.zRecoSmeared;
                
                track.plane1_point.sigma_x = hit.sigma_x;  // Original uncertainty
                track.plane1_point.sigma_y = 0.0;          // No uncertainty
                track.plane1_point.sigma_z = hit.sigma_z;  // Original uncertainty
                
            } else if (pattern.plane1_hits == 2) {
                // Two hits: take middle point
                const auto& hit1 = pattern.plane1_data[0];
                const auto& hit2 = pattern.plane1_data[1];
                
                track.plane1_point.x = 0.5 * (hit1.xRecoSmeared + hit2.xRecoSmeared);
                track.plane1_point.y = 0.0;  // Fixed at 0
                track.plane1_point.z = 0.5 * (hit1.zRecoSmeared + hit2.zRecoSmeared );
                
                track.plane1_point.sigma_x = 4.6;  // Fixed uncertainty for 2-hit average
                track.plane1_point.sigma_y = 0.0;  // No uncertainty
                track.plane1_point.sigma_z = 0.5 * std::sqrt(hit1.sigma_z * hit1.sigma_z + hit2.sigma_z * hit2.sigma_z);  // Average of original uncertainties
            }
            
            track.plane1_point.valid = true;
        }
        
        // Process Plane 2 if we have hits
        if (pattern.plane2_hits > 0) {
            track.plane2_point.eventID = pattern.eventID;
            track.plane2_point.planeID = 2;
            track.plane2_point.num_hits_used = pattern.plane2_hits;
            track.has_plane2 = true;
            
            // Store bar IDs used
            for (const auto& hit : pattern.plane2_data) {
                track.plane2_point.barIDs_used.push_back(hit.barID);
            }
            
            if (pattern.plane2_hits == 1) {
                // Single hit: keep original reconstruction
                const auto& hit = pattern.plane2_data[0];
                track.plane2_point.x = hit.xRecoSmeared;
                track.plane2_point.y = -500.0;  // Fixed at -500mm
                track.plane2_point.z = hit.zRecoSmeared;
                
                track.plane2_point.sigma_x = hit.sigma_x;  // Original uncertainty
                track.plane2_point.sigma_y = 0.0;          // No uncertainty
                track.plane2_point.sigma_z = hit.sigma_z;  // Original uncertainty
                
            } else if (pattern.plane2_hits == 2) {
                // Two hits: take middle point
                const auto& hit1 = pattern.plane2_data[0];
                const auto& hit2 = pattern.plane2_data[1];
                
                track.plane2_point.x = 0.5 * (hit1.xRecoSmeared + hit2.xRecoSmeared);
                track.plane2_point.y = -500.0;  // Fixed at -500mm
                track.plane2_point.z = 0.5 * (hit1.zRecoSmeared + hit2.zRecoSmeared);
                
                track.plane2_point.sigma_x = 0.5 * std::sqrt(hit1.sigma_x * hit1.sigma_x + hit2.sigma_x * hit2.sigma_x);  // Average of original uncertainties
                track.plane2_point.sigma_y = 0.0;  // No uncertainty
                track.plane2_point.sigma_z = 4.6;  // Fixed uncertainty for 2-hit average
            }
            
            track.plane2_point.valid = true;
        }
        
        // Mark track as valid only if we have both planes
        if (track.has_plane1 && track.has_plane2) {
            track.valid_track = true;
            valid_events++;
            
            // Count statistics
            if (pattern.plane1_hits == 1 && pattern.plane2_hits == 1) events_1p1_1p2++;
            else if (pattern.plane1_hits == 1 && pattern.plane2_hits == 2) events_1p1_2p2++;
            else if (pattern.plane1_hits == 2 && pattern.plane2_hits == 1) events_2p1_1p2++;
            else if (pattern.plane1_hits == 2 && pattern.plane2_hits == 2) events_2p1_2p2++;
        }
        
        tracks.push_back(track);
    }
    
    // Print statistics
    std::cout << "Valid tracks created: " << valid_events << std::endl;
    std::cout << "Events discarded: " << discarded_events << std::endl;
    std::cout << "Success rate: " << 100.0 * valid_events / eventPatterns.size() << "%" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Track type breakdown:" << std::endl;
    std::cout << "  1P1-1P2: " << events_1p1_1p2 << " (" << 100.0 * events_1p1_1p2 / valid_events << "%)" << std::endl;
    std::cout << "  1P1-2P2: " << events_1p1_2p2 << " (" << 100.0 * events_1p1_2p2 / valid_events << "%)" << std::endl;
    std::cout << "  2P1-1P2: " << events_2p1_1p2 << " (" << 100.0 * events_2p1_1p2 / valid_events << "%)" << std::endl;
    std::cout << "  2P1-2P2: " << events_2p1_2p2 << " (" << 100.0 * events_2p1_2p2 / valid_events << "%)" << std::endl;
    std::cout << std::endl;
    
    
    return tracks;
}

void PlotTrackReconstruction(const std::vector<EventTrack>& tracks) {
    
    // Create histograms for track points
    TH1F *h_x_p1 = new TH1F("h_x_p1_track", "Plane 1;x (mm);Events", 60, -400, 400);
    TH1F *h_z_p1 = new TH1F("h_z_p1_track", "Plane 1;z (mm);Events", 100, -400, 400);
    TH1F *h_x_p2 = new TH1F("h_x_p2_track", "Plane 2;x (mm);Events", 100, -800, 800);
    TH1F *h_z_p2 = new TH1F("h_z_p2_track", "Plane 2;z (mm);Events", 100, -800, 800);
     
    // Create 2D track visualization
    TH2F *h_track_xz = new TH2F("h_track_xz", "Track Points;x (mm);z (mm)", 100, -800, 800, 100, -800, 800);
    
    SetHistogramStyle(h_x_p1, kBlue+1, kBlue+2, "x (mm)", "Number of Events");
    SetHistogramStyle(h_x_p2, kRed+1, kRed+2, "x (mm)", "Number of Events");
    SetHistogramStyle(h_z_p1, kGreen+1, kGreen+2, "z (mm)", "Number of Events");
    SetHistogramStyle(h_z_p2, kMagenta+1, kMagenta+2, "z (mm)", "Number of Events");

    // Set 2D histogram styling
    h_track_xz->GetXaxis()->SetTitleSize(0.045);
    h_track_xz->GetYaxis()->SetTitleSize(0.045);
    h_track_xz->GetXaxis()->SetLabelSize(0.04);
    h_track_xz->GetYaxis()->SetLabelSize(0.04);
    h_track_xz->GetXaxis()->SetTitleOffset(1.1);
    h_track_xz->GetYaxis()->SetTitleOffset(1.2);
    h_track_xz->SetStats(kTRUE);

    // Count valid tracks and fill histograms
    int valid_tracks = 0;
    int total_tracks = 0;
    for (const auto& track : tracks) {
        total_tracks++;
        
        if (!track.valid_track) continue;
        valid_tracks++;
        
        // Fill position histograms
        if (track.has_plane1) {
            h_x_p1->Fill(track.plane1_point.x);
            h_z_p1->Fill(track.plane1_point.z);
            h_track_xz->Fill(track.plane1_point.x, track.plane1_point.z);
        }
        
        if (track.has_plane2) {
            h_x_p2->Fill(track.plane2_point.x);
            h_z_p2->Fill(track.plane2_point.z);
            h_track_xz->Fill(track.plane2_point.x, track.plane2_point.z);
        }
    }

    TCanvas *cTrackX = new TCanvas("cTrackX", "Track X Coordinates", 1200, 600);
    cTrackX->Divide(2, 1);
    
    cTrackX->cd(1);
    SetCanvasStyle(gPad);
    h_x_p1->Draw("hist");

    cTrackX->cd(2);
    SetCanvasStyle(gPad);
    h_x_p2->Draw("hist");

    cTrackX->Update();
    gSystem->ProcessEvents();

    TCanvas *cTrackZ = new TCanvas("cTrackZ", "Track Z Coordinates", 1200, 600);
    cTrackZ->Divide(2, 1);
    
    cTrackZ->cd(1);
    SetCanvasStyle(gPad);
    h_z_p1->Draw("hist");
    
    cTrackZ->cd(2);
    SetCanvasStyle(gPad);
    h_z_p2->Draw("hist");
    
    cTrackZ->Update();
    gSystem->ProcessEvents();

    
    TCanvas *cTrack2D = new TCanvas("cTrack2D", "Track Points XZ View", 800, 600);
    SetCanvasStyle(cTrack2D);
    
    // Use a better color palette for 2D plot
    gStyle->SetPalette(kViridis);
    h_track_xz->Draw("COLZ");
    cTrack2D->Update();
    gSystem->ProcessEvents();
    
}

std::vector<PredictedImpact> PredictImpactPoints(const std::vector<EventTrack>& tracks, bool verbose = false) {
    std::cout << "\nPredicting impact points... " << std::endl;
    std::cout << "Processing " << tracks.size() << " tracks" << std::endl;
    
    std::vector<PredictedImpact> predictions;
    predictions.reserve(tracks.size());
    
    int valid_predictions = 0;
    int invalid_tracks = 0;
    
    for (const auto& track : tracks) {
        PredictedImpact prediction;
        prediction.eventID = track.eventID;
        
        // Only process valid tracks
        if (!track.valid_track) {
            invalid_tracks++;
            predictions.push_back(prediction); // Add invalid prediction
            continue;
        }
        
        // Extract coordinates from track points
        double x1 = track.plane1_point.x;    // Plane 1: y = 0
        double z1 = track.plane1_point.z;
        double sigma_x1 = track.plane1_point.sigma_x;
        double sigma_z1 = track.plane1_point.sigma_z;
        
        double x2 = track.plane2_point.x;    // Plane 2: y = -500
        double z2 = track.plane2_point.z;
        double sigma_x2 = track.plane2_point.sigma_x;
        double sigma_z2 = track.plane2_point.sigma_z;
        
        // Linear extrapolation to y = 500 mm
        // The line goes from (x1, 0, z1) to (x2, -500, z2)
        // We want to find (x3, 500, z3)
        
        // Using your formulas:
        prediction.x_pred = 2.0 * x1 - x2;
        prediction.y_pred = 500.0;  // Fixed target y-coordinate
        prediction.z_pred = 2.0 * z1 - z2;
        
        // Calculate uncertainties using error propagation
        prediction.sigma_x_pred = std::sqrt(4.0 * sigma_x1 * sigma_x1 + sigma_x2 * sigma_x2);
        prediction.sigma_y_pred = 0.0;  // No uncertainty in y (it's fixed)
        prediction.sigma_z_pred = std::sqrt(4.0 * sigma_z1 * sigma_z1 + sigma_z2 * sigma_z2);
        
        // Store reference to input points
        prediction.plane1_point = track.plane1_point;
        prediction.plane2_point = track.plane2_point;
        
        prediction.valid_prediction = true;
        valid_predictions++;
        
        predictions.push_back(prediction);
    }
    
    return predictions;
}

void PlotPredictedImpacts(const std::vector<PredictedImpact>& predictions) {
    std::cout << "\nPlotting predicted impact points... " << std::endl;

    // Create histograms for predicted coordinates
    TH1F *h_x_pred = new TH1F("h_x_pred", "y = 500 mm;x_{pred} (mm);Events", 100, -50, 50);
    TH1F *h_z_pred = new TH1F("h_z_pred", "y = 500 mm;z_{pred} (mm);Events", 100, -65, 65);

    // Create histograms for uncertainties
    TH1F *h_sigma_x_pred = new TH1F("h_sigma_x_pred", "y = 500 mm;#sigma_{x,pred} (mm);Events", 50, 10, 30);
    TH1F *h_sigma_z_pred = new TH1F("h_sigma_z_pred", "y = 500 mm;#sigma_{z,pred} (mm);Events", 50, 10, 30);

    // Create 2D impact map
    TH2F *h_impact_map = new TH2F("h_impact_map", "y = 500 mm; x_{pred} (mm);z_{pred} (mm)", 
                                  100, -1000, 1000, 100, -1000, 1000);
    
    // Create track visualization in XY and ZY projections
    TH2F *h_track_xy = new TH2F("h_track_xy", "Track Projection XY;x (mm);y (mm)", 100, -1000, 1000, 60, -600, 600);
    TH2F *h_track_zy = new TH2F("h_track_zy", "Track Projection ZY;z (mm);y (mm)", 100, -1000, 1000, 60, -600, 600);

    SetHistogramStyle(h_x_pred, kGreen+1, kGreen+2, "x_{pred} (mm)", "Number of Events");
    SetHistogramStyle(h_z_pred, kMagenta+1, kMagenta+2, "z_{pred} (mm)", "Number of Events");
    SetHistogramStyle(h_sigma_x_pred, kOrange+1, kOrange+2, "#sigma_{x,pred} (mm)", "Number of Events");
    SetHistogramStyle(h_sigma_z_pred, kCyan+1, kCyan+2, "#sigma_{z,pred} (mm)", "Number of Events");

    // Set 2D histogram styling
    auto set2DStyle = [](TH2F* hist, const std::string& xTitle, const std::string& yTitle) {
        hist->GetXaxis()->SetTitle(xTitle.c_str());
        hist->GetYaxis()->SetTitle(yTitle.c_str());
        hist->GetXaxis()->SetTitleSize(0.045);
        hist->GetYaxis()->SetTitleSize(0.045);
        hist->GetXaxis()->SetLabelSize(0.04);
        hist->GetYaxis()->SetLabelSize(0.04);
        hist->GetXaxis()->SetTitleOffset(1.1);
        hist->GetYaxis()->SetTitleOffset(1.2);
        hist->SetStats(kTRUE);
    };

    set2DStyle(h_impact_map, "x_{pred} (mm)", "z_{pred} (mm)");
    set2DStyle(h_track_xy, "x (mm)", "y (mm)");
    set2DStyle(h_track_zy, "z (mm)", "y (mm)");
    // Fill histograms
    int valid_count = 0;
    for (const auto& pred : predictions) {
        if (!pred.valid_prediction) continue;
        
        valid_count++;
        
        // Fill prediction histograms
        h_x_pred->Fill(pred.x_pred);
        h_z_pred->Fill(pred.z_pred);
        h_sigma_x_pred->Fill(pred.sigma_x_pred);
        h_sigma_z_pred->Fill(pred.sigma_z_pred);
        h_impact_map->Fill(pred.x_pred, pred.z_pred);
        
        // Fill track projections
        double x1 = pred.plane1_point.x;
        double z1 = pred.plane1_point.z;
        double x2 = pred.plane2_point.x;
        double z2 = pred.plane2_point.z;
        
        // Add track points
        h_track_xy->Fill(x1, 0);      // Plane 1
        h_track_xy->Fill(x2, -500);   // Plane 2
        h_track_xy->Fill(pred.x_pred, 500);  // Predicted impact
        
        h_track_zy->Fill(z1, 0);      // Plane 1
        h_track_zy->Fill(z2, -500);   // Plane 2
        h_track_zy->Fill(pred.z_pred, 500);  // Predicted impact
    }
    
    std::cout << "Filled histograms with " << valid_count << " valid predictions" << std::endl;
    
    
    // === Canvas 1: Predicted Positions ===
    TCanvas *cPredictedPositions = new TCanvas("cPredictedPositions", "Predicted Impact Positions", 1200, 500);
    cPredictedPositions->Divide(2, 1);
    cPredictedPositions->cd(1); 
    SetCanvasStyle(gPad);
    h_x_pred->Draw("hist");
    
    cPredictedPositions->cd(2); 
    SetCanvasStyle(gPad);
    h_z_pred->Draw("hist");

    
    cPredictedPositions->Update();
    gSystem->ProcessEvents();
    
    // === Canvas 2: Prediction Uncertainties ===
    TCanvas *cPredictedUncertainties = new TCanvas("cPredictedUncertainties", "Prediction Uncertainties", 1200, 500);
    cPredictedUncertainties->Divide(2, 1);
    cPredictedUncertainties->cd(1); 
    SetCanvasStyle(gPad);
    h_sigma_x_pred->Draw("hist");

    
    cPredictedUncertainties->cd(2); 
    SetCanvasStyle(gPad);
    h_sigma_z_pred->Draw("hist");

    
    cPredictedUncertainties->Update();
    gSystem->ProcessEvents();
    
    TCanvas *cImpactMap = new TCanvas("cImpactMap", "Impact Map at y=500mm", 800, 600);
    SetCanvasStyle(cImpactMap);
    gStyle->SetPalette(kViridis);
    h_impact_map->Draw("COLZ");
    cImpactMap->Update();
    gSystem->ProcessEvents();
    
    TCanvas *cTrackProjections = new TCanvas("cTrackProjections", "Track Projections", 1200, 500);
    cTrackProjections->Divide(2, 1);
    cTrackProjections->cd(1); 
    SetCanvasStyle(gPad);
    gStyle->SetPalette(kViridis);
    h_track_xy->Draw("COLZ");
    h_track_xy->SetTitle("Track Projection XY;x(mm);y (mm)");
    
    cTrackProjections->cd(2); 
    SetCanvasStyle(gPad);
    gStyle->SetPalette(kViridis);
    h_track_zy->Draw("COLZ");
    h_track_zy->SetTitle("Track Projection ZY;z (mm);y (mm)");
    
    cTrackProjections->Update();
    gSystem->ProcessEvents();
    
    // Print some statistics
    std::cout << "Impact prediction results:" << std::endl;
    std::cout << "Predicted x - Mean: " << h_x_pred->GetMean() << " ± " << h_x_pred->GetRMS() << " mm" << std::endl;
    std::cout << "Predicted z - Mean: " << h_z_pred->GetMean() << " ± " << h_z_pred->GetRMS() << " mm" << std::endl;
    std::cout << "Average x uncertainty: " << h_sigma_x_pred->GetMean() << " mm" << std::endl;
    std::cout << "Average z uncertainty: " << h_sigma_z_pred->GetMean() << " mm" << std::endl;
    std::cout << std::endl;
}

void AnalyzePredictionQuality(const std::vector<PredictedImpact>& predictions) {
    std::cout << "\nPrediction quality analysis" << std::endl;

    // Separate predictions by track type
    std::vector<double> x_pred_1p1_1p2, z_pred_1p1_1p2, sigma_x_1p1_1p2, sigma_z_1p1_1p2;
    std::vector<double> x_pred_mixed, z_pred_mixed, sigma_x_mixed, sigma_z_mixed;
    std::vector<double> x_pred_2p1_2p2, z_pred_2p1_2p2, sigma_x_2p1_2p2, sigma_z_2p1_2p2;
    
    for (const auto& pred : predictions) {
        if (!pred.valid_prediction) continue;
        
        int p1_hits = pred.plane1_point.num_hits_used;
        int p2_hits = pred.plane2_point.num_hits_used;
        
        if (p1_hits == 1 && p2_hits == 1) {
            x_pred_1p1_1p2.push_back(pred.x_pred);
            z_pred_1p1_1p2.push_back(pred.z_pred);
            sigma_x_1p1_1p2.push_back(pred.sigma_x_pred);
            sigma_z_1p1_1p2.push_back(pred.sigma_z_pred);
        } else if (p1_hits == 2 && p2_hits == 2) {
            x_pred_2p1_2p2.push_back(pred.x_pred);
            z_pred_2p1_2p2.push_back(pred.z_pred);
            sigma_x_2p1_2p2.push_back(pred.sigma_x_pred);
            sigma_z_2p1_2p2.push_back(pred.sigma_z_pred);
        } else {
            x_pred_mixed.push_back(pred.x_pred);
            z_pred_mixed.push_back(pred.z_pred);
            sigma_x_mixed.push_back(pred.sigma_x_pred);
            sigma_z_mixed.push_back(pred.sigma_z_pred);
        }
    }
    
    auto printStats = [](const std::string& category, const std::vector<double>& sigma_x, const std::vector<double>& sigma_z) {
        if (sigma_x.empty()) return;
        
        double mean_sigma_x = 0, mean_sigma_z = 0;
        for (size_t i = 0; i < sigma_x.size(); i++) {
            mean_sigma_x += sigma_x[i];
            mean_sigma_z += sigma_z[i];
        }
        mean_sigma_x /= sigma_x.size();
        mean_sigma_z /= sigma_z.size();
        
        std::cout << category << " (" << sigma_x.size() << " events):" << std::endl;
        std::cout << "  Average σ_X: " << mean_sigma_x << " mm" << std::endl;
        std::cout << "  Average σ_Z: " << mean_sigma_z << " mm" << std::endl;
        std::cout << std::endl;
    };
    
    printStats("1P1-1P2 tracks", sigma_x_1p1_1p2, sigma_z_1p1_1p2);
    printStats("Mixed tracks (1P1-2P2, 2P1-1P2)", sigma_x_mixed, sigma_z_mixed);
    printStats("2P1-2P2 tracks", sigma_x_2p1_2p2, sigma_z_2p1_2p2);
}

ImpactFitResults FitPredictedImpacts(const std::vector<PredictedImpact>& predictions, 
                                   double timeSmearSigma_ns, bool verbose = false, bool showPlots = false) {
    
    ImpactFitResults results;
    results.timeSmearSigma = timeSmearSigma_ns;
    
    if (predictions.empty()) {
        if (verbose) std::cerr << "Error: No predictions to fit!" << std::endl;
        return results;
    }
    
    // Count valid predictions and collect data for statistical analysis
    std::vector<double> x_values, z_values;
    for (const auto& pred : predictions) {
        if (pred.valid_prediction) {
            x_values.push_back(pred.x_pred);
            z_values.push_back(pred.z_pred);
        }
    }
    
    results.n_valid_predictions = x_values.size();
    
    if (results.n_valid_predictions < 10) {
        if (verbose) std::cout << "Warning: Too few valid predictions for fitting (" << results.n_valid_predictions << ")" << std::endl;
        return results;
    }
    
    if (verbose) {
        std::cout << "\nFitting impact distributions" << std::endl;
        std::cout << "Time smearing sigma: " << timeSmearSigma_ns << " ns" << std::endl;
        std::cout << "Valid predictions: " << results.n_valid_predictions << std::endl;
    }
    
    // === Calculate statistical means and uncertainties ===
    auto calculateStatistics = [](const std::vector<double>& values) -> std::pair<double, double> {
        if (values.empty()) return {0.0, 0.0};
        
        // Calculate sample mean: m = (1/n) * Σ(y_j)
        double mean = 0.0;
        for (double val : values) {
            mean += val;
        }
        mean /= values.size();
        
        // Calculate sample standard deviation: s = sqrt(Σ(y_j - m)^2 / (n-1))
        double variance = 0.0;
        for (double val : values) {
            variance += (val - mean) * (val - mean);
        }
        double sample_std = std::sqrt(variance / (values.size() - 1));
        
        // Standard deviation of the mean: S = s / sqrt(n)
        double mean_uncertainty = sample_std / std::sqrt(values.size());
        
        return {mean, mean_uncertainty};
    };
    
    auto [stat_mean_x, stat_error_x] = calculateStatistics(x_values);
    auto [stat_mean_z, stat_error_z] = calculateStatistics(z_values);
    
    results.statistical_mean_x = stat_mean_x;
    results.statistical_error_x = stat_error_x;
    results.statistical_mean_z = stat_mean_z;
    results.statistical_error_z = stat_error_z;
    
    // === Create histograms for fitting ===
    
    // Determine appropriate ranges for fitting
    auto getRange = [](const std::vector<double>& values) -> std::pair<double, double> {
        if (values.empty()) return {-100, 100};
        
        double mean = 0, rms = 0;
        for (double val : values) mean += val;
        mean /= values.size();
        
        for (double val : values) rms += (val - mean) * (val - mean);
        rms = std::sqrt(rms / values.size());
        
        double range = 4.0 * rms; // 4-sigma range
        return {mean - range, mean + range};
    };
    
    auto [x_min, x_max] = getRange(x_values);
    auto [z_min, z_max] = getRange(z_values);

    std::string x_hist_name = Form("h_x_fit_sigma%.3f", timeSmearSigma_ns);
    std::string z_hist_name = Form("h_z_fit_sigma%.3f", timeSmearSigma_ns);
    
    
    TH1F *h_x_fit = new TH1F(x_hist_name.c_str(), Form("#sigma = %.1f ns;x_{pred} (mm);Events", timeSmearSigma_ns), 
                             100, x_min, x_max);
    TH1F *h_z_fit = new TH1F(z_hist_name.c_str(), Form("#sigma = %.1f ns;z_{pred} (mm);Events", timeSmearSigma_ns), 
                             100, z_min, z_max);
    
    // Fill histograms
    for (const auto& pred : predictions) {
        if (pred.valid_prediction) {
            h_x_fit->Fill(pred.x_pred);
            h_z_fit->Fill(pred.z_pred);
        }
    }
    
    // Set histogram styles
    if (showPlots) {
        h_x_fit->SetFillColorAlpha(kBlue, 0.3);
        h_x_fit->SetLineColor(kBlue+2);
        h_x_fit->SetLineWidth(2);
        h_z_fit->SetFillColorAlpha(kRed, 0.3);
        h_z_fit->SetLineColor(kRed+2);
        h_z_fit->SetLineWidth(2);
    }
    
    // === Function to fit Gaussian ===
    auto fitGaussian = [verbose, timeSmearSigma_ns](TH1F* hist, const std::string& name) -> std::tuple<double, double, double, double, double> {
        if (hist->GetEntries() < 10) {
            if (verbose) std::cout << "Warning: " << name << " has too few entries for fitting" << std::endl;
            return {0.0, 0.0, 0.0, 0.0, 0.0};
        }
        
        // Initial parameter estimates
        double hist_mean = hist->GetMean();
        double hist_rms = hist->GetRMS();
        double hist_max = hist->GetMaximum();
        
        // Define fit range
        double fit_min = hist_mean - 3.0 * hist_rms;
        double fit_max = hist_mean + 3.0 * hist_rms;
        
        // Create Gaussian function
        std::string func_name = Form("gaus_%s_sigma%.3f", name.c_str(), timeSmearSigma_ns);
        TF1 *gaus = new TF1(func_name.c_str(), "gaus", fit_min, fit_max);
        gaus->SetParameters(hist_max, hist_mean, hist_rms);
        gaus->SetParLimits(1, fit_min, fit_max); // Constrain mean
        gaus->SetParLimits(2, 0.1 * hist_rms, 10.0 * hist_rms); // Constrain sigma
        
        // Set fit function style
        gaus->SetLineColor(kRed);
        gaus->SetLineWidth(3);
        gaus->SetLineStyle(1);
        
        // Perform fit
        TFitResultPtr fitResult = hist->Fit(gaus, "RQSE"); // R=range, Q=quiet, S=return result, E=improved errors
        
        double fitted_mean = gaus->GetParameter(1);
        double fitted_mean_err = gaus->GetParError(1);
        double fitted_sigma = gaus->GetParameter(2);
        double fitted_sigma_err = gaus->GetParError(2);
        double chi2_ndf = (gaus->GetNDF() > 0) ? gaus->GetChisquare() / gaus->GetNDF() : 0.0;
        
        if (verbose) {
            std::cout << "\n" << name << " Fit Results:" << std::endl;
            std::cout << "  Entries:          " << hist->GetEntries() << std::endl;
            std::cout << "  Histogram Mean:   " << hist->GetMean() << " ± " << hist->GetMeanError() << " mm" << std::endl;
            std::cout << "  Histogram RMS:    " << hist->GetRMS() << " ± " << hist->GetRMSError() << " mm" << std::endl;
            std::cout << "  Fitted Mean:      " << fitted_mean << " ± " << fitted_mean_err << " mm" << std::endl;
            std::cout << "  Fitted Sigma:     " << fitted_sigma << " ± " << fitted_sigma_err << " mm" << std::endl;
            std::cout << "  Chi²/NDF:         " << chi2_ndf << std::endl;
            std::cout << "  Fit Range:        [" << fit_min << ", " << fit_max << "] mm" << std::endl;
        }
        
        return {fitted_mean, fitted_sigma, fitted_sigma_err, chi2_ndf, fitted_mean_err};
    };
    
    // Perform fits
    auto [mean_x, sigma_x, sigma_x_err, chi2_x, mean_x_err] = fitGaussian(h_x_fit, "X_Prediction");
    auto [mean_z, sigma_z, sigma_z_err, chi2_z, mean_z_err] = fitGaussian(h_z_fit, "Z_Prediction");
    
    // Store results
    results.mean_x_pred = mean_x;
    results.sigma_x_pred = sigma_x;
    results.sigma_x_pred_err = sigma_x_err;
    results.chi2_x_ndf = chi2_x;
    
    results.mean_z_pred = mean_z;
    results.sigma_z_pred = sigma_z;
    results.sigma_z_pred_err = sigma_z_err;
    results.chi2_z_ndf = chi2_z;
    
    // Create plots if requested
    if (showPlots) {
        std::string canvas_name = Form("cImpactFits_sigma%.3f", timeSmearSigma_ns);
        TCanvas *cImpactFits = new TCanvas(canvas_name.c_str(), 
                                          Form("Impact Distribution Fits (#sigma=%.1f ns)", timeSmearSigma_ns), 
                                          1200, 500);
        cImpactFits->SetBit(kCanDelete, kFALSE); // Prevent automatic deletion
        cImpactFits->Clear(); // Clear any existing content
        cImpactFits->Divide(2, 1);
        
        cImpactFits->cd(1);
        SetCanvasStyle(gPad);
        gPad->Clear();
        h_x_fit->Draw("hist");

        std::string func_x_name = Form("gaus_X_Prediction_sigma%.3f", timeSmearSigma_ns);
        TF1 *func_x = h_x_fit->GetFunction(func_x_name.c_str());
        if (func_x) {
            func_x->SetLineColor(kRed);
            func_x->SetLineWidth(3);
            func_x->Draw("same");
        }
        
        cImpactFits->cd(2);
        SetCanvasStyle(gPad);
        gPad->Clear();
        h_z_fit->Draw("hist");

        std::string func_z_name = Form("gaus_Z_Prediction_sigma%.3f", timeSmearSigma_ns);
        TF1 *func_z = h_z_fit->GetFunction(func_z_name.c_str());
        if (func_z) {
            func_z->SetLineColor(kRed);
            func_z->SetLineWidth(3);
            func_z->Draw("same");
        }
        
        cImpactFits->Update();
        cImpactFits->Modified();
        gSystem->ProcessEvents();
    }
    
    if (verbose) {
        std::cout << "\n Statistical and fitted results: " << std::endl;
        std::cout << "X Coordinate:" << std::endl;
        std::cout << "  Statistical Mean: " << stat_mean_x << " ± " << stat_error_x << " mm" << std::endl;
        std::cout << "  Fitted Mean:      " << mean_x << " ± " << mean_x_err << " mm" << std::endl;
        std::cout << "Z Coordinate:" << std::endl;
        std::cout << "  Statistical Mean: " << stat_mean_z << " ± " << stat_error_z << " mm" << std::endl;
        std::cout << "  Fitted Mean:      " << mean_z << " ± " << mean_z_err << " mm" << std::endl;
        std::cout << std::endl;
    }
    
    // Clean up histograms if not plotting
    if (!showPlots) {
        delete h_x_fit;
        delete h_z_fit;
    }
    
    return results;
}

void PlotImpactResolutionStudy(const std::vector<ImpactFitResults>& impactResults) {
    if (impactResults.empty()) {
        std::cerr << "Error: No impact results to plot!" << std::endl;
        return;
    }

    std::cout << "\nPlotting impact resolution study..." << std::endl;
    std::cout << "Processing " << impactResults.size() << " data points" << std::endl;
    
    // Force ROOT to not use any existing canvas
    gROOT->SetBatch(kFALSE);

    // Extract data for plotting
    int n = impactResults.size();
    double* timeSmear = new double[n];
    double* timeSmear_err = new double[n];
    
    double* sigma_x = new double[n];
    double* sigma_x_err = new double[n];
    double* sigma_z = new double[n];
    double* sigma_z_err = new double[n];

    double* stat_error_x = new double[n];
    double* stat_error_z = new double[n];
    
    double* mean_x_stat = new double[n];
    double* mean_x_stat_err = new double[n];
    double* mean_z_stat = new double[n];
    double* mean_z_stat_err = new double[n];
    
    double* mean_x_fit = new double[n];
    double* mean_x_fit_err = new double[n];
    double* mean_z_fit = new double[n];
    double* mean_z_fit_err = new double[n];
    
    for (int i = 0; i < n; i++) {
        timeSmear[i] = impactResults[i].timeSmearSigma;
        timeSmear_err[i] = 0.0; // No error on input parameter
        
        sigma_x[i] = impactResults[i].sigma_x_pred;
        sigma_x_err[i] = impactResults[i].sigma_x_pred_err;
        sigma_z[i] = impactResults[i].sigma_z_pred;
        sigma_z_err[i] = impactResults[i].sigma_z_pred_err;

        // DEBUG: Print the values to see what's happening
        std::cout << "Point " << i << ": σ_t=" << timeSmear[i] 
                  << ", σ_x=" << sigma_x[i] << "±" << sigma_x_err[i]
                  << ", σ_z=" << sigma_z[i] << "±" << sigma_z_err[i]
                  << ", N_predictions=" << impactResults[i].n_valid_predictions << std::endl;
        
        // Check for zero or invalid values
        if (sigma_x[i] <= 0) {
            std::cout << "  WARNING: σ_x is zero or negative!" << std::endl;
        }
        if (sigma_z[i] <= 0) {
            std::cout << "  WARNING: σ_z is zero or negative!" << std::endl;
        }

        stat_error_x[i] = impactResults[i].statistical_error_x;
        stat_error_z[i] = impactResults[i].statistical_error_z;
        
        mean_x_stat[i] = impactResults[i].statistical_mean_x;
        mean_x_stat_err[i] = impactResults[i].statistical_error_x;
        mean_z_stat[i] = impactResults[i].statistical_mean_z;
        mean_z_stat_err[i] = impactResults[i].statistical_error_z;
        
        mean_x_fit[i] = impactResults[i].mean_x_pred;
        mean_x_fit_err[i] = impactResults[i].sigma_x_pred; // Using sigma error as mean error
        mean_z_fit[i] = impactResults[i].mean_z_pred;
        mean_z_fit_err[i] = impactResults[i].sigma_z_pred;
    }
    
    // Create TGraphErrors objects
    TGraphErrors *gr_sigma_x = new TGraphErrors(n, timeSmear, sigma_x, timeSmear_err, sigma_x_err);
    TGraphErrors *gr_sigma_z = new TGraphErrors(n, timeSmear, sigma_z, timeSmear_err, sigma_z_err);

    TGraphErrors *gr_stat_error_x = new TGraphErrors(n, timeSmear, stat_error_x, timeSmear_err, nullptr);
    TGraphErrors *gr_stat_error_z = new TGraphErrors(n, timeSmear, stat_error_z, timeSmear_err, nullptr);

    
    TGraphErrors *gr_mean_x_stat = new TGraphErrors(n, timeSmear, mean_x_stat, timeSmear_err, mean_x_stat_err);
    TGraphErrors *gr_mean_z_stat = new TGraphErrors(n, timeSmear, mean_z_stat, timeSmear_err, mean_z_stat_err);
    
    TGraphErrors *gr_mean_x_fit = new TGraphErrors(n, timeSmear, mean_x_fit, timeSmear_err, mean_x_fit_err);
    TGraphErrors *gr_mean_z_fit = new TGraphErrors(n, timeSmear, mean_z_fit, timeSmear_err, mean_z_fit_err);
    
    // Style the sigma graphs
    gr_sigma_x->SetMarkerStyle(20);
    gr_sigma_x->SetMarkerColor(kBlue);
    gr_sigma_x->SetLineColor(kBlue);
    gr_sigma_x->SetMarkerSize(1.2);
    
    gr_sigma_z->SetMarkerStyle(21);
    gr_sigma_z->SetMarkerColor(kRed);
    gr_sigma_z->SetLineColor(kRed);
    gr_sigma_z->SetMarkerSize(1.2);

    gr_stat_error_x->SetMarkerStyle(24);
    gr_stat_error_x->SetMarkerColor(kGreen+2);
    gr_stat_error_x->SetLineColor(kGreen+2);
    gr_stat_error_x->SetMarkerSize(1.0);
    
    gr_stat_error_z->SetMarkerStyle(25);
    gr_stat_error_z->SetMarkerColor(kOrange+2);
    gr_stat_error_z->SetLineColor(kOrange+2);
    gr_stat_error_z->SetMarkerSize(1.0);
    
    // Style the mean graphs
    gr_mean_x_stat->SetMarkerStyle(24);
    gr_mean_x_stat->SetMarkerColor(kGreen+2);
    gr_mean_x_stat->SetLineColor(kGreen+2);
    gr_mean_x_stat->SetMarkerSize(1.0);
    
    gr_mean_z_stat->SetMarkerStyle(25);
    gr_mean_z_stat->SetMarkerColor(kMagenta+2);
    gr_mean_z_stat->SetLineColor(kMagenta+2);
    gr_mean_z_stat->SetMarkerSize(1.0);
    
    gr_mean_x_fit->SetMarkerStyle(20);
    gr_mean_x_fit->SetMarkerColor(kGreen+2);
    gr_mean_x_fit->SetLineColor(kGreen+2);
    gr_mean_x_fit->SetMarkerSize(0.8);
    
    gr_mean_z_fit->SetMarkerStyle(21);
    gr_mean_z_fit->SetMarkerColor(kMagenta+2);
    gr_mean_z_fit->SetLineColor(kMagenta+2);
    gr_mean_z_fit->SetMarkerSize(0.8);

    // Find X-axis range for horizontal line
    double x_min = *std::min_element(timeSmear, timeSmear + n);
    double x_max = *std::max_element(timeSmear, timeSmear + n);
    // Create horizontal line at 22 mm
    TLine *horizontal_line = new TLine(x_min, 22.0, x_max+0.03, 22.0);
    horizontal_line->SetLineColor(kOrange);
    horizontal_line->SetLineWidth(3);
    horizontal_line->SetLineStyle(2); // Dashed line style
    
    // === Canvas 1: Resolution vs Time Smearing ===
    TCanvas *cResolution = new TCanvas("cImpactResolution", "Impact Resolution vs Time Smearing", 1200, 600);
    cResolution->cd(); // Make sure this canvas is active
    SetCanvasStyle(cResolution);
    cResolution->Clear(); // Clear any previous content

    cResolution->SetGridx(1);
    cResolution->SetGridy(1);
    gStyle->SetGridColor(kGray);
    gStyle->SetGridStyle(1);
    gStyle->SetGridWidth(1);

    gr_sigma_x->SetTitle("Impact Resolution vs Time Resolution; #sigma_{t} (ns); #sigma_{spatial} (mm)");
    gr_sigma_x->Draw("APE");
    gr_sigma_x->GetXaxis()->SetTitleSize(0.045);
    gr_sigma_x->GetYaxis()->SetTitleSize(0.045);
    gr_sigma_x->GetXaxis()->SetLabelSize(0.04);
    gr_sigma_x->GetYaxis()->SetLabelSize(0.04);
    
    gr_sigma_z->Draw("PE same");

    horizontal_line->Draw("same"); // Draw horizontal line at 22 mm

    
    // Add legend
    TLegend *legRes = new TLegend(0.15, 0.75, 0.50, 0.88);
    legRes->AddEntry(gr_sigma_x, "#sigma_{x}", "pe");
    legRes->AddEntry(gr_sigma_z, "#sigma_{z}", "pe");
    legRes->SetTextSize(0.035);
    legRes->SetBorderSize(1);
    legRes->SetFillColor(kWhite);
    legRes->Draw();
    
    cResolution->Update();
    cResolution->Modified();
    gSystem->ProcessEvents();
    
    // === Canvas 2: Mean Values vs Time Smearing ===
    TCanvas *cMeans = new TCanvas("cImpactMeans", "Impact Mean Values vs Time Resolution", 1200, 800);
    cMeans->cd(); // Make sure this canvas is active
    cMeans->Clear(); // Clear any previous content
    cMeans->Divide(1, 2);
    
    // X means
    cMeans->cd(1);
    SetCanvasStyle(gPad);
    gPad->Clear(); // Clear the pad
    gr_mean_x_stat->SetTitle("X Mean Values vs Time Resolution; #sigma_{t} (ns); x_{mean} (mm)");
    gr_mean_x_stat->Draw("APE");
    gr_mean_x_stat->GetXaxis()->SetTitleSize(0.045);
    gr_mean_x_stat->GetYaxis()->SetTitleSize(0.045);
    gr_mean_x_fit->Draw("PE same");
    
    TLegend *legX = new TLegend(0.15, 0.75, 0.55, 0.88);
    legX->AddEntry(gr_mean_x_stat, "Statistical Mean", "pe");
    legX->AddEntry(gr_mean_x_fit, "Fitted Mean", "pe");
    legX->SetTextSize(0.035);
    legX->SetBorderSize(1);
    legX->SetFillColor(kWhite);
    legX->Draw();
    
    // Z means
    cMeans->cd(2);
    SetCanvasStyle(gPad);
    gr_mean_z_stat->SetTitle("Z Mean Values vs Time Resolution;#sigma_{t} (ns); z_{mean} (mm)");
    gr_mean_z_stat->Draw("APE");
    gr_mean_z_stat->GetXaxis()->SetTitleSize(0.045);
    gr_mean_z_stat->GetYaxis()->SetTitleSize(0.045);
    gr_mean_z_fit->Draw("PE same");
    
    TLegend *legZ = new TLegend(0.15, 0.75, 0.55, 0.88);
    legZ->AddEntry(gr_mean_z_stat, "Statistical Mean", "pe");
    legZ->AddEntry(gr_mean_z_fit, "Fitted Mean", "pe");
    legZ->SetTextSize(0.035);
    legZ->SetBorderSize(1);
    legZ->SetFillColor(kWhite);
    legZ->Draw();
    
    cMeans->Update();
    gSystem->ProcessEvents();
    
    // === Print numerical results ===
    std::cout << "Impact resolution study results:" << std::endl;

    
    std::cout << std::setw(10) << "σ_time" 
              << std::setw(15) << "σ_X (mm)" 
              << std::setw(15) << "σ_Z (mm)" 
              << std::setw(15) << "Mean_X (mm)" 
              << std::setw(15) << "Mean_Z (mm)" 
              << std::setw(10) << "N_events" << std::endl;
    std::cout << std::string(85, '-') << std::endl;
    
    for (int i = 0; i < n; i++) {
        std::cout << std::setw(10) << std::fixed << std::setprecision(2) << timeSmear[i]
                  << std::setw(15) << std::setprecision(2) << sigma_x[i] << "±" << sigma_x_err[i]
                  << std::setw(15) << std::setprecision(2) << sigma_z[i] << "±" << sigma_z_err[i]
                  << std::setw(15) << std::setprecision(3) << mean_x_stat[i] << "±" << mean_x_stat_err[i]
                  << std::setw(15) << std::setprecision(3) << mean_z_stat[i] << "±" << mean_z_stat_err[i]
                  << std::setw(10) << impactResults[i].n_valid_predictions << std::endl;
    }
    
    // Clean up arrays
    delete[] timeSmear; delete[] timeSmear_err;
    delete[] sigma_x; delete[] sigma_x_err;
    delete[] sigma_z; delete[] sigma_z_err;
    delete[] mean_x_stat; delete[] mean_x_stat_err;
    delete[] mean_z_stat; delete[] mean_z_stat_err;
    delete[] mean_x_fit; delete[] mean_x_fit_err;
    delete[] mean_z_fit; delete[] mean_z_fit_err;

}

void analysis_bars(){

    BarData bars;
    if (LoadBarHitsData("data/all_data_with_qpu_complete.root", bars)) {
        std::cout << "Loaded " << bars.eventID.size() << " entries" << std::endl;
    }

    bool muonsOnly = true;

    PlotParticleHistogram(bars);
    PlotMeanGlobalPositions(bars, muonsOnly);

    PlotColormaps(bars, muonsOnly);

    PlotTimeDistributions(bars, muonsOnly);

    // Load geometry
    auto geometry = read_geometry_vector("data/geometry.csv");
    if (geometry.empty()) {
        std::cerr << "Error: No geometry data loaded!" << std::endl;
        return;
    }

    std::vector<RecoData> reco_results;  // ← Fix: use vector to store results
    bool earliestImpact = true;

    std::vector<double> timeSmearSigmas = {0.001, 0.01,0.1, 0.2, 0.3, 0.5};
    // std::vector<FitResults> vectorResults;

    // for (const auto& timeSmearSigma_ns : timeSmearSigmas) {
    //     ProcessData(bars, reco_results, geometry, timeSmearSigma_ns, muonsOnly, earliestImpact);
    //     // PlotRecoResults(reco_results, timeSmearSigma_ns);
    //     FitResults results = FitGaussianAndExtractSigma(reco_results, timeSmearSigma_ns, true, true);

    //     // Store the results in the vectorResults vector
    //     vectorResults.push_back(results);
    // }

    // PlotTimeSmearingResults(vectorResults);

    double timeSmearSigma_ns = 0.1;
    ProcessData(bars, reco_results, geometry, timeSmearSigma_ns, muonsOnly, earliestImpact);

    PlotRecoResults(reco_results, timeSmearSigma_ns);
    
    std::vector<EventHitPattern> eventPatterns = AnalyzeEventHitPatterns(reco_results, true);

    HitPatternStatistics stats = CalculateHitPatternStatistics(eventPatterns);
    PlotHitPatternStatistics(stats);

    
    AnalyzeDetailedHitPatterns(eventPatterns);
    PlotBarIDDistribution(eventPatterns);


    std::cout << "Starting track reconstruction... " << std::endl;

    std::vector<EventTrack> tracks = ReconstructEventTracks(eventPatterns, true);
    PlotTrackReconstruction(tracks);


    std::cout << "Starting impact point prediction... " << std::endl;


    std::vector<PredictedImpact> predictions = PredictImpactPoints(tracks, true);

    PlotPredictedImpacts(predictions);
    AnalyzePredictionQuality(predictions);

    std::cout << "Starting impact resolution study... " << std::endl;
    std::vector<ImpactFitResults> impactResults;

    for (const auto& timeSmearSigma_ns : timeSmearSigmas) {
        std::cout << "\n--- Processing σ = " << timeSmearSigma_ns << " ns ---" << std::endl;
        
        // Process data with current time smearing
        ProcessData(bars, reco_results, geometry, timeSmearSigma_ns, muonsOnly, earliestImpact);
        
        // Analyze hit patterns and reconstruct tracks
        std::vector<EventHitPattern> eventPatterns = AnalyzeEventHitPatterns(reco_results, false); // verbose=false for clean output
        std::vector<EventTrack> tracks = ReconstructEventTracks(eventPatterns, false);
        std::vector<PredictedImpact> predictions = PredictImpactPoints(tracks, false);
        
        // Fit the impact distributions
        bool showPlots = (timeSmearSigma_ns == 0.1); // Only show plots for one example
        ImpactFitResults fitResults = FitPredictedImpacts(predictions, timeSmearSigma_ns, true, showPlots);
        
        impactResults.push_back(fitResults);
    }

    // Plot the resolution study results
    PlotImpactResolutionStudy(impactResults);



}
