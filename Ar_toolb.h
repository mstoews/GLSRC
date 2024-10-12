#ifndef __AR_TOOLB_H
#define __AR_TOOLB_H

_CLASSDEF(TARToolBox)
//class TARToolBox : public TMainDialog
class TARToolBox : public TWindow
{
 public:  
  PAINTSTRUCT ps;
  POINT pt;
  HBRUSH hCaptionBrush;
  HPEN hOldPen;
  HFONT  hOldFont,hCaptionFont;
  char   szCaption[128];
  RECT rc;
  HDC hDC;

 PTSuperButton bSearchAccounts;  
 PTSuperButton bBottom;
 PTSuperButton bTop;
 PTSuperButton bNext;
 PTSuperButton bPrevious;
 PTSuperButton bNew;
 PTSuperButton bSlips;
 PTSuperButton bOk;
 PTSuperButton bDelete;
 PTSuperButton bCancel;
 PTSuperButton bMemo;
 PTSuperButton bCustomers;

  TARToolBox(PTWindowsObject AParent,LPSTR ATitle):TWindow(AParent,ATitle)
  {
  };
  virtual void SetupWindow();
  virtual void VendorList      (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST]	{Send(ID_SEARCHCUST);};
  virtual void SlipList        (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST]	{Send(ID_SLIPLIST);};
  virtual void SearchGL        (RTMessage Msg) = [ID_FIRST + ID_VIEW]		{Send(ID_VIEW);};
  virtual void New             (RTMessage Msg) = [ID_FIRST + ID_NEW]		{Send(ID_NEW);};
  virtual void ProductList     (RTMessage Msg) = [ID_FIRST + ID_PRODUCT]	{Send(ID_PRODUCT);};
  virtual void Top             (RTMessage Msg) = [ID_FIRST + ID_TOP]		{Send(ID_TOP);};
  virtual void Bottom          (RTMessage Msg) = [ID_FIRST + ID_BOTTOM]		{Send(ID_BOTTOM);};
  virtual void Next            (RTMessage Msg) = [ID_FIRST + ID_NEXT]		{Send(ID_NEXT);};
  virtual void Previous        (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS]	{Send(ID_PREVIOUS);};
  virtual void Delete          (RTMessage Msg) = [ID_FIRST + ID_DELETE]		{Send(ID_DELETE);};
  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL]		{Send(IDCANCEL);};
  virtual void Memo            (RTMessage Msg) = [ID_FIRST + ID_MEMO]           {Send(ID_MEMO);};
  virtual void WMActivate      (RTMessage Msg) = [WM_FIRST + WM_ACTIVATE];
  virtual void WMPaint         (RTMessage Msg) = [WM_FIRST + WM_PAINT];
  virtual void WMNchittest     (RTMessage Msg) = [WM_FIRST + WM_NCHITTEST];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK]		{Send(IDOK);};
  LRESULT Send(WPARAM ID)
   {
    LRESULT rc = SendMessage(Parent->HWindow,WM_COMMAND,ID,NULL);
    return rc; 
   };
 void SetBitmap(int ID, int iPosition);

 virtual BOOL CanClose(){return FALSE;};
};
#endif