#ifndef __GL_USER_H
#define __GL_USER_H
/*

Structure for table   D:\GL\DBF\GL_USER.DBF
Table type            DBASE
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  PRG                        CHARACTER         10           N
	 2  DBF                        CHARACTER         10           Y
	 3  LOCKTYPE                   CHARACTER          1           N
------------------------------------------------------------------
** Total **                                         22

Structure for table   C:\GL\DBF\GL_PRG.DBF
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  PRG                        CHARACTER         10           N
	 2  NAME                       CHARACTER         20           N
------------------------------------------------------------------
** Total **                                         31

*/
_CLASSDEF(Tglprg)
class Tglprg : public Data4
{
 public:
  Field4  prg;
  Field4  name;

  inline void init(){
		prg.init(data,1);
		name.init(data,2);
  }


  LPSTR GetDbfName(){ return "GL_PRG";}

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

  Tglprg(){};
  Tglprg(Code4 *cb,LPSTR name=0):Data4 (cb,GetDbfName())
  {
	if(!isValid())
	 return;
	init();
  }
 ~Tglprg(){}

};

_CLASSDEF(Tgluser)
class Tgluser :public Data4
{
 public:
  Field4  prg;
  Field4  dbf;
  Field4  locktype;


  inline void init()
  {
	prg.init(data,1);
	dbf.init(data,2);
	locktype.init(data,3);
  }
  LPSTR GetDbfName(){ return "GL_USER";}
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

  Tgluser(){};
  Tgluser(Code4 *cb,LPSTR name=0):Data4 (cb,GetDbfName())
  {
	if(!isValid())
	 return;
	init();
  };
 ~Tgluser()
  {

  }
  };

_CLASSDEF(Tgldbf)
class Tgldbf :public Data4
{
 public:
  Field4   prg;
  Field4   dbf;
  Field4   user;
  Field4   date;
  Field4   batchno;
  Field4   status;

  inline void init()
  {
	prg.init(data,1);
	dbf.init(data,2);
	user.init(data,3);
	date.init(data,4);
	batchno.init(data,5);
	status.init(data,6);
  }
  LPSTR GetDbfName(){ return "GL_DBF";}
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

  Tgldbf(){};
  Tgldbf(Code4 *cb,LPSTR name=0):Data4 (cb,GetDbfName())
  {
	if(!isValid())
	 return;
	init();
  };
};

_CLASSDEF(Tgllock)
class Tgllock
{
  private:
  Tgluser gluser;
  Tgldbf  gldbf;
  Tglprg  glprg;
  public:
  Tgllock();
  LockPrg(LPSTR szProgram);
  UnLockPrg(LPSTR szProgram);
  BOOL LockDbf(LPSTR szDbf);
  BOOL UnLockDbf(LPSTR szDbf);
  BOOL LockRec(LPSTR szDbf,long lBatchno);
  BOOL UnLockRec(LPSTR szDbf,long lBatchno);
  long lBatchno(LPSTR szPrg);
};
#endif
