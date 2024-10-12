#ifndef __XACCT_H
#define __XACCT_H
 _CLASSDEF(Tgl_xacct)
 class Tgl_xacct:public Data4
 {
  public:
  Field4 *oldacct;
  Field4 *dept;
  Field4 *olddesc;
  Field4 *newacct;
  Field4 *newdept;

  Tgl_xacct(Code4 *cb,LPSTR name=0):Data4(cb,"gl_xacct")
  {
  top();
  oldacct  = (Field4 *) new Field4(*(Data4 *)this,"oldacct");
  dept     = (Field4 *) new Field4(*(Data4 *)this,"dept");
  olddesc  = (Field4 *) new Field4(*(Data4 *)this,"olddesc");
  newacct  = (Field4 *) new Field4(*(Data4 *)this,"newacct");
  newdept  = (Field4 *) new Field4(*(Data4 *)this,"newdept");
  };
  ~Tgl_xacct(){close();};
  };

#endif