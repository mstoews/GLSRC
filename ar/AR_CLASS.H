#include "glb.h"
#include "gl_pick.h"
#include "db_vend.h"
#include "db_apdst.h"
#include "db_apset.h"
#include "gl_dlg.h"
#include "db_dist.h"
#include "gl_acct.h"



extern Code4 cb;


_CLASSDEF(TAPVendorList)
class TAPVendorList : public TPickList
{
 public:
 PTListArray ListArray;
 PTapvendor  apvendor;
 Field4 *vendorno;
 Field4 *vdname;
 TAPVendorList(PTWindowsObject AParent, LPSTR AName,PTapvendor vd):
  TPickList (AParent, AName) {apvendor=vd; ListArray = new TListArray(1000, 0, 50);};
 ~TAPVendorList();
 virtual void SetupWindow();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};

_CLASSDEF(TAPBatchList)
class TAPBatchList : public TPickList
{
 public:
 PTListArray ListArray;
 PTapbtch  apbtch;
 PTapcash  apcash;
 TAPBatchList(PTWindowsObject AParent, LPSTR AName,PTapbtch glb):
  TPickList (AParent, AName) {apbtch=glb; ListArray = new TListArray(1000, 0, 50);};
 TAPBatchList(PTWindowsObject AParent, LPSTR AName,PTapcash glb):
  TPickList (AParent, AName) {apcash=glb; ListArray = new TListArray(1000, 0, 50);};
 ~TAPBatchList();
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void Cancel        (RTMessage Msg) = [ID_FIRST + IDCANCEL] {ShutDownWindow();};
 virtual void SetupWindow();
};



_CLASSDEF (TAPDistList)
class TAPDistList : public TDialog
{
 public:
  SS_CELLTYPE 	CellType;
  HWND SHWindow;
  PTapbtchd apbtd;
  PTapcashd apcashd;
  TAPDistList(PTWindowsObject AParent, LPSTR ATitle,PTapbtchd bdp):TDialog(AParent,ATitle)
  {  
  apbtd = bdp;
  };
  TAPDistList(PTWindowsObject AParent, LPSTR ATitle,PTapcashd bdp):TDialog(AParent,ATitle)
  {  
  apcashd = bdp;
  };
  TAPDistList(PTWindowsObject AParent, LPSTR ATitle):TDialog(AParent,ATitle){};
  virtual void SetupWindow();
  virtual void SelectCell     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
  virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
  virtual void SetListSize(int Rows);
  virtual void SetField(int ID,Field4 *field);
  virtual void SetField(int ID,char  *field);
  virtual void SetField(int ID,long   field);
  virtual void SetField(int ID,double field);

};








