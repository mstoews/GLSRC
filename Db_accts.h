#ifndef __ACCTS_H
#define __ACCTS_H
#include "gl_field.h"



 _CLASSDEF(Tglaccts)
 class Tglaccts:public Data4
 {
  private:
  BOOL bInitialized;
  public:
  Field4  accounts;
  Field4  dept;
  Field4  descript;
  Field4  fdescript;
  Field4  type;
  Field4  curr_bal;
  Field4  category;

  Field4  budget1;
  Field4  budget2;
  Field4  budget3;
  Field4  budget4;
  Field4  budget5;
  Field4  budget6;
  Field4  budget7;
  Field4  budget8;
  Field4  budget9;
  Field4  budget10;
  Field4  budget11;
  Field4  budget12;

  Field4  period1;
  Field4  period2;
  Field4  period3;
  Field4  period4;
  Field4  period5;
  Field4  period6;
  Field4  period7;
  Field4  period8;
  Field4  period9;
  Field4  period10;
  Field4  period11;
  Field4  period12;
  Field4  openbal;

  Field4  previous1;
  Field4  previous2;
  Field4  previous3;
  Field4  previous4;
  Field4  previous5;
  Field4  previous6;
  Field4  previous7;
  Field4  previous8;
  Field4  previous9;
  Field4  previous10;
  Field4  previous11;
  Field4  previous12;
  Tag4    account_mdx;
  Tag4    type_mdx;
  Tglaccts();
  void init(void);
  BOOL NotInitialized()
  {
	 if(bInitialized)
		return FALSE;
	 else
		return TRUE;
  }

  Tglaccts(Code4 *cb,LPSTR name=0):Data4(cb,"GLACCTS")
  {
	 if(!isValid())
		return;
	 init();
  };
  double dOpenbal(){return double (Str4ptr(openbal.str() ) );}
  double dCurrbal(){return double (Str4ptr(curr_bal.str() ) );}
  double dPeriod1(){return double (Str4ptr(period1.str() ) );}
  double dPeriod2(){return double (Str4ptr(period2.str() ) );}
  double dPeriod3(){return double (Str4ptr(period3.str() ) );}
  double dPeriod4(){return double (Str4ptr(period4.str() ) );}
  double dPeriod5(){return double (Str4ptr(period5.str() ) );}
  double dPeriod6(){return double (Str4ptr(period6.str() ) );}
  double dPeriod7(){return double (Str4ptr(period7.str() ) );}
  double dPeriod8(){return double (Str4ptr(period8.str() ) );}
  double dPeriod9(){return double (Str4ptr(period9.str() ) );}
  double dPeriod10(){return double (Str4ptr(period10.str() ) );}
  double dPeriod11(){return double (Str4ptr(period11.str() ) );}
  double dPeriod12(){return double (Str4ptr(period12.str() ) );}
  double dPrevious12(){return double (Str4ptr(previous12.str() ) );}
  double dPeriod(char *szAccount, char *szDept, int iPeriod);
  void   UpdateAccount(char *szSeek,int iPeriod,double dDebit,double dCredit);

  BOOL SetIndexAccts(int order = 1)
  {
	 switch(order)
	 {
	 case 1: select(account_mdx);  break;
	 case 2: select(type_mdx);     break;
	 default: select (account_mdx);
	 }
	 return TRUE;
  }
  LPSTR GetDbfName(){ return "GLACCTS";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if( isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	account_mdx.init(data,"ACCOUNTS");
	type_mdx.init(data,"TYPE");
	return cb.errorCode ;
  }

  ~Tglaccts()
  {
  }
 };

#endif