_CLASSDEF(Tglbtchd)
class Tglbtchd :public Data4
{
 public:

  Field4 *batchno;
  Field4 *mode;
  Field4 *journal;
  Field4 *period;
  Field4 *sourcecode;
  Field4 *date;
  Field4 *reference;
  Field4 *descript;
  Field4 *cheque;
  Field4 *account;
  Field4 *dept;
  Field4 *debit;
  Field4 *credit;

  Tglbtchd(Code4 *cb,LPSTR name=0):Data4 (cb,"gl_btchd")
  {
  batchno    = (Field4 *) new Field4(*(Data4 *)this,"batchno");
  mode       = (Field4 *) new Field4(*(Data4 *)this,"mode");
  journal    = (Field4 *) new Field4(*(Data4 *)this,"journal");
  period     = (Field4 *) new Field4(*(Data4 *)this,"period");
  sourcecode = (Field4 *) new Field4(*(Data4 *)this,"sourcecode");
  date       = (Field4 *) new Field4(*(Data4 *)this,"date");
  reference  = (Field4 *) new Field4(*(Data4 *)this,"reference");
  descript   = (Field4 *) new Field4(*(Data4 *)this,"descript");
  cheque     = (Field4 *) new Field4(*(Data4 *)this,"cheque");
  account    = (Field4 *) new Field4(*(Data4 *)this,"account");
  account->trim();
  dept       = (Field4 *) new Field4(*(Data4 *)this,"dept");
  dept->trim();
  debit      = (Field4 *) new Field4(*(Data4 *)this,"debit");
  credit     = (Field4 *) new Field4(*(Data4 *)this,"credit");
  };
~Tglbtchd(){close();};
  double Batchno() { return double(Str4ptr(batchno->str()));};
  double Debit()   { return double(Str4ptr(debit->str()));};
  double Credit()  { return double(Str4ptr(credit->str()));};
};
