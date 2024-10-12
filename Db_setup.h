_CLASSDEF (Tapsetup)
class Tapsetup : public Data4
{
 public:
  Field4 company;
  Field4 account;
  Field4 dept;
  Field4 cash;
  Field4 cash_dept;
  Field4 batchno;
  Tapsetup(Code4 *cb,LPSTR name=0):Data4(cb,"ap_setup")
  {
	if(!isValid())
	 return;
	top();
   company    init(data,"company");
   account    init(data,"account");
   dept       init(data,"dept");
   cash       init(data,"cash");
   cash_dept  init(data,"cash_dept");
	batchno    init(data,"batchno");
  };
  ~Tapsetup(){close();
  };
 };
