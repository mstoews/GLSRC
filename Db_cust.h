#ifndef _DB_CUST_H
#define _DB_CUST_H
_CLASSDEF (Tarcust)
class Tarcust : public Data4
{
 public:
  Field4  vendorno;
  Field4  vdshort;
  Field4  vdname;
  Field4  address1;
  Field4  address2;
  Field4  address3;
  Field4  postal;
  Field4  phone;
  Field4  fax;
  Field4  s_add1;
  Field4  s_add2;
  Field4  s_add3;
  Field4  s_postal;
  Field4  s_phone;
  Field4  s_fax;
  Field4  depart;
  Field4  account;
  Field4  ar_dept;
  Field4  ar_acct;
  Field4  vat_acct;
  Field4  vat_dept;
  Field4  contact;
  Field4  terms;
  Field4  terms_days;
  Field4  terms_perc;
  Field4  terms_txt;
  Field4  credit;
  Field4  vat;
  Field4  vin;
  Field4  withtax;
  Field4  withdept;
/*
Structure for table   C:\GL\BOIE\AR_CUST.DBF
Table type            DBASE
Number of records     1026
Last update           96/04/08
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  VENDORNO                   CHARACTER          6           Y
	 2  VDSHORT                    CHARACTER         12           N
	 3  VDNAME                     CHARACTER         40           N
	 4  ADDRESS1                   CHARACTER         30           N
	 5  ADDRESS2                   CHARACTER         30           N
	 6  ADDRESS3                   CHARACTER         30           N
	 7  POSTAL                     CHARACTER          8           N
	 8  PHONE                      CHARACTER         16           N
	 9  FAX                        CHARACTER         16           N
	10  S_ADD1                     CHARACTER         40           N
	11  S_ADD2                     CHARACTER         40           N
	12  S_ADD3                     CHARACTER         40           N
	13  S_POSTAL                   CHARACTER          8           N
	14  S_PHONE                    CHARACTER         16           N
	15  S_FAX                      CHARACTER         16           N
	16  DEPART                     CHARACTER          4           N
	17  ACCOUNT                    CHARACTER          7           N
	18  AR_DEPT                    CHARACTER          4           N
	19  AR_ACCT                    CHARACTER          7           N
	20  VAT_ACCT                   CHARACTER          7           N
	21  VAT_DEPT                   CHARACTER          4           N
	22  CONTACT                    CHARACTER         12           N
	23  TERMS                      NUMERIC            3           N
	24  TERMS_DAYS                 NUMERIC            3           N
	25  TERMS_PERC                 NUMERIC            2           N
	26  TERMS_TXT                  CHARACTER         20           N
	27  CREDIT                     NUMERIC           16    2      N
	28  VAT                        NUMERIC            3           N
	29  VIN                        CHARACTER         20           N
	30  WITHTAX                    CHARACTER          7           N
	31  WITHDEPT                   CHARACTER          4           N
	32  UPDATE                     DATE               8           N
	33  USER                       CHARACTER         10           N
	34  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        516

  */
  inline void init()
  {
		vendorno.init(data, 1 );
		vdshort.init(data, 2 );
		vdname	.init(data, 3 );
		address1.init(data, 4 );
		address2.init(data, 5 );
		address3.init(data, 6 );
		postal	.init(data, 7 );
		phone	.init(data, 8 );
		fax		.init(data, 9 );
		s_add1	.init(data, 10 );
		s_add2	.init(data, 11 );
		s_add3	.init(data, 12 );
		s_postal.init(data, 13 );
		s_phone.init(data, 14 );
		s_fax	.init(data, 15 );
		depart	.init(data, 16 );
		account.init(data, 17 );
		ar_dept.init(data, 18 );
		ar_acct.init(data, 19 );
		vat_acct.init(data, 20 );
		vat_dept.init(data, 21 );
		contact.init(data, 22 );
		terms	.init(data, 23 );
		terms_days.init(data, 24 );
		terms_perc.init(data,25 );
		terms_txt.init(data, 26 );
		credit	.init(data, 27 );
		vat		.init(data, 28 );
		vin		.init(data, 29 );
		withtax.init(data, 30 );
		withdept.init(data, 31 );
		SetIndexVendorno();
  }
  Tarcust(){};
  Tarcust(Code4 *cb,LPSTR name=0):Data4(cb,"ar_cust")
	{
	 if(isValid())
	 {
		 init();
	 }

	};
	~Tarcust()
	{
	 // close();
 }
	int open(Code4& cb);
	virtual int iVat() {return int ( Str4ptr(vat.str()) );}
	inline void SetIndexVendorno(){SetIndex("vendorno");}
	inline LPSTR GetDbfName(){return "ar_cust";}
 };
#endif
