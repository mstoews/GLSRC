#ifndef __DB_APBTC_H
#define __DB_APBTC_H
_CLASSDEF(Tapbtch)
class Tapbtch : public Data4
{
 public:
   Field4 *batchno;
   Field4 *mode;
   Field4 *adj;
   Field4 *period;
   Field4 *vendorno;
   Field4 *invoiceno;
   Field4 *orderno;
   Field4 *duedate;
   Field4 *invdate;
   Field4 *terms;
   Field4 *vatpercent;
   Field4 *rebate;
   Field4 *date;
   Field4 *reference;
   Field4 *account;
   Field4 *depart;
   Field4 *descript;
   Field4 *payable;
   Field4 *rebateamt;
   Field4 *vatamt;
   Field4 *adjamt;
   Field4 *entryno;
   Field4 *vin;
   Field4 *units;
   Field4 *orderdate;
   Field4memo *memo;


  Tapbtch(Code4 *cb,LPSTR name=0) : Data4 (cb,"ar_btch")
  {
   SetIndex("batchno");
   batchno   = (Field4 *) new Field4(*(Data4 *)this,"batchno");
   adj       = (Field4 *) new Field4(*(Data4 *)this,"adj");
   vendorno  = (Field4 *) new Field4(*(Data4 *)this,"vendorno");
   invoiceno = (Field4 *) new Field4(*(Data4 *)this,"invoiceno");
   orderno   = (Field4 *) new Field4(*(Data4 *)this,"orderno");
   duedate   = (Field4 *) new Field4(*(Data4 *)this,"duedate");
   invdate   = (Field4 *) new Field4(*(Data4 *)this,"invdate");
   mode      = (Field4 *) new Field4(*(Data4 *)this,"mode");
   period    = (Field4 *) new Field4(*(Data4 *)this,"period");
   date      = (Field4 *) new Field4(*(Data4 *)this,"date");
   terms     = (Field4 *) new Field4(*(Data4 *)this,"terms");
   vatpercent= (Field4 *) new Field4(*(Data4 *)this,"vatpercent");
   rebate    = (Field4 *) new Field4(*(Data4 *)this,"rebate");
   reference = (Field4 *) new Field4(*(Data4 *)this,"reference");
   account   = (Field4 *) new Field4(*(Data4 *)this,"account");
   depart    = (Field4 *) new Field4(*(Data4 *)this,"depart");
   descript  = (Field4 *) new Field4(*(Data4 *)this,"descript");
   payable   = (Field4 *) new Field4(*(Data4 *)this,"payable");
   rebateamt = (Field4 *) new Field4(*(Data4 *)this,"rebateamt");
   vatamt    = (Field4 *) new Field4(*(Data4 *)this,"vatamt");
   adjamt    = (Field4 *) new Field4(*(Data4 *)this,"adjamt");
   entryno   = (Field4 *) new Field4(*(Data4 *)this,"entryno");
   vin       = (Field4 *) new Field4(*(Data4 *)this,"vin");
   units     = (Field4 *) new Field4(*(Data4 *)this,"units");
   orderdate = (Field4 *) new Field4(*(Data4 *)this,"orderdate");
   memo      = (Field4memo *) new Field4(*(Data4 *)this, "memofield");

  };
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dRebateamt(){ return double (Str4ptr(rebateamt->str()));};
  double dPayable(){ return double (Str4ptr(payable->str()));};
  double dVatamt()       { return double (Str4ptr(vatamt->str()));};
  double dAdjamt(){ return double (Str4ptr(adjamt->str()));};
  int  iPeriod() { return int (Str4ptr(period->str()));};
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
  Field4 *product;
  Field4 *medrep;
  Field4 *drcode;
  Field4 *dept;
  Field4 *debit;
  Field4 *credit;

  Tapbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_btchd")
  {
  SetIndex("batchno");
  batchno     =  new Field4(*(Data4 *)this,"batchno");
  entryno     =  new Field4(*(Data4 *)this,"entryno");
  descript    =  new Field4(*(Data4 *)this,"descript");
  account     =  new Field4(*(Data4 *)this,"account");
  dept        =  new Field4(*(Data4 *)this,"dept");
  debit       =  new Field4(*(Data4 *)this,"debit");
  credit      =  new Field4(*(Data4 *)this,"credit");
  iCurrentRecord = 0;
  };
 ~Tapbtchd(){close();};
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dCredit(){ return double (Str4ptr(credit->str()));};
  double dDebit(){ return double (Str4ptr(debit->str()));};
  long iJournal() { return long (Str4ptr(entryno->str()));};
};
#endif
