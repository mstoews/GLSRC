#ifndef _DB_GLSUB_H
#define _DB_GLSUB_H
 _CLASSDEF(Tglsub)
 class Tglsub : public Data4
 {
  public:
  Field4 report;
  Field4 sub;
  Field4 account;
  Field4 dept;
  Field4 depart;
  Field4 seqno;
  Field4 field;
  Field4 entry;
  Field4 user;
  Field4 update;

  Tglsub(){};
  Tglsub(Code4 *cb,LPSTR name=0):Data4(cb,"gl_btch")
	{
		if(!isValid())
	 return;
	 init();
	};

	void init()
	{
	report.init(data,1);
	sub.init(data,2);
	account.init(data,3);
	dept.init(data,4);
	depart.init(data,5);
	seqno.init(data,6);
	field.init(data,7);
	entry.init(data,8);
	user.init(data,9);
	update.init(data,10);
	}

	~Tglsub()
	{
	}

  LPSTR GetDbfName(){ return "GL_SUB";}
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
	return cb.errorCode ;
  }
 };
#endif


/*
Structure for table   D:\GL\EIWA_DBF\GL_SUB.DBF
Table type            DBASE
Number of records     163
Last update           02/20/97
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  REPORT                     CHARACTER         10           N
	 2  SUB                        CHARACTER         10           Y
	 3  ACCOUNT                    CHARACTER          7           Y
	 4  DEPT                       CHARACTER          4           N
	 5  DEPART                     CHARACTER         10           N
	 6  SEQNO                      NUMERIC           10           N
	 7  FIELD                      CHARACTER          1           N
	 8  ENTRY                      DATE               8           N
	 9  USER                       CHARACTER         10           N
	10  UPDATE                     CHARACTER         26           N
------------------------------------------------------------------
** Total **                                         97
*/
