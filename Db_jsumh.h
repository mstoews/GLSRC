#ifndef __DB_JSUMH_H
#define __DB_JSUMH_H
/*
Structure for table   C:\GL\DBF\GL_JSUMH.DBF
Table type            DBASE
Number of records     0
Last update           96/07/23
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
    1  BATCHNO                    NUMERIC            6           Y
    2  TYPE                       CHARACTER          3           N
    3  LOOKUP                     CHARACTER         18           Y
    4  UPDATE                     DATE               8           N
    5  USER                       CHARACTER         10           N
    6  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                         72

*/

_CLASSDEF(Tjsumh)
class Tjsumh : public Data4
{
 public:
  Field4 glbatch;
  Field4 type;
  Field4 lookup;

  void init()
  {
	glbatch   .init(data,1);
	type      .init(data,2);
	lookup    .init(data,3);
  }

  Tjsumh(){};
  Tjsumh(Code4 *cb,LPSTR name=0):Data4(cb,"gl_jsumh")
  {
	 if(isValid())
	  init();
  };
  LPSTR GetDbfName(){ return "gl_jsumh";}
  inline int open(Code4& cb)
  {
	Data4::open(cb,GetDbfName()) ;
	if( cb.errorCode != 0 )
	{
		if(! isValid() )
			close() ;
		return cb.errorCode ;
	}
	else
	init();
	return cb.errorCode ;
  }

	long   lglbatch() { return long(Str4ptr(glbatch.str ()));};
 };
#endif