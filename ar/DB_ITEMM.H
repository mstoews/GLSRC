#ifndef __DB_ITEMM_H
#define __DB_ITEMM_H
/* Item master class */
_CLASSDEF(Tprods)
class Tprods : public Data4
{
 public:
  Field4* fPcode;
  Field4* fPname;
  Field4* fFulln;
  Field4* fPtype;
  Field4* fClass;
  Field4* fSkbn; 
  Field4* fShop1;
  Field4* fPlace;
  Field4* fUnit; 
  Field4* fLotsize;
  Field4* fOnhand;
  Field4* fLmhand;
  Field4* fLmprice;
  Field4* fAvprice;
  Field4* fStprice;

  Tprods(Code4 *cb,LPSTR name=0):Data4 (cb,"gl_itemm")
  {
  fPcode 	= (Field4 *) new Field4(*(Data4 *)this,"fpcode");
  fPname 	= (Field4 *) new Field4(*(Data4 *)this,"fpname");
  fFulln 	= (Field4 *) new Field4(*(Data4 *)this,"ffulln");
  fPtype 	= (Field4 *) new Field4(*(Data4 *)this,"fptype");
  fClass 	= (Field4 *) new Field4(*(Data4 *)this,"fclass");
  fSkbn  	= (Field4 *) new Field4(*(Data4 *)this,"fskbn");
  fShop1 	= (Field4 *) new Field4(*(Data4 *)this,"fshop1");
  fPlace 	= (Field4 *) new Field4(*(Data4 *)this,"fplace");
  fUnit  	= (Field4 *) new Field4(*(Data4 *)this,"funit");
  fLotsize	= (Field4 *) new Field4(*(Data4 *)this,"flotsize");
  fOnhand	= (Field4 *) new Field4(*(Data4 *)this,"fonhand");
  fLmhand	= (Field4 *) new Field4(*(Data4 *)this,"flmhand");
  fLmprice	= (Field4 *) new Field4(*(Data4 *)this,"flmprice");
  fAvprice	= (Field4 *) new Field4(*(Data4 *)this,"favprice");
  fStprice	= (Field4 *) new Field4(*(Data4 *)this,"fstprice");
  }
  double dAvprice() { return double(Str4ptr(fAvprice->str()));};
  double dStprice() { return double(Str4ptr(fStprice->str()));};
  double dLotsize() { return double(Str4ptr(fLotsize->str()));};
  double dOnHand()  { return double(Str4ptr(fOnhand->str()));};
  double dLmHand()  { return double(Str4ptr(fLmhand->str()));};
  double dLmPrice() { return double(Str4ptr(fLmprice->str()));};
  virtual void Man(){SetIndex("MAN");};
  ~Tprods(){close();};
 };

#endif
