_CLASSDEF(TIndex)
class TIndex : public Data4
{
 public:
 Field4 *fDbf;
 Field4 *fTag;
 Field4 *fDescript;
 Field4 *fType;

 TIndex(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_sys")
 {
  top();
  SetIndex("fdbf");
  fDbf        = (Field4 *) new Field4(*(Data4 *)this,"fdbf");
  fTag        = (Field4 *) new Field4(*(Data4 *)this,"ftag");
  fDescript   = (Field4 *) new Field4(*(Data4 *)this,"fdescript");
  fType       = (Field4 *) new Field4(*(Data4 *)this,"ftype");
 }
 ~TIndex(){;unlock(); close();};
};

_CLASSDEF(TTagx)
class TTagx : public Data4
{
 public:
 Field4 *fDbf;
 Field4 *fTagname;
 Field4 *fTag;
 Field4 *fFilter;
 Field4 *fUnique;
 Field4 *fDescending;

 TTagx(Code4 *cb,LPSTR name=0):Data4 (cb,"ar_tag")
 {
  top();
  SetIndex("fdbf");
  fDbf        = (Field4 *) new Field4(*(Data4 *)this,"fdbf");
  fTagname    = (Field4 *) new Field4(*(Data4 *)this,"tagname");
  fTag        = (Field4 *) new Field4(*(Data4 *)this,"Tag");
  fFilter     = (Field4 *) new Field4(*(Data4 *)this,"Filter");
  fUnique     = (Field4 *) new Field4(*(Data4 *)this,"Unique");
  fDescending = (Field4 *) new Field4(*(Data4 *)this,"Descending");
 }
 ~TTagx(){;unlock(); close();};
};

