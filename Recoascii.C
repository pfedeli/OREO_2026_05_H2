#include "Recoascii.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <array>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"

void Recoascii(TString spill_number)
{

    Configdaq daqdata = loaddaq("Recoascii.cfg");
    Linepars linedata = loadlinepars("Recoascii.cfg");
    // oreopars oreodata = loadoreopars("Recoascii.cfg");

    TString ascii_file = daqdata.ascii_path + "run" + spill_number + ".dat";
    TString root_file = daqdata.root_path + "run" + spill_number + ".root";
    // TString reco_root_file = daqdata.reco_root_path + "reco" + spill_number + ".root";
    //  std::cout << "CERCO IL FILE IN: " << root_file << std::endl;
    std::ifstream in(ascii_file);
    if (!in.is_open())
    {
        std::cout << "ascii " << ascii_file << " not found!" << std::endl;
        return;
    };

    TFile *f = new TFile(root_file, "RECREATE");
    TTree *t = new TTree("t", "t");

    std::vector<Double_t> xpos(daqdata.nTele);
    std::vector<Int_t> nstrip(daqdata.nTele);
    std::vector<Int_t> nclu(daqdata.nTele);
    std::vector<Long64_t> digiBase(daqdata.nDigi);
    std::vector<Long64_t> digiTime(daqdata.nDigi);
    std::vector<Long64_t> digiPH(daqdata.nDigi);
    std::vector<Double_t> gonioInfo(daqdata.nGonio);
    Int_t spillnumber;
    Int_t motorstepnumber;
    Int_t eventnumber;

    // reco variab/les
    Recodata event(daqdata.nDigi);

    t->Branch("xpos", xpos.data(), Form("xpos[%d]/D", daqdata.nTele));
    if (daqdata.nStrip_flag)
        t->Branch("xstrip", nstrip.data(), Form("xstrip[%d]/I", daqdata.nTele));
    if (daqdata.nClu_flag)
        t->Branch("nclu", nclu.data(), Form("nclu[%d]/I", daqdata.nTele));
    t->Branch("digiBase", digiBase.data(), Form("digiBase[%d]/L", daqdata.nDigi));
    t->Branch("digiPH", digiPH.data(), Form("digiPH[%d]/L", daqdata.nDigi));
    t->Branch("digiTime", digiTime.data(), Form("digiTime[%d]/L", daqdata.nDigi));
    t->Branch("gonioInfo", gonioInfo.data(), Form("gonioInfo[%d]/D", daqdata.nGonio));
    if (daqdata.Spillnumber_flag)
        t->Branch("spillnumber", &spillnumber, "spillnumber/I");
    if (daqdata.Motorstepnumber_flag)
        t->Branch("motorstepnumber", &motorstepnumber, "motorstepnumber/I");
    if (daqdata.eventnumber_flag)
        t->Branch("eventnumber", &eventnumber, "eventnumber/I");
    t->Branch("tele1x", &event.tele1x, "tele1x/D");
    t->Branch("tele1y", &event.tele1y, "tele1y/D");
    t->Branch("tele2x", &event.tele2x, "tele2x/D");
    t->Branch("tele2y", &event.tele2y, "tele2y/D");
    t->Branch("oreox", &event.oreox, "oreox/D");
    t->Branch("oreoy", &event.oreoy, "oreoy/D");
    t->Branch("thetax_in", &event.thetax_in, "thetax_in/D");
    t->Branch("thetay_in", &event.thetay_in, "thetay_in/D");
    t->Branch("enecry", event.enecry.data(), Form("enecry[%d]/L", daqdata.nDigi));
    t->Branch("eneup", &event.eneup, "eneup/D");
    t->Branch("enedown", &event.enedown, "enedown/D");
    t->Branch("enetot", &event.enetot, "enetot/D");
    t->Branch("enelg", &event.enelg, "enelg/D");

    std::string line;
    while (std::getline(in, line))
    {

        std::istringstream ss(line);

        for (int i = 0; i < daqdata.nTele; ++i)
            ss >> xpos[i];
        if (daqdata.nStrip_flag)
            for (int i = 0; i < daqdata.nTele; ++i)
                ss >> nstrip[i];
        if (daqdata.nClu_flag)
            for (int i = 0; i < daqdata.nTele; ++i)
                ss >> nclu[i];

        for (int i = 0; i < daqdata.nDigi; ++i)
            ss >> digiBase[i];
        for (int i = 0; i < daqdata.nDigi; ++i)
            ss >> digiPH[i];
        for (int i = 0; i < daqdata.nDigi; ++i)
            ss >> digiTime[i];

        for (int i = 0; i < daqdata.nGonio; ++i)
            ss >> gonioInfo[i];

        if (daqdata.Spillnumber_flag)
            ss >> spillnumber;
        if (daqdata.Motorstepnumber_flag)
            ss >> motorstepnumber;
        if (daqdata.eventnumber_flag)
            ss >> eventnumber;

        // reco vars
        event.tele1x = xpos[0] - linedata.x1corr;
        event.tele1y = xpos[1] - linedata.x2corr;
        event.tele2x = xpos[2] - linedata.x3corr;
        event.tele2y = xpos[3] - linedata.x4corr;

        event.thetax_in = atan2(event.tele2x - event.tele1x, linedata.z12) - linedata.thetax_in_corr;
        event.thetay_in = atan2(event.tele2y - event.tele1y, linedata.z12) - linedata.thetay_in_corr;

        event.oreox = event.tele1x + tan(event.thetax_in) * (linedata.z12 + linedata.z2c);
        event.oreoy = event.tele1y + tan(event.thetay_in) * (linedata.z12 + linedata.z2c);

        for (Int_t ch = 0; ch < daqdata.nDigi; ch++)
        {
            event.enecry[ch] = digiPH[ch] * daqdata.eq[ch];
        }
        for (Int_t ch = 0; ch < daqdata.nDigi; ch++)
        {
            if (ch > 5 && ch < 15)
            {
                event.eneup += event.enecry[ch];
            }
            if (ch > 14 && ch < 24)
            {
                event.enedown += event.enecry[ch];
            }
            if (ch == 0 || ch == 3)
            {
                event.enelg += event.enecry[ch];
            }
        }

        event.eneup = event.eneup * daqdata.calib_m_up + daqdata.calib_q_up;
        event.enedown = event.enedown * daqdata.calib_m_down + daqdata.calib_q_down;
        event.enelg = event.enelg * daqdata.lg_calib_m + daqdata.lg_calib_q;

        event.enetot = event.eneup * daqdata.eq_up_down + event.enedown;

        if (!ss.fail())
        {
            t->Fill();
            ++event.nFilled;
        }
        else
        {
            std::cout << line << std::endl;
            std::cout << "failed stripping" << std::endl;
        }
        event.eneup = event.enedown = event.enetot = event.enelg = 0;
    }

    f->cd();
    f->Write();
    f->Close();
}
