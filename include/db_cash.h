#ifndef __DB_CASH_H
#define __DB_CASH_H
_CLASSDEF(Tapcash)
class Tapcash : public Data4
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
	Field4 *chequeno;
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
	Field4 *vatacct;
	Field4 *vatdept;

  Tapcash(Code4 *cb,LPSTR name=0) : Data4 (cb,"ap_cash")
  {
	SetIndex("batchno");

	batchno 		= new Field4 (*(Data4 *) this, 1 );
	mode			= new Field4 (*(Data4 *) this, 2 );
	type 			= new Field4 (*(Data4 *) this, 3 );
	adj 			= new Field4 (*(Data4 *) this, 4 );
	period 		= new Field4 (*(Data4 *) this, 5 );
	vendorno 	= new Field4 (*(Data4 *) this, 6 );
	vdname 		= new Field4 (*(Data4 *) this, 7 );
	invoiceno 	= new Field4 (*(Data4 *) this, 8 );
	chequeno 	= new Field4 (*(Data4 *) this, 9 );
	orderno 		= new Field4 (*(Data4 *) this, 10 );
	duedate 		= new Field4 (*(Data4 *) this, 11 );
	invdate 		= new Field4 (*(Data4 *) this, 12 );
	reference 	= new Field4 (*(Data4 *) this, 13 );
	descript 	= new Field4 (*(Data4 *) this, 14 );
	date 			= new Field4 (*(Data4 *) this, 15 );
	account 		= new Field4 (*(Data4 *) this, 16 );
	depart 		= new Field4 (*(Data4 *) this, 17 );
	payable 		= new Field4 (*(Data4 *) this, 18 );
	rebateamt 	= new Field4 (*(Data4 *) this, 19 );
	entryno 		= new Field4 (*(Data4 *) this, 20 );
	vatpercent 	= new Field4 (*(Data4 *) this, 21 );
	rebate 		= new Field4 (*(Data4 *) this, 22 );
	terms 		= new Field4 (*(Data4 *) this, 23 );
	vatamt 		= new Field4 (*(Data4 *) this, 24 );
	vatacct		= new Field4 (*(Data4 *) this, 25 );
	vatdept 		= new Field4 (*(Data4 *) this, 26 );

  };
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dPayable() { return double (Str4ptr(payable->str()));};
  BOOL Open();
};



_CLASSDEF(Tapcashd)
class Tapcashd :public Data4
{
 public:
  int iCurrentRecord;

  Field4 *batchno;
  Field4 *entryno;
  Field4 *descript;
  Field4 *medreps;
  Field4 *product;
  Field4 *drcodes;
  Field4 *account;
  Field4 *dept;
  Field4 *debit;
  Field4 *credit;


  Tapcashd(Code4 *cb,LPSTR name=0):Data4 (cb,"ap_cashd")
  {
  SetIndex("batchno");
  batchno 	= new Field4 (*(Data4 *) this, 1 );
  entryno 	= new Field4 (*(Data4 *) this, 2 );
  descript 	= new Field4 (*(Data4 *) this, 3 );
  medreps 	= new Field4 (*(Data4 *) this, 4 );
  product 	= new Field4 (*(Data4 *) this, 5 );
  drcodes 	= new Field4 (*(Data4 *) this, 6 );
  account 	= new Field4 (*(Data4 *) this, 7 );
  dept 		= new Field4 (*(Data4 *) this, 8 );
  debit 		= new Field4 (*(Data4 *) this, 9 );
  credit 	= new Field4 (*(Data4 *) this, 10 );

  iCurrentRecord = 0;
  };
 ~Tapcashd(){close();};
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dDebit(){return double (Str4ptr(debit->str()));};
  double dCredit() {return double (Str4ptr( credit->str()));};
  int    iJournal(){return int    (Str4ptr(entryno->str()));};

};
#endif