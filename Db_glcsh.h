#ifndef __DB_GLCSH_H
#define __DB_GLCSH_H
extern HINSTANCE hInst;

/*
Structure for table   C:\GL\DBF\GL_CASH.DBF
Table type            DBASE
Number of records     13
Last update           10/10/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
    1  BATCHNO                    NUMERIC            6           Y
    2  ACCOUNT                    CHARACTER          7           N
    3  DEPT                       CHARACTER          4           N
    4  APACCOUNT                  CHARACTER          7           N
    5  APDEPT                     CHARACTER          4           N
    6  AMOUNT                     NUMERIC           20    2      N
    7  PERIOD                     NUMERIC            2           N
    8  UPDATE                     DATE               8           N
    9  USER                       CHARACTER         10           N
   10  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                         95*/

_CLASSDEF(Tglcash)
class Tglcash :public Data4
{
 public:
  int iCurrentRecord;

  Field4  batchno;
  Field4  account;
  Field4  dept;
  Field4  apaccount;
  Field4  apdept;
  Field4  amount;
  Field4  period;

  BOOL init(void)
  {
	batchno.init(data,1);
	account.init(data,2);
	dept.init(data,3);
	apaccount.init(data,4);
	apdept.init(data,5);
	amount.init(data,6);
	period.init(data,7);
	 return TRUE;
  }
  Tglcash(){};
  Tglcash(Code4 *cb,LPSTR name=0):Data4 (cb,"GL_CASH")
  {
	if(!isValid())
		return;
	init();
  };
 ~Tglcash(){
 }
  LPSTR GetDbfName(){ return "GL_CASH";}
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
	{
	 init();
	 SetIndex("no");
	}
	return cb.errorCode ;
  }

  long lBatchno() { return long (Str4ptr(batchno.str()));};
};

#endif
