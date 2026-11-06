#ifndef __AR_JSUM_H
#define __AR_JSUM_H
_CLASSDEF(Tarjsum)
class Tarjsum : public Data4
{
 public:
  Field4 *account;
  Field4 *dept;
  Field4 *debit;
  Field4 *credit;
  Field4 *mode;
  Tarjsum(Code4 *cb,LPSTR name=0):Data4(cb,"ar_jsum")
  {
   SetIndex("account");
   top();
   account   = (Field4 *) new Field4(*(Data4 *) this, "account");
   dept      = (Field4 *) new Field4(*(Data4 *) this, "dept");
   debit     = (Field4 *) new Field4(*(Data4 *) this, "debit");
   credit    = (Field4 *) new Field4(*(Data4 *) this, "credit");
   mode      = (Field4 *) new Field4(*(Data4 *) this, "mode");
  };
  ~Tarjsum(){close();};
   double Debit()   { return double(Str4ptr(debit->str()));};
   double Credit()  { return double(Str4ptr(credit->str ()));};

 };



#endif
