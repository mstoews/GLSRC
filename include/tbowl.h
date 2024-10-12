/***********************************************************************
* FarPoint Technologies  (C) Copyright 1991, 1992  All Rights Reserved
* portions (C) Copyright 1991, 1992 Applegate Sotware
***********************************************************************/

#ifndef __TBOWL_H
#define __TBOWL_H

#ifndef __OWL_H
#include <owl.h>
#endif

#ifndef __EDIT_H
#include <edit.h>
#endif

#ifndef __LISTBOX_H
#include <listbox.h>
#endif

#ifndef TOOLBOX_H
#include "toolbox.h"
#endif

/*********************************************************
* User callable functions to support ToolBox edit fields
*********************************************************/

_CLASSDEF(TExEdit)
class _EXPORT TExEdit: public TEdit
{
public:
  TExEdit(PTWindowsObject AParent, int AnId, LPSTR AText, int X,
	  int Y, int W, int H, WORD ATextLen, BOOL Multiline,
	  PTModule AModule = NULL)
	: TEdit(AParent, AnId, AText, X,Y,W,H, ATextLen, Multiline, AModule) {};

  TExEdit(PTWindowsObject AParent, int ResourceId, WORD ATextLen,
	  PTModule AModule = NULL)
	: TEdit(AParent, ResourceId, ATextLen, AModule) {};

  BOOL Get3D(LPEDIT3DFORMAT lpFormat)
	{ return (BOOL)SendMessage(HWindow, EM_GET3D, 0, (LPARAM)lpFormat); }

  BOOL Set3D(BOOL DoShow, LPEDIT3DFORMAT lpFormat)
	{ return (BOOL)SendMessage(HWindow, EM_SET3D, (WPARAM)DoShow, (LPARAM)lpFormat); }

  TExEdit& operator=(LPSTR aText)
	{ SetText(aText); return *this; }

protected:
  virtual LPSTR GetClassName()
	{ return "tbEdit"; }

}; // end class TExEdit

_CLASSDEF(TExCheckBox)
class _EXPORT TExCheckBox : public TControl
{
public:
   TExCheckBox (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	     int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};
 
