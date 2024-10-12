#ifndef __DB_VEND_H
#define __DB_VEND_H
_CLASSDEF (Tapvendor)

class Tapvendor : public Data4
{
 public:
  Field4 *vendorno;
  Field4 *vdshort;
  Field4 *vdname;
  Field4 *address1;
  Field4 *address2;
  Field4 *address3;
  Field4 *postal;
  Field4 *fax;
  Field4 *phone;
  Field4 *contact;
  Field4 *account;
  Field4 *depart;
  Field4 *descript;
  Field4 *type;
  Field4 *ref;
  Field4 *status;
  Field4 *terms;
  Field4 *vat;
  Field4 *percentage;
  Field4 *rebate;
  Field4 *vin;
  Field4 *medrep;
  Field4 *vat_acct;
  Field4 *vat_dept;
  Field4 *discount_days;
  Field4 *discount_percent;


  Tapvendor(Code4 *cb,LPSTR name=0):Data4(cb,"ap_vend")
   {
    //SetIndex("Vendorno");
    top();
    vendorno = (Field4 *) new Field4(*(Data4 *) this,1);
    vdshort  = (Field4 *) new Field4(*(Data4 *) this,2);
    vdname    = (Field4 *) new Field4(*(Data4 *) this,3);
    address1 = (Field4 *) new Field4(*(Data4 *) this,4);
    address2 = (Field4 *) new Field4(*(Data4 *) this,5);
    address3 = (Field4 *) new Field4(*(Data4 *) this,6);
    postal   = (Field4 *) new Field4(*(Data4 *) this,7);
    phone    = (Field4 *) new Field4(*(Data4 *) this,8);
    fax      = (Field4 *) new Field4(*(Data4 *) this,9);
    depart   = (Field4 *) new Field4(*(Data4 *) this,10);
    account  = (Field4 *) new Field4(*(Data4 *) this,11);
    vat_dept   = (Field4 *) new Field4(*(Data4 *) this,12);
    vat_acct   = (Field4 *) new Field4(*(Data4 *) this,13);
    descript = (Field4 *) new Field4(*(Data4 *) this,16);
    contact  = (Field4 *) new Field4(*(Data4 *) this,17);
    type     = (Field4 *) new Field4(*(Data4 *) this,18);
    medrep   = (Field4 *) new Field4(*(Data4 *) this,19);
    status   = (Field4 *) new Field4(*(Data4 *) this,20);
    terms      = (Field4 *) new Field4(*(Data4 *) this,21);
    vat        = (Field4 *) new Field4(*(Data4 *) this,22);
    percentage = (Field4 *) new Field4(*(Data4 *) this,25);
    rebate     = (Field4 *) new Field4(*(Data4 *) this,26);
	 vin        = (Field4 *) new Field4(*(Data4 *) this,27);
	 discount_days = (Field4 *) new Field4(*(Data4 *) this,28);
	 discount_percent = (Field4 *) new Field4(*(Data4 *) this,29);

	};
	~Tapvendor(){close();};
	virtual int iVat() {return int ( Str4ptr(vat->str()) );}
	virtual double dPercentage(){return double ( Str4ptr(percentage->str()) );}
	virtual double dRebate(){return double ( Str4ptr(rebate->str()) );}

 };
#endif