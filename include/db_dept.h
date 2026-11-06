_CLASSDEF (Tgldept)
class Tgldept : public Data4
{
 public:
 Field4 *dept;
 Field4 *descript;
 Tgldept(Code4 *cb,LPSTR name=0):Data4(cb,"gldept")
 {
  SetIndex("depart");
  top();
  dept     = (Field4 *) new Field4(*(Data4 *)this,"depart");
  descript = (Field4 *) new Field4(*(Data4 *)this,"descript");
 };
 ~Tgldept(){close();};
 };

