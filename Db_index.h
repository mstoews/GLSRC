#ifndef __GL_SYS
#define __GL_SYS


_CLASSDEF(TGlsys)
class TGlsys : public Data4
{
 public:
 Field4 fDbf;
 Field4 fTag;
 Field4 fDescript;
 Field4 fType;
 TGlsys(){};
 TGlsys(Code4 *cb,LPSTR name=0):Data4 (cb,"gl_sys")
 {
  if(!isValid())
	 return;
  init();	 
 }
 void init(void)
 {
  fDbf        .init(data,1);
  fTag        .init(data,2);
  fDescript   .init(data,3);
  fType       .init(data,4);
 }
 ~TGlsys(){
 };
};

_CLASSDEF(TTagx)
class TTagx : public Data4
{
 public:
 Field4 fDbf;
 Field4 fTagname;
 Field4 fTag;
 Field4 fFilter;
 Field4 fUnique;
 Field4 fDescending;

 TTagx(){};
 TTagx(Code4 *cb,LPSTR name=0):Data4 (cb,"gl_tag")
 {
  if(!isValid())
	return;
	init();
 };
 int iUnique() {return int(Str4ptr(fUnique.str()));};
 int iDescending() {return int(Str4ptr(fDescending.str() ) ); };
 void init(void)
 {
  fDbf        .init(data,1);
  fTagname    .init(data,2);
  fTag        .init(data,3);
  fFilter     .init(data,4);
  fUnique     .init(data,5);
  fDescending .init(data,6);

 }
 ~TTagx()
 {
  }
};
#endif
