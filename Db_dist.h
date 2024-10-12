#ifndef __GL_DIST_H
#define __GL_DIST_H

_CLASSDEF (Tgldist)
class Tgldist : public Data4
{
 public:
  Field4 account;
  Field4 dept;
  Field4 drcode;
  Field4 medrep;
  Field4 prod;
  Field4 period;
  Field4 date;
  Field4 type;
  Field4 batchno;
  Field4 reference;
  Field4 cheque;
  Field4 descript;
  Field4 debit;
  Field4 credit;
  //Field4 fiscalyr;
  void init(void)
  {
	SetIndex("account");
	/*
Structure for table   C:\GL\DBF\GL_DIST.DBF
Table type            DBASE
Number of records     2
Last update           96/03/25
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  ACCOUNT                    CHARACTER          7           N
	 2  DEPT                       CHARACTER          4           N
	 3  DRCODE                     CHARACTER          5           N
	 4  MEDREP                     CHARACTER          5           N
	 5  PROD                       CHARACTER          7           N
    6  PERIOD                     CHARACTER          4           N
    7  DATE                       CHARACTER         10           N
    8  TYPE                       CHARACTER          3           N
	 9  BATCHNO                    NUMERIC            6           Y
	10  REFERENCE                  CHARACTER         30           N
	11  CHEQUE                     CHARACTER          7           N
	12  DESCRIPT                   CHARACTER         40           Y
	13  DEBIT                      NUMERIC           16    2      N
	14  CREDIT                     NUMERIC           16    2      N
	15  UPDATE                     DATE               8           N
	16  USER                       CHARACTER         10           N
	17  TIME                       CHARACTER         26           N
	18  FISCALYR                   CHARACTER          4           N
------------------------------------------------------------------
** Total **                                        209

	*/
	account.init(data,1);
	dept.init(data,2);
	drcode.init(data,3);
	medrep.init(data,4);
	prod.init(data,5);
	period.init(data,6);
	date.init(data,7);
	type.init(data,8);
	batchno.init(data,9);
	reference.init(data,10);
	cheque.init(data,11);
	descript.init(data,12);
	debit.init(data,13);
	credit.init(data,14);
	//fiscalyr.init(data,18);
  }
  Tgldist(){};
  Tgldist(Code4 *cb,LPSTR name=0):Data4(cb,"gl_dist")
  {
	if(!isValid())
	 return;

	init();
  };

  LPSTR GetDbfName(){ return "GL_DIST";}
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

  ~Tgldist()
  {
	//close();
  };
 };




#endif