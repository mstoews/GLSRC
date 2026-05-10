#ifndef __DB_ACCESS_H
#define __DB_ACCESS_H
_CLASSDEF(TAccess)
class TAccess : public Data4
{
 public:
 Field4 *fUsername;
 Field4 *fDescript;
 Field4 *fDate;
 Field4 *fPassword;
 Field4 *fLevel;

 TAccess(Code4 *cb,LPSTR name=0):Data4 (cb,"access.psw")
 {
  top();
  SetIndex("u");
  fUsername  =  new Field4(*(Data4 *)this,1);
  fDescript  =  new Field4(*(Data4 *)this,2);
  fDate      =  new Field4(*(Data4 *)this,3);
  fPassword  =  new Field4(*(Data4 *)this,4);
  fLevel     =  new Field4(*(Data4 *)this,5);
 }
};

_CLASSDEF(TAccessl)
class TAccessl : public Data4
{
 public:
 Field4 *fLevel;
 Field4 *fMenuCode;

 TAccessl(Code4 *cb,LPSTR name=0):Data4 (cb,"accessl")
 {
  SetIndex("l");
  fLevel     =  new Field4(*(Data4 *)this,1);
  fMenuCode  =  new Field4(*(Data4 *)this,2);
 }

};


#endif