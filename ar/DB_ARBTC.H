#ifndef _DB_ARBTC_H
#define _DB_ARBTC_H
_CLASSDEF(Tapbtch)
class Tapbtch : public Data4
{
 public:
   Field4 *batchno;
   Field4 *mode;
   Field4 *period;
   Field4 *vendorno;
   Field4 *invoiceno;
   Field4 *orderno;
   Field4 *duedate;
   Field4 *invdate;
   Field4 *terms;
   Field4 *vatpercent;
   Field4 *date;
   Field4 *reference;
   Field4 *account;
   Field4 *depart;
   Field4 *descript;
   Field4 *payable;
   Field4 *vatamt;
	Field4 *entryno;
   Field4 *vin;
   Field4 *orderdate;
   Field4 *units;
   Field4memo *memo;
  Tapbtch(Code4 *cb,LPSTR name=0): Data4 (cb,"ar_btch")
  {
   SetIndex("batchno");
	batchno   = (Field4 *) new Field4(*(Data4 *)this,1);
	mode      = (Field4 *) new Field4(*(Data4 *)this,2);
	period    = (Field4 *) new Field4(*(Data4 *)this,5);
	vendorno  = (Field4 *) new Field4(*(Data4 *)this,6);
	invoiceno = (Field4 *) new Field4(*(Data4 *)this,7);
	orderno   = (Field4 *) new Field4(*(Data4 *)this,8);
	orderdate = (Field4 *) new Field4(*(Data4 *)this,9);
	vin       = (Field4 *) new Field4(*(Data4 *)this,10);
	units     = (Field4 *) new Field4(*(Data4 *)this,11);
	duedate   = (Field4 *) new Field4(*(Data4 *)this,12);
	invdate   = (Field4 *) new Field4(*(Data4 *)this,13);
	reference = (Field4 *) new Field4(*(Data4 *)this,14);
	descript  = (Field4 *) new Field4(*(Data4 *)this,15);
	date      = (Field4 *) new Field4(*(Data4 *)this,16);
	account   = (Field4 *) new Field4(*(Data4 *)this,17);
	depart    = (Field4 *) new Field4(*(Data4 *)this,18);
	payable   = (Field4 *) new Field4(*(Data4 *)this,19);
	entryno   = (Field4 *) new Field4(*(Data4 *)this,21);
	vatpercent= (Field4 *) new Field4(*(Data4 *)this,22);
	terms     = (Field4 *) new Field4(*(Data4 *)this,24);
	vatamt    = (Field4 *) new Field4(*(Data4 *)this,25);
	memo      = (Field4memo *) new Field4memo(*(Data4 *)this,27);
  };

  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dPayable(){ return double (Str4ptr(payable->str()));};
  double dVatamt(){ return double (Str4ptr(vatamt->str()));};
  double dVat(){ return double (Str4ptr(vatamt->str()));};
  BOOL Open();
  BOOL Posted();
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
  Field4 *drcode;
  Field4 *qty;
  Field4 *price;
  Field4 *debit;
  Field4 *credit;

  Tapbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_btchd")
  {
  SetIndex("batchno");
  batchno     =  new Field4(*(Data4 *)this,1);
  entryno     =  new Field4(*(Data4 *)this,2);
  descript    =  new Field4(*(Data4 *)this,3);
  medreps     =  new Field4(*(Data4 *)this,4);
  drcode      =  new Field4(*(Data4 *)this,5);
  product     =  new Field4(*(Data4 *)this,6);
  qty         =  new Field4(*(Data4 *)this,7);
  price       =  new Field4(*(Data4 *)this,8);
  account     =  new Field4(*(Data4 *)this,9);
  dept        =  new Field4(*(Data4 *)this,10);
  debit       =  new Field4(*(Data4 *)this,11);
  credit      =  new Field4(*(Data4 *)this,12);
  iCurrentRecord = 0;
  };
 ~Tapbtchd(){close();};
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dCredit(){ return double (Str4ptr(credit->str()));};
  double dDebit(){ return double (Str4ptr(debit->str()));};
  long iJournal() { return long (Str4ptr(entryno->str()));};
  };
#endif
