#ifndef __GL_BTCHE_H
#define __GL_BTCHE_H

#include "gl_toolb.h"
#include "gl_acctl.h"
//#include "gl_distl.h"
#include "db_drcod.h"
#include "ap_class.h"

#define WM_CURRENTBATCH  WM_USER + 33

#define ID_COMMIT   1056

_CLASSDEF (TGLModifyBatch)
class TGLModifyBatch : public TDialog
{
 public:
  PTglbtchd glbtchd;
  TGLModifyBatch(PTWindowsObject AParent, LPSTR ATitle,PTglbtchd bdp):TDialog(AParent,ATitle)
  {
	glbtchd = bdp;
  };
  virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
};


// Database Declaration classes.


_CLASSDEF(TGLViewBatchList)
class TGLViewBatchList : public TWindow
{
 public:
 PTListWindow List;
 PTglbtch  glbtch;
 TGLViewBatchList(PTWindowsObject AParent, LPSTR AName,PTglbtch glb):TWindow (AParent, AName)
  {
   glbtch=glb;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GL_BTCH";}
};




_CLASSDEF (TGLBatchEntry)
class TGLBatchEntry : public TMainDialog
 {
	public:
	Code4 *cb;
	TGLBatchEntry(PTWindowsObject AParent,int ATitle):TMainDialog(AParent,ATitle)
	{
	  AcctList = NULL;
	  D = NULL;
	  B = NULL;
	};

	~TGLBatchEntry()
	{
		if(AcctList != NULL)
		  delete AcctList;
		if(D != NULL)
		  delete D;
		if(B != NULL)
		  delete B;
	}
	BOOL bNewSlipList;
	BOOL bMakeSlipList;
	BOOL bAcctList;
	BOOL bMedReps;
	BOOL bProducts;
	BOOL bTransaction;
	BOOL bModify;
	long glbtchCurrentRecord;

	Tglaccts   glaccts;
	Tglbtch    glbtch;
	Tglbtchd   glbtchd;
	Tprods     glprod;
	//Tglmedreps glmedrep;
	Tgldept    gldept;
	Tgldrcodes  gldrcodes;

	TAccountList *AcctList;
	TGLDistList *D;
	TGLViewBatchList *B;


	Date4    today;
	char     szMode[2];
	double DebitTotal;
	double CreditTotal;
	double lDifference;
   long   lPeriod;

	virtual LPSTR GetClassName(){ return "GLBatchEntry"; }
	virtual void SetupWindow();
	virtual void Cancel           (RTMessage Msg) = [ID_FIRST + IDCANCEL];
	virtual void ViewBatch        (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST];
	virtual void ViewAccts        (RTMessage Msg) = [ID_FIRST + ID_VIEW];
	virtual void CurrentBatch     (RTMessage Msg) = [CM_FIRST + CM_CURRENTBATCH];
	virtual void Delete           (RTMessage Msg) = [ID_FIRST + ID_DELETE];
	virtual void Next    	      (RTMessage Msg) = [ID_FIRST + ID_NEXT];
	virtual void Previous	      (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS];
	virtual void Top     	      (RTMessage Msg) = [ID_FIRST + ID_TOP];
	virtual void Bottom 		      (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
	virtual void Add              (RTMessage Msg) = [ID_FIRST + ID_ADD];
	virtual void New              (RTMessage Msg) = [ID_FIRST + ID_NEW];
	virtual void ShowToolBox      (RTMessage Msg) = [ID_FIRST + IDLISTBOX];
	virtual void ShowTools        (RTMessage Msg) = [CM_FIRST + IDLISTBOX]{ShowToolBox(Msg);};
	virtual void RollBack         (RTMessage Msg) = [CM_FIRST + CM_ROLLBACK];
	virtual void SetPeriods       (RTMessage Msg) = [ID_FIRST + ID_PERIOD];
	virtual void Print            (RTMessage Msg) = [ID_FIRST + ID_PRINT];
	virtual void ChangeDept       (RTMessage Msg) = [ID_FIRST + ID_DEPTCODES];
	void SetMode (char *szSetMode){strcpy(szMode,szSetMode);};
	long GetPeriod();
	void AppendHeader(long iBatchno);
	void ReplaceHeader(void);
	void AppendDetail(long iBatchno);
	void ReplaceDetail(void);
	void AppendNewEntry(void);
	void Clear();
	int CheckGLAccount ();
	void LoadDoctors(void);
	//void LoadMedReps(void);
	void LoadDepartments(void);
	void LoadProducts(void);
	//char *GetMedRep();
	char *GetDept();
	char *GetProduct();
	char *GetDoctor();
	int CheckGLDept    ();
	BOOL TestDatabase();
	int right,left,bottom,top;
 };

#endif
