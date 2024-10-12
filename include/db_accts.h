#ifndef __ACCTS_H
#define __ACCTS_H
 _CLASSDEF(Tglaccts)
 class Tglaccts:public Data4
 {
  public:
  Field4 *accounts;
  Field4 *dept;
  Field4 *descript;
  Field4 *fdescript;
  Field4 *type;
  Field4 *curr_bal;
  Field4 *category;
  Field4 *budget1;
  Field4 *budget2;
  Field4 *budget3;
  Field4 *budget4;
  Field4 *budget5;
  Field4 *budget6;
  Field4 *budget7;
  Field4 *budget8;
  Field4 *budget9;
  Field4 *budget10;
  Field4 *budget11;
  Field4 *budget12;

  Field4 *period1;
  Field4 *period2;
  Field4 *period3;
  Field4 *period4;
  Field4 *period5;
  Field4 *period6;
  Field4 *period7;
  Field4 *period8;
  Field4 *period9;
  Field4 *period10;
  Field4 *period11;
  Field4 *period12;
  Field4 *openbal;

  Field4 *previous1;
  Field4 *previous2;
  Field4 *previous3;
  Field4 *previous4;
  Field4 *previous5;
  Field4 *previous6;
  Field4 *previous7;
  Field4 *previous8;
  Field4 *previous9;
  Field4 *previous10;
  Field4 *previous11;
  Field4 *previous12;

  Tglaccts(Code4 *cb,LPSTR name=0):Data4(cb,"glaccts")
  {
  if(isValid())
  {
   SetIndex("accounts");
   top();
   accounts = (Field4 *) new Field4(*(Data4 *)this,"account");
   dept     = (Field4 *) new Field4(*(Data4 *)this,"dept");
   descript = (Field4 *) new Field4(*(Data4 *)this,"descript");
   fdescript= (Field4 *) new Field4(*(Data4 *)this,"fdescript");
   type     = (Field4 *) new Field4(*(Data4 *)this,"type");
   curr_bal = (Field4 *) new Field4(*(Data4 *)this,"curr_bal");
   category = (Field4 *) new Field4(*(Data4 *)this,"category");


  budget1  = (Field4 *) new Field4(*(Data4 *)this,"budget1");
  budget2  = (Field4 *) new Field4(*(Data4 *)this,"budget2");
  budget3  = (Field4 *) new Field4(*(Data4 *)this,"budget3");
  budget4  = (Field4 *) new Field4(*(Data4 *)this,"budget4");
  budget5  = (Field4 *) new Field4(*(Data4 *)this,"budget5");
  budget6  = (Field4 *) new Field4(*(Data4 *)this,"budget6");
  budget7  = (Field4 *) new Field4(*(Data4 *)this,"budget7");
  budget8  = (Field4 *) new Field4(*(Data4 *)this,"budget8");
  budget9  = (Field4 *) new Field4(*(Data4 *)this,"budget9");
  budget10  = (Field4 *) new Field4(*(Data4 *)this,"budget10");
  budget11  = (Field4 *) new Field4(*(Data4 *)this,"budget11");
  budget12  = (Field4 *) new Field4(*(Data4 *)this,"budget12");

  period1  = (Field4 *) new Field4(*(Data4 *)this,"period1");
  period2  = (Field4 *) new Field4(*(Data4 *)this,"period2");
  period3  = (Field4 *) new Field4(*(Data4 *)this,"period3");
  period4  = (Field4 *) new Field4(*(Data4 *)this,"period4");
  period5  = (Field4 *) new Field4(*(Data4 *)this,"period5");
  period6  = (Field4 *) new Field4(*(Data4 *)this,"period6");
  period7  = (Field4 *) new Field4(*(Data4 *)this,"period7");
  period8  = (Field4 *) new Field4(*(Data4 *)this,"period8");
  period9  = (Field4 *) new Field4(*(Data4 *)this,"period9");
  period10  = (Field4 *) new Field4(*(Data4 *)this,"period10");
  period11  = (Field4 *) new Field4(*(Data4 *)this,"period11");
  period12  = (Field4 *) new Field4(*(Data4 *)this,"period12");
  openbal   = (Field4 *) new Field4(*(Data4 *)this,"openbal");

  previous1  = (Field4 *) new Field4(*(Data4 *)this,"previous1");
  previous2  = (Field4 *) new Field4(*(Data4 *)this,"previous2");
  previous3  = (Field4 *) new Field4(*(Data4 *)this,"previous3");
  previous4  = (Field4 *) new Field4(*(Data4 *)this,"previous4");
  previous5  = (Field4 *) new Field4(*(Data4 *)this,"previous5");
  previous6  = (Field4 *) new Field4(*(Data4 *)this,"previous6");
  previous7  = (Field4 *) new Field4(*(Data4 *)this,"previous7");
  previous8  = (Field4 *) new Field4(*(Data4 *)this,"previous8");
  previous9  = (Field4 *) new Field4(*(Data4 *)this,"previous9");
  previous10  = (Field4 *) new Field4(*(Data4 *)this,"previous10");
  previous11  = (Field4 *) new Field4(*(Data4 *)this,"previous11");
  previous12  = (Field4 *) new Field4(*(Data4 *)this,"previous12");
  }
  };
  double Open_bal(){return double (Str4ptr(openbal->str() ) );}
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


  double dBudget1(){return double (Str4ptr(budget1->str() ) );}
  double dBudget2(){return double (Str4ptr(budget2->str() ) );}
  double dBudget3(){return double (Str4ptr(budget3->str() ) );}
  double dBudget4(){return double (Str4ptr(budget4->str() ) );}
  double dBudget5(){return double (Str4ptr(budget5->str() ) );}
  double dBudget6(){return double (Str4ptr(budget6->str() ) );}
  double dBudget7(){return double (Str4ptr(budget7->str() ) );}
  double dBudget8(){return double (Str4ptr(budget8->str() ) );}
  double dBudget9(){return double (Str4ptr(budget9->str() ) );}
  double dBudget10(){return double (Str4ptr(budget10->str() ) );}
  double dBudget11(){return double (Str4ptr(budget11->str() ) );}
  double dBudget12(){return double (Str4ptr(budget12->str() ) );}

  double dPrevious1(){return double (Str4ptr(previous1->str() ) );}
  double dPrevious2(){return double (Str4ptr(previous2->str() ) );}
  double dPrevious3(){return double (Str4ptr(previous3->str() ) );}
  double dPrevious4(){return double (Str4ptr(previous4->str() ) );}
  double dPrevious5(){return double (Str4ptr(previous5->str() ) );}
  double dPrevious6(){return double (Str4ptr(previous6->str() ) );}
  double dPrevious7(){return double (Str4ptr(previous7->str() ) );}
  double dPrevious8(){return double (Str4ptr(previous8->str() ) );}
  double dPrevious9(){return double (Str4ptr(previous9->str() ) );}
  double dPrevious10(){return double (Str4ptr(previous10->str() ) );}
  double dPrevious11(){return double (Str4ptr(previous11->str() ) );}
  double dPrevious12(){return double (Str4ptr(previous12->str() ) );}

  double dPeriod(char *szAccount, char *szDept, int iPeriod);
  void   UpdateAccount(char *szSeek,int iPeriod,double dDebit,double dCredit);

  };

#endif