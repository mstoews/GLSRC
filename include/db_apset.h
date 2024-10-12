_CLASSDEF(Tapsetup)
class Tapsetup : public Data4
{
 public:
 Field4 *name1;
 Field4 *account;
 Field4 *depart;
 Field4 *type;

  Tapsetup(Code4 *cb,LPSTR name=0) : Data4 (cb,"ap_setup")
  {
   SetIndex("name");
   name1     = (Field4 *) new Field4(*(Data4 *)this,"name");
   account   = (Field4 *) new Field4(*(Data4 *)this,"account");
   depart    = (Field4 *) new Field4(*(Data4 *)this,"dept");
   type      = (Field4 *) new Field4(*(Data4 *)this,"type");
  };
};

