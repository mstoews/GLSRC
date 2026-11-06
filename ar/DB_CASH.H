_CLASSDEF(Tapcash)
class Tapcash : public Data4
{
 public:
   Field4 *batchno;
   Field4 *mode;
   Field4 *adj;
   Field4 *period;
   Field4 *vendorno;
   Field4 *vdname;
   Field4 *invoiceno;
   Field4 *chequeno;
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
   Field4 *entryno;


  Tapcash(Code4 *cb,LPSTR name=0) : Data4 (cb,"ar_cash")
  {
   SetIndex("batchno");
   batchno   = (Field4 *) new Field4(*(Data4 *)this,"batchno");
   adj       = (Field4 *) new Field4(*(Data4 *)this,"adj");
   vendorno  = (Field4 *) new Field4(*(Data4 *)this,"vendorno");
   vdname    = (Field4 *) new Field4(*(Data4 *)this,"vdname");
   invoiceno = (Field4 *) new Field4(*(Data4 *)this,"invoiceno");
   chequeno  = (Field4 *) new Field4(*(Data4 *)this,"chequeno");
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
   entryno   = (Field4 *) new Field4(*(Data4 *)this,"entryno");
  };
  long lBatchno() { return long (Str4ptr(batchno->str()));};
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
  Field4 *account;
  Field4 *dept;
  Field4 *debit;
  Field4 *credit;
  Field4 *drcodes;

  Tapcashd(Code4 *cb,LPSTR name=0):Data4 (cb,"ap_cashd")
  {
  SetIndex("batchno");
  batchno     = (Field4 *) new Field4(*(Data4 *)this,"batchno");
  entryno     = (Field4 *) new Field4(*(Data4 *)this,"entryno");
  descript    = (Field4 *) new Field4(*(Data4 *)this,"descript");
  account     = (Field4 *) new Field4(*(Data4 *)this,"account");
  dept        = (Field4 *) new Field4(*(Data4 *)this,"dept");
  debit       = (Field4 *) new Field4(*(Data4 *)this,"debit");
  credit      = (Field4 *) new Field4(*(Data4 *)this,"credit");
  drcodes     = new Field4(*(Data4 *)this,"drcodes");
  iCurrentRecord = 0;
  };
 ~Tapcashd(){close();};
  long lBatchno() { return long (Str4ptr(batchno->str()));};
  double dDebit(){return double (Str4ptr(debit->str()));};
  double dCredit() {return double (Str4ptr( credit->str()));};
  int    iJournal(){return int    (Str4ptr(entryno->str()));};

};
