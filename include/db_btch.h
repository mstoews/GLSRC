 _CLASSDEF(Tglbtch)
 class Tglbtch : public Data4
 {  
  public:
   Field4 *batchno;
   Field4 *mode;
   Field4 *period;
   Field4 *date;
   Field4 *descript;
   Field4 *debit;
   Field4 *credit;
   Field4 *entryno;
   Field4 *cheque;

 Tglbtch(Code4 *cb,LPSTR name=0):Data4(cb,"gl_btch")
   { 

   SetIndex("batchno");
   batchno   = (Field4 *) new Field4(*(Data4 *)this,"batchno");
   mode      = (Field4 *) new Field4(*(Data4 *)this,"mode");
   period    = (Field4 *) new Field4(*(Data4 *)this,"period");
   date      = (Field4 *) new Field4(*(Data4 *)this,"date");
   descript  = (Field4 *) new Field4(*(Data4 *)this,"descript");
   debit     = (Field4 *) new Field4(*(Data4 *)this,"debit");
   credit    = (Field4 *) new Field4(*(Data4 *)this,"credit");
   entryno   = (Field4 *) new Field4(*(Data4 *)this,"entryno");
   cheque    = (Field4 *) new Field4(*(Data4 *)this,"cheque");

   };

   ~Tglbtch(){close();};

   long Batchno() { long db; db = atol(batchno->str()); return db;};
   long Debit()   { long db; db = atol(debit->str()); return db;};
   long Credit()  { long db; db = atol(credit->str()); return db;};
   long Entryno() { long db; db = atol(entryno->str()); return db;};
 };
