#ifndef __DB_DEPT_H
#define __DB_DEPT_H
#include "gl_dlg.h"
#include "tlist_w.h"

/*
Structure for table   C:\GL\DBF\GL_DEPT.DBF
Table type            DBASE
Number of records     1
Last update           12/04/96
------------------------------------------------------------------
Field  Field Name                 Type          Length  Dec  Index
	 1  DEPARTP                    CHARACTER         10           Y
	 2  DEPARTC                    CHARACTER         10           N
	 3  DESCRIPT                   CHARACTER         40           N
	 4  TYPE                       CHARACTER          1           N
	 5  UPDATE                     DATE               8           N
	 6  USER                       CHARACTER         10           N
	 7  TIME                       CHARACTER         26           N
------------------------------------------------------------------
** Total **                                        106
*/

_CLASSDEF (Tgldept)
class Tgldept : public Data4
{
 public:
 Tag4 dept_tag;

 Field4 deptp;
 Field4 dept;
 Field4 type;
 Field4 descript;


 Tgldept(){};
 void init(void)
 {
  deptp.init(*(Data4 *)this,1);
  dept.init(*(Data4 *)this,2);
  descript.init(*(Data4 *)this,3);
  type.init(*(Data4 *)this,4);
  dept_tag.initDefault(data);
 }
 Tgldept(Code4 *cb,LPSTR name=0):Data4(cb,GetDbfName())
 {
	if(!isValid())
	 return;
  init();

 };

  LPSTR GetDbfName(){ return "GL_DEPT";}
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


 ~Tgldept()
 {
 }
};

_CLASSDEF(TDeptList)
class TDeptList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 string strIndex;
 long   lRows;
 PTListWindow List;
 Tgldept gldept;
 TDeptList(PTWindowsObject AParent, LPSTR AName):TWindow (AParent, AName){
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
 }
 void SetInfo(string index,long rows){
  strIndex = index;
  lRows = rows;
 }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GL_DEPT";}
};
#endif
