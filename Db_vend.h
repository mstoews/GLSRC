#ifndef __DB_VEND_H
#define __DB_VEND_H
_CLASSDEF (Tapvendor)

class Tapvendor : public Data4
{
 public:
  Field4  vendorno;
  Field4  vdshort;
  Field4  vdname;
  Field4  address1;
  Field4  address2;
  Field4  address3;
  Field4  postal;
  Field4  fax;
  Field4  phone;
  Field4  contact;
  Field4  account;
  Field4  depart;
  Field4  ap_acct;
  Field4  ap_dept;
  Field4  descript;
  Field4  type;
  Field4  ref;
  Field4  status;
  Field4  terms;
  Field4  vat;
  Field4  percentage;
  Field4  rebate;
  Field4  vin;
  Field4  medrep;
  Field4  vat_acct;
  Field4  vat_dept;
  Field4  discount_days;
  Field4  discount_percent;
  Field4  ex_acct;
  Field4  ex_dept;
  Field4  vat_num;

  Tapvendor(){};
  void init()
  {
	 top();
	 vendorno.init(data,1);
	 vdshort .init(data,2);
	 vdname   .init(data,3);
	 address1.init(data,4);
	 address2.init(data,5);
	 address3.init(data,6);
	 postal  .init(data,7);
	 phone   .init(data,8);
	 fax     .init(data,9);
	 depart  .init(data,10);
	 account .init(data,11);
	 vat_dept  .init(data,12);
	 vat_acct  .init(data,13);
	 ap_acct   .init(data,14);
	 ap_dept   .init(data,15);
	 descript.init(data,16);
	 contact .init(data,17);
	 type    .init(data,18);
	 medrep  .init(data,19);
	 status  .init(data,20);
	 terms     .init(data,21);
	 vat       .init(data,22);
	 percentage.init(data,25);
	 rebate    .init(data,26);
	 vin       .init(data,27);
	 discount_days.init(data,28);
	 discount_percent.init(data,29);
	 ex_acct.init(data,33);
	 ex_dept.init(data,34);
	 vat_num.init(data,35);
	 return;
  }
/*
Number of records     2
Last update           01/31/97
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
    1  VENDORNO                   CHARACTER          6           Y
    2  VDSHORT                    CHARACTER         12           N
    3  VDNAME                     CHARACTER         40           N
    4  ADDRESS1                   CHARACTER         30           N
    5  ADDRESS2                   CHARACTER         30           N
    6  ADDRESS3                   CHARACTER         30           N
    7  POSTAL                     CHARACTER          7           N
    8  PHONE                      CHARACTER         16           N
    9  FAX                        CHARACTER         16           N
   10  DEPART                     CHARACTER          4           N
   11  ACCOUNT                    CHARACTER          7           N
   12  VAT_DEPT                   CHARACTER          4           N
   13  VAT_ACCT                   CHARACTER          7           N
   14  AP_ACCT                    CHARACTER          7           N
   15  AP_DEPT                    CHARACTER          4           N
   16  DESCRIPT                   CHARACTER         40           N
   17  CONTACT                    CHARACTER         20           N
   18  TYPE                       CHARACTER          1           N
   19  MEDREP                     CHARACTER          6           N
   20  STATUS                     CHARACTER          3           N
   21  TERMS                      NUMERIC            3           N
   22  VAT                        NUMERIC            1           N
   23  FIVE                       NUMERIC            1           N
   24  THREE                      NUMERIC            1           N
   25  PERCENTAGE                 NUMERIC            2           N
   26  REBATE                     NUMERIC            2           N
   27  VIN                        CHARACTER         20           N
	28  DIS_DAYS                   NUMERIC            2           N
   29  DIS_PERCT                  NUMERIC            2           N
   30  UPDATE                     DATE               8           N
   31  USER                       CHARACTER         10           N
   32  TIME                       CHARACTER         26           N
   33  EX_ACCT                    CHARACTER          7           N
	34  EX_DEPT                    CHARACTER          4           N
   35  VAT_NUM                    CHARACTER         20           N
------------------------------------------------------------------
** Total **                                        400*/
  Tapvendor(Code4 *cb,LPSTR name=0):Data4(cb,"ap_vend")
	{
	 if(!isValid())
	  return;
	 init();
	};
  LPSTR GetDbfName(){ return "ap_vend";}
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

	~Tapvendor(){
	}
	virtual int iVat() {return int ( Str4ptr(vat.str()) );}
	virtual double dPercentage(){return double ( Str4ptr(percentage.str()) );}
	virtual double dRebate(){return double ( Str4ptr(rebate.str()) );}
	inline void SetIndexVendor(){SetIndex("vendorno");}
 };
#endif