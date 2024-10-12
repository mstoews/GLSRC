#ifndef __DB_JSUM_H
#define __DB_JSUM_H

/*
Structure for table   C:\GL\DBF\AP_JSUM.DBF
Field  Field Name                 Type          Length  Dec  Index
    1  ACCOUNT                    CHARACTER          7           N
    2  DEPT                       CHARACTER          4           N
    3  LOOKUP                     CHARACTER         18           N
    4  PERIOD                     CHARACTER          4           N
    5  MODE                       CHARACTER          1           N
    6  DATE                       DATE               8           N
    7  TYPE                       CHARACTER          3           N
    8  BATCHNO                    NUMERIC            6           N
    9  GLBATCHNO                  NUMERIC            6           N
   10  REFERENCE                  CHARACTER         30           N
   11  CHEQUE                     CHARACTER          7           N
   12  DESCRIPT                   CHARACTER         40           N
   13  DEBIT                      NUMERIC           16    2      N
   14  CREDIT                     NUMERIC           16    2      N
   15  UPDATE                     DATE               8           N
   16  USER                       CHARACTER         10           N
   17  TIME                       CHARACTER         26           N
   18  DEPART                     CHARACTER         10           N
------------------------------------------------------------------
** Total **                                        221*/

_CLASSDEF(Tapjsum)
class Tapjsum : public Data4
{
 public:
  Field4  account;
  Field4  dept;
  Field4  lookup;
  Field4  mode;    // Indicates whether the entry has been posted to the GL 1 = not posted; 0 = posted.
  Field4  period;
  Field4  date;
  Field4  type;
  Field4  batchno;
  Field4  glbatchno;
  Field4  reference;
  Field4  cheque;
  Field4  descript;
  Field4  debit;
  Field4  credit;
  Field4  depart;
  Tapjsum(){};

  inline void init()
  {
	account.init(data, 1);
	dept.init(data, 2);
	lookup.init(data,3);
	period.init(data, 4);
	mode.init(data, 5);
	date.init(data, 6);
	type.init(data, 7);
	batchno.init(data, 8);
	glbatchno.init(data,9);
	reference.init(data, 10);
	cheque.init(data, 11);
	descript.init(data,12);
	debit.init(data,13);
	credit.init(data,14);
	depart.init(data,18);

  }


  Tapjsum(Code4 *cb,LPSTR name=0):Data4(cb,"ap_jsum")
  {
	if(!isValid())
	 return;
	init();
  };
  ~Tapjsum(){
  };
  LPSTR GetDbfName(){ return "ap_jsum";}
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
  	double Debit()   { return double(Str4ptr(debit.str()));};
	double Credit()  { return double(Str4ptr(credit.str ()));};

 };



#endif
