/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "Riostream.h" 

#include "Analysis/Tools/interface/super_novosibirsk.h" 
#include "RooAbsReal.h" 
#include "RooAbsCategory.h" 
#include <math.h>
#include <iostream>
#include "TMath.h" 



ClassImp(super_novosibirsk) 

 super_novosibirsk::super_novosibirsk(const char *name, const char *title, 
                        RooAbsReal& _Mass,
                        RooAbsReal& _p0,
                        RooAbsReal& _p1,
                        RooAbsReal& _p3,
                        RooAbsReal& _p4,
                        RooAbsReal& _p5,
                        RooAbsReal& _p6) :
   RooAbsPdf(name,title), 
   Mass("Mass","Mass",this,_Mass),
   p0("p0","p0",this,_p0),
   p1("p1","p1",this,_p1),
   p3("p3","p3",this,_p3),
   p4("p4","p4",this,_p4),
   p5("p5","p5",this,_p5),
   p6("p6","p6",this,_p6)
 { 
 } 


 super_novosibirsk::super_novosibirsk(const super_novosibirsk& other, const char* name) :  
   RooAbsPdf(other,name), 
   Mass("Mass",this,other.Mass),
   p0("p0",this,other.p0),
   p1("p1",this,other.p1),
   p3("p3",this,other.p3),
   p4("p4",this,other.p4),
   p5("p5",this,other.p5),
   p6("p6",this,other.p6)
 { 
 } 



 Double_t super_novosibirsk::evaluate() const 
 { 
   Double_t first, second, inside, sigma0;
   const Double_t xi = 2*TMath::Sqrt(TMath::Log(4));
   sigma0 = 2/xi*TMath::ASinH(p5*xi/2);
   Double_t sigma02 = TMath::Power(sigma0, 2);
   first = 0.5*(TMath::Erf(p0*(Mass - p1)) + 1);
   inside = 1 - (Mass - p3)*p5/p4 - TMath::Power(Mass - p3, 2)*p5*p6/p4;
   // if (inside < 0) {
   //   std::cerr << "Invalid value" << std::endl;
   //   return TMath::Power(10, 2);
   // }
   second = TMath::Exp(-0.5/sigma02*TMath::Power(TMath::Log(inside), 2) - \
                          0.5*sigma02);
   return first*second;
 }



