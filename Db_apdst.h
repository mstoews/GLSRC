#ifndef __DB_APDST_H
#define __DB_APDST_H
_CLASSDEF (Tapdist)
class Tapdist : public Data4
{
 public:
  Field4  account;
  Field4  dept;
  Field4  apaccount;
  Field4  apdepart;
  Field4  period;
  Field4  date;
  Field4  duedate;
  Field4  batchno;
  Field4  invoiceno;
  Field4  paymentctl;
  Field4  vendorno;
  Field4  chequeno;
  Field4  reference;
  Field4  descript;
  Field4  amount;
  Field4  paymentreq;
  Field4  datepaid;
  Field4  rebateamt;
  Field4  amountpaid;
  Field4  adjustment;
  Field4  vatamt;
  Field4  rebate;
  Field4  ex_acct;
  Field4  ex_dept;
  Field4  ex_amount;
  //Field4  fiscalyr;
  Tag4    VendorTag;

  Tapdist(){};
  inline void init()
  {
	 if(!isValid())
		return;
	account.init 	(data,  1);
	dept.init 		(data,  2);
	apaccount.init (data,  3);
	apdepart.init 	(data,  4);
	period.init 	(data,  5);
	date.init 		(data,  6);
	duedate.init 	(data,  7);
	batchno.init 	(data,  8);
	invoiceno.init (data,  9);
	paymentctl.init(data,  10);
	vendorno.init 	(data,  11);
	chequeno.init 	(data,  12);
	reference.init (data,  13);
	descript.init 	(data,  14);
	amount.init 	(data,  15);
	rebateamt.init (data,  16);
	amountpaid.init(data,  17);
	paymentreq.init(data,  18);
	datepaid.init 	(data,  19);
	vatamt.init 	(data,  20);
	adjustment.init(data,  21);
	rebate.init 	(data,  22);
	ex_acct.init   (data,  26);
	ex_dept.init   (data,  27);
	ex_amount.init (data,  28);
}


  Tapdist(Code4 *cb,LPSTR name=0):Data4(cb,"ap_dist")
  {
	 if(!isValid())
		return;
	 init();
/*
Structure for table   C:\GL\DBF\AP_DIST.DBF
Table type            DBASE
Number of records     799
Last update           11/13/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  ACCOUNT                    CHARACTER          7           Y
	 2  DEPT                       CHARACTER          4           Y
	 3  APACCOUNT                  CHARACTER          7           N
	 4  APDEPART                   CHARACTER          4           N
	 5  PERIOD                     NUMERIC            2           N
	 6  DATE                       DATE               8           N
	 7  DUEDATE                    DATE               8           N
	 8  BATCHNO                    NUMERIC            6           Y
	 9  INVOICENO                  CHARACTER          8           N
	10  PAYMENTCTL                 CHARACTER          1           N
	11  VENDORNO                   CHARACTER          6           Y
	12  CHEQUENO                   NUMERIC           10           Y
	13  REFERENCE                  CHARACTER         40           N
	14  DESCRIPT                   CHARACTER         40           N
	15  AMOUNT                     NUMERIC           16    2      N
	16  REBATEAMT                  NUMERIC           16    2      N
	17  AMOUNTPAID                 NUMERIC           16    2      N
	18  PAYMENTREQ                 NUMERIC           10           Y
	19  DATEPAID                   DATE               8           N
	20  VATAMT                     NUMERIC           16    2      N
	21  ADJUSTMENT                 NUMERIC           16    2      N
	22  REBATE                     CHARACTER          2           N
	23  UPDATE                     DATE               8           N
	24  USER                       CHARACTER         10           N
	25  TIME                       CHARACTER         26           N
	26  EX_ACCT                    CHARACTER          7           N
	27  EX_DEPT                    CHARACTER          4           N
	28  EX_AMOUNT                  NUMERIC           20    2      N
------------------------------------------------------------------
** Total **                                        327
*/
  };

  LPSTR GetDbfName(){ return "AP_DIST";}
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

