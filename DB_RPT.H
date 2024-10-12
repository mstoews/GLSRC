#ifndef __GL_RPT_H
#define __GL_RPT_H

/*
Structure for table   D:\GL\DBF\GL_RPT.DBF
Table type            DBASE
Number of records     4
Last update           21/01/97
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  REPORT                     CHARACTER         10           Y
	 2  DESC                       CHARACTER         40           N
	 3  RPT_FILE                   CHARACTER         11           N
	 4  ENTRY                      DATE               8           N
	 5  USER                       CHARACTER         10           N
	 6  UPDATE                     CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        106
*/

class TGl_rpt : public Data4
{
  public:
  Field4 report;
  Field4 desc;
  Field4 rpt_dept;
  void init()
  {
	report.init(data,1);
	desc.init(data,2);
	rpt_dept.init(data,3);
  }

  TGl_rpt(){}
  TGl_rpt(Code4 *cb,LPSTR name = 0) : Data4 (cb,"GL_RPT")
  {
  };

  LPSTR GetDbfName(){ return "GL_RPT";}

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

/*
Structure for table   D:\GL\DBF\GL_SUB.DBF
Table type            DBASE
Number of records     157
Last update           21/01/97
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  SUB                        CHARACTER         10           Y
	 2  ACCOUNT                    CHARACTER          7           N
	 3  DEPT                       CHARACTER          4           N
	 4  DEPART                     CHARACTER         10           N
	 5  SEQNO                      NUMERIC           10           N
	 6  FIELD                      CHARACTER          1           N
	 7  ENTRY                      DATE               8           N
	 8  USER                       CHARACTER         10           N
	 9  UPDATE                     CHARACTER         26           N
------------------------------------------------------------------
** Total **                                         87
*/

class TGl_sub : public Data4
{
  public:
  Field4 sub;
  Field4 account;
  Field4 dept;
  Field4 depart;
  Field4 seqno;
  Field4 field;

  TGl_sub(){}
  TGl_sub(Code4* cb,LPSTR name = 0):Data4(cb,"GL_SUB"){}
  void init()
  {
	 sub.init(data,1);
	 account.init(data,2);
	 dept.init(data,3);
	 depart.init(data,4);
	 seqno.init(data,5);
	 field.init(data,6);
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

/*
Structure for table   D:\GL\DBF\GL_CAT.DBF
Table type            DBASE
Number of records     12
Last update           21/01/97
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  REPORT                     CHARACTER         10           Y
	 2  SUB                        CHARACTER         10           N
	 3  SEQNO                      NUMERIC           10           N
	 4  DESC                       CHARACTER         40           N
------------------------------------------------------------------
** Total **                                         71
*/

class TGl_cat : public Data4
{
  private:
  Field4 report;
  Field4 sub;
  Field4 seqno;
  Field4 desc;
  void init()
  {
	report.init(data,1);
	sub.init(data,2);
	seqno.init(data,3);
	desc.init(data,4);
  }
  public:
  TGl_cat(){};
  TGl_cat(Code4 *cb,LPSTR name = 0) : Data4 (cb,"GL_CAT"){};

  LPSTR GetDbfName(){ return "GL_CAT";}

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
