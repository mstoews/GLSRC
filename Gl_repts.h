
_CLASSDEF(Tglreports)
class Tglreports : public Data4
{
 public:
  Field4 *accounts;
  Field4 *dept;
  Field4 *descript;
  Field4 *type;
  Field4 *curr_bal;

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

 Tglreports(Code4 *cb,LPSTR name):Data4(cb,"gl_repts")
 {
  SetIndex("account");
  top(); 
  accounts = (Field4 *) new Field4(*(Data4 *)this,"account");
  dept     = (Field4 *) new Field4(*(Data4 *)this,"dept");
  descript = (Field4 *) new Field4(*(Data4 *)this,"descript");
  type     = (Field4 *) new Field4(*(Data4 *)this,"type");
  curr_bal = (Field4 *) new Field4(*(Data4 *)this,"curr_bal");
  
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
   
  };
    
  ~Tglreports(){close();};
 };
