#ifndef __GL_SUM_H
#define __GL_SUM_H
/*
Table type            DBASE
Number of records     12
Last update           97/01/05
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
    1  ACCOUNT                    CHARACTER          7           N
    2  DEPT                       CHARACTER          4           N
    3  PERIOD                     CHARACTER          4           N
    4  MODE                       CHARACTER          1           N
    5  DATE                       DATE               8           N
    6  TYPE                       CHARACTER          3           N
    7  BATCHNO                    NUMERIC            6           Y
    8  REFERENCE                  CHARACTER         30           N
    9  CHEQUE                     CHARACTER          7           N
   10  DESCRIPT                   CHARACTER         40           Y
   11  DEBIT                      NUMERIC           16    2      N
   12  CREDIT                     NUMERIC           16    2      N
   13  DEPART                     CHARACTER         10           N
------------------------------------------------------------------
** Total **                                        153
*** INTERRUPTED ***

*/

_CLASSDEF(Tglsum)
class Tglsum : public Data4
{
 public:
  Field4 account;
  Field4 dept;
  Field4 mode;    // Indicates whether the entry has been posted to the GL 1 = not posted; 0 = posted.
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
  void init()
  {
	account   .init(data, 1);  //"account");
	dept      .init(data, 2);  //"dept");
	period    .init(data, 3);  //"period");
	mode      .init(data, 4);  //"mode");
	date      .init(data, 5);  //"date");
	type      .init(data, 6);  //"type");
	batchno   .init(data, 7);  //"batchno");
	reference .init(data, 8);  //"reference");
	cheque    .init(data, 9);  //"cheque");
	descript  .init(data, 10); //"descript");
	debit     .init(data, 11); //"debit");
	credit    .init(data, 12); //"credit");
	depart    .init(data, 13); // depart
  }
  Tglsum(){};
  Tglsum(Code4 *cb,LPSTR name=0):Data4(cb,"gl_sum")
  {
	 if(isValid())
	 init();
  };
  ~Tglsum(){};

	double Batchno() { return double(Str4ptr(batchno.str()));};
	double Debit()   { return double(Str4ptr(debit.str()));};
	double Credit()  { return double(Str4ptr(credit.str ()));};
 };
#endif
