_CLASSDEF(Tglcrept)
class Tglcrept : public Data4
{
  public:
  Field4 *descript;
  Field4 *cname;
  Field4 *fname;

  Tglcrept(Code4 *cb,LPSTR name=0):Data4(cb,"gl_rept")
  {  
  SetIndex("descript");
  top();  
  descript = (Field4 *) new Field4(*(Data4 *)this,"descript");
  cname     = (Field4 *) new Field4(*(Data4 *)this,"name");
  fname    = (Field4 *) new Field4(*(Data4 *)this,"fname");
  }
};


