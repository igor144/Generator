//____________________________________________________________________________
/*
 Copyright (c) 2003-2011, GENIE Neutrino MC Generator Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author: Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
         STFC, Rutherford Appleton Laboratory

 For the class documentation see the corresponding header file.

 Important revisions after version 2.0.0 :

*/
//____________________________________________________________________________

#include "Messenger/Messenger.h"
#include "Utils/StringUtils.h"
#include "Utils/SystemUtils.h"
#include "Utils/Style.h"
#include "validation/NuXSec/NuXSecData.h"

using namespace genie;
using namespace genie::mc_vs_data;

//____________________________________________________________________________
NuXSecData::NuXSecData()
{
  this->Init();
}
//____________________________________________________________________________
NuXSecData::~NuXSecData()
{
  this->CleanUp();
}
//____________________________________________________________________________
bool NuXSecData::Read(string data_archive_file_name)
{
  // get TTree with neutrino scattering data
  if( ! utils::system::FileExists(data_archive_file_name) ) {
      LOG("NuXSecData", pERROR) 
         << "Can not find file: " << data_archive_file_name;
      return false;
  }
  fNuXSecDataFile = new TFile(data_archive_file_name.c_str(),"read");  
  fNuXSecDataTree = (TTree *) fNuXSecDataFile->Get("nuxsnt");
  if(!fNuXSecDataTree) {
      LOG("NuXSecData", pERROR) 
         << "Can not find TTree `nuxsnt' in file: " << data_archive_file_name;
      return false;
  }

  // set branch addresses 
  fNuXSecDataTree->SetBranchAddress ("dataset",    (void*)fDataset  );
  fNuXSecDataTree->SetBranchAddress ("citation",   (void*)fCitation );
  fNuXSecDataTree->SetBranchAddress ("E",          &fE              );
  fNuXSecDataTree->SetBranchAddress ("Emin",       &fEmin           );
  fNuXSecDataTree->SetBranchAddress ("Emax",       &fEmax           );
  fNuXSecDataTree->SetBranchAddress ("xsec",       &fXSec           );
  fNuXSecDataTree->SetBranchAddress ("xsec_err_p", &fXSecErrP       );
  fNuXSecDataTree->SetBranchAddress ("xsec_err_m", &fXSecErrM       );

  return true;
}
//____________________________________________________________________________
vector<TGraphAsymmErrors *> 
  NuXSecData::Data(string keys, double Emin, double Emax)
{
  vector<string> keyv = utils::str::Split(keys,";");
  unsigned int ndatasets = keyv.size();

  vector<TGraphAsymmErrors *> data(ndatasets);

  for(unsigned int idataset = 0; idataset < ndatasets; idataset++) {

    fNuXSecDataTree->Draw("E", Form("dataset==\"%s\"",keyv[idataset].c_str()), "goff");
    int npoints = fNuXSecDataTree->GetSelectedRows();
    double *  x    = new double[npoints];
    double *  dxl  = new double[npoints];
    double *  dxh  = new double[npoints];
    double *  y    = new double[npoints];
    double *  dyl  = new double[npoints];
    double *  dyh  = new double[npoints];
    string label = "";
    int ipoint=0;
    for(int i = 0; i < fNuXSecDataTree->GetEntries(); i++) {
      fNuXSecDataTree->GetEntry(i);
      if(strcmp(fDataset,keyv[idataset].c_str()) == 0) {
        if(ipoint==0) {
          label = Form("%s [%s]", fDataset, fCitation);
        }
        x   [ipoint] = fE;
        dxl [ipoint] = (fEmin > 0) ? TMath::Max(0., fE-fEmin) : 0.;
        dxh [ipoint] = (fEmin > 0) ? TMath::Max(0., fEmax-fE) : 0.;
	y   [ipoint] = fXSec;
	dyl [ipoint] = fXSecErrM;
	dyh [ipoint] = fXSecErrP;
	ipoint++;
      } 
    }//i
    TGraphAsymmErrors * gr = new TGraphAsymmErrors(npoints,x,y,dxl,dxh,dyl,dyh);
    int sty = kDataPointStyle[idataset];
    int col = kDataPointColor[idataset];
    utils::style::Format(gr,col, kSolid, 1, col, sty, 1.5);
    gr->SetTitle(label.c_str());
    data.push_back(gr);
  }

  return data;
}
//____________________________________________________________________________
void NuXSecData::Init(void)
{
  fNuXSecDataFile = 0;
  fNuXSecDataTree = 0;
/*
  // tree brancges
  char   fDataset  [buffer_size];
  char   fCitation [buffer_size];
  double fE;
  double fEmin;
  double fEmax;
  double fXSec;
  double fXSecErrP;
  double fXSecErrM;
*/
}
//____________________________________________________________________________
void NuXSecData::CleanUp(void)
{

}
//____________________________________________________________________________
 