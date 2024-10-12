#ifndef __DB_APBTC_H
#define __DB_APBTC_H
extern HINSTANCE hInst;


_CLASSDEF(Tapbtch)
class Tapbtch : public Data4
{
 public:
   Field4  batchno;
	Field4  mode;
	Field4  type;
	Field4  adj;
	Field4  period;
	Field4  vendorno;
	Field4  vdname;
	Field4  invoiceno;
	Field4  orderno;
	Field4  duedate;
	Field4  invdate;
	Field4  terms;
	Field4  vatpercent;
	Field4  rebate;
	Field4  date;
	Field4  reference;
	Field4  account;
	Field4  depart;
	Field4  descript;
	Field4  payable;
	Field4  rebateamt;
	Field4  vatamt;
	Field4  adjamt;
	Field4  entryno;
	//Field4  fiscalyr;

  inline BOOL init(void)
  {
	int i = 1;
	batchno.init(data,1);
	mode.init(data,2);
	type.init(data,3);
	adj.init(data,4);
	period.init(data,5);
	vendorno.init(data,6);
	vdname.init(data,7);
	invoiceno.init(data,8);
	orderno.init(data,9);
	duedate.init(data,10);
	invdate.init(data,11);
	reference.init(data,12);
	descript.init(data,13);
	date.init(data,14);
	account.init(data,15);
	depart.init(data,16);
	payable.init(data,17);
	rebateamt.init(data,18);
	entryno.init(data,19);
	vatpercent.init(data,20);
	rebate.init(data,21);
	terms.init(data,22);
	vatamt.init(data,23);
	adjamt.init(data,24);
	return TRUE;
  }

  Tapbtch(){};
  Tapbtch(Code4 *cb,LPSTR name=0) : Data4 (cb,"AP_BTCH")
  {
  };

  long lBatchno() { return long (Str4ptr(batchno.str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt.str()));};
  double dPayable(){ return double (Str4ptr(payable.str()));};
  double dVatamt()       { return double (Str4ptr(vatamt.str()));};
  double dAdjamt(){ return double (Str4ptr(adjamt.str()));};
  int  iPeriod() { return int (Str4ptr(period.str()));};
  void SetIndexBatchno();
  BOOL Open();
  BOOL Posted();
  BOOL Closed();
  BOOL New(long iBatchno);
  LPSTR GetDbfName(){ return "AP_BTCH";}
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

  ~Tapbtch()
  {
  }
};



_CLASSDEF(Tapbtchd)
class Tapbtchd :public Data4
{
 public:
  int iCurrentRecord;

  Field4  batchno;
  Field4  entryno;
  Field4  descript;
  Field4  account;
  Field4  product;
  Field4  depart;
  Field4  drcodes;
  Field4  dept;
  Field4  debit;
  Field4  credit;
  /*
  D:\AMMIC
Structure for table   D:\AMMIC\AP_BTCHD.DBF
Table type            DBASE
Number of records     526
Last update           10/31/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  BATCHNO                    NUMERIC            6           Y
	 2  ENTRYNO                    NUMERIC            3           N
	 3  DESCRIPT                   CHARACTER         40           N
	 4  MEDREP                     CHARACTER          5           N
	 5  DRCODE                     CHARACTER          5           N
	 6  PRODUCT                    CHARACTER          7           N
	 7  ACCOUNT                    CHARACTER          7           N
	 8  DEPT                       CHARACTER          4           N
	 9  DEBIT                      NUMERIC           16    2      N
	10  CREDIT                     NUMERIC           16    2      N
------------------------------------------------------------------
** Total **                                        110

*/
  BOOL init(void)
  {
	batchno.init(data,1);
	entryno.init(data,2);
	descript.init(data,3);
	depart.init(data,4);
	drcodes.init(data,5);
	product.init(data,6);
	account.init(data,7);
	dept.init(data,8);
	debit.init(data,9);
	credit.init(data,10);
	iCurrentRecord = 0;
	 return TRUE;
  }
  Tapbtchd(){};
  Tapbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,GetString(hInst,8187))
  {
	if(!isValid())
		return;
	init();
  };
 ~Tapbtchd(){
 }
  LPSTR GetDbfName(){ return GetString(hInst,8187);}
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

  long lBatchno() { return long (Str4ptr(batchno.str()));};
  double dCredit(){ return double (Str4ptr(credit.str()));};
  double dDebit(){ return double (Str4ptr(debit.str()));};
  long iJournal() { return long (Str4ptr(entryno.str()));};

};

_CLASSDEF(Tap_suppl)
class Tap_suppl :public Data4
{
 public:
  Field4  batchno;
  Field4  product;
  Field4  medreps;
  Field4  drcodes;
  BOOL init(void)
  {
	batchno.init(data,1);
	medreps.init(data,4);
	drcodes.init(data,5);
	product.init(data,6);
   return TRUE;
  }
  Tap_suppl(Code4 *cb,LPSTR name=0):Data4 (cb,"ap_suppl")
  {
	if(!isValid())
		return;
	 init();
  };
 ~Tap_suppl()
 {
	close();
 }
  long lBatchno() { return long (Str4ptr(batchno.str()));};
};

#endif