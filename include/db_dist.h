#ifndef __AR_JSUM_H
#define __AR_JSUM_H
_CLASSDEF(Tarjsum)
class Tgldist : public Data4
{
 public:
  Field4 *account;
  Field4 *dept;
  Field4 *mode;    // Indicates whether the entry has been posted to the GL 1 = not posted; 0 = posted.
  Field4 *type;
  Field4 *period;
  Field4 *date;
  Field4 *batchno;
  Field4 *reference;
  Field4 *cheque;
  Field4 *descript;
  Field4 *debit;
  Field4 *credit;
  Tgldist(Code4 *cb,LPSTR name=0):Data4(cb,"ar_jsum")
  {
   SetIndex("account");
   top();
   account   = (Field4 *) new Field4(*(Data4 *) this, "account");
   dept      = (Field4 *) new Field4(*(Data4 *) this, "dept");
   mode      = (Field4 *) new Field4(*(Data4 *) this, "mode");
   type      = (Field4 *) new Field4(*(Data4 *) this, "type");
   period    = (Field4 *) new Field4(*(Data4 *) this, "period");
   date      = (Field4 *) new Field4(*(Data4 *) this, "date");
   batchno   = (Field4 *) new Field4(*(Data4 *) this, "batchno");
   reference = (Field4 *) new Field4(*(Data4 *) this, "reference");
   cheque    = (Field4 *) new Field4(*(Data4 *) this,"cheque");
   descript  = (Field4 *) new Field4(*(Data4 *) this, "descript");
   debit     = (Field4 *) new Field4(*(Data4 *) this, "debit");
   credit    = (Field4 *) new Field4(*(Data4 *) this, "credit");
  };
  ~Tgldist(){close();};
 };



#endif
