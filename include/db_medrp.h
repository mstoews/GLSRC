#ifndef __DB_MEDRP_H
#define __DB_MEDRP_H
_CLASSDEF (Tglmedreps)
class Tglmedreps : public Data4
{
 public:
  Field4 *repcode;
  Field4 *rpshort;
  Field4 *rpname;
  Field4 *address1;
  Field4 *address2;
  Field4 *address3;
  Field4 *postal;
  Field4 *phone;
  Field4 *fax;
  Field4 *contact;
  Field4 *ref;
  Field4 *status;
  Field4 *descript;
  Field4 *terms;
  Field4 *balance;
  Field4 *creditlmt;

  Tglmedreps(Code4 *cb,LPSTR name=0):Data4(cb,"gl_medrp")
   {
    SetIndex("repcode");
    top();
    repcode    = (Field4 *) new Field4(*(Data4 *) this,"repcode");
    rpshort    = (Field4 *) new Field4(*(Data4 *) this,"rpshort");
    rpname     = (Field4 *) new Field4(*(Data4 *) this,"rpname");
    address1   = (Field4 *) new Field4(*(Data4 *) this,"address1");
    address2   = (Field4 *) new Field4(*(Data4 *) this,"address2");
    address3   = (Field4 *) new Field4(*(Data4 *) this,"address3");
    postal     = (Field4 *) new Field4(*(Data4 *) this,"postal");
    phone      = (Field4 *) new Field4(*(Data4 *) this,"phone");
    fax        = (Field4 *) new Field4(*(Data4 *) this,"fax");
    contact    = (Field4 *) new Field4(*(Data4 *) this,"contact");
    ref        = (Field4 *) new Field4(*(Data4 *) this,"ref");
    status     = (Field4 *) new Field4(*(Data4 *) this,"status");
    descript   = (Field4 *) new Field4(*(Data4 *) this,"descript");
    terms      = (Field4 *) new Field4(*(Data4 *) this,"terms");
    balance    = (Field4 *) new Field4(*(Data4 *) this,"balance");
    creditlmt  = (Field4 *) new Field4(*(Data4 *) this,"creditlmt");
   };
   ~Tglmedreps(){close();};
 };
#endif