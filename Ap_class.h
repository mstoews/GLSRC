#ifndef __AP_CLASS_H
#define __AP_CLASS_H
#include "glb.h"
#include "gl_pick.h"
#include "db_vend.h"
#include "db_apdst.h"
#include "db_apset.h"
#include "gl_dlg.h"
#include "db_dist.h"
#include "gl_acct.h"
#include "db_cash.h"
#include "db_btchd.h"
#include "db_arbtc.h"


class Tapbtchd;

void PrintSlip(LPSTR szReportName,int iBatchno);

_CLASSDEF (TAPDistList)
class TAPDistList : public TWindow
{
 public:
  TSpread* List;
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  Tapbtchd* apbtd;
  Tapcashd* apcashd;
  short d0Width,d2Width,d3Width,d4Width,d5Width;
  int   iRight,iLeft,iTop,iBottom;
  long iRows;
  TAPDistList(PTWindowsObject AParent,LPSTR ATitle,Tapbtchd* bdp):TWindow(AParent,ATitle)
  {
	apbtd = bdp;
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
	Attr.Style |= WS_THICKFRAME;

  };
  TAPDistList(PTWindowsObject AParent,LPSTR ATitle,Tapcashd* bdp):TWindow(AParent,ATitle)
  {
	apcashd = bdp;
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
	Attr.Style |= WS_THICKFRAME;
  };

  TAPDistList(PTWindowsObject AParent,LPSTR ATitle):TWindow(AParent,ATitle)
  {
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
	Attr.Style |= WS_THICKFRAME;
  };

  virtual void SetupWindow();
  virtual void WMSize         (RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void SetListSize(int Rows);
  virtual void SetField(int ID,Field4 *field);
  virtual void SetField(int ID,char  *field);
  virtual void SetField(int ID,long   field);
  virtual void SetField(int ID,double field);
  virtual LPSTR GetClassName(){ return "AP_DISTLIST";}
  virtual LPSTR GetReportName() { return "ap_slips.rpt";}
  virtual void SavePosition();
  virtual BOOL SetPosition();
  virtual BOOL CanClose(){return FALSE;}
};

_CLASSDEF (TGLDistList)
class TGLDistList : public TAPDistList
{
 public:
 Tglbtchd* glbtchd;
 TGLDistList(PTWindowsObject AParent,LPSTR ATitle,Tglbtchd* btd):TAPDistList(AParent,ATitle)
  {
	glbtchd = btd;
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
	Attr.Style |= WS_THICKFRAME;
  };
 virtual void SetupWindow();
 virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual LPSTR GetClassName(){ return "GL_DISTLIST";}
 virtual LPSTR GetReportName() { return "gl_slips.rpt";}
 virtual void  SavePosition();
 void SetParentHandle(HWND hwnd)
 {
   HWindow = hwnd;
 }
};

_CLASSDEF (TARDistList)
class TARDistList : public TAPDistList
{
 public:
 Tarbtchd* arbtchd;
 TARDistList(PTWindowsObject AParent,LPSTR ATitle,Tarbtchd* ard):TAPDistList(AParent,ATitle)
  {
	arbtchd = ard;
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
	Attr.Style |= WS_THICKFRAME;
  };
 virtual void SetupWindow();
 virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual LPSTR GetClassName(){ return "AR_DISTLIST";}
 virtual LPSTR GetReportName() { return "ar_slips.rpt";}
};

_CLASSDEF (TAPCashList)
class TAPCashList : public TAPDistList
{
 public:
 Tapcashd* apcashd;
 TAPCashList(PTWindowsObject AParent,LPSTR ATitle,Tapcashd* apc ):TAPDistList(AParent,ATitle)
  {
	apcashd = apc;
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
	Attr.Style |= WS_THICKFRAME;
  };
 virtual void SetupWindow();
 virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual LPSTR GetClassName(){ return "AP_CASHLIST";}
 virtual LPSTR GetReportName() { return "ca_slips.rpt";}
 ~TAPCashList(){delete List;};
};






#endif
