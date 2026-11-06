#ifndef __ACCTS_H
#define __ACCTS_H


 _CLASSDEF(Tglaccts)
 class Tglaccts:public Data4
 {
  public:
  Field4  *accounts;
  Field4  *dept;
  Field4  *descript;
  Field4  *fdescript;
  Field4  *type;
  Field4  *curr_bal;

  Field4  *budget1;
  Field4  *budget2;
  Field4  *budget3;
  Field4  *budget4;
  Field4  *budget5;
  Field4  *budget6;
  Field4  *budget7;
  Field4  *budget8;
  Field4  *budget9;
  Field4  *budget10;
  Field4  *budget11;
  Field4  *budget12;

  Field4  *period1;
  Field4  *period2;
  Field4  *period3;
  Field4  *period4;
  Field4  *period5;
  Field4  *period6;
  Field4  *period7;
  Field4  *period8;
  Field4  *period9;
  Field4  *period10;
  Field4  *period11;
  Field4  *period12;
  Field4  *openbal;

  Field4  *previous1;
  Field4  *previous2;
  Field4  *previous3;
  Field4  *previous4;
  Field4  *previous5;
  Field4  *previous6;
  Field4  *previous7;
  Field4  *previous8;
  Field4  *previous9;
  Field4  *previous10;
  Field4  *previous11;
  Field4  *previous12;

  Tglaccts(Code4 *cb,LPSTR name=0):Data4(cb,"glaccts")
  {
  SetIndex("accounts");
  top();  
  accounts =  new Field4 (*(Data4 *)this,1);
  dept     =  new Field4 (*(Data4 *)this,2);
  descript =  new Field4 (*(Data4 *)this,3);
  fdescript=  new Field4 (*(Data4 *)this,4);
  type     =  new Field4 (*(Data4 *)this,5);
  curr_bal =  new Field4 (*(Data4 *)this,7);
  

  period1  =  new Field4 (*(Data4 *)this,8);
  period2  =  new Field4 (*(Data4 *)this,9);
  period3  =  new Field4 (*(Data4 *)this,10);
  period4  =  new Field4 (*(Data4 *)this,11);
  period5  =  new Field4 (*(Data4 *)this,12);
  period6  =  new Field4 (*(Data4 *)this,13);
  period7  =  new Field4 (*(Data4 *)this,14);
  period8  =  new Field4 (*(Data4 *)this,15);
  period9  =  new Field4 (*(Data4 *)this,16);
  period10  =  new Field4 (*(Data4 *)this,17);
  period11  =  new Field4 (*(Data4 *)this,18);
  period12  =  new Field4 (*(Data4 *)this,19);
  openbal   =  new Field4 (*(Data4 *)this,20);
  previous1  =  new Field4 (*(Data4 *)this,21);
  previous2  =  new Field4 (*(Data4 *)this,22);
  previous3  =  new Field4 (*(Data4 *)this,23);
  previous4  =  new Field4 (*(Data4 *)this,24);
  previous5  =  new Field4 (*(Data4 *)this,25);
  previous6  =  new Field4 (*(Data4 *)this,26);
  previous7  =  new Field4 (*(Data4 *)this,27);
  previous8  =  new Field4 (*(Data4 *)this,28);
  previous9  =  new Field4 (*(Data4 *)this,29);
  previous10  =  new Field4 (*(Data4 *)this,30);
  previous11  =  new Field4 (*(Data4 *)this,31);
  previous12  =  new Field4 (*(Data4 *)this,32);

  budget1  =  new Field4 (*(Data4 *)this,33);
  budget2  =  new Field4 (*(Data4 *)this,34);
  budget3  =  new Field4 (*(Data4 *)this,35);
  budget4  =  new Field4 (*(Data4 *)this,36);
  budget5  =  new Field4 (*(Data4 *)this,37);
  budget6  =  new Field4 (*(Data4 *)this,38);
  budget7  =  new Field4 (*(Data4 *)this,39);
  budget8  =  new Field4 (*(Data4 *)this,40);
  budget9  =  new Field4 (*(Data4 *)this,41);
  budget10  =  new Field4 (*(Data4 *)this,42);
  budget11  =  new Field4 (*(Data4 *)this,43);
  budget12  =  new Field4 (*(Data4 *)this,44);

  };
  ~Tglaccts(){close();};
  double Open_bal(){return double (Str4ptr( period1->str() ) );}
  double Curr_bal(){return double (Str4ptr(curr_bal->str() ) );}
  double dPeriod1(){return double (Str4ptr(period1->str() ) );}
  double dPeriod2(){return double (Str4ptr(period2->str() ) );}
  double dPeriod3(){return double (Str4ptr(period3->str() ) );}
  double dPeriod4(){return double (Str4ptr(period4->str() ) );}
  double dPeriod5(){return double (Str4ptr(period5->str() ) );}
  double dPeriod6(){return double (Str4ptr(period6->str() ) );}
  double dPeriod7(){return double (Str4ptr(period7->str() ) );}
  double dPeriod8(){return double (Str4ptr(period8->str() ) );}
  double dPeriod9(){return double (Str4ptr(period9->str() ) );}
  double dPeriod10(){return double (Str4ptr(period10->str() ) );}
  double dPeriod11(){return double (Str4ptr(period11->str() ) );}
  double dPeriod12(){return double (Str4ptr(period12->str() ) );}
  double dPrevious12(){return double (Str4ptr(previous12->str() ) );}
  double dPeriod(char *szAccount, char *szDept, int iPeriod);
  void   UpdateAccount(char *szSeek,int iPeriod,double dDebit,double dCredit);
  };


#endif