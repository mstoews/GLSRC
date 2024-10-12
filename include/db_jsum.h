#ifndef __GL_DIST_H
#define __GL_DIST_H
_CLASSDEF(Tgldist)
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
  Tgldist(Code4 *cb,LPSTR name=0):Data4(cb,"ap_jsum")
  {
   SetIndex("account");
   top();
   account   = (Field4 *) new Field4(*(Data4 *) this, 1);
   dept      = (Field4 *) new Field4(*(Data4 *) this, 2);
   period    = (Field4 *) new Field4(*(Data4 *) this, 3);
   mode      = (Field4 *) new Field4(*(Data4 *) this, 4);
   date      = (Field4 *) new Field4(*(Data4 *) this, 5);
   type      = (Field4 *) new Field4(*(Data4 *) this, 6);
   batchno   = (Field4 *) new Field4(*(Data4 *) this, 7);
   reference = (Field4 *) new Field4(*(Data4 *) this, 8);
   cheque    = (Field4 *) new Field4(*(Data4 *) this, 9);
   descript  = (Field4 *) new Field4(*(Data4 *) this, 10);
   debit     = (Field4 *) new Field4(*(Data4 *) this, 11);
   credit    = (Field4 *) new Field4(*(Data4 *) this, 12);
  };
  ~Tgldist(){close();};
 };



#endif
