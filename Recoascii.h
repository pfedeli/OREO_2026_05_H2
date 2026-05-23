#include "TString.h"
#include "TEnv.h"

struct Configdaq
{
    TString ascii_path;
    TString root_path;
    TString reco_root_path;
    Int_t nTele;
    Bool_t nStrip_flag;
    Bool_t nClu_flag;
    Int_t nDigi;
    Int_t nGonio;
    Bool_t Spillnumber_flag;
    Bool_t Motorstepnumber_flag;
    Bool_t eventnumber_flag;
    std::vector<Double_t> eq;
    Double_t calib_m_up;
    Double_t calib_q_up;
    Double_t calib_m_down;
    Double_t calib_q_down;
    Double_t lg_calib_m;
    Double_t lg_calib_q;
    Double_t eq_up_down;
};

struct Linepars
{
    Double_t z12;
    Double_t z2c;
    Double_t x1corr;
    Double_t x2corr;
    Double_t x3corr;
    Double_t x4corr;
    Double_t thetax_in_corr;
    Double_t thetay_in_corr;
};

struct Recodata
{

    Double_t tele1x;
    Double_t tele1y;
    Double_t tele2x;
    Double_t tele2y;
    Long64_t nFilled = 0;
    Double_t oreox;
    Double_t oreoy;
    Double_t thetax_in;
    Double_t thetay_in;
    std::vector<Long64_t> enecry;
    Double_t eneup;
    Double_t enedown;
    Double_t enetot;
    Double_t enelg;
    Recodata(Int_t nDigi) : enecry(nDigi) {};
};

Configdaq loaddaq(TString configpath)
{
    TEnv env;
    if (env.ReadFile(configpath.Data(), kEnvChange) != 0) // 0 if successful
    {
        std::cout << "Error: impossible to open config file " << configpath << std::endl;
    }
    Configdaq cfg;
    cfg.ascii_path = env.GetValue("ascii_path", "");
    cfg.root_path = env.GetValue("root_path", "");
    cfg.reco_root_path = env.GetValue("reco_root_path", "");
    cfg.nTele = env.GetValue("nTele", 1);
    cfg.nStrip_flag = env.GetValue("nStrip", false);
    cfg.nClu_flag = env.GetValue("nClu", false);
    cfg.nDigi = env.GetValue("nDigi", 0);
    cfg.nGonio = env.GetValue("nGonio", 0);
    cfg.Spillnumber_flag = env.GetValue("Spillnumber", false);
    cfg.Motorstepnumber_flag = env.GetValue("Motorstepnumber", false);
    cfg.eventnumber_flag = env.GetValue("eventnumber", false);

    for (Int_t a = 0; a < cfg.nDigi; a++)
    {
        TString eqName = Form("eq%d", a);
        cfg.eq.push_back(env.GetValue(eqName, 1.0));
    }

    cfg.calib_m_up = env.GetValue("calib_m_up", 1.0);
    cfg.calib_q_up = env.GetValue("calib_q_up", 1.0);
    cfg.calib_m_down = env.GetValue("calib_m_down", 1.0);
    cfg.calib_q_down = env.GetValue("calib_q_down", 1.0);
    cfg.lg_calib_m = env.GetValue("lg_calib_m", 1.0);
    cfg.lg_calib_q = env.GetValue("lg_calib_q", 1.0);
    cfg.eq_up_down = env.GetValue("eq_up_down", 1.0);

    std::cout << "--- Check Caricamento Config ---" << std::endl;
    std::cout << "File ASCII: " << cfg.ascii_path << std::endl;
    std::cout << "Telescopi:  " << cfg.nTele / 2 << std::endl;
    std::cout << "nDigi: " << cfg.nDigi << std::endl;

    if (cfg.nDigi > 0)
    {
        std::cout << "Coefficienti EQ caricati: ";
        for (Int_t i = 0; i < (Int_t)cfg.eq.size(); i++)
        {
            std::cout << cfg.eq[i] << (i == cfg.eq.size() - 1 ? "" : ", ");
        }
        std::cout << std::endl;
    }
    return cfg;
}

Linepars loadlinepars(TString configpath)
{
    TEnv env;
    if (env.ReadFile(configpath.Data(), kEnvChange) != 0) // 0 if successful
    {
        std::cout << "Error: impossible to open config file " << configpath << std::endl;
    }
    Linepars lnps;
    lnps.z12 = env.GetValue("z12", 0.0);
    lnps.z2c = env.GetValue("z2c", 0.0);
    lnps.x1corr = env.GetValue("x1corr", 0.0);
    lnps.x2corr = env.GetValue("x2corr", 0.0);
    lnps.x3corr = env.GetValue("x3corr", 0.0);
    lnps.x4corr = env.GetValue("x4corr", 0.0);
    lnps.thetax_in_corr = env.GetValue("thetax_in_corr", 0.0);
    lnps.thetay_in_corr = env.GetValue("thetay_in_corr", 0.0);

    std::cout << "tele corr: " << lnps.x1corr << " " << lnps.x2corr << " " << lnps.x3corr << " " << lnps.x4corr << " " << std::endl;
    std::cout << "theta corr: " << lnps.thetax_in_corr << " " << lnps.thetay_in_corr << std::endl;

    return lnps;
}

Bool_t cut(Recodata datatobecutted)
{
    Bool_t position = (datatobecutted.tele1x < 0 || datatobecutted.tele1x > 2) || (datatobecutted.tele1y < 0 || datatobecutted.tele1y > 2) || (datatobecutted.tele2x < 0 || datatobecutted.tele2x > 2) || (datatobecutted.tele2y < 0 || datatobecutted.tele2y > 2);

    Bool_t energy = datatobecutted.enetot < 4000;
    Bool_t cutted = position || energy;
    return cutted;
}