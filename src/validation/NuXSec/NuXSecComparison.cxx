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

#include "validation/NuXSec/NuXSecComparison.h"

using namespace genie;
using namespace genie::mc_vs_data;

//____________________________________________________________________________
NuXSecComparison::NuXSecComparison(
    string label, string dataset_keys, NuXSecFunc * xsec_func,
    double Emin,  double Emax, 
    bool in_logx, bool in_logy,  bool scale_with_E
) :
fLabel       (label),
fDataSetKeys (dataset_keys),
fXSecFunc    (xsec_func),   
fEmin        (Emin),
fEmax        (Emax),
fInLogX      (in_logx),
fInLogY      (in_logy),
fScaleWithE  (scale_with_E)
{

}
//____________________________________________________________________________
NuXSecComparison::~NuXSecComparison()
{

}
//____________________________________________________________________________

