#ifndef _DB_ARCTL_H
#define _DB_ARCTL_H
_CLASSDEF(Tarctrl)
class Tarctrl : public Data4
{
 public:
   Field4 *account;
   Field4 *depart;
   Field4 *amount;

  Tarctrl(Code4 *cb,LPSTR name=0) : Data4 (cb,"ar_ctrl")
  {
   SetIndex("account");
   account   = (Field4 *) new Field4(*(Data4 *)this,"account");
   depart    = (Field4 *) new Field4(*(Data4 *)this,"depart");
   amount    = (Field4 *) new Field4(*(Data4 *)this,"amount");
  }
  double dAmount()  {return double (Str4ptr(amount->str()));};

};
#endif
