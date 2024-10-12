/*
Structure for table   C:\GL\DBF\AP_JSUM.DBF
Table type            DBASE
Number of records     2219
Last update           96/07/23
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
    1  ACCOUNT                    CHARACTER          7           N
    2  DEPT                       CHARACTER          4           N
    3  LOOKUP                     CHARACTER         18           Y
    4  PERIOD                     CHARACTER          4           N
    5  MODE                       CHARACTER          1           N
    6  DATE                       DATE               8           N
    7  TYPE                       CHARACTER          3           N
    8  BATCHNO                    NUMERIC            6           Y
    9  GLBATCHNO                  NUMERIC            6           N
   10  REFERENCE                  CHARACTER         30           N
   11  CHEQUE                     CHARACTER          7           N
   12  DESCRIPT                   CHARACTER         40           Y
   13  DEBIT                      NUMERIC           16    2      N
   14  CREDIT                     NUMERIC           16    2      N
   15  UPDATE                     DATE               8           N
   16  USER                       CHARACTER         10           N
	17  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        211





_CLASSDEF(Tapjsum)
class Tapjsum : public Data4
{
 public:
  Field4 account;
  Field4 dept;
  Field4 debit;
  Field4 credit;
  Field4 mode;
  Field4 glbatchno;
  Field4 lookup;
  void init(){
	account   .init(data,1);
	dept      .init(data,2);
	lookup    .init(data,3);
	mode      .init(data,5);
	glbatchno .init(data,9);
	debit     .init(data,13);
	credit    .init(data,14);
  }
  Tapjsum(){};
  Tapjsum(Code4 *cb,LPSTR name=0):Data4(cb,"ap_jsum")
  {
	 if(isValid())
	  init();
  };
  ~Tapjsum(){};
	double Debit()   { return double(Str4ptr(debit.str()));};
	double Credit()  { return double(Str4ptr(credit.str ()));};
 };
 */