   TExCheckBox (PTWindowsObject AParent, int ResourceId, 
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

// Attributes
  WORD GetCheck()
        { return (BOOL)::SendMessage(HWindow, BM_GETCHECK, 0, 0L); }

  WORD GetState()
        { return (WORD)::SendMessage(HWindow, BM_GETSTATE, 0, 0L); }

// Operations
  void SetCheck(WORD aCheck)
        { ::SendMessage(HWindow, BM_SETCHECK, aCheck, 0L); }

  void SetPict(WORD aType, HBITMAP hBitMap)
        { ::SendMessage(HWindow, BM_SETPICT, aType, (LONG)hBitMap); }

  void SetPict(WORD aType, LPSTR lpszBitMap)
        { ::SendMessage(HWindow, BM_SETPICT, aType, (LONG)lpszBitMap); }

  void SetState(WORD aState)
        { ::SendMessage(HWindow, BM_SETSTATE, aState, 0L); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbCheckBox"; }

}; // end class TExCheckBox



const MAXDATELEN = 12;

_CLASSDEF(TDateEdit)
class _EXPORT TDateEdit: public TExEdit
{
 public:                                                         
  TDateEdit(PTWindowsObject AParent, int AnId, LPSTR AText, int X,
	    int Y, int W, int H, WORD ATextLen=16, BOOL Multiline=FALSE,
	    PTModule AModule = NULL)
	: TExEdit(AParent, AnId, AText, X, Y, W, H, ATextLen, 
	  Multiline, AModule) {};

  TDateEdit (PTWindowsObject AParent, int ResourceId, WORD ATextLen=16,
	     PTModule AModule = NULL)
	: TExEdit(AParent, ResourceId, ATextLen, AModule) {};

  BOOL GetDMY(LPDATE ADate)
	{ return DateGetDMY(HWindow, ADate); }

  BOOL GetFormat(LPDATEFORMAT aDateFmt)
	{ return DateGetFormat(HWindow, aDateFmt); }

  BOOL SetDMY(LPDATE ADate)
	{ return DateSetDMY(HWindow, ADate); }

  BOOL SetFormat(LPDATEFORMAT aDateFmt)
	{ return DateSetFormat(HWindow, aDateFmt); }

  BOOL SetRange(LPDATE aMinDate, LPDATE aMaxDate) 
	{ return DateSetRange(HWindow, aMinDate, aMaxDate); }

  void SetToday()
	{ SendMessage(HWindow, WM_KEYDOWN, VK_F3, 0); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbDate"; }

}; // end class TDateEdit


_CLASSDEF(TDirList)
class _EXPORT TDirList : public TControl
{
public:
  TDirList(PTWindowsObject AParent, int AnId, LPSTR ATitle,
	    int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {}

  TDirList(PTWindowsObject AParent, int ResourceId, PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {} 

  void GetColor(LPDIRLISTCOLOR lpDLColor)
	{ SendMessage(HWindow, DLM_GETCOLOR, 0, (LPARAM)lpDLColor); }

  LONG GetDirCnt(void)
	{ return SendMessage(HWindow, DLM_GETDIRCNT, 0, 0L); }

  void GetSel(LPSTR aDirBuf)
	{ SendMessage(HWindow, DLM_GETSEL, 0, (LPARAM)aDirBuf); }

  void OpenChildren (WORD aChildParam)
	{ SendMessage(HWindow, DLM_OPENCHILDREN, aChildParam, 0); }

  void ReadDrive(char aDrive, FARPROC aCallback)
	{ SendMessage(HWindow, DLM_READDRIVE, (WPARAM)aDrive, (LPARAM)aCallback); }

  void SetColor(LPDIRLISTCOLOR lpDLColor)
	{ SendMessage(HWindow, DLM_SETCOLOR, 0, (LPARAM)lpDLColor); }

  void SetCurDir(LPCSTR aDir)
        { ::SendMessage(HWindow, DLM_SETCURDIR, 0, (LPARAM)aDir); }

  void GetCurDir(LPCSTR aDir)
        { ::SendMessage(HWindow, DLM_GETCURDIR, 0, (LPARAM)aDir); }

  void SetSortMethod(LONG aMethod)
	{ SendMessage(HWindow, DLM_SETSORTMETHOD, 0, aMethod); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbDirList"; }

}; // end class TDirList


_CLASSDEF(TFileList)
class _EXPORT TFileList : public TControl
{
public:
  TFileList (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	     int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {}

  TFileList (PTWindowsObject AParent, int ResourceId, 
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {}

  void GetColor(LPFILELISTCOLOR lpFLColor)
	{ SendMessage(HWindow, FLM_GETCOLOR, 0, (LPARAM)lpFLColor); }

  int GetFileCount(void)
	{ return (int)SendMessage(HWindow, FLM_GETFILECOUNT, 0, 0); }

  void GetSelFile(LPSTR aFileName)
	{ SendMessage(HWindow, FLM_GETSELFILE, 0, (LPARAM)aFileName); }

  void GetCurFile(LPSTR aFileName)
        { ::SendMessage(HWindow, FLM_GETCURFILE, 0, (LPARAM)aFileName); }

  void GetSel(LPSTR aFileName)
        { ::SendMessage(HWindow, FLM_GETSEL, 0, (LPARAM)aFileName); }

  int  GetSelCount(void)
        { return (int)::SendMessage(HWindow, FLM_GETSELCOUNT, 0, 0L); }

  void SelectAll(void)
        { ::SendMessage(HWindow, FLM_SELECTALL, 0, 0L); }

  void SetCurFile(void)
       { ::SendMessage(HWindow, FLM_SETCURFILE, 0, 0L); }

  void ReadDirectory(LPSTR aDir)
	{ SendMessage(HWindow, FLM_READDIRECTORY, 0, (LPARAM)aDir); }

  void SetColor(LPFILELISTCOLOR lpFLColor)
       { SendMessage(HWindow, FLM_SETCOLOR, 0, (LPARAM)lpFLColor); }

  void SetFileSpec(LPCSTR aFileSpec)
       { SendMessage(HWindow, FLM_SETFILESPEC, 0, (LPARAM)aFileSpec); }

  void SetSelFile(LPCSTR aFileName)
       { SendMessage (HWindow, FLM_SETSELFILE, 0, (LPARAM)aFileName); }

  void SetSortMethod(LONG aMethod)
       { SendMessage(HWindow, FLM_SETSORTMETHOD, 0, aMethod); }

  void SetSortOrder(WORD aOrder)
       { SendMessage(HWindow, FLM_SETSORTORDER, aOrder, 0); }

  void SetViewType(LONG aViewType)
       { SendMessage(HWindow, FLM_SETVIEWTYPE, 0, aViewType); }

 protected:
  virtual LPSTR GetClassName()
	{ return "tbFileList"; }

}; // end class TFileList



_CLASSDEF(TDirFile)
class _EXPORT TDirFile : public TFileList
{
 public:
  TDirFile (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	    int X, int Y, int W, int H, PTModule AModule=NULL)
	: TFileList (AParent, AnId, ATitle, X, Y, W, H, AModule) {}

  TDirFile (PTWindowsObject AParent, int ResourceId, PTModule AModule=NULL)
	: TFileList (AParent, ResourceId, AModule) {}

  WORD GetSize()
	{ return (WORD)SendMessage(HWindow, DFM_GETSIZE, 0, 0L); }

  void SetSize(WORD aSize)
	{ SendMessage(HWindow, DFM_SETSIZE, aSize, 0L); }

  // from TDirList
  void GetColor(LPDIRLISTCOLOR lpDLColor)
	{ SendMessage(HWindow, DLM_GETCOLOR, 0, (LPARAM)lpDLColor); }

  LONG GetDirCnt(void)
	{ return SendMessage(HWindow, DLM_GETDIRCNT, 0, 0L); }
		  
  void GetSelDir(LPSTR aDirBuf)
	{ SendMessage(HWindow, DLM_GETSELDIR, 0, (LPARAM)aDirBuf); }

  void OpenChildren(WORD aChildParam)
	{ SendMessage(HWindow, DLM_OPENCHILDREN, aChildParam, 0L); }

  void ReadDrive(char aDrive, FARPROC aCallback)
	{ SendMessage(HWindow, DLM_READDRIVE, (WPARAM)aDrive, (LPARAM)aCallback); }

  void SetColor(LPDIRLISTCOLOR lpDLColor)
	{ SendMessage(HWindow, DLM_SETCOLOR, 0, (LPARAM)lpDLColor); }

  void SetSelDir(LPCSTR aDir)
	{ SendMessage(HWindow, DLM_SETSELDIR, 0, (LPARAM)aDir); }

  void SetSortMethod(LONG aMethod)
	{ SendMessage(HWindow, DLM_SETSORTMETHOD, 0, aMethod); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbDirFile"; }

}; // end class TDirFile


_CLASSDEF(TDriveList)
class _EXPORT TDriveList : public TControl
{
public:
  TDriveList (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	      int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {}

  TDriveList(PTWindowsObject AParent, int ResourceId, 
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {}

  void GetColor(LPDRIVELISTCOLOR lpDriveListColor)
	{ SendMessage(HWindow, DVLM_GETCOLOR, 0, (LPARAM)lpDriveListColor); }

  WORD GetDriveCount(void)
	{ return (WORD)SendMessage(HWindow, DVLM_GETDRIVECOUNT, 0, 0L); }

  char GetSelDrive(void)
	{  return (char )SendMessage(HWindow, DVLM_GETSELDRIVE, 0, 0L); }

  void SetColor(LPDRIVELISTCOLOR lpDriveListColor)
	{ SendMessage(HWindow, DVLM_SETCOLOR, 0, (LPARAM)lpDriveListColor); }

  void SetSelDrive(char aDrive)
	{ SendMessage(HWindow, DVLM_SETSELDRIVE, (WPARAM)aDrive, 0L); }

 protected:
  virtual LPSTR GetClassName ()
	{ return "tbDriveList"; }

}; // end class TDriveList


_CLASSDEF(TFloatEdit)
class _EXPORT TFloatEdit: public TExEdit
{
public:
  TFloatEdit(PTWindowsObject AParent, int AnId, LPSTR AText, int X,
	     int Y, int W, int H, WORD ATextLen=16, BOOL Multiline=FALSE,
	     PTModule AModule = NULL)
	: TExEdit(AParent, AnId, AText, X, Y, W, H, ATextLen, Multiline, 
	  AModule) {};

  TFloatEdit(PTWindowsObject AParent, int ResourceId, WORD ATextLen=16,
	     PTModule AModule = NULL)
	: TExEdit(AParent, ResourceId, ATextLen, AModule) {};

  BOOL GetValue(LPDOUBLE lpDouble)
	{ return FloatGetValue(HWindow, lpDouble); }

  BOOL SetValue(double dfDouble)
	{ return FloatSetValue(HWindow, dfDouble); }

  BOOL SetRange(double dMin, double dMax)
	{ return FloatSetRange(HWindow, dMin, dMax); }

  BOOL GetRange(LPDOUBLE lpMin, LPDOUBLE lpMax)
	{ return FloatGetRange(HWindow, lpMin, lpMax); }

  BOOL SetFormat(const LPFLOATFORMAT ff)
	{ return FloatSetFormat(HWindow, ff); }

  BOOL GetFormat(LPFLOATFORMAT ff)
	{ return FloatGetFormat(HWindow, ff); }

  void SetStyle(long Style)
	{ FloatSetStyle(HWindow, Style); }

  BOOL SetMask(LPSTR szMask)
	{ return FloatSetMask(HWindow, szMask); }

  BOOL GetMask(LPSTR szMask)
	{ return FloatGetMask(HWindow, szMask); }

  TFloatEdit& operator=(double aValue)
	{ SetValue(aValue); return *this; }

  operator double()
	{ double RetVal; GetValue(&RetVal); return RetVal; }

protected:
  virtual LPSTR GetClassName()
	{ return "tbFloat"; }

}; // end class TFloatEdit


_CLASSDEF(TGauge)
class _EXPORT TGauge : public TControl
{
public:
  TGauge (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	  int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};

  TGauge (PTWindowsObject AParent, int ResourceId, PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

  BOOL GetPos(LPINT lpPos)
	{ return GaugeGetPos(HWindow, lpPos); }

  BOOL GetRange(LPINT lpMin, LPINT lpMax)
	{ return GaugeGetRange(HWindow, lpMin, lpMax); }

  BOOL SetColors(COLORREF RGBBack, COLORREF RGBPlate, COLORREF RGBText,
		 COLORREF RGBFill)
	{ return GaugeSetColors(HWindow, RGBBack, RGBPlate, RGBText, RGBFill); }

  BOOL SetPos(int aPos)
	{ return GaugeSetPos(HWindow, aPos); }

  BOOL SetRange(int aMin, int aMax)
	{ return GaugeSetRange(HWindow, aMin, aMax); }

  LPSTR SetTitle(LPSTR aTitle)
	{ return GaugeSetTitle(HWindow, aTitle); }

 protected:
  virtual LPSTR GetClassName()
	{ return "tbGauge"; }

}; // end class TGauge


_CLASSDEF(TImprint)
class _EXPORT TImprint : public TControl
{
public:
   TImprint (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	     int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};
 
   TImprint (PTWindowsObject AParent, int ResourceId, 
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

   void GetColor(LPIMPRINTCOLOR lpColor)
	{ SendMessage(HWindow, IM_GETCOLOR, 0, (LPARAM)lpColor); }

   LONG GetFrameSize(void)
	{ return SendMessage(HWindow, IM_GETFRAMESIZE, 0, 0L); }

   LONG GetLineCnt(void)
	{ return SendMessage(HWindow, IM_GETLINESIZE, 0, 0L); }

   LONG GetLineSize(void)
	{ return SendMessage(HWindow, IM_GETLINECNT, 0, 0L); }

   LONG GetShadowSize(void)
	{ return SendMessage(HWindow, IM_GETSHADOWSIZE, 0, 0L); }

   void SetColor(LPIMPRINTCOLOR lpColor)
	{ SendMessage(HWindow, IM_SETCOLOR, 0, (LPARAM)lpColor); }

   void SetFrameSize(WORD aFrameSize)
	{ SendMessage(HWindow, IM_SETFRAMESIZE, aFrameSize, 0L); }

   void SetLineCnt(WORD HorzLines, WORD VertLines) 
	{ SendMessage(HWindow, IM_SETLINECNT, 0, MAKELPARAM(HorzLines, VertLines)); }

   void SetLineSize(WORD aLineSize)
	{ SendMessage(HWindow, IM_SETLINESIZE, aLineSize, 0); }

   void SetShadowSize(WORD aShadowSize)
	{ SendMessage(HWindow, IM_SETSHADOWSIZE, aShadowSize, 0); }

protected:
    virtual LPSTR GetClassName()
	{ return "tbImprint"; }

}; // end class TImprint


_CLASSDEF(TIntEdit)
class _EXPORT TIntEdit: public TExEdit
{
public:
  TIntEdit(PTWindowsObject AParent, int AnId, LPSTR AText, int X,
	   int Y, int W, int H, WORD ATextLen=10, BOOL Multiline=FALSE,
	   PTModule AModule = NULL)
	: TExEdit(AParent, AnId, AText, X, Y, W, H, ATextLen, Multiline, 
	  AModule) {};

  TIntEdit(PTWindowsObject AParent, int ResourceId, WORD ATextLen=10,
	   PTModule AModule = NULL)
	   : TExEdit(AParent, ResourceId, ATextLen, AModule) {};

  BOOL GetMask(LPSTR AMask)
	{ return IntGetMask(HWindow, AMask); }

  BOOL GetRange(LPLONG lpMin, LPLONG lpMax)
	{ return IntGetRange(HWindow, lpMin, lpMax); }

  BOOL GetSpin(LPBOOL lpfSpinWrap, LPLONG lplSpinInc)
	{ return IntGetSpin(HWindow, lpfSpinWrap, lplSpinInc); }

  BOOL GetValue(LPLONG lpLong)
	{ return IntGetValue(HWindow, lpLong); }

  BOOL SetMask(LPSTR szMask)
	{ return IntSetMask(HWindow, szMask); }

  BOOL SetRange(LONG lMin, LONG lMax)
	{ return IntSetRange(HWindow, lMin, lMax); }

  BOOL SetSpin(BOOL fSpin, BOOL fSpinWrap, long lSpinInc)
	{ return IntSetSpin(HWindow, fSpin, fSpinWrap, lSpinInc); }

  BOOL SetValue(long lLong)
	{ return IntSetValue(HWindow, lLong); }

  TIntEdit& operator=(LONG aValue)
	{ SetValue(aValue); return *this; }

  operator LONG()
	{ LONG RetVal; GetValue(&RetVal); return RetVal; }

protected:
  virtual LPSTR GetClassName(void)
	{ return "tbInteger"; }

}; // end class TIntEdit


_CLASSDEF(TExListBox)
class _EXPORT TExListBox : public TListBox
{
 public:
  TExListBox (PTWindowsObject AParent, int AnId, int X, int Y,
	      int W, int H, PTModule AModule = NULL)
	: TListBox(AParent, AnId, X, Y, W, H, AModule) {};

  TExListBox (PTWindowsObject AParent, int ResourceId,
	      PTModule AModule = NULL)
	: TListBox (AParent, ResourceId, AModule) {};

  LONG GetDblClkHPos(void)
	{ return SendMessage(HWindow, LB_GETDBLCLKHPOS, 0, 0); }

  LONG GetDblClkItem(void)
	{ return SendMessage(HWindow, LB_GETDBLCLKITEM, 0, 0); }

  LONG GetTextExtent(WORD aFlag, LPSTR aStr)
	{ return SendMessage(HWindow, LB_GETTEXTEXTENT, aFlag, (LPARAM)aStr); }

  void SetSelectionMode(WORD aSelMode)
	{ SendMessage(HWindow, LB_SETSELECTIONMODE, aSelMode, 0); }

 protected:
  virtual LPSTR GetClassName()
	{ return "tbListBox"; }

}; // end class TExListbox


_CLASSDEF(TPicEdit)
class _EXPORT TPicEdit : public TExEdit
{
public:
  TPicEdit(PTWindowsObject AParent, int AnId, LPSTR AText, int X,
	   int Y, int W, int H, WORD ATextLen=0, BOOL Multiline=FALSE,
	   PTModule AModule = NULL);

  TPicEdit(PTWindowsObject AParent, int ResourceId, WORD ATextLen=0,
	   PTModule AModule = NULL);

  virtual BOOL Create(void);

  void FormatText(LPSTR lpszDefText, LPSTR lpszMask, LPSTR lpszText, LPSTR lpszTextFrmt,
      BOOL fTextUnformatted)
      { :: PicFormatText(HWindow, lpszDefText, lpszMask, lpszText, lpszTextFrmt, fTextUnformatted); }

  void GetMask(LPSTR AMask)
	{ SendMessage(HWindow, EPM_GETMASK, 0, (LPARAM)AMask); }

  void GetValue(LPSTR AValue)
	{ SendMessage(HWindow, EPM_GETVALUE, 0, (LPARAM)AValue); }

  void SetCallBack(FARPROC AProc)
	{ SendMessage(HWindow, EPM_SETCALLBACK, 0, (LPARAM)AProc); }

  BOOL SetMask(LPCSTR AMask)
	{ return (BOOL)SendMessage(HWindow, EPM_SETMASK, 0, (LPARAM)AMask); }

  void SetValue(LPCSTR AValue)
	{ SendMessage(HWindow, EPM_SETVALUE, 0, (LPARAM)AValue); }

protected:

  virtual LPSTR GetClassName()
	{ return "tbPic"; }

}; // end class TPicEdit


_CLASSDEF(TProgress)
class _EXPORT TProgress : public TControl
{
public:
  TProgress (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	     int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};

  TProgress (PTWindowsObject AParent, int ResourceId, 
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

  LONG GetPos(void)
	{ return SendMessage(HWindow, PM_GETPOS, 0, 0L); }

  LONG GetRange(void)
	{ return SendMessage(HWindow, PM_GETRANGE, 0, 0L); }

  void SetPos(WORD aPos)
	{ SendMessage(HWindow, PM_SETPOS, aPos, 0L); }

  void SetRange(WORD aMin, WORD aMax)
	{ SendMessage(HWindow, PM_SETRANGE, 0, MAKELPARAM(aMin, aMax)); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbProgress"; }

}; // end class TProgress


_CLASSDEF(TExRadioButton)
class _EXPORT TExRadioButton : public TControl
{
public:
   TExRadioButton (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	     int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};
 
   TExRadioButton (PTWindowsObject AParent, int ResourceId, 
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

// Attributes
  WORD GetCheck()
        { return (BOOL)::SendMessage(HWindow, BM_GETCHECK, 0, 0L); }

  WORD GetState()
        { return (WORD)::SendMessage(HWindow, BM_GETSTATE, 0, 0L); }

// Operations
  void SetCheck(WORD aCheck)
        { ::SendMessage(HWindow, BM_SETCHECK, aCheck, 0L); }

  void SetPict(WORD aType, HBITMAP hBitMap)
        { ::SendMessage(HWindow, BM_SETPICT, aType, (LONG)hBitMap); }

  void SetPict(WORD aType, LPSTR lpszBitMap)
        { ::SendMessage(HWindow, BM_SETPICT, aType, (LONG)lpszBitMap); }

  void SetState(WORD aState)
        { ::SendMessage(HWindow, BM_SETSTATE, aState, 0L); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbRadioButton"; }

}; // end class TExRadioButton


_CLASSDEF(TSpinButton)
class _EXPORT TSpinButton : public TControl
{
 public:
  TSpinButton (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	       int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl(AParent, AnId, ATitle, X, Y, W, H, AModule) {};

  TSpinButton (PTWindowsObject AParent, int ResourceId, 
	       PTModule AModule=NULL)
	: TControl(AParent, ResourceId, AModule) {};

 protected:
  virtual LPSTR GetClassName()
	{ return "tbSpinButton"; }

}; // end class TSpinButton


_CLASSDEF(TStatusBar)
class _EXPORT TStatusBar : public TControl
{
public:
  LPSTATUSBARITEM Items;
  int ItemCount;

  TStatusBar (PTWindowsObject AParent, int AnId, LPSTR ATitle, int X, int Y,
	      int W, int H, LPSTATUSBARITEM TheItems, int AnItemCount,
	      PTModule AModule=NULL);

  TStatusBar (PTWindowsObject AParent, int ResourceId,
	      LPSTATUSBARITEM TheItems, int AnItemCount,
	      PTModule AModule=NULL);
 
  void GetColor (LPSTATUSBARCOLOR lpSBColor)
	{ SendMessage(HWindow, SBRM_GETCOLOR, 0, (LPARAM)lpSBColor); }

  void GetItemRect(WORD aItemID, LPRECT lpRect)
	{  SendMessage(HWindow, SBRM_GETITEMRECT, aItemID, (LPARAM)lpRect); }

  void GetItemText(WORD aItemID, LPSTR aBuff)
	{  SendMessage(HWindow, SBRM_GETITEMTEXT, aItemID, (LPARAM)aBuff); }

  void SetColor(LPSTATUSBARCOLOR lpSBColor)
	{ SendMessage(HWindow, SBRM_SETCOLOR, 0, (LPARAM)lpSBColor); }

  BOOL SetItems(WORD nItems, LPSTATUSBARITEM Items)
	{ return (BOOL)SendMessage(HWindow, SBRM_SETITEMS, nItems, (LPARAM)Items); }

  BOOL SetItemText(WORD aItemID, LPCSTR aText)
	{ return (BOOL)SendMessage (HWindow, SBRM_SETITEMTEXT,  aItemID, (LPARAM)aText); }

  void SetProgressPos(WORD ItemID, LONG position)
        { ::SendMessage(HWindow, SBRM_SETPROGRESSPOS, ItemID, position); }


  void SetProgressRange(WORD aItemID, WORD aMinRange, WORD aMaxRange)
	{ SendMessage(HWindow, SBRM_SETPROGRESSRANGE, aItemID, MAKELPARAM(aMinRange, aMaxRange)); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbStatus"; }
  virtual BOOL Create();

}; // end class TStatusBar


_CLASSDEF(TSuperButton)
class _EXPORT TSuperButton : public TControl
{
public:
  TSuperButton(PTWindowsObject AParent, int AnId, LPSTR ATitle,
	      int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};

  TSuperButton(PTWindowsObject AParent, int ResourceId, 
	       PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

  BOOL GetButtonState()
	{ return (BOOL)::SendMessage(HWindow, SBM_GETBUTTONSTATE, 0, 0L); }
    
  void GetColor(LPSUPERBTNCOLOR lpSBColor)
	{ SendMessage(HWindow, SBM_GETCOLOR, 0, (LPARAM)lpSBColor); }

  WORD GetShadowSize()
	{ return (BOOL)::SendMessage(HWindow, SBM_GETSHADOWSIZE, 0, 0L); }

  BOOL GetStretch(void)
	{ return (BOOL)SendMessage(HWindow, SBM_GETSTRETCH, 0, 0L); }

  void SetButtonState(BOOL fButtonDown)
	{ ::SendMessage(HWindow, SBM_SETBUTTONSTATE, (WORD)fButtonDown, 0L); }
        
  void SetButtonType(WORD state)
	{ ::SendMessage(HWindow, SBM_SETBUTTONTYPE, (WORD)state, 0L); }

  void SetColor(LPSUPERBTNCOLOR lpSBColor)
	{ SendMessage(HWindow, SBM_SETCOLOR, 0, (LPARAM)lpSBColor); }

  BOOL SetPict(WORD aPicType, LPCSTR aPicName)
	{  return (BOOL)SendMessage(HWindow, SBM_SETPICT, aPicType, (LPARAM)aPicName); }

  BOOL SetPictBtnDown(WORD aPicType, LPCSTR aPicName)
	{ return (BOOL)SendMessage(HWindow, SBM_SETPICT_BTNDOWN, aPicType, (LPARAM)aPicName); }

  BOOL SetPictDisabled(WORD aPicType, LPCSTR aPicName)
	{ return (BOOL)SendMessage(HWindow, SBM_SETPICT_DISABLED, aPicType, (LPARAM)aPicName); }

  void SetShadowSize (WORD size)
        { ::SendMessage(HWindow, SBM_SETSHADOWSIZE, size, 0L); }
        
  void SetStyle(WORD aBtnStyle)
	{ SendMessage(HWindow, SBM_SETSTYLE, aBtnStyle, 0L); }

  void SetStretch(BOOL fDoStrech)
	{ SendMessage(HWindow, SBM_SETSTRETCH, (WPARAM)fDoStrech, 0L); }

protected:
  virtual LPSTR GetClassName ()
	{ return "tbSuperbtn"; }

}; // end class TSuperButton


const MAXTIMELEN = 11;

_CLASSDEF(TTimeEdit)
class _EXPORT TTimeEdit: public TExEdit
{
public:
  TTimeEdit(PTWindowsObject AParent, int AnId, LPSTR AText, int X,
	    int Y, int W, int H, WORD ATextLen=6, BOOL Multiline=FALSE,
	    PTModule AModule = NULL)
	: TExEdit(AParent, AnId, AText, X, Y, W, H, ATextLen, Multiline, AModule) {};

  TTimeEdit(PTWindowsObject AParent, int ResourceId, WORD ATextLen=6,
	    PTModule AModule = NULL)
	: TExEdit(AParent, ResourceId, ATextLen, AModule) {};

  BOOL GetFormat(LPTIMEFORMAT aTimeFmt)
	{ return TimeGetFormat(HWindow, aTimeFmt); }

  BOOL SetFormat(LPTIMEFORMAT aTimeFmt)
	{ return TimeSetFormat(HWindow, aTimeFmt); }

  BOOL SetRange(LPTIME aMinDate, LPTIME aMaxDate)
	{  return TimeSetRange(HWindow, aMinDate, aMaxDate); }

protected:
  virtual LPSTR GetClassName()
	{ return "tbTime"; }

};  // end class TTimeEdit


_CLASSDEF(TToolBox)
class _EXPORT TToolBox : public TWindow
{
public:
  int ColCnt;
  int RowCnt;
  int BorderSize;

  TToolBox(PTWindowsObject AParent, LPSTR ATitle, int aColCnt, int aRowCnt,
	   int aH=30, int aW=30, PTModule AModule=NULL);

  void AddItem(LPTOOLBOXITEM lpTBItem)
	{ SendMessage(HWindow, TBXM_ADDITEM, 0, (LPARAM)lpTBItem); }

  BOOL ChangeItem(WORD AnID, LPTOOLBOXITEM lpnItem)
	{ return (BOOL)SendMessage(HWindow, TBXM_CHANGEITEM, AnID, (LPARAM)lpnItem); }

  void DeleteItem(WORD AnID)
	{ SendMessage(HWindow, TBXM_DELETEITEM, AnID, 0L); }

  void GetColor(LPTOOLBOXCOLOR lpTBColor)
	{ SendMessage(HWindow, TBXM_GETCOLOR, 0, (LPARAM)lpTBColor); }

  LONG GetItemCnt(void)
	{ return SendMessage(HWindow, TBXM_GETITEMCNT, 0, 0L); }

  BOOL GetItemRect(WORD AnID, LPRECT lpRect)
	{ return (BOOL)SendMessage(HWindow, TBXM_GETITEMRECT, AnID, (LPARAM)lpRect); }

  LONG GetItemState(WORD aItemID)
	{ return SendMessage (HWindow, TBXM_GETITEMSTATE, aItemID, 0L); }

  void InsertItem(WORD aIndex, LPTOOLBOXITEM lpTBItem)
	{  SendMessage (HWindow, TBXM_INSERTITEM, aIndex, (LPARAM)lpTBItem); }

  void SetColor(LPTOOLBOXCOLOR lpTBColor)
	{ SendMessage (HWindow, TBXM_SETCOLOR, 0, (LPARAM)lpTBColor); }

  BOOL SetGroup(WORD AnID, WORD Pixels, WORD AStyle)
	{ return (BOOL)SendMessage(HWindow, TBXM_SETGROUP, AnID, MAKELPARAM(Pixels, AStyle)); }

  void SetItemState(WORD aItemID, LONG aItemState)
	{  SendMessage (HWindow, TBXM_SETITEMSTATE, aItemID, aItemState); }

protected:
  void WMPaint(RTMessage Msg) = [WM_FIRST + WM_PAINT]
	{ DefWndProc(Msg); } 

  virtual BOOL Create (void);

  virtual LPSTR GetClassName ()
	{ return "tbToolBox"; }

}; // end class TToolBox


_CLASSDEF(TViewPic)
class _EXPORT TViewPic : public TControl
{
 public:
  TViewPic (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	    int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl(AParent, AnId, ATitle, X, Y, W, H, AModule) {};

  TViewPic(PTWindowsObject AParent, int ResourceId, PTModule AModule=NULL)
	: TControl(AParent, ResourceId, AModule) {};

  void Clear(void)
	{ SendMessage(HWindow, VPM_CLEAR, 0, 0L); }

  void FreezeAnimation(void)
	{ SendMessage(HWindow, VPM_FREEZEANIMATION, 0, 0L); }

  HPALETTE GetPalette(WORD AFrameID)
	{ return (HPALETTE)SendMessage(HWindow, VPM_GETPALETTE, AFrameID, 0L); }

  LONG GetPointerPos(void)
	{ return SendMessage(HWindow, VPM_GETPOINTERPOS, 0, 0L); }

  LONG SetFrames(LPVIEWPICT_ANIMATE lpViewPictAnimate)
	{ return SendMessage(HWindow, VPM_SETFRAMES, 0, (LPARAM)lpViewPictAnimate); }

  void SetPalette(WORD AFrameID, HPALETTE aPalette)
	{ SendMessage(HWindow, VPM_SETPALETTE, AFrameID, (LPARAM)aPalette); }

  LONG SetPicture(WORD aStyle, LPCSTR aName)
	{ return SendMessage(HWindow, VPM_SETPICTURE, aStyle, (LPARAM)aName); } 

  void SetStyle(LONG aStyle)
	{ SendMessage(HWindow, VPM_SETSTYLE, 0, aStyle); }

  void ShowNextFrame(void)  
	{ SendMessage(HWindow, VPM_SHOWNEXTFRAME, 0, 0L); }

  void ShowPrevFrame(void)
	{ SendMessage(HWindow, VPM_SHOWPREVFRAME, 0, 0L); }

  void StartAnimation(void) 
	{ SendMessage(HWindow, VPM_STARTANIMATION, 0, 0L); }

 protected:
  virtual LPSTR GetClassName ()
	{ return "tbViewPict"; }

}; // end class TViewPic


_CLASSDEF(TViewText)
class _EXPORT TViewText : public TControl
{
public:
  TViewText (PTWindowsObject AParent, int AnId, LPSTR ATitle,
	     int X, int Y, int W, int H, PTModule AModule=NULL)
	: TControl (AParent, AnId, ATitle, X, Y, W, H, AModule) {};

  TViewText (PTWindowsObject AParent, int ResourceId,
	     PTModule AModule=NULL)
	: TControl (AParent, ResourceId, AModule) {};

  void AddString(LPCSTR aStr)
	{ SendMessage(HWindow, VTM_ADDSTRING, 0, (LPARAM)aStr); }

  void Clear(void)  
	{ SendMessage(HWindow, VTM_CLEAR, 0, 0L); }

  void DeleteString(WORD aInd)
	{ SendMessage(HWindow, VTM_DELETESTRING, aInd, 0L); }

  void GetColor(LPVIEWTEXTCOLOR lpColor)
	{ SendMessage(HWindow, VTM_GETCOLOR, 0, (LPARAM)lpColor); }

  LONG GetLineCount(void)
	{ return SendMessage(HWindow, VTM_GETLINECOUNT, 0, 0); }

  void GetString(WORD aInd, LPCSTR aStr)
	{ SendMessage(HWindow, VTM_GETSTRING, aInd, (LPARAM)aStr); }

  LONG GetStringLen(WORD aInd)
	{ return SendMessage(HWindow, VTM_GETSTRINGLEN, aInd, 0L); }

  LONG GetTopIndex(void)
	{ return SendMessage(HWindow, VTM_GETTOPINDEX, 0, 0L); }

  void HighlightText(WORD aIndex, WORD aColPos, WORD numberOfChars)
	{ SendMessage(HWindow, VTM_HIGHLIGHTTEXT, aIndex, MAKELPARAM (aColPos, numberOfChars)); }

  void InsertString(WORD aLine, LPCSTR aStr)
	{ SendMessage(HWindow, VTM_INSERTSTRING, aLine, (LPARAM)aStr); }

  void Redraw(void)
	{ SendMessage(HWindow, VTM_REDRAW, 0, 0L); }

  BOOL Search(WORD searchMethod, LPCSTR aText)
	{ return (BOOL)SendMessage(HWindow, VTM_SEARCH, searchMethod, (LPARAM)aText); }

  BOOL SearchRepeat(void)
	{ return (BOOL)SendMessage(HWindow, VTM_SEARCHREPEAT, 0, 0L); }

  void SetColHeaders(WORD RowsToLock)
	{ SendMessage(HWindow, VTM_SETCOLHEADERS, RowsToLock, 0L); }

  void SetColor(LPVIEWTEXTCOLOR lpColor)
	{ SendMessage(HWindow, VTM_SETCOLOR, 0, (LPARAM)lpColor); }

  void SetHeader(WORD HeaderLines)
	{ SendMessage(HWindow, VTM_SETHEADER, HeaderLines, 0L); }

  void SetRowHeaders(WORD ColsToLock)
	{ SendMessage(HWindow, VTM_SETROWHEADERS, ColsToLock, 0L); }

  void SetTopIndex(WORD aLineIndex)
	{ SendMessage(HWindow, VTM_SETTOPINDEX, aLineIndex, 0L); }

protected:
  virtual LPSTR GetClassName ()
	{ return "tbViewText"; }

}; // end class TViewText


_CLASSDEF(TSpread)
class _EXPORT TSpread: public TControl
{
public:
  TSpread (PTWindowsObject AParent, int AnId, LPSTR ATitle, int X,
		int Y, int W, int H, PTModule AModule = NULL)
	: TControl(AParent, AnId, ATitle, X, Y, W, H, AModule) {}


  TSpread (PTWindowsObject AParent, int ResourceId, 
		PTModule AModule = NULL)
	: TControl(AParent, ResourceId, AModule) {}

  void Clear();

  void SetupWindow(void)
   {
	 RECT rc;
	 SS_CELLTYPE 	CellType;
	 LOGFONT 	lfFont;
	 HFONT   	hFont;
 

	 SetBool(SSB_SHOWROWHEADERS,FALSE);
	 SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
	 SetBool(SSB_SCROLLBAREXTMODE,TRUE);
	 SetBool(SSB_NOBEEP,TRUE);
	 SetBool(SSB_ALLOWUSERRESIZE,FALSE);
	 SetBool(SSB_HORZSCROLLBAR,FALSE);

	 lfFont.lfHeight          = -12;
	 lfFont.lfWidth           = 0;
	 lfFont.lfEscapement      = 0;
	 lfFont.lfOrientation     = 0;
	 lfFont.lfWeight          = FW_NORMAL;
	 lfFont.lfItalic          = 0;
	 lfFont.lfUnderline       = 0;
	 lfFont.lfStrikeOut       = 0;
	 lfFont.lfCharSet         = 1; //default character set.
	 lfFont.lfOutPrecision    = 0;
	 lfFont.lfClipPrecision   = 0;
	 lfFont.lfQuality         = 0;
	 lfFont.lfPitchAndFamily  = FF_SWISS;
	 StrCpy (lfFont.lfFaceName, "HELV");
	 hFont = CreateFontIndirect((LPLOGFONT) &lfFont); 
	 SetFont(SS_ALLCOLS, SS_ALLROWS,hFont, TRUE);
   }

  BOOL AddCustomFunction(LPSTR FunctionName, short ParameterCnt,
			 FARPROC lpfnFunctionProc)
	{ return SSAddCustomFunction(HWindow, FunctionName,
		 ParameterCnt, lpfnFunctionProc); }

  BOOL AddMultiSelBlocks(LPSS_SELBLOCK lpSelBlock)
	{ return SSAddMultiSelBlocks(HWindow, lpSelBlock); }

  BOOL BuildDependencies(void)
	{ return SSBuildDependencies(HWindow); }

  BOOL ClearCell(SS_COORD Col, SS_COORD Row)
	{ return SSClear(HWindow, Col, Row); }

  BOOL ClearData(SS_COORD Col, SS_COORD Row)
	{ return SSClearData(HWindow, Col, Row); }

  BOOL ClearDataRange(SS_COORD Col,  SS_COORD Row, 
		       SS_COORD Col2, SS_COORD Row2)
	{ return SSClearDataRange(HWindow, Col, Row, Col2, Row2); }

  BOOL ClearRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2)
	{ return SSClearRange(HWindow, Col, Row, Col2, Row2); }

  BOOL ClipboardCopy(void)
	{ return (BOOL)SSClipboardCopy(HWindow); }

  BOOL ClipboardCut(void)
	{ return SSClipboardCut(HWindow); }

  BOOL ClipboardPaste(void)
	{ return SSClipboardPaste(HWindow); }

  BOOL ClipIn(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
	      LPSTR Buffer, long BufferLen)
	{ return SSClipIn(HWindow, Col, Row, Col2, Row2, Buffer, BufferLen); }

  GLOBALHANDLE ClipOut(SS_COORD Col, SS_COORD Row,
		       SS_COORD Col2,SS_COORD Row2)
	{ return SSClipOut(HWindow, Col, Row, Col2, Row2); }

  BOOL ClipValueIn(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
	      LPSTR Buffer, long BufferLen)
	{ return SSClipValueIn(HWindow, Col, Row, Col2, Row2, Buffer, BufferLen); }

  GLOBALHANDLE ClipValueOut(SS_COORD Col, SS_COORD Row,
			    SS_COORD Col2,SS_COORD Row2)
	{ return SSClipValueOut(HWindow, Col, Row, Col2, Row2); }

  short ColWidthToLogUnits (double dfColWidth)
	{ return SSColWidthToLogUnits(HWindow, dfColWidth); }

  long ComboBoxSendMessage (short dCol, short dRow, WORD Msg, WORD wParam, LONG lParam)
	{ return SSComboBoxSendMessage(HWindow, dCol, dRow, Msg, wParam, lParam); }

  BOOL CopyRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
		 SS_COORD Row2, SS_COORD ColDest, SS_COORD RowDest)
	{ return SSCopyRange(HWindow, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL DelCol(SS_COORD Col)
	{ return SSDelCol(HWindow, Col); }

  BOOL DelColRange(SS_COORD Col, SS_COORD Col2)
	{ return SSDelColRange(HWindow, Col, Col2); }

  BOOL DelRow(SS_COORD Row)
	{ return SSDelRow(HWindow, Row); }

  BOOL DelRowRange(SS_COORD Row, SS_COORD Row2)
	{ return SSDelRowRange(HWindow, Row, Row2); }

  BOOL DeSelectBlock(void)
	{ return SSDeSelectBlock (HWindow); }

  void FloatFormat(LPSS_CELLTYPE lpCellType, double dfVal, LPSTR lpszText, BOOL fValue)
	{ SSFloatFormat(HWindow, lpCellType, dfVal, lpszText, fValue); }

  BOOL GetActiveCell (LPSS_COORD Col, LPSS_COORD Row)
	{ return SSGetActiveCell(HWindow, Col, Row); }

  void GetAutoSizeVisible (LPSS_COORD lpVisibleCols, LPSS_COORD lpVisibleRows)
	{ SSGetAutoSizeVisible(HWindow, lpVisibleCols, lpVisibleRows); }

  BOOL GetBool(short nIndex)
	{ return SSGetBool(HWindow, nIndex); }
 
  BOOL GetBorder(SS_COORD Col, SS_COORD Row, LPWORD lpwBorderType,
		 LPWORD lpwBorderStyle, LPCOLORREF lpColor)
	{ return SSGetBorder(HWindow, Col, Row, lpwBorderType, lpwBorderStyle, lpColor); }

  long GetBottomRightCell(LPSS_COORD ACol, LPSS_COORD ARow)
	{ return SSGetBottomRightCell(HWindow, ACol, ARow); }

  WORD GetButtonDrawMode(void)
	{ return SSGetButtonDrawMode(HWindow); }

  BOOL GetButtonText(SS_COORD Col, SS_COORD Row, LPSTR lpText)
	{ return SSGetButtonText(HWindow, Col, Row, lpText); }

  WNDPROC GetCallBack(void)
	{ return SSGetCallBack(HWindow); }

  BOOL GetCellFromPixel(LPSS_COORD lpCol, LPSS_COORD lpRow,
			 short MouseX, short MouseY)
	{ return SSGetCellFromPixel(HWindow, lpCol, lpRow, MouseX, MouseY); }

  BOOL GetCellRect(SS_COORD Col, SS_COORD Row, LPRECT lpRect)
	{ return SSGetCellRect(HWindow, Col, Row, lpRect); }

  void GetCellSendingMsg(LPSS_COORD lpCol, LPSS_COORD lpRow)
	{ SSGetCellSendingMsg(HWindow, lpCol, lpRow); }

  BOOL GetCellType(SS_COORD Col, SS_COORD Row, LPSS_CELLTYPE CellType)
	{ return SSGetCellType(HWindow, Col, Row, CellType); }

  void GetClientRect(LPRECT lpRect)
	{ SSGetClientRect(HWindow, lpRect); }

  BOOL GetColPageBreak(SS_COORD Col)
       { return SSGetColPageBreak(HWindow, Col); }

  BOOL GetColor(SS_COORD Col, SS_COORD Row, LPCOLORREF lpBackground,
		 LPCOLORREF lpForeground)
	{ return SSGetColor(HWindow, Col, Row, lpBackground, lpForeground); }

  BOOL GetColUserData(SS_COORD Col,LPLONG lplUserData)
	{ return SSGetColUserData(HWindow, Col, lplUserData); }

  BOOL GetColWidth(SS_COORD Col, LPDOUBLE lpColWidth)
	{ return SSGetColWidth(HWindow, Col, lpColWidth);  }

  BOOL GetColWidthInPixels(SS_COORD Col, LPSHORT lpdWidthPixels)
	{ return SSGetColWidthInPixels(HWindow, Col, lpdWidthPixels); }

  BOOL GetCurrSelBlockPos(LPSS_COORD CurrentCol, LPSS_COORD CurrentRow)
	{ return SSGetCurrSelBlockPos(HWindow, CurrentCol, CurrentRow); }

  HCURSOR GetCursor(WORD wType)
	{ return SSGetCursor(HWindow, wType); }
	
  short GetData(SS_COORD Col, SS_COORD Row, LPSTR Data)
	{ return SSGetData(HWindow, Col, Row, Data); }

  BOOL GetDataCnt(LPSS_COORD lpColCnt, LPSS_COORD lpRowCnt)
	{ return SSGetDataCnt(HWindow, lpColCnt, lpRowCnt); }

  short GetDataLen(SS_COORD Col, SS_COORD Row)
	{ return SSGetDataLen(HWindow, Col, Row); }

  BOOL GetDefFloatFormat(LPFLOATFORMAT lpFloatFormat)
	{ return SSGetDefFloatFormat(HWindow, lpFloatFormat); }

  WORD GetEditEnterAction(void)
	 { return SSGetEditEnterAction(HWindow); }

  BOOL GetEditMode(void)
	{ return SSGetEditMode(HWindow); }

  BOOL GetFirstValidCell(LPSS_COORD lpCol, LPSS_COORD lpRow)
       { return SSGetFirstValidCell(HWindow, lpCol, lpRow); }

  BOOL GetFloat(SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfValue)
	{ return SSGetFloat(HWindow, Col, Row, lpdfValue); }

  HFONT GetFont(SS_COORD Col, SS_COORD Row)
	{ return SSGetFont(HWindow, Col, Row); }

  short GetFormula(SS_COORD Col, SS_COORD Row, LPSTR lpFormula)
	{ return SSGetFormula(HWindow, Col, Row, lpFormula); }

  short GetFormulaLen(SS_COORD Col, SS_COORD Row)
	{ return SSGetFormulaLen(HWindow, Col, Row); }

  BOOL GetFreeze(LPSS_COORD lpColsFrozen, LPSS_COORD lpRowsFrozen)
	{ return SSGetFreeze(HWindow, lpColsFrozen, lpRowsFrozen); }

  void GetGrayAreaColor(LPCOLORREF lpBackground, LPCOLORREF lpForeground)
	{ SSGetGrayAreaColor(HWindow, lpBackground, lpForeground); }

  WORD GetGridType(void)
	{ return SSGetGridType(HWindow); }

  COLORREF GetGridColor(void)
	{ return SSGetGridColor(HWindow); }

  BOOL GetInteger(SS_COORD Col, SS_COORD Row, LPLONG lplValue)
	{ return SSGetInteger(HWindow, Col, Row, lplValue); }

  BOOL GetLastValidCell(LPSS_COORD lpCol, LPSS_COORD lpRow)
       { return SSGetLastValidCell(HWindow, lpCol, lpRow); }

  BOOL GetLock(SS_COORD Col, SS_COORD Row)
	{ return SSGetLock(HWindow, Col, Row); }

  void GetLockColor(LPCOLORREF lpBackground, LPCOLORREF lpForeground)
	{ SSGetLockColor(HWindow, lpBackground, lpForeground); }

  SS_COORD GetMaxCols(void)
	{ return SSGetMaxCols (HWindow); }

  SS_COORD GetMaxRows(void)
	{
	  if ( GetBool(SSB_VIRTUALMODE) )
	    return VGetMax();
	  else
	    return SSGetMaxRows(HWindow);
	}

  BOOL GetMaxTextCellSize(SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight)
	{ return SSGetMaxTextCellSize(HWindow, Col, Row, lpdfWidth, lpdfHeight); }

  BOOL GetMaxTextColWidth(SS_COORD Col, LPDOUBLE lpdfColWidth)
	{ return SSGetMaxTextColWidth(HWindow, Col, lpdfColWidth); }

  BOOL GetMaxTextRowHeight(SS_COORD Row, LPDOUBLE lpdfRowHeight)
	{ return SSGetMaxTextRowHeight(HWindow, Row, lpdfRowHeight); }

  GLOBALHANDLE GetMultiSelBlocks(LPSHORT lpdSelBlockCnt)
	{ return SSGetMultiSelBlocks(HWindow, lpdSelBlockCnt); }

  WORD GetOperationMode(void)
	{ return SSGetOperationMode(HWindow); }

  HWND GetOwner(void)
	{ return SSGetOwner(HWindow); }

  BOOL GetRowHeight(SS_COORD Row, LPDOUBLE lpHeight)
	{ return SSGetRowHeight(HWindow, Row, lpHeight);  }

  BOOL GetRowHeightInPixels(SS_COORD Row, LPSHORT lpdHeightPixels)
	{ return SSGetRowHeightInPixels(HWindow, Row, lpdHeightPixels); }

  BOOL GetRowPageBreak(SS_COORD Row)
       { return SSGetRowPageBreak(HWindow, Row); }

  BOOL GetRowUserData(SS_COORD Row, LPLONG lplUserData)
	{ return SSGetRowUserData(HWindow, Row, lplUserData); }

  BOOL GetSel(LPSHORT SelStart, LPSHORT SelEnd)
	{ return SSGetSel (HWindow, SelStart, SelEnd); }

  WORD GetSelBlockOptions(void)
	{ return SSGetSelBlockOptions(HWindow); }

  BOOL GetSelectBlock(LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
	{ return SSGetSelectBlock (HWindow, CellUL, CellLR); }

  GLOBALHANDLE GetSelText (void)
	{ return SSGetSelText (HWindow); }

  void GetShadowColor(LPCOLORREF lpShadowColor, LPCOLORREF lpShadowText,
		      LPCOLORREF lpShadowDark,  LPCOLORREF lpShadowLight)
	{ SSGetShadowColor (HWindow, lpShadowColor, lpShadowText,
			    lpShadowDark, lpShadowLight); }

  short GetUserResizeCol(SS_COORD Col)
	{ return SSGetUserResizeCol(HWindow, Col); }

  short GetUserResizeRow(SS_COORD Row)
	{ return SSGetUserResizeRow(HWindow, Row); }

  long GetTopLeftCell(LPSS_COORD ACol, LPSS_COORD ARow)
	{ return SSGetTopLeftCell (HWindow, ACol, ARow); }

  WORD GetUnitType(void)
    { return SSGetUnitType(HWindow); }

  LONG GetUserData(void)
	{ return SSGetUserData(HWindow); }

  WORD GetUserResize(void)
	{ return SSGetUserResize(HWindow); }

  short GetValue(SS_COORD Col, SS_COORD Row, LPSTR lpBuffer)
	{ return SSGetValue (HWindow, Col, Row, lpBuffer); }

  short GetValueLen(SS_COORD Col, SS_COORD Row)
	{ return SSGetValueLen (HWindow, Col, Row); }

  BOOL GetVisible(SS_COORD Col, SS_COORD Row, short Visible)
	{ return SSGetVisible (HWindow, Col, Row, Visible); }

  BOOL InsCol(SS_COORD Col)
	{ return SSInsCol (HWindow, Col); }

  BOOL InsColRange(SS_COORD Col, SS_COORD Col2)
	{ return SSInsColRange(HWindow, Col, Col2); }

  BOOL InsRow(SS_COORD Row)
	{ return SSInsRow (HWindow, Row); }

  BOOL InsRowRange(SS_COORD Row, SS_COORD Row2)
	{ return SSInsRowRange(HWindow, Row, Row2); }

  BOOL IsCellInSelection(SS_COORD Col, SS_COORD Row)
	{ return SSIsCellInSelection(HWindow, Col, Row); }

  BOOL LoadFromBuffer(GLOBALHANDLE hBuffer, long lBufferLen)
	{ return SSLoadFromBuffer (HWindow, hBuffer, lBufferLen); }

  BOOL LoadFromFile(LPSTR lpFileName)
	{ return SSLoadFromFile (HWindow, lpFileName); }

  void LogUnitsToColWidth(short dUnits, LPDOUBLE lpWidth)
	{ SSLogUnitsToColWidth(HWindow, dUnits, lpWidth); }

  void LogUnitsToRowHeight(SS_COORD Row, short dUnits, LPDOUBLE lpHeight)
	{ SSLogUnitsToRowHeight(HWindow, Row, dUnits, lpHeight); }

  BOOL MoveRange(SS_COORD Col,SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
		 SS_COORD ColDest, SS_COORD RowDest)
	{ return SSMoveRange(HWindow, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL Print(LPSTR lpszPrintJobName, LPSS_PRINTFORMAT lpPrintFormat,
	     FARPROC lpfnPrintProc, LONG lAppData, FARPROC lpfnAbortProc)
	{ return SSPrint(HWindow, lpszPrintJobName, lpPrintFormat,
		 lpfnPrintProc, lAppData, lpfnAbortProc); }

  BOOL ReCalc(void)
	 { return SSReCalc (HWindow); }

  BOOL ReplaceSel(LPSTR lpText)
	{ return SSReplaceSel (HWindow, lpText); }

  short RowHeightToLogUnits(SS_COORD Row, double dfRowHeight)
	{ return RowHeightToLogUnits (Row, dfRowHeight); }

  GLOBALHANDLE SaveToBuffer(long far *lpBufferLen, BOOL fDataOnly)
	{ return SSSaveToBuffer (HWindow, lpBufferLen, fDataOnly); }
		    
  BOOL SaveToFile(LPSTR lpFileName, BOOL fDataOnly)
	{ return SSSaveToFile (HWindow, lpFileName, fDataOnly); }

/*
  BOOL SelModeSendMessage(WORD wMsg, long lParam1, long lParam2, long lParam3)
       {  return SSSelModeSendMessage(HWindow,(WORD) wMsg,(unsigned) lParam1,(unsigned)lParam2,(unsigned) lParam3); }
*/

  BOOL SetActiveCell(SS_COORD Col, SS_COORD Row)
	{ return SSSetActiveCell (HWindow, Col, Row); }

  void SetAutoSizeVisible(short VisibleCols, short VisibleRows)
	 { SSSetAutoSizeVisible (HWindow, VisibleCols, VisibleRows); }

  BOOL SetBool(short nIndex, BOOL bNewVal)
	{ return SSSetBool (HWindow, nIndex, bNewVal); }

  BOOL SetBorder(SS_COORD Col, SS_COORD Row, WORD wBorderType,
		 WORD wBorderStyle, COLORREF Color)
	{ return SSSetBorder(HWindow, Col, Row, wBorderType, wBorderStyle, Color); }

  BOOL SetBorderRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
		      SS_COORD Row2, WORD wBorderType, WORD wBorderStyle,
		      COLORREF Color)
	{ return SSSetBorderRange(HWindow, Col, Row, Col2, Row2, wBorderType, wBorderStyle, Color); }

  WORD SetButtonDrawMode(WORD wMode)
	{ return SSSetButtonDrawMode(HWindow, wMode); }

  BOOL SetButtonText(SS_COORD Col, SS_COORD Row, LPSTR lpText)
	{ return SSSetButtonText(HWindow, Col, Row, lpText); }

  WNDPROC SetCallBack(WNDPROC lpfnCallBack)
	{ return SSSetCallBack(HWindow, lpfnCallBack); }

  BOOL SetCellType(SS_COORD Col, SS_COORD Row, LPSS_CELLTYPE CellType)
	{ return SSSetCellType(HWindow, Col, Row, CellType); }

  BOOL SetCellTypeRange(SS_COORD Col,  SS_COORD Row, SS_COORD Col2,
			SS_COORD Row2, LPSS_CELLTYPE CellType)
	{ return SSSetCellTypeRange(HWindow, Col, Row, Col2, Row2, CellType); }

  BOOL SetColor(SS_COORD Col, SS_COORD Row, COLORREF Background,
		COLORREF Foreground)
	{ return SSSetColor(HWindow, Col, Row, Background, Foreground); }

  BOOL SetColorRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
		     COLORREF Background, COLORREF Foreground)
	{ return SSSetColorRange(HWindow, Col, Row, Col2, Row2, Background, Foreground); }

  BOOL SetColPageBreak(SS_COORD Col, BOOL fPageBreak)
       { return SSSetColPageBreak(HWindow, Col, fPageBreak); }

  BOOL SetColUserData(SS_COORD Col, long lUserData)
	{ return SSSetColUserData(HWindow, Col, lUserData); }

  BOOL SetColWidth(SS_COORD Col, double Width)
	{ return SSSetColWidth(HWindow, Col, Width); }

  BOOL SetColHeaderDisplay(WORD wDisplay)
	{ return SSSetColHeaderDisplay(HWindow,wDisplay);}


  BOOL SetColWidthInPixels(SS_COORD Col, short dWidth)
	{ return SSSetColWidthInPixels(HWindow, Col, dWidth); }

  BOOL SetCursor(WORD wType, HCURSOR hCursor)
	{ return SSSetCursor(HWindow, wType, hCursor); }

  BOOL SetData(SS_COORD Col, SS_COORD Row, LPSTR lpData)
	{ return SSSetData (HWindow, Col, Row, lpData); }

  BOOL SetDataRange(SS_COORD Col, SS_COORD Row,
		    SS_COORD Col2,SS_COORD Row2, LPSTR lpData)
	{ return SSSetDataRange (HWindow, Col, Row, Col2, Row2, lpData); }

  BOOL SetDefFloatFormat(LPFLOATFORMAT lpFloatFormat)
	{ return SSSetDefFloatFormat(HWindow, lpFloatFormat); }

  WORD SetEditEnterAction(WORD wAction)
	 { return SSSetEditEnterAction(HWindow, wAction); }

  BOOL SetEditMode(BOOL fEditModeOn)
	{ return SSSetEditMode(HWindow, fEditModeOn); }

  BOOL SetFont(SS_COORD Col, SS_COORD Row, HFONT hFont, BOOL fDeleteFont)
	{ return SSSetFont(HWindow, Col, Row, hFont, fDeleteFont); }

  BOOL SetFontRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
		    SS_COORD Row2, HFONT hFont, BOOL fDeleteFont)
	{ return SSSetFontRange(HWindow, Col, Row, Col2, Row2, hFont, fDeleteFont); }

  BOOL SetFormula(SS_COORD Col, SS_COORD Row, LPSTR Formula,
		  BOOL BuildDependencies)
	{ return SSSetFormula (HWindow, Col, Row, Formula, BuildDependencies); }

  BOOL SetFormulaRange(SS_COORD Col, SS_COORD Row,
		       SS_COORD Col2,SS_COORD Row2,
		       LPSTR Formula, BOOL BuildDependencies)
	{ return SSSetFormulaRange(HWindow, Col, Row, Col2, Row2, Formula, 
				   BuildDependencies); }

  BOOL SetFloat(SS_COORD Col, SS_COORD Row, double dfValue)
	{ return SSSetFloat(HWindow, Col, Row, dfValue); }

  BOOL SetFloatRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, double dfValue)
	{ return SSSetFloatRange(HWindow, Col, Row, Col2, Row2, dfValue); }

  BOOL SetFreeze(short ColsFrozen, short RowsFrozen)
	 { return SSSetFreeze (HWindow, ColsFrozen, RowsFrozen); }

  void SetGrayAreaColor(COLORREF Background, COLORREF Foreground)
	{ SSSetGrayAreaColor(HWindow, Background, Foreground); }

  WORD SetGridType(WORD wGridType)
	{ return SSSetGridType(HWindow, wGridType); }

  COLORREF SetGridColor(COLORREF Color)
	{ return SSSetGridColor(HWindow, Color); }

  BOOL SetInteger(SS_COORD Col, SS_COORD Row, long lValue)
	{ return SSSetInteger(HWindow, Col, Row, lValue); }

  BOOL SetIntegerRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, long lValue)
	{ return SSSetIntegerRange(HWindow, Col, Row, Col2, Row2, lValue); }

  BOOL SetLock(SS_COORD Col, SS_COORD Row, BOOL Lock)
	{ return SSSetLock (HWindow, Col, Row, Lock); }

  void SetLockColor(COLORREF Background, COLORREF Foreground)
	 { SSSetLockColor(HWindow, Background, Foreground); }

  BOOL SetLockRange(SS_COORD Col,  SS_COORD Row,
		    SS_COORD Col2, SS_COORD Row2, BOOL Lock)
	{ return SSSetLockRange (HWindow, Col, Row, Col2, Row2, Lock); }

  void SetMaxCols(SS_COORD MaxCols)
	{ SSSetMaxCols(HWindow, MaxCols); }

  void SetMaxRows(SS_COORD MaxRows)
	{ if ( GetBool(SSB_VIRTUALMODE) )
	    VSetMax(MaxRows);
	  else
	    SSSetMaxRows(HWindow, MaxRows);
	}

  BOOL SetMultiSelBlocks(LPSS_SELBLOCK lpSelBlockList, short dSelBlockCnt)
	{ return SSSetMultiSelBlocks(HWindow, lpSelBlockList, dSelBlockCnt); }

  WORD SetOperationMode(WORD wMode)
	{ return SSSetOperationMode(HWindow, wMode); }
  
  HWND SetOwner(HWND hWndOwner)
	{ return SSSetOwner(HWindow, hWndOwner); }

  BOOL SetRowHeight(SS_COORD Row, double Height)
	{ return SSSetRowHeight (HWindow, Row, Height); }

  BOOL SetRowHeightInPixels(SS_COORD Row, short dHeight)
	{ return SSSetRowHeightInPixels(HWindow, Row, dHeight); }

  BOOL SetRowPageBreak(SS_COORD Row, BOOL fPageBreak)
       { return SSSetRowPageBreak(HWindow, Row, fPageBreak); }

  BOOL SetRowUserData(SS_COORD Row, long lUserData)
	{ return SSSetRowUserData(HWindow, Row, lUserData); }

  BOOL SetSel(short SelStart, short SelEnd)
	{ return SSSetSel(HWindow, SelStart, SelEnd); }

  BOOL SetSelectBlock(LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
	{ return SSSetSelectBlock (HWindow, CellUL, CellLR); }

  void SetShadowColor(COLORREF ShadowColor, COLORREF ShadowText,
		      COLORREF ShadowDark,  COLORREF ShadowLight)
	{ SSSetShadowColor(HWindow, ShadowColor, ShadowText,
			   ShadowDark, ShadowLight); }

  LPSS_CELLTYPE SetTypeButton(LPSS_CELLTYPE CellType, long Style,
			      LPSTR Text, LPSTR Picture, 
			      short nPictureType, LPSTR PictureDown,
			      short nPictureDownType, short ButtonType,
			      short nShadowSize, LPSUPERBTNCOLOR lpColor)
	{ return SSSetTypeButton(HWindow, CellType, Style, 
				 Text, Picture, nPictureType,
				 PictureDown, nPictureDownType,
				 ButtonType, nShadowSize, lpColor); }
 
  LPSS_CELLTYPE SetTypeCheckBox(LPSS_CELLTYPE lpCellType,
				long lStyle, LPSTR lpText,
				LPSTR lpPictUp, WORD wPictUpType,
				LPSTR lpPictDown, WORD wPictDownType,
				LPSTR lpPictFocusUp, WORD wPictFocusUpType,
				LPSTR lpPictFocusDown, WORD wPictFocusDownType,
				LPSTR lpPictDisabledUp, WORD wPictDisabledUpType,
				LPSTR lpPictDisabledDown, WORD wPictDisabledDownType)
	{ return SSSetTypeCheckBox(HWindow, lpCellType, lStyle, lpText, lpPictUp, wPictUpType,
				   lpPictDown, wPictDownType, lpPictFocusUp, wPictFocusUpType,
				   lpPictFocusDown, wPictFocusDownType, lpPictDisabledUp, wPictDisabledUpType,
				   lpPictDisabledDown, wPictDisabledDownType) ; }

  LPSS_CELLTYPE SetTypeComboBox(LPSS_CELLTYPE CellType, long Style,
				LPSTR lpItems)
	{ return SSSetTypeComboBox(HWindow, CellType, Style, lpItems); }

  LPSS_CELLTYPE SetTypeDate(LPSS_CELLTYPE CellType, long Style,
			    LPDATEFORMAT lpFormat, LPDATE lpMin, 
			    LPDATE lpMax)
	{ return SSSetTypeDate(HWindow, CellType, Style, lpFormat, lpMin, lpMax); }

  LPSS_CELLTYPE SetTypeEdit(LPSS_CELLTYPE CellType, long Style, short Len,
			    short ChrSet, char ChrCase)
	{ return SSSetTypeEdit(HWindow, CellType, Style, Len, ChrSet, ChrCase); }


  LPSS_CELLTYPE SetTypeFloat(LPSS_CELLTYPE CellType, long Style,
			     short Left, short Right, double Min,
			     double Max)
	{ return SSSetTypeFloat(HWindow, CellType, Style, Left, Right, Min, Max); }

  LPSS_CELLTYPE SetTypeFloatExt(LPSS_CELLTYPE CellType, long Style,
			     short Left, short Right, double Min,
			     double Max, LPFLOATFORMAT lpFormat)
	{ return SSSetTypeFloatExt(HWindow, CellType, Style, Left, Right, Min, Max, lpFormat); }

  LPSS_CELLTYPE SetTypeInteger(LPSS_CELLTYPE CellType, long lMin, long lMax)
	{ return SSSetTypeInteger(HWindow, CellType, lMin, lMax); }

  LPSS_CELLTYPE SetTypeIntegerExt(LPSS_CELLTYPE CellType, long lStyle, long lMin,
				  long lMax, BOOL fSpinWrap, long lSpinInc)
	{ return SSSetTypeIntegerExt(HWindow, CellType, lStyle, lMin, lMax, fSpinWrap, lSpinInc); }

  LPSS_CELLTYPE SetTypeOwnerDraw(LPSS_CELLTYPE lpCellType, long lStyle)
	{ return SSSetTypeOwnerDraw(HWindow, lpCellType, lStyle); }

  LPSS_CELLTYPE SetTypePic(LPSS_CELLTYPE CellType, long Style, LPSTR Mask)
	{ return SSSetTypePic(HWindow, CellType, Style, Mask); }

  LPSS_CELLTYPE SetTypePicture(LPSS_CELLTYPE CellType, long Style,
			       LPSTR PictName)
	{ return SSSetTypePicture(HWindow, CellType, Style, PictName); }

  LPSS_CELLTYPE SetTypePictureHandle(LPSS_CELLTYPE CellType, long Style, HANDLE hPict,
				     HPALETTE hPal, BOOL fDeleteHandle)
	{ return SSSetTypePictureHandle(HWindow, CellType, Style, hPict, hPal, fDeleteHandle); }

  LPSS_CELLTYPE SetTypeStaticText(LPSS_CELLTYPE CellType, short TextStyle)
	{ return SSSetTypeStaticText(HWindow, CellType, TextStyle); }

  LPSS_CELLTYPE SetTypeTime(LPSS_CELLTYPE CellType, long Style,
			    LPTIMEFORMAT lpFormat, LPTIME lpMin,
			    LPTIME lpMax)
	{ return SSSetTypeTime (HWindow, CellType, Style, lpFormat, lpMin, lpMax); }

  WORD SetUnitType(WORD wUnitType)
	{ return SSSetUnitType(HWindow, wUnitType); }

  LONG SetUserData(LONG lUserData)
	{ return SSSetUserData(HWindow, lUserData); }

  short SetUserResizeCol(SS_COORD Col, short dOption)
	{ return SSSetUserResizeCol(HWindow, Col, dOption); }

  short SetUserResizeRow(SS_COORD Row, short dOption)
	{ return SSSetUserResizeRow(HWindow, Row, dOption); }

  WORD SetUserResize(WORD wUserResize)
	{ return SSSetUserResize(HWindow, wUserResize); }

  WORD SetSelBlockOptions(WORD wOption)
	{ return SSSetSelBlockOptions(HWindow, wOption); }

  BOOL SetValue(SS_COORD Col, SS_COORD Row, LPSTR lpData)
	 { return SSSetValue (HWindow, Col, Row, lpData); }

  BOOL SetValueRange(SS_COORD Col,  SS_COORD Row,
		     SS_COORD Col2, SS_COORD Row2, LPSTR lpData)
	 { return SSSetValueRange (HWindow, Col, Row, Col2, Row2, lpData); }

  BOOL ShowActiveCell(short Position)
	{ return SSShowActiveCell (HWindow, Position); }

  BOOL ShowCell(SS_COORD Col, SS_COORD Row, short Position)
	{ return SSShowCell (HWindow, Col, Row, Position); }

  BOOL ShowCol(SS_COORD Col, BOOL fShow)
	{ return SSShowCol(HWindow, Col, fShow); }

  BOOL ShowRow(SS_COORD Row, BOOL fShow)
	{ return SSShowRow(HWindow, Row, fShow); }

  BOOL Sort(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPSS_SORT lpSort)
	{ return SSSort(HWindow, Col, Row, Col2, Row2, lpSort); }

  BOOL SwapRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
		 SS_COORD ColDest, SS_COORD RowDest)
	{ return SSSwapRange(HWindow, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL ValidateFormula (LPSTR lpszFormula)
	{ return SSValidateFormula (HWindow, lpszFormula); }

  void VGetBufferSize(LPSS_COORD lpdBufferSize, LPSS_COORD lpdOverlap)
	{ SSVGetBufferSize(HWindow, lpdBufferSize, lpdOverlap); }

  SS_COORD VGetMax()
	{ return SSVGetMax(HWindow); }

  SS_COORD VGetPhysBufferSize(void)
	{ return SSVGetPhysBufferSize(HWindow); }

  SS_COORD VGetPhysBufferTop(void)
	{ return SSVGetPhysBufferTop(HWindow); }

  LONG VGetStyle(void)
	{ return SSVGetStyle(HWindow); }

  BOOL VScrollGetSpecial(LPWORD lpwOptions)
	{ return SSVScrollGetSpecial(HWindow, lpwOptions); }

  BOOL VScrollSetSpecial(BOOL fUseSpecialVScroll, WORD wOptions)
	{ return SSVScrollSetSpecial(HWindow, fUseSpecialVScroll, wOptions); }

  void VSetBufferSize(SS_COORD dBufferSize, SS_COORD dOverlap)
	{ SSVSetBufferSize(HWindow, dBufferSize, dOverlap); }

  SS_COORD VSetMax(SS_COORD Max)
	{ return SSVSetMax(HWindow, Max); }

  LONG VSetStyle(LONG lStyle)
	{ return SSVSetStyle(HWindow, lStyle); }


protected:
  virtual LPSTR GetClassName()
	{ return "tbSpread"; }

}; // end class TSpread

#endif
