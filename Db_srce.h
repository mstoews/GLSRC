_CLASSDEF (Tglsource)
class Tglsource : public Data4
{
public:
 Field4 *sourcecode;
 Field4 *reference;
 Field4 *descript;
 Field4 *account;
 Field4 *dept;
 Tglsource(Code4 *cb,LPSTR name=0):Data4(cb,"gl_srcc")
 {
  SetIndex("sourcecode");
  top();  
  sourcecode = (Field4 *) new Field4(*(Data4 *)this,"sourcecode");
  reference  = (Field4 *) new Field4(*(Data4 *)this,"reference");
  descript   = (Field4 *) new Field4(*(Data4 *)this,"descript");
  account    = (Field4 *) new Field4(*(Data4 *)this,"account");
  dept       = (Field4 *) new Field4(*(Data4 *)this,"dept");
 };
 ~Tglsource()
  {
   //close();
 };
};

