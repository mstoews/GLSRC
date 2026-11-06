#include <dir.h>
#define   REINDEX "Use the maintainance functions \nto reindex the all databases."
#define   ID_SPREAD 3888

#define   ID_ROW     3889
#define   ID_COL     3890
#define   ID_TEXT    3891
#define   ID_NUMBER  3892
#define   ID_ACCT    3893
#define   ID_SEARCH  3894
#define   ID_ITEM    3895
#define   ID_COPY    10076
#define   ID_OUTLINE 4001
#define   ID_TOP     4002
#define   ID_BOTTOM  4003
#define   ID_RIGHT   4004
#define   ID_LEFT    4005
#define   ID_NONE    4006
#define   ID_SOLID   4007
#define   ID_DASH    4008
#define   ID_DOT     4009
#define   ID_DASHDOT 4010

BOOL FAR PASCAL dGetPeriodValue(LPSS_VALUE Result, LPSS_VALUE Values, short ValueCnt);
double GetPeriod  (LPSTR lpAccountNo,LPSTR lpDept,int dPeriod);
double GetGLAmount(LPSTR lpAccountNo,LPSTR lpDept);
double GetOpenBalance(LPSTR lpAccountNo,LPSTR lpDept);
BOOL FAR PASCAL dGetPeriodValue(LPSS_VALUE Result, LPSS_VALUE Values, short ValueCnt);
BOOL FAR PASCAL dGetAcctValue  (LPSS_VALUE Result, LPSS_VALUE Values, short ValueCnt);
BOOL FAR PASCAL dGetBudgetValue(LPSS_VALUE Result, LPSS_VALUE Values, short ValueCnt);
BOOL FAR PASCAL dGetOpenBalance(LPSS_VALUE Result, LPSS_VALUE Values, short ValueCnt);




_CLASSDEF (TSetBorder)
class TSetBorder : public TDialog
{
  public:
  PTSpread Spread;
  SS_CELLCOORD UL;
  SS_CELLCOORD LR;
  WORD BORDERTYPE;
  WORD BORDERSTYLE;
  int row;
  int col;
  int erow;
  int ecol;
  TSetBorder(PTWindowsObject AParent,LPSTR AName,int Row,int Col, int eRow, int eCol, PTSpread pSpread)
  : TDialog(AParent,AName)
  {
   Spread = pSpread;
   row = Row;
   col = Col;
   erow = eRow;
   ecol = eCol;
  };
  TSetBorder(PTWindowsObject AParent,LPSTR AName,int Row,int Col,PTSpread pSpread)
  : TDialog(AParent,AName)
  {
   Spread = pSpread;
   row = Row;
   col = Col;
   erow = 0;
   ecol = 0;
  };

  virtual void WMInitDialog     (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
  virtual void Ok               (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void NoBorderBtn       (RTMessage Msg) = [ID_FIRST + ID_OUTLINE];
  virtual void TopBtn           (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void BottomBtn        (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void RightBtn         (RTMessage Msg) = [ID_FIRST + ID_LEFT];
  virtual void LeftBtn          (RTMessage Msg) = [ID_FIRST + ID_RIGHT];
  virtual void NoneBtn          (RTMessage Msg) = [ID_FIRST + ID_NONE];
  virtual void SolidBtn         (RTMessage Msg) = [ID_FIRST + ID_SOLID];
  virtual void DashBtn          (RTMessage Msg) = [ID_FIRST + ID_DASH];
  virtual void DotBtn           (RTMessage Msg) = [ID_FIRST + ID_DOT];
  virtual void DashDotBtn       (RTMessage Msg) = [ID_FIRST + ID_DASHDOT];

};

_CLASSDEF(TWorksheet)
class TWorksheet: public TWindow
{
 public:
 PTSpread        Spread;
 SS_CELLCOORD    UL,LR;
 SS_CELLTYPE 	 CellType;
 LOGFONT         lfFont;
 HFONT   	 hFont;
 LPSTR           lpFormula;
 char szFileName [MAXPATH];
 char szTitle    [MAXPATH];
 FARPROC lpfnGetPeriod;
 FARPROC lpfnGetCurrPeriod;
 FARPROC lpfnGetOpen;
 HWND SHWindow;
 RECT Rect;
 TWorksheet(PTWindowsObject AParent, LPSTR ATitle):TWindow(AParent,ATitle)
 {
 Spread = new TSpread(this,ID_SPREAD,"",0,0,0,0);
 strcpy(szTitle,ATitle);
 };
 ~TWorksheet(){
  FreeProcInstance(lpfnGetPeriod);
  FreeProcInstance(lpfnGetCurrPeriod);
  FreeProcInstance(lpfnGetOpen);
 }
 virtual void SetupWindow();
 virtual void WMSize         (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void WMRButtonDown  (RTMessage Msg) = [WM_FIRST + SSM_RBUTTON];
 virtual void Cut            (RTMessage Msg) = [CM_FIRST + 101];
 virtual void Copy           (RTMessage Msg) = [CM_FIRST + 102];
 virtual void Paste          (RTMessage Msg) = [CM_FIRST + 103];
 virtual void Border         (RTMessage Msg) = [CM_FIRST + 104];
 virtual void Font           (RTMessage Msg) = [CM_FIRST + 105];
 virtual void Color          (RTMessage Msg) = [CM_FIRST + 106];
 virtual void Number         (RTMessage Msg) = [CM_FIRST + 107];
 virtual void Character      (RTMessage Msg) = [CM_FIRST + 108];
 virtual void AccountNo      (RTMessage Msg) = [CM_FIRST + 109];
 virtual void PrintSheet     (RTMessage Msg) = [CM_FIRST + 110];
 virtual void CMOpen         (RTMessage Msg) = [CM_FIRST + 112];
 virtual void CMSave         (RTMessage Msg) = [CM_FIRST + 113];
 virtual BOOL CMSaveAs       (RTMessage Msg) = [CM_FIRST + 114];
 virtual void SetPageBreak   (RTMessage Msg) = [CM_FIRST + 115];
 virtual BOOL Save();
 virtual BOOL SaveAs();
 virtual void Open(char *szFileName);
 virtual void WMMDIActivate  (TMessage& Msg);
 virtual BOOL CanClose ();
 virtual void GetWindowClass(WNDCLASS& wc);
};

_CLASSDEF (TSetValue)
class  TSetValue : public TDialog
{
 public:
  PTGLAcctList GLAcctList;
  PTglaccts glaccts;
  PTSpread Spread;
  SS_CELLTYPE 	CellType;
  int row;
  int col;
  TSetValue(PTWindowsObject AParent,LPSTR AName,int Row,int Col,PTSpread pSpread)
  : TDialog(AParent,AName)
  {
   Spread = pSpread;
   row = Row;
   col = Col;
  };
  virtual void WMInitDialog     (RTMessage Msg) = [WM_FIRST + WM_INITDIALOG];
  virtual void Ok               (RTMessage Msg) = [ID_FIRST + IDOK];
  virtual void HandleSearchBtn  (RTMessage Msg) = [ID_FIRST + ID_SEARCH];
};

