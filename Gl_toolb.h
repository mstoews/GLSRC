#ifndef __GL_TOOLB_H
#define __GL_TOOLB_H

#include "gl_manil.h"

_CLASSDEF(TGLToolBox)
class TGLToolBox : public TWindow
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

  TGLToolBox(PTWindowsObject AParent,LPSTR ATitle):TWindow(AParent,ATitle){};  
  virtual void SetupWindow();
  virtual void VendorList      (RTMessage Msg) = [ID_FIRST + ID_SEARCHCUST]{Send(ID_VIEW);};
  virtual void SlipList        (RTMessage Msg) = [ID_FIRST + ID_SLIPLIST]	{Send(Msg.WParam);};
  virtual void SearchGL        (RTMessage Msg) = [ID_FIRST + ID_VIEW]		{Send(Msg.WParam);};
  virtual void New             (RTMessage Msg) = [ID_FIRST + ID_NEW]		   {Send(Msg.WParam);};
  virtual void ProductList     (RTMessage Msg) = [ID_FIRST + ID_PRODUCT]	{Send(ID_PRODUCT);};
  virtual void Top             (RTMessage Msg) = [ID_FIRST + ID_TOP]		   {Send(Msg.WParam);};
  virtual void Bottom          (RTMessage Msg) = [ID_FIRST + ID_BOTTOM]		{Send(Msg.WParam);};
  virtual void Next            (RTMessage Msg) = [ID_FIRST + ID_NEXT]		{Send(Msg.WParam);};
  virtual void Previous        (RTMessage Msg) = [ID_FIRST + ID_PREVIOUS]	{Send(Msg.WParam);};
  virtual void Delete          (RTMessage Msg) = [ID_FIRST + ID_DELETE]		{Send(Msg.WParam);};
  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL]		{Send(Msg.WParam);};
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK]		{Send(ID_ADD);};
  virtual void WMActivate      (RTMessage Msg) = [WM_FIRST + WM_ACTIVATE];
  virtual void WMPaint         (RTMessage Msg) = [WM_FIRST + WM_PAINT];
  virtual void WMNchittest     (RTMessage Msg) = [WM_FIRST + WM_NCHITTEST];
  LRESULT Send(WPARAM ID)
	{
	 LRESULT rc = SendMessage(Parent->HWindow,WM_COMMAND,ID,NULL);
	 return rc;
	};
 void SetBitmap(int ID,int iPosition);

 virtual BOOL CanClose(){return FALSE;};
};
#endif