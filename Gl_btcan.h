#ifndef __GL_BTCAN_H
#define __GL_BTCAN_H
#include "gl_dlg.h"
#include "db_apbtc.h"
#include "db_cash.h"
#include "db_arbtc.h"
_CLASSDEF(TCancelBatch)
class TCancelBatch : public TMainDialog
{
 public:
 int iCurrentTab;
 Tglbtch glbtch;
 Tglbtchd glbtchd;
 Tapbtchd apbtchd;
 Tapbtch  apbtch;
 Tapcash  apcash;
 Tapcashd  apcashd;
 Tarbtchd arbtchd;
 Tarbtch  arbtch;
 TSpread*  List;
 string   strCancelled;

 TCancelBatch(PTWindowsObject AParent, int AName):TMainDialog(AParent,AName)
 {
	List = new TSpread(this,ID_DISTLIST);
 };

 ~TCancelBatch()
 {
   delete List;
 }
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void GLSelectSlip   (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void HasChanged     (RTMessage Msg) = [ID_FIRST + 1001];

 void RedrawGL();
 void CancelGL(long lBatchno,char* szBatchno);

 void RedrawAP();
 void CancelAP(long lBatchno,char* szBatchno);

 void RedrawCash();
 void CancelCash(long lBatchno,char* szBatchno);

 void RedrawAR();
 void CancelAR(long lBatchno,char* szBatchno);
};
#endif

