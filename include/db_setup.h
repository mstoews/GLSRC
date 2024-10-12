_CLASSDEF (Tapsetup)
class Tapsetup : public Data4
{
 public:
  Field4 *company;
  Field4 *account;
  Field4 *dept;
  Field4 *cash;
  Field4 *cash_dept;
  Field4 *batchno;
  Tapsetup(Code4 *cb,LPSTR name=0):Data4(cb,"ap_setup")
  {
   //tag_select(tag("company"));
   top();
   company    = (Field4 *) new Field4(*(Data4 *) this, "company");
   account    = (Field4 *) new Field4(*(Data4 *) this, "account");
   dept       = (Field4 *) new Field4(*(Data4 *) this, "dept");
   cash       = (Field4 *) new Field4(*(Data4 *) this, "cash");
   cash_dept  = (Field4 *) new Field4(*(Data4 *) this, "cash_dept");
   batchno    = (Field4 *) new Field4(*(Data4 *) this, "batchno");
  };
  ~Tapsetup(){close();};
 };
