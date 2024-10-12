#ifndef _TTAB_H
#define _TTAB_H


#define SHOW_AP    (WM_USER + 1)
#define SHOW_AR    (WM_USER + 2)
#define SHOW_CASH  (WM_USER + 3)
#define SHOW_GL    (WM_USER + 4)
#define AP_TAB     1500
#define AR_TAB     1501
#define CASH_TAB   1502
#define GL_TAB	    1503


_CLASSDEF (TTabControl)
class TTabControl : public TMainDialog
{
 public:
		HBITMAP tab_r;
		HBITMAP tab_c;
		HBITMAP tab_l;
		HBITMAP untab_r;
		HBITMAP untab_c;
		HBITMAP untab_l;
		int nPtSize;
		PSTR pszFace;
		string strAR,strAP,strCash,strGL;

 TTabControl(PTWindowsObject AParent,LPSTR ATitle):TMainDialog(AParent,ATitle){};
 virtual void SetupWindow       ();
 virtual void WMDrawItem        (RTMessage Msg) = [WM_FIRST + WM_DRAWITEM];
 inline virtual void AccountPayable    (RTMessage Msg) = [ID_FIRST + AP_TAB]
	{
	  SendMessage(Parent->HWindow,WM_COMMAND,SHOW_AP,NULL);
	};
 inline  virtual void AccountsReceivable(RTMessage Msg) = [ID_FIRST + AR_TAB]
 {
  SendMessage(Parent->HWindow,WM_COMMAND,SHOW_AR,NULL);
 };
 inline  virtual void Cash           (RTMessage Msg) = [ID_FIRST + CASH_TAB]
 {
	SendMessage(Parent->HWindow,WM_COMMAND,SHOW_CASH,NULL);
 };
 inline  virtual void GeneralLedger (RTMessage Msg) = [ID_FIRST + GL_TAB]
 {
	SendMessage (Parent->HWindow,WM_COMMAND,SHOW_GL,NULL);
 };

 void  Selected(LPDRAWITEMSTRUCT lpdis,string strTab);
 void  UnSelected(LPDRAWITEMSTRUCT lpdis,string strTab);
 ~TTabControl()
 {
 		DeleteObject(tab_c);
		DeleteObject(tab_l);
		DeleteObject(tab_r);
		DeleteObject(untab_c);
		DeleteObject(untab_l);
		DeleteObject(untab_r);

 }
};

class TPostEntries : public TMainDialog
{
 public:
 TPostBatch 	*GLForm;
 TTabControl 	*Tab;

 char szTransaction;

 TPostEntries(TWindowsObject *parent,int ATitle) : TMainDialog(parent,ATitle){}
 virtual void SetupWindow();
 virtual BOOL CanClose();
 virtual void RedrawGL  (RTMessage Msg) = [CM_FIRST + SHOW_GL];
 virtual void RedrawAP  (RTMessage Msg) = [CM_FIRST + SHOW_AP];
 virtual void RedrawAR  (RTMessage Msg) = [CM_FIRST + SHOW_AR];
 virtual void RedrawCash(RTMessage Msg) = [CM_FIRST + SHOW_CASH];
};


#endif