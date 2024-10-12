#ifndef __DB_CASH_H
#define __DB_CASH_H
_CLASSDEF(Tapcash)
class Tapcash : public Data4
{
 public:
	Field4 batchno;
	Field4 mode;
	Field4 type;
	Field4 adj;
	Field4 period;
	Field4 vendorno;
	Field4 vdname;
	Field4 invoiceno;
	Field4 chequeno;
	Field4 orderno;
	Field4 duedate;
	Field4 invdate;
	Field4 reference;
	Field4 descript;
	Field4 date;
	Field4 account;
	Field4 depart;
	Field4 payable;
	Field4 rebateamt;
	Field4 entryno;
	Field4 vatpercent;
	Field4 rebate;
	Field4 terms;
	Field4 vatamt;
	Field4 vatacct;
	Field4 vatdept;

  Tapcash(Code4 *cb,LPSTR name=0) : Data4 (cb,"ap_cash")
  {
	if(!isValid())
	 return;
	else
	 init();
  };

  inline void init()
  {
	SetIndex("batchno");
	batchno 		.init(data, 1 );
	mode			.init(data, 2 );
	type 			.init(data, 3 );
	adj 			.init(data, 4 );
	period 		.init(data, 5 );
	vendorno 	.init(data, 6 );
	vdname 		.init(data, 7 );
	invoiceno 	.init(data, 8 );
	chequeno 	.init(data, 9 );
	orderno 		.init(data, 10 );
	duedate 		.init(data, 11 );
	invdate 		.init(data, 12 );
	reference 	.init(data, 13 );
	descript 	.init(data, 14 );
	date 			.init(data, 15 );
	account 		.init(data, 16 );
	depart 		.init(data, 17 );
	payable 		.init(data, 18 );
	rebateamt 	.init(data, 19 );
	entryno 		.init(data, 20 );
	vatpercent 	.init(data, 21 );
	rebate 		.init(data, 22 );
	terms 		.init(data, 23 );
	vatamt 		.init(data, 24 );
	vatacct		.init(data, 25 );
	vatdept 		.init(data, 26 );

  }
  long lBatchno     () { return long   (Str4ptr(batchno.str()));};
  double dPayable   () { return double (Str4ptr(payable.str()));};
  double dVatamt    () { return double (Str4ptr(vatamt.str()));};
  double dRebateamt () { return double (Str4ptr(rebateamt.str()));};
  BOOL Open();
  Tapcash(){};
  ~Tapcash()
  {
  }
  LPSTR GetDbfName(){ return "ap_cash";}
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


};



_CLASSDEF(Tapcashd)
class Tapcashd :public Data4
{
 public:
  long iCurrentRecord;

  Field4 batchno;
  Field4 entryno;
  Field4 descript;
  Field4 medreps;
  Field4 product;
  Field4 drcodes;
  Field4 account;
  Field4 dept;
  Field4 debit;
  Field4 credit;

  inline void init()
  {
	 SetIndex("batchno");
	 batchno 	.init(data, 1 );
	 entryno 	.init(data, 2 );
	 descript 	.init(data, 3 );
	 medreps 	.init(data, 4 );
	 product 	.init(data, 5 );
	 drcodes 	.init(data, 6 );
	 account 	.init(data, 7 );
	 dept 		.init(data, 8 );
	 debit 		.init(data, 9 );
	 credit 	.init(data, 10 );
  }
  Tapcashd(){};
  Tapcashd(Code4 *cb,LPSTR name=0):Data4 (cb,"ap_cashd")
  {
	if(!isValid())
	 return;
	else
	 init();
  iCurrentRecord = 0;
  };
 ~Tapcashd()
  {

  }
  long lBatchno() { return long (Str4ptr(batchno.str()));};
  double dDebit(){return double (Str4ptr(debit.str()));};
  double dCredit() {return double (Str4ptr( credit.str()));};
  int    iJournal(){return int    (Str4ptr(entryno.str()));};
  LPSTR GetDbfName(){ return "ap_cashd";}
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

};
#endif