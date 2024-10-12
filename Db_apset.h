#ifndef __APSETUP_H
#define __APSETUP_H
_CLASSDEF(Tapsetup)
class Tapsetup : public Data4
{
 public:
 Field4 name1;
 Field4 account;
 Field4 depart;
 Field4 type;

 inline void init()
 {
	SetIndex("name");
	name1.init(data,1);
	account.init(data,2);
	depart.init(data,3);
	type.init(data,4);
 }
 Tapsetup(){};
 Tapsetup(Code4 *cb,LPSTR name=0) : Data4 (cb,"ap_setup")
  {
	 if(!isValid())
		return;

  };
  LPSTR GetDbfName(){ return "ap_setup";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if(! isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
  }

  ~Tapsetup()
  {
	  }
};
#endif
