#ifndef __AR_JSUM_H
#define __AR_JSUM_H
_CLASSDEF(Tarjsum)
class Tarjsum : public Data4
{
 public:
	 // MODE Indicates whether the entry has been posted to the GL 1 = not posted; 0 = posted.
  Field4 account;
  Field4 dept;
  Field4 mode;
  Field4 type;
  Field4 period;
  Field4 date;
  Field4 batchno;
  Field4 reference;
  Field4 cheque;
  Field4 descript;
  Field4 debit;
  Field4 credit;
  Field4 depart;
  inline void init()
  {
	account	 .init(data, 1 );
	dept		 .init(data, 2 );
	period	 .init(data, 3 );
	mode		 .init(data, 4 );
	date		 .init(data, 5 );
	type		 .init(data, 6 );
	batchno	 .init(data, 7 );
	reference .init(data, 8 );
	cheque	 .init(data, 9 );
	descript	 .init(data, 10 );
	debit		 .init(data, 11 );
	credit	 .init(data, 12 );
	depart    .init(data, 16 ); // depart code not part of primary key.
  }
  Tarjsum(){};
  Tarjsum(Code4 *cb,LPSTR name=0):Data4(cb,"ar_jsum")
  {
	if(!isValid())
	 return;
	init();
};
  ~Tarjsum()
	{
	  //close();
	 }
  LPSTR GetDbfName(){ return "AR_JSUM";}
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

/*
Structure for table   C:\GL\DATA\AR_JSUM.DBF
Table type            DBASE
Number of records     24762
Last update           97/01/05
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
    1  ACCOUNT                    CHARACTER          7           N
    2  DEPT                       CHARACTER          4           N
    3  PERIOD                     CHARACTER          4           N
    4  MODE                       CHARACTER          1           N
    5  DATE                       DATE               8           N
    6  TYPE                       CHARACTER          3           N
    7  BATCHNO                    NUMERIC            6           N
    8  REFERENCE                  CHARACTER         30           N
    9  CHEQUE                     CHARACTER          7           N
	10  DESCRIPT                   CHARACTER         40           N
	11  DEBIT                      NUMERIC           16    2      N
	12  CREDIT                     NUMERIC           16    2      N
	13  UPDATE                     DATE               8           N
	14  USER                       CHARACTER         10           N
	15  TIME                       CHARACTER         26           N
	16  DEPART                     CHARACTER         10           N
------------------------------------------------------------------
** Total **                                        197
*/
