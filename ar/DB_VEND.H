#ifndef _DB_VEND_H
#define _DB_VEND_H
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
  Field4 *s_add1;
  Field4 *s_add2;
  Field4 *s_add3;
  Field4 *s_postal;
  Field4 *s_fax;
  Field4 *s_phone;
  Field4 *contact;
  Field4 *account;
  Field4 *depart;
  Field4 *ar_dept;
  Field4 *ar_acct;
  Field4 *vat_dept;
  Field4 *vat_acct;
  Field4 *terms;
  Field4 *vat;
  Field4 *vin;

  Tapvendor(Code4 *cb,LPSTR name=0):Data4(cb,"ar_cust")
   {
    //SetIndex("Vendorno");
	 top();
	 vendorno =  new Field4(*(Data4 *) this,1);
	 vdshort  =  new Field4(*(Data4 *) this,2);
	 vdname   =  new Field4(*(Data4 *) this,3);
	 address1 =  new Field4(*(Data4 *) this,4);
	 address2 =  new Field4(*(Data4 *) this,5);
	 address3 =  new Field4(*(Data4 *) this,6);
	 postal   =  new Field4(*(Data4 *) this,7);
	 phone    =  new Field4(*(Data4 *) this,8);
	 fax      =  new Field4(*(Data4 *) this,9);
	 s_add1   =  new Field4(*(Data4 *) this,10);
	 s_add2   =  new Field4(*(Data4 *) this,11);
	 s_add3   =  new Field4(*(Data4 *) this,12);
	 s_postal =  new Field4(*(Data4 *) this,13);
	 s_phone  =  new Field4(*(Data4 *) this,14);
	 s_fax    =  new Field4(*(Data4 *) this,15);
	 depart   =  new Field4(*(Data4 *) this,16);
	 account  =  new Field4(*(Data4 *) this,17);
	 ar_dept  =  new Field4(*(Data4 *) this,18);
	 ar_acct  =  new Field4(*(Data4 *) this,19);
	 vat_dept =  new Field4(*(Data4 *) this,21);
	 vat_acct =  new Field4(*(Data4 *) this,20);
	 contact  =  new Field4(*(Data4 *) this,22);
	 terms    =  new Field4(*(Data4 *) this,23);
	 vat      =  new Field4(*(Data4 *) this,28);
	 vin      =  new Field4(*(Data4 *) this,29);
   };
   ~Tapvendor(){close();};
   virtual int iVat() {return int ( Str4ptr(vat->str()) );}
 };
#endif
