#ifndef __DB_ITEMM_H
#define __DB_ITEMM_H
/* Item master class */
_CLASSDEF(Tprods)
class Tprods : public Data4
{
 public:
  Field4 fPcode;
  Field4 fPname;
  Field4 fFulln;
  Field4 fPtype;
  Field4 fUnit;
  Field4 fLotsize;
  Field4 fOnhand;
  Field4 fLmhand;
  Field4 fAvprice;

  inline void init()
  {
  fPcode 	.init(data,1);
  fPname 	.init(data,2);
  fFulln 	.init(data,3);
  fPtype 	.init(data,4);
  fUnit  	.init(data,11);
  fLotsize	.init(data,12);
  fOnhand	.init(data,13);
  fLmhand	.init(data,14);
  fAvprice	.init(data,16);
  }

  Tprods(){};
  Tprods(Code4 *cb,LPSTR name=0):Data4 (cb,"gl_itemm")
  {
	  if(!isValid())
			return;
	  init();
  }
  double dAvprice() { return double(Str4ptr(fAvprice.str()));};
  double dLotsize() { return double(Str4ptr(fLotsize.str()));};
  double dOnHand()  { return double(Str4ptr(fOnhand.str()));};
  double dLmHand()  { return double(Str4ptr(fLmhand.str()));};
  LPSTR GetDbfName(){ return "GL_ITEMM";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if( isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
  }

  ~Tprods()
  {
    //close();
  };
 };

#endif