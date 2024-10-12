#ifndef __DB_DRCOD_H
#define __DB_DRCOD_H
_CLASSDEF (Tgldrcodes)
class Tgldrcodes : public Data4
{
 public:
  Field4 *drcode;
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
  Field4 *territory;

  Tgldrcodes(Code4 *cb,LPSTR name=0):Data4(cb,"gl_drcod")
   {
    SetIndex("drcode");
    top();
    drcode    = (Field4 *) new Field4(*(Data4 *) this,1);
    rpshort    = (Field4 *) new Field4(*(Data4 *) this,2);
    rpname     = (Field4 *) new Field4(*(Data4 *) this,3);
    address1   = (Field4 *) new Field4(*(Data4 *) this,4);
    address2   = (Field4 *) new Field4(*(Data4 *) this,5);
    address3   = (Field4 *) new Field4(*(Data4 *) this,6);
    postal     = (Field4 *) new Field4(*(Data4 *) this,7);
    phone      = (Field4 *) new Field4(*(Data4 *) this,8);
    fax        = (Field4 *) new Field4(*(Data4 *) this,9);
    contact    = (Field4 *) new Field4(*(Data4 *) this,10);
    ref        = (Field4 *) new Field4(*(Data4 *) this,11);
    status     = (Field4 *) new Field4(*(Data4 *) this,12);
    descript   = (Field4 *) new Field4(*(Data4 *) this,13);
    terms      = (Field4 *) new Field4(*(Data4 *) this,14);
    balance    = (Field4 *) new Field4(*(Data4 *) this,15);
    creditlmt  = (Field4 *) new Field4(*(Data4 *) this,16);
    territory  = (Field4 *) new Field4(*(Data4 *) this,17);
   };
   ~Tgldrcodes(){close();};
 };
#endif