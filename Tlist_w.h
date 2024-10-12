#ifndef __TLIST_W_H
#define __TLIST_W_H

_CLASSDEF(TListWindow)
class TListWindow : public TSpread
{
public:
  Code4 *cb;
  BOOL bRedraw,bCanClose;
  SS_CELLTYPE 	CellType;
  short iHeight,iHeader,Rows;
  UINT iTop,iLeft,iRight,iBottom,iField1,iField2,iField3,iFieldNumber;
  int  iSeekLength;
  string strSeekBuffer;
  RECT  rc,wd;
  //Data4 *dBase;
  COLORREF lpBack, lpFore;
  UINT iTimer;
  char szlpAWindow[9];
  char szIdx[9];
  char szQuery[100];
  TListWindow (PTWindowsObject AParent, int AnId, LPSTR ATitle, int X,
		int Y, int W, int H,LPSTR AWindow, PTModule AModule = NULL)
	: TSpread(AParent, AnId, ATitle, X, Y, W, H, AModule)
	{
	 strcpy(szlpAWindow,AWindow);
	 bRedraw = FALSE;
	 bCanClose = FALSE;
	}
  TListWindow (PTWindowsObject AParent, int ResourceId,
		PTModule AModule = NULL)
	: TSpread(AParent, ResourceId, AModule)	{}
  ~TListWindow();
  virtual void SetupWindow();
  virtual void SetParamenters     (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
  virtual void WMSize (RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void Cancel (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void GetKey (RTMessage Msg) = [WM_FIRST + WM_CHAR];
  void SetDesc();
  void SetFieldType(int iType,long nColumn);
//virtual void SetTimer (RTMessage Msg) = [WM_FIRST + WM_TIMER];
  virtual BOOL CanClose();
  BOOL SetFields(char *szdBase,char *szIndex,int iNumberOfFields = 0);
  BOOL SetFields();
  void Redraw(void);
  BOOL SetPosition();
  void Redraw(char *);
  BOOL Seek (UINT ID);
  BOOL SavePosition();
  //void Seek(char *szSeek,int len);
};

#endif
