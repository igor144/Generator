//____________________________________________________________________________
/*!

\class   genie::flux::GCylindTH1Flux

\brief   A simple GENIE flux driver. Generates a 'cylindrical' neutrino beam
         along the input direction, with the input transverse radius and
         centered at the input beam spot position.
         The energies are generated from the input energy spectrum (TH1D)
         Multiple neutrino species can be generated (you will need to supply
         an energy spectrum for each).

\author  Costas Andreopoulos <C.V.Andreopoulos@rl.ac.uk>
         CCLRC, Rutherford Appleton Laboratory

\created July 4, 2005

*/
//____________________________________________________________________________

#include <algorithm>

#include <TH1D.h>
#include <TF1.h>
#include <TVector3.h>

#include "Conventions/Constants.h"
#include "FluxDrivers/GCylindTH1Flux.h"
#include "Messenger/Messenger.h"
#include "Numerical/RandomGen.h"
#include "PDG/PDGCodeList.h"
#include "Utils/PrintUtils.h"

using namespace genie;
using namespace genie::constants;
using namespace genie::flux;

//____________________________________________________________________________
GCylindTH1Flux::GCylindTH1Flux()
{
  this->Initialize();
}
//___________________________________________________________________________
GCylindTH1Flux::~GCylindTH1Flux()
{
  this->CleanUp();
}
//___________________________________________________________________________
const PDGCodeList & GCylindTH1Flux::FluxParticles(void)
{
  return (*fPdgCList);
}
//___________________________________________________________________________
double GCylindTH1Flux::MaxEnergy(void)
{
  return fMaxEv;
}
//___________________________________________________________________________
bool GCylindTH1Flux::GenerateNext(void)
{
  //-- Reset previously generated neutrino code / 4-p / 4-x
  this->ResetSelection();

  //-- Generate an energy from the 'combined' spectrum histogram
  //   and compute the momentum vector
  double Ev = (double) fTotSpectrum->GetRandom();

  TVector3 p3(*fDirVec); // momentum along the neutrino direction
  p3.SetMag(Ev);         // with |p|=Ev

  fgP4.SetPxPyPzE(p3.Px(), p3.Py(), p3.Pz(), Ev);

  //-- Select a neutrino species from the flux fractions at the
  //   selected energy
  fgPdgC = (*fPdgCList)[this->SelectNeutrino(Ev)];

  //-- Compute neutrino 4-x

  // Create a vector (vec) that points to a random position at a disk
  // of radius Rt passing through the origin, perpendicular to the
  // input direction.
  TVector3 vec0(*fDirVec);           // vector along the input direction
  TVector3 vec = vec0.Orthogonal();  // orthogonal vector

  double psi = this->GeneratePhi();  // rndm angle [0,2pi]
  double Rt  = this->GenerateRt();   // rndm R [0,Rtransverse]

  vec.Rotate(psi,vec0); // rotate around original vector
  vec.SetMag(Rt);       // set new norm

  // Set the neutrino position as beam_spot + vec
  double x = fBeamSpot->X() + vec.X();
  double y = fBeamSpot->Y() + vec.Y();
  double z = fBeamSpot->Z() + vec.Z();

  fgX4.SetXYZT(x,y,z,0.);

  return true;
}
//___________________________________________________________________________
int GCylindTH1Flux::PdgCode(void)
{
  LOG("Flux", pINFO) << "Generated neutrino pdg-code: " << fgPdgC;
  return fgPdgC;
}
//___________________________________________________________________________
const TLorentzVector & GCylindTH1Flux::Momentum(void)
{
  LOG("Flux", pINFO)
        << "Generated neutrino p4: " << utils::print::P4AsShortString(&fgP4);
  return fgP4;
}
//___________________________________________________________________________
const TLorentzVector & GCylindTH1Flux::Position(void)
{
  LOG("Flux", pINFO)
             << "Generated neutrino x4: " << utils::print::X4AsString(&fgX4);
  return fgX4;
}
//___________________________________________________________________________
void GCylindTH1Flux::Initialize(void)
{
  LOG("Flux", pINFO) << "Initializing GCylindTH1Flux driver";

  fMaxEv       = 0;
  fPdgCList    = new PDGCodeList;
  fTotSpectrum = 0;
  fDirVec      = 0;
  fBeamSpot    = 0;
  fRt          = 0;
  fRtDep       = 0;

  this->ResetSelection();
  this->SetRtDependence("x");
  //eg, other example: this->SetRtDependence("pow(x,2)");
}
//___________________________________________________________________________
void GCylindTH1Flux::ResetSelection(void)
{
// initializing running neutrino pdg-code, 4-position, 4-momentum
  fgPdgC = 0;
  fgP4.SetPxPyPzE (0.,0.,0.,0.);
  fgX4.SetXYZT    (0.,0.,0.,0.);
}
//___________________________________________________________________________
void GCylindTH1Flux::CleanUp(void)
{
  LOG("Flux", pINFO) << "Cleaning up...";

  if (fDirVec     ) delete fDirVec;
  if (fBeamSpot   ) delete fBeamSpot;
  if (fPdgCList   ) delete fPdgCList;
  if (fTotSpectrum) delete fTotSpectrum;
  if (fRtDep      ) delete fRtDep;

  unsigned int nspectra = fSpectrum.size();
  for(unsigned int i = 0; i < nspectra; i++) {
     TH1D * spectrum = fSpectrum[i];
     delete spectrum;
     spectrum = 0;
  }
}
//___________________________________________________________________________
void GCylindTH1Flux::SetNuDirection(const TVector3 & direction)
{
  if(fDirVec) delete fDirVec;
  fDirVec = new TVector3(direction);
}
//___________________________________________________________________________
void GCylindTH1Flux::SetBeamSpot(const TVector3 & spot)
{
  if(fBeamSpot) delete fBeamSpot;
  fBeamSpot = new TVector3(spot);
}
//___________________________________________________________________________
void GCylindTH1Flux::SetTransverseRadius(double Rt)
{
  LOG ("Flux", pINFO) << "Setting R[transverse] = " << Rt;
  fRt = Rt;

  if(fRtDep) fRtDep->SetRange(0,Rt);
}
//___________________________________________________________________________
void GCylindTH1Flux::AddEnergySpectrum(int nu_pdgc, TH1D * spectrum)
{
  fPdgCList->push_back(nu_pdgc);

  bool accepted = (count(fPdgCList->begin(),fPdgCList->end(),nu_pdgc) == 1);
  if(!accepted) {
     LOG ("Flux", pWARN)
            << "The pdg-code isn't recognized and the spectrum was ignored";
  } else {
     fSpectrum.push_back(spectrum);

     int    nb  = spectrum->GetNbinsX();
     Axis_t max = spectrum->GetBinLowEdge(nb)+spectrum->GetBinWidth(nb);
     fMaxEv = TMath::Max(fMaxEv, (double)max);

     this->AddAllFluxes(); // update combined flux
  }
}
//___________________________________________________________________________
void GCylindTH1Flux::SetRtDependence(string rdep)
{
// Set the (functional form of) Rt dependence as string, eg "x*x+sin(x)"
// You do not need to set this method. The default behaviour is to generate
// flux neutrinos uniformly over the area of the cylinder's cross section.

  if(fRtDep) delete fRtDep;

  fRtDep = new TF1("rdep", rdep.c_str(), 0,fRt);
}
//___________________________________________________________________________
void GCylindTH1Flux::AddAllFluxes(void)
{
  LOG("Flux", pINFO) << "Computing combined flux";

  if(fTotSpectrum) delete fTotSpectrum;

  vector<TH1D *>::const_iterator spectrum_iter;

  unsigned int inu=0;
  for(spectrum_iter = fSpectrum.begin();
                       spectrum_iter != fSpectrum.end(); ++spectrum_iter) {
     TH1D * spectrum = *spectrum_iter;

     if(inu==0) { fTotSpectrum = new TH1D(*spectrum); }
     else       { fTotSpectrum->Add(spectrum);        }
     inu++;
  }
}
//___________________________________________________________________________
int GCylindTH1Flux::SelectNeutrino(double Ev)
{
  const unsigned int n = fPdgCList->size();
  double fraction[n];

  vector<TH1D *>::const_iterator spectrum_iter;

  unsigned int inu=0;
  for(spectrum_iter = fSpectrum.begin();
                       spectrum_iter != fSpectrum.end(); ++spectrum_iter) {
     TH1D * spectrum = *spectrum_iter;
     fraction[inu++] = spectrum->GetBinContent(spectrum->FindBin(Ev));
  }

  double sum = 0;
  for(inu = 0; inu < n; inu++) {
     sum += fraction[inu];
     fraction[inu] = sum;
     LOG("Flux", pDEBUG) << "SUM-FRACTION(0->" << inu <<") = " << sum;
  }

  RandomGen * rnd = RandomGen::Instance();
  double R = sum * rnd->Random2().Rndm();

  LOG("Flux", pDEBUG) << "R e [0,SUM] = " << R;

  for(inu = 0; inu < n; inu++) {if ( R < fraction[inu] ) return inu;}

  LOG("Flux", pERROR) << "Could not select a neutrino species";
  assert(false);

  return -1;
}
//___________________________________________________________________________
double GCylindTH1Flux::GeneratePhi(void) const
{
  RandomGen * rnd = RandomGen::Instance();
  double phi = 2.*kPi*(rnd->Random2().Rndm()); // [0,2pi]
  return phi;
}
//___________________________________________________________________________
double GCylindTH1Flux::GenerateRt(void) const
{
  double Rt = fRtDep->GetRandom(); // rndm R [0,Rtransverse]
  return Rt;
}
//___________________________________________________________________________
