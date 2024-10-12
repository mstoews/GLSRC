#ifndef __DB_ACCESS_H
#define __DB_ACCESS_H
_CLASSDEF(TAccess)
class TAccess : public Data4
{
 public:
 Field4 fUsername;
 Field4 fDescript;
 Field4 fDate;
 Field4 fPassword;
 Field4 fLevel;
 TAccess (){};
 TAccess(Code4 *cb,LPSTR name=0):Data4 (cb,GetDbfName())
 {
	 if(!isValid())
		return;

  top();
  SetIndex("u");
  fUsername.init(data,1);
  fDescript.init(data,2);
  fDate.init(data,3);
  fPassword.init(data,4);
  fLevel.init(data,5);
 }
 LPSTR GetDbfName() { return "access.pws"; }

};

_CLASSDEF(TAccessl)
class TAccessl : public Data4
{
 public:
 Field4 fLevel;
 Field4 fDescript;
 Field4 fMenuCode;

 TAccessl(Code4 *cb,LPSTR name=0):Data4 (cb,GetDbfName())
 {
	 if(!isValid())
		return;
  SetIndex("l");
  fLevel.init(data,1);
  fDescript.init(data,2);
  fMenuCode.init(data,3);
 }
  LPSTR GetDbfName() { return "accessl.pws"; }
};


#endif