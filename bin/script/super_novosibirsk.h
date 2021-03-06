/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
  * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/

#ifndef SUPER_NOVOSIBIRSK
#define SUPER_NOVOSIBIRSK

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"
 
class super_novosibirsk : public RooAbsPdf {
public:
  super_novosibirsk() {} ; 
  super_novosibirsk(const char *name, const char *title,
	      RooAbsReal& _Mass,
	      RooAbsReal& _p0,
	      RooAbsReal& _p1,
	      RooAbsReal& _p3,
	      RooAbsReal& _p4,
	      RooAbsReal& _p5,
	      RooAbsReal& _p6);
  super_novosibirsk(const super_novosibirsk& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new super_novosibirsk(*this,newname); }
  inline virtual ~super_novosibirsk() { }

protected:

  RooRealProxy Mass ;
  RooRealProxy p0 ;
  RooRealProxy p1 ;
  RooRealProxy p3 ;
  RooRealProxy p4 ;
  RooRealProxy p5 ;
  RooRealProxy p6 ;
  
  Double_t evaluate() const ;

private:

  ClassDef(super_novosibirsk,1) // Your description goes here...
};
 
#endif
