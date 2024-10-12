#ifndef __DB_APBTC_H
#define __DB_APBTC_H
_CLASSDEF(Tapbtch)
class Tapbtch : public Data4
{
 public:
	Field4 *batchno;
	Field4 *mode;
	Field4 *type;
	Field4 *adj;
	Field4 *period;
	Field4 *vendorno;
	Field4 *vdname;
	Field4 *invoiceno;
	Field4 *orderno;
	Field4 *duedate;
	Field4 *invdate;
	Field4 *reference;
	Field4 *descript;
	Field4 *date;
	Field4 *account;
	Field4 *depart;
	Field4 *payable;
	Field4 *rebateamt;
	Field4 *entryno;
	Field4 *vatpercent;
	Field4 *rebate;
	Field4 *terms;
	Field4 *vatamt;
	Field4 *adjamt;



  Tapbtch(Code4 *cb,LPSTR name=0) : Data4 (cb,"ap_btch")
  {
	SetIndex("batchno");
	batchno	 = new Field4 (*(Data4 *) this, 1 ); 
	mode		 = new Field4 (*(Data4 *) this, 2 );
	type		 = new Field4 (*(Data4 *) this, 3 );
	adj		 = new Field4 (*(Data4 *) this, 4 );
	period	 = new Field4 (*(Data4 *) this, 5 );
	vendorno	 = new Field4 (*(Data4 *) this, 6);
	vdname	 = new Field4 (*(Data4 *) this, 7 );
	invoiceno = new Field4 (*(Data4 *) this, 8 );
	orderno	 = new Field4 (*(Data4 *) this, 9 );
	duedate	 = new Field4 (*(Data4 *) this, 10 );
	invdate	 = new Field4 (*(Data4 *) this, 11 );
	reference = new Field4 (*(Data4 *) this, 12 );
	descript	 = new Field4 (*(Data4 *) this, 13 );
	date		 = new Field4 (*(Data4 *) this, 14 );
	account	 = new Field4 (*(Data4 *) this, 15 );
	depart	 = new Field4 (*(Data4 *) this, 16 );
	payable	 = new Field4 (*(Data4 *) this, 17 );
	rebateamt = new Field4 (*(Data4 *) this, 18 );
	entryno	 = new Field4 (*(Data4 *) this, 19 );
	vatpercent = new Field4 (*(Data4 *) this, 20 );
	rebate	 = new Field4 (*(Data4 *) this, 21 );
	terms		 = new Field4 (*(Data4 *) this, 22 );
	vatamt	 = new Field4 (*(Data4 *) this, 23 );
	adjamt	 = new Field4 (*(Data4 *) this, 24 );
  };
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt->str()));};
  double dPayable(){ return double (Str4ptr(payable->str()));};
  double dVatamt()       { return double (Str4ptr(vatamt->str()));};
  double dAdjamt(){ return double (Str4ptr(adjamt->str()));};
  int  iPeriod() { return int (Str4ptr(period->str()));};
  BOOL Open();
  BOOL Posted();
  BOOL Closed();
  BOOL New(long iBatchno);
};



_CLASSDEF(Tapbtchd)
class Tapbtchd :public Data4
{
 public:
  int iCurrentRecord;

  Field4 *batchno;
  Field4 *entryno;
  Field4 *descript;
  Field4 *account;
  Field4 *dept;
  Field4 *product;
  Field4 *medreps;
  Field4 *drcodes;
  Field4 *debit;
  Field4 *credit;

  Tapbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"ap_btchd")
  {
  SetIndex("batchno");
  batchno     = (Field4 *) new Field4(*(Data4 *)this,1);
  entryno     = (Field4 *) new Field4(*(Data4 *)this,2);
  descript    = (Field4 *) new Field4(*(Data4 *)this,3);
  medreps     = (Field4 *) new Field4(*(Data4 *)this,4);  
  product     = (Field4 *) new Field4(*(Data4 *)this,5);
  drcodes     = (Field4 *) new Field4(*(Data4 *)this,6);
  account     = (Field4 *) new Field4(*(Data4 *)this,7);
  dept        = (Field4 *) new Field4(*(Data4 *)this,8);
  debit       = (Field4 *) new Field4(*(Data4 *)this,9);
  credit      = (Field4 *) new Field4(*(Data4 *)this,10);
  iCurrentRecord = 0;
  };
 ~Tapbtchd(){close();};
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dCredit(){ return double (Str4ptr(credit->str()));};
  double dDebit(){ return double (Str4ptr(debit->str()));};
  long iJournal() { return long (Str4ptr(entryno->str()));};
};
#endif