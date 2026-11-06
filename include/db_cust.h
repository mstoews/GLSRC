_CLASSDEF (Tarcust)
class Tarcust : public Data4
{
 public:
  Field4 *custno;
  Field4 *custshort;
  Field4 *custname;
  Field4 *address1;
  Field4 *address2;
  Field4 *address3;
  Field4 *postal;
  Field4 *phone;
  Field4 *contact;
  Field4 *ref;
  Field4 *status;
  Tarcust(Code4 *cb,LPSTR name=0):Data4(cb,"ar_cust")
   {
    SetIndex("custno");
    top();
    custno     = (Field4 *) new Field4(*(Data4 *) this,"custno");
    custshort  = (Field4 *) new Field4(*(Data4 *) this,"custshort");
    custname   = (Field4 *) new Field4(*(Data4 *) this,"custname");
    address1   = (Field4 *) new Field4(*(Data4 *) this,"address1");
    address2   = (Field4 *) new Field4(*(Data4 *) this,"address2");
    address3   = (Field4 *) new Field4(*(Data4 *) this,"address3");
    postal     = (Field4 *) new Field4(*(Data4 *) this,"postal");
    phone      = (Field4 *) new Field4(*(Data4 *) this,"phone");
    contact    = (Field4 *) new Field4(*(Data4 *) this,"contact");
    ref        = (Field4 *) new Field4(*(Data4 *) this,"ref");
    status     = (Field4 *) new Field4(*(Data4 *) this,"status");
   };
   ~Tarcust(){close();};
 };
