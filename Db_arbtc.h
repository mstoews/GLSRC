#ifndef _DB_ARBTC_H
#define _DB_ARBTC_H



_CLASSDEF(Tarbtch)
class Tarbtch : public Data4
{
 public:
	Field4  batchno;
	Field4  mode;
	Field4  type;
	Field4  adj;
	Field4  period;
	Field4  vendorno;
	Field4  invoiceno;
	Field4  orderno;
	Field4  orderdate;
	Field4  vin;
	Field4  units;
	Field4  duedate;
	Field4  invdate;
	Field4  reference;
	Field4  descript;
	Field4  date;
	Field4  account;
	Field4  depart;
	Field4  sales_acct;
	Field4  sales_dept;
	Field4  payable;
	Field4  rebateamt;
	Field4  entryno;
	Field4  vatpercent;
	Field4  rebate;
	Field4  terms;
	Field4  vatamt;
	Field4  adjamt;
	Field4  qty;
	//Field4memo memo;
/*
Table type            DBASE
Number of records     2450
Last update           13/08/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  BATCHNO                    NUMERIC            6           Y
	 2  MODE                       CHARACTER          1           N
	 3  TYPE                       CHARACTER          2           N
	 4  ADJ                        CHARACTER          1           N
	 5  PERIOD                     NUMERIC            2           N
	 6  VENDORNO                   CHARACTER          6           N
	 7  INVOICENO                  CHARACTER          8           N
	 8  ORDERNO                    CHARACTER         10           N
	 9  ORDERDATE                  DATE               8           N
	10  VIN                        CHARACTER         10           N
	11  UNITS                      CHARACTER          7           N
	12  DUEDATE                    DATE               8           N
	13  INVDATE                    DATE               8           N
	14  REFERENCE                  CHARACTER         40           N
	15  DESCRIPT                   CHARACTER         10           N
	16  DATE                       DATE               8           N
	17  ACCOUNT                    CHARACTER          7           N
	18  DEPART                     CHARACTER          4           N
	19  SALES_ACCT                 CHARACTER          7           N
	20  SALES_DEPT                 CHARACTER          4           N
	21  PAYABLE                    NUMERIC           16    2      N
	22  REBATEAMT                  NUMERIC           16    2      N
	23  ENTRYNO                    NUMERIC            3           N
	24  VATPERCENT                 NUMERIC            2           N
	25  REBATE                     NUMERIC            2           N
	26  TERMS                      NUMERIC            3           N
	27  VATAMT                     NUMERIC           16    2      N
	28  ADJAMT                     NUMERIC           16    2      N
   29  QTY                        NUMERIC           16    2      N
   30  UPDATE                     DATE               8           N
   31  USER                       CHARACTER         10           N
   32  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        292

*/
  inline void init()
  {
	batchno.init(data,1);
	mode.init(data,2);
	type.init(data,3);
	adj.init(data,4);
	period.init(data,5);
	vendorno.init(data,6);
	invoiceno.init(data,7);
	orderno.init(data,8);
	orderdate.init(data,9);
	vin.init(data,10);
	units.init(data,11);
	duedate.init(data,12);
	invdate.init(data,13);
	reference.init(data,14);
	descript.init(data,15);
	date.init(data,16);
	account.init(data,17);
	depart.init(data,18);
	sales_acct.init(data,19);
	sales_dept.init(data,20);
	payable.init(data,21);
	rebateamt.init(data,22);
	entryno.init(data,23);
	vatpercent.init(data,24);
	rebate.init(data,25);
	terms.init(data,26);
	vatamt.init(data,27);
	adjamt.init(data,28);
	qty.init(data,29);
	//memo.init(data,30);

  }
  Tarbtch(){};
  Tarbtch(Code4 *cb,LPSTR name=0): Data4 (cb,"ar_btch")
  {
	if(!isValid())
	 return;
	else
	 init();
  };

  LPSTR GetDbfName(){ return "AR_BTCH";}
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
  double dPayable(){ return double (Str4ptr(payable.str()));};
  double dVatamt(){ return double (Str4ptr(vatamt.str()));};
  double dVat(){ return double (Str4ptr(vatamt.str()));};
  int iPeriod(){return int(Str4ptr(period.str()));};
  void SetIndexBatchno() { SetIndex("Batchno");};
  BOOL Open();
  BOOL Posted();
  BOOL New(long iBatchno);
  ~Tarbtch()
  {

  }
};



_CLASSDEF(Tarbtchd)
class Tarbtchd :public Data4
{
 public:
  int iCurrentRecord;

  Field4  batchno;
  Field4  entryno;
  Field4  descript;
  Field4  depart;
  Field4  drcode;
  Field4  product;
  Field4  qty;
  Field4  price;
  Field4  account;
  Field4  dept;
  Field4  debit;
  Field4  credit;

/*
Structure for table   C:\GL\DBF\AR_BTCHD.DBF
Table type            DBASE
Number of records     0
Last update           96/02/01
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  BATCHNO                    NUMERIC            6           Y
	 2  ENTRYNO                    NUMERIC            3           N
	 3  DESCRIPT                   CHARACTER         40           N
	 4  MEDREP                     CHARACTER          5           N
	 5  PRODUCT                    CHARACTER          7           N
	 6  DRCODE                     CHARACTER          5           N
	 7  QTY                        NUMERIC           14    2      N
	 8  PRICE                      NUMERIC           14    2      N
	 9  ACCOUNT                    CHARACTER          7           N
	10  DEPT                       CHARACTER          4           N
	11  DEBIT                      NUMERIC           16    2      N
	12  CREDIT                     NUMERIC           16    2      N
------------------------------------------------------------------
** Total **                                        138

*/

  inline void init()
  {
	batchno.init(data,1);
  entryno.init(data,2);
  descript.init(data,3);
  depart.init(data,4);
  product.init(data,5);
  drcode.init(data,6);
  qty.init(data,7);
  price.init(data,8);
  account.init(data,9);
  dept.init(data,10);
  debit.init(data,11);
  credit.init(data,12);
  iCurrentRecord = 0;

  }
  LPSTR GetDbfName(){ return "AR_BTCHD";}
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

  Tarbtchd(){};
  Tarbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_btchd")
  {
	if(!isValid())
	 return;
	init();
  };
 ~Tarbtchd()
  {

  }
  long lBatchno() { return long (Str4ptr(batchno.str()));};
  double dCredit(){ return double (Str4ptr(credit.str()));};
  double dDebit(){ return double (Str4ptr(debit.str()));};
  long iJournal() { return long (Str4ptr(entryno.str()));};
  };

_CLASSDEF(Tar_suppl)
class Tar_suppl :public Data4
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
  Tar_suppl(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_suppl")
  {
	if(!isValid())
		return;
	 init();
  };
 ~Tar_suppl()
 {
 }
  long lBatchno() { return long (Str4ptr(batchno.str()));};
};


#endif