  ~Tapdist()
	{
	 }
  double dAmount(){ return double (Str4ptr(amount.str()));};
  double dAmountpaid(){ return double (Str4ptr(amountpaid.str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt.str()));};
  double dAdjustment(){ return double (Str4ptr(adjustment.str()));};
  double dGainOrLoss(){ return double (Str4ptr(ex_amount.str()));};
  long   lRequest()   { return long   (Str4ptr(paymentreq.str()));};
  long   lChequeno()   { return long   (Str4ptr(chequeno.str()));};
  char   *szAmountDue()
  {
	  double dAmountDue = dAmount() - dAmountpaid();
	  Str4large strAmountPaid;
	  strAmountPaid.assignDouble(dAmountDue,16,2);
	  return strAmountPaid.str();
  }

  inline void SetIndexChequeno(){SetIndex("CHEQUENO");}
  inline void SetIndexInvoiceno(){SetIndex("INVOICENO");}
  inline void SetIndexVerified(){SetIndex("VERIFIED");}
  inline void SetIndexVerifiedCleared(){SetIndex("LASTCHEQNO");}
  inline void SetIndexVendorno(){SetIndex("VENDORNO");}

  BOOL Open()     {if(strcmp(paymentctl.str(),"O") == 0)return TRUE;else return FALSE;};
  BOOL Paid()     {if(strcmp(paymentctl.str(),"P") == 0)return TRUE;else return FALSE;};
  BOOL Requested(){if(strcmp(paymentctl.str(),"R") == 0)return TRUE;else return FALSE;};
  BOOL Held()     {if(strcmp(paymentctl.str(),"H") == 0)return TRUE;else return FALSE;};
  BOOL Forced()   {if(strcmp(paymentctl.str(),"F") == 0)return TRUE;else return FALSE;};
 };



_CLASSDEF (Tapreqst)
class Tapreqst : public Data4
{
 public:
  Field4  account;
  Field4  dept;
  Field4  apaccount;
  Field4  apdepart;
  Field4  period;
  Field4  date;
  Field4  batchno;
  Field4  invoiceno;
  Field4  paymentctl;
  Field4  vendorno;
  Field4  chequeno;
  Field4  amount;
  Tag4    VendorTag;

  Tapreqst(){};
  inline void init()
  {
	 if(!isValid())
		return;
	account.init 	(data,  1);
	dept.init 		(data,  2);
	apaccount.init (data,  3);
	apdepart.init 	(data,  4);
	period.init 	(data,  5);
	date.init 		(data,  6);
	batchno.init 	(data,  7);
	invoiceno.init (data,  8);
	paymentctl.init(data,  9);
	vendorno.init 	(data,  10);
	chequeno.init 	(data,  11);
	amount.init 	(data,  12);
  }
/*
Structure for table   D:\GL\DBF\AP_REQST.DBF
Table type            DBASE
Number of records     0
Last update           06/10/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  ACCOUNT                    CHARACTER          7           N
	 2  DEPT                       CHARACTER          4           N
	 3  APACCOUNT                  CHARACTER          7           N
	 4  APDEPART                   CHARACTER          4           N
	 5  PERIOD                     NUMERIC            2           N
	 6  DATE                       DATE               8           N
	 7  BATCHNO                    NUMERIC            6           N
	 8  INVOICENO                  CHARACTER          8           N
	 9  PAYMENTCTL                 CHARACTER          1           N
	10  VENDORNO                   CHARACTER          6           N
	11  CHEQUENO                   NUMERIC           10           N
	12  AMOUNT                     NUMERIC           16    2      N
	13  UPDATE                     DATE               8           N
	14  USER                       CHARACTER         10           N
	15  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        124
*/

  Tapreqst(Code4 *cb,LPSTR name=0):Data4(cb,"ap_reqst")
  {
	 if(!isValid())
		return;
	 init();

  };

  LPSTR GetDbfName(){ return "AP_REQST";}
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
  ~Tapreqst(){}
  double dAmount(){ return double (Str4ptr(amount.str()));};
 };
#endif
