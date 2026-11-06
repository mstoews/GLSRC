#ifndef __GLCLASS_H
#define __GLCLASS_H
#include <toolbox.h>
#include <tbowl.h>

#include "gl.h"
#include "mdi.h"
#include "array.h"
#include "db_profi.h"
#include "db_dist.h"
#include "db_cust.h"
#include "db_dept.h"
#include "db_btch.h"
#include "db_accts.h"
#include "db_btchd.h"
#include "db_srce.h"

_CLASSDEF (TWorksheet)
class TWorksheet;


_CLASSDEF(TGLClient)
class TGLClient : public TMDIClient
{
 public:
  TGLClient(PTMDIFrame AParent,PTModule AModule = NULL):TMDIClient(AParent){};
  virtual void WMRButtonDown(RTMessage  Msg) = [WM_FIRST + WM_RBUTTONDOWN];
};

_CLASSDEF (TStatBar)
class TStatBar : public TDialog
{
 public:
  TStatBar(PTWindowsObject AParent,LPSTR AName): TDialog(AParent,AName){};
  virtual void WMPaint(TMessage& Msg)       = [WM_FIRST + WM_PAINT];
  virtual void WMInitDialog (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
};


_CLASSDEF(TGLBudgetInq)
class TGLBudgetInq : public TDialog
{
 public:
 Tglaccts *glaccts; 
 Tgldept  *gldept;


 TGLBudgetInq(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

_CLASSDEF(TGLTransInq)
class TGLTransInq : public TDialog
{
 public:
 PTglaccts glaccts; 
 PTgldist  gldist;
 PTglbtchd glbtchd;

 TGLTransInq(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog(AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void Print          (RTMessage Msg) = [ID_FIRST + ID_PRINT];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
};

_CLASSDEF (TARCustAdd)
class TARCustAdd : public TDialog
{
 public:  
 PTarcust arcust;
 TARCustAdd(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL): TDialog (AParent,AName){};
 virtual void WMInitDialog   (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
 virtual void MainOptions    (RTMessage Msg) = [ID_FIRST + IDMAINOPTIONS];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
};

_CLASSDEF (TARCustMod)
class TARCustMod : public TARCustAdd
{
 public:
 TARCustMod(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL):TARCustAdd(AParent,AName){};
 virtual void WMInitDialog  (RTMessage Msg)  = [WM_FIRST + WM_INITDIALOG];
};

_CLASSDEF (TARCustDel)
class TARCustDel : public TARCustAdd
{
 public:
 TARCustDel(PTWindowsObject AParent, LPSTR AName, PTModule AModule = NULL):TARCustAdd(AParent,AName){};
 virtual void WMInitDialog  (RTMessage Msg)  = [WM_FIRST + WM_INITDIALOG];
 virtual void Ok            (RTMessage Msg)  = [ID_FIRST + IDOK];
};



_CLASSDEF(TListArray)
class TListArray : public Array
{
public:
  TListArray(int upper, int lower = 0, sizeType aDelta = 0)
    : Array(upper, lower, aDelta){};
  virtual classType isA() const { return __firstUserClass + 1; }
  virtual Pchar nameOf() const { return "TListArray"; }
};


_CLASSDEF(TListItem)
class TListItem : public Object
{
 public:
  int index,rec;
  TListItem(int Aindex,int Arec){index = Aindex, rec = Arec;}
  virtual classType isA() const { return __firstUserClass; }
  virtual Pchar nameOf() const { return "TListItem"; }
  virtual hashValueType hashValue() const { return 0; }
  virtual int isEqual(RCObject AListItem) const
    { return index == ((RTListItem)AListItem).index && rec == ((RTListItem)AListItem).rec; }
  virtual void printOn(Rostream outputStream) const
    { outputStream << "(" << index << "," << rec << ")"; }

};

_CLASSDEF(TGLBatchList)
class TGLBatchList : public TDialog
{
 public:
 PTSpread List;
 PTListArray ListArray;
 PTglbtch  glbtch;
 TGLBatchList(PTWindowsObject AParent, LPSTR AName,PTglbtch glb):
  TDialog (AParent, AName) {glbtch=glb; ListArray = new TListArray(1000, 0, 50);List = new TSpread(this,ID_DISTLIST);};
 ~TGLBatchList();
 virtual void WMInitDialog  (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG]{};
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();

};




_CLASSDEF(TARCustList)
class TARCustList : public TDialog
{
 public:
 PTListArray ListArray;
 PTarcust  arcust;
 Field4 *custno;
 Field4 *custname;
 TARCustList(PTWindowsObject AParent, LPSTR AName,PTarcust ar):
  TDialog (AParent, AName) {arcust=ar; ListArray = new TListArray(1000, 0, 50);};
 ~TARCustList();
 virtual void WMInitDialog  (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};





_CLASSDEF (TARMainCust)
class TARMainCust : public TDialog
{
 public :
 TARMainCust(PTWindowsObject AParent, LPSTR AName) : TDialog (AParent,AName){}
 virtual void MainOptions(RTMessage Msg)  = [ID_FIRST + IDMAINOPTIONS];
};

_CLASSDEF(TARInquire)
class TARInquire:public TDialog
{
public:
 TARInquire(PTWindowsObject AParent, LPSTR AName): TDialog(AParent,AName){}
 virtual void ARInqOptions (RTMessage Msg) = [ID_FIRST + IDINQOPTIONS];
};



BOOL FAR PASCAL DistListPrintProc(HDC hDC,short dCommand,short nPageNum,long lAppData);
#endif __GLCLASS_H
