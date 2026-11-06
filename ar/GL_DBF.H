

 _CLASSDEF(Tglbtch)
 class Tglbtch
 {
  public:
   d4Data glbtch;
   f4Field *batchno;
   f4Field *mode;
   f4Field *descript;
   f4Field *debit;
   f4Field *credit;
   f4Field *entryno;

  Tglbtch()
   {
   glbtch.open(&cb,"gl_btch");
   glbtch.top();
   batchno = (f4Field *) new f4Field(glbtch,"batchno");
   mode = (f4Field *) new f4Field(glbtch,"mode");
   descript = (f4Field *) new f4Field(glbtch,"descript");
   debit = (f4Field *) new f4Field(glbtch,"descript");
   credit = (f4Field *) new f4Field(glbtch,"credit");
   entryno = (f4Field *) new f4Field(glbtch,"entryno");
   };

  ~Tglbtch(){glbtch.close();};
   long Debit(){ long db; db = atol(debit->str()); return db;};
   long Credit(){ long db; db = atol(credit->str()); return db;};
   long Entryno(){ long db; db = atol(entryno->str()); return db;};
 };

 _CLASSDEF(Tglaccts)
 class Tglaccts
 {
  public:
  d4Data dbf;
  
  f4Field *accounts;
  f4Field *dept;
  f4Field *descript;
  f4Field *type;
  f4Field *open_bal;
  f4Field *curr_bal;

  Tglaccts(){
  dbf.open(&cb,"gl_accts");
  dbf.top();
  accounts = (f4Field *) new f4Field(dbf,"account");
  dept     = (f4Field *) new f4Field(dbf,"dept");
  descript = (f4Field *) new f4Field(dbf,"descript");
  type     = (f4Field *) new f4Field(dbf,"type");
  open_bal = (f4Field *) new f4Field(dbf,"open_bal");
  curr_bal = (f4Field *) new f4Field(dbf,"curr_bal");
  };
  ~Tglaccts(){dbf.close();};
  long Open_bal(){long db; db = atol(open_bal->str());return db;};
  long Curr_bal(){long db; db = atol(curr_bal->str());return db;};
  };


_CLASSDEF(Tglbtchd)
class Tglbtchd
{
 public:
  d4Data dbf;
    
  f4Field *batchno;
  f4Field *journal;
  f4Field *sourcecode;
  d4date  *date;
  f4Field *reference;
  f4Field *descript;
  f4Field *account;
  f4Field *dept;
  f4Field *debit;
  f4Field *credit;

  Tglbtchd(){
  batchno    = (f4Field *) new f4Field(dbf,"batchno");
  journal    = (f4Field *) new f4Field(dbf,"journal");
  sourcecode = (f4Field *) new f4Field(dbf,"sourcecode");
  date       = (d4date  *) new d4Date (dbf,"date");
  reference  = (f4Field *) new f4Field(dbf,"reference");
  descript   = (f4Field *) new f4Field(dbf,"descript");
  account    = (f4Field *) new f4Field(dbf,"account");
  dept       = (f4Field *) new f4Field(dbf,"dept");
  debit      = (f4Field *) new f4Field(dbf,"debit");
  credit     = (f4Field *) new f4Field(dbf,"credit");
  };
~Tglbtchd(){dbf.close();};
  long Debit(){ long db; db = atol(debit->str()); return db;};
  long Credit(){ long db; db = atol(credit->str()); return db;};
};

