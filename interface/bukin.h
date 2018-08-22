/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
  * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/

#ifndef BUKIN
#define BUKIN

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"


namespace analysis {
  namespace tools {
 

class bukin : public RooAbsPdf {
public:
  bukin() {} ; 
  bukin(const char *name, const char *title,
	      RooAbsReal& _x,
	      RooAbsReal& _Ap,
	      RooAbsReal& _Xp,
	      RooAbsReal& _sp,
	      RooAbsReal& _rho1,
	      RooAbsReal& _rho2,
	      RooAbsReal& _xi);
  bukin(const bukin& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new bukin(*this,newname); }
  inline virtual ~bukin() { }

protected:

  RooRealProxy x ;
  RooRealProxy Ap ;
  RooRealProxy Xp ;
  RooRealProxy sp ;
  RooRealProxy rho1 ;
  RooRealProxy rho2 ;
  RooRealProxy xi ;
  
  Double_t evaluate() const ;

private:

  ClassDef(bukin,1) // Your description goes here...
};


  }
}
#endif
