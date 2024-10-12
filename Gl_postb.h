#ifndef _GL_POSTB_H
#define _GL_POSTB_H
//#include "gl_dlg.h"
//#include "db_accts.h"
//#include "db_profi.h"
//#include "db_btchd.h"
//#include "db_btch.h"
//#include "db_arbtc.h"
//#include "db_apbtc.h"
//#include "db_dist.h"
//#include "db_apdst.h"
//#include "db_ardst.h"
#include "db_arjsm.h"
//#include "db_cash.h"
//#include "db_jsum.h"
//#include "widgets.h"

_CLASSDEF(TPostBatch)
class TPostBatch : public TMainDialog
{
 public:
 int iCurrentTab;
 Code4 *cb;
 char szBatchno[8];
 double dAPControl;
 Tglbtch  glbtch;
 Tglbtchd glbtchd;
 Tglaccts glaccts;
 Tgldist  gldist;

 Tarbtch  arbtch;
 Tarbtchd arbtchd;
 Tardist  ardist;
 Tarjsum  arjsum;


 Tapbtch  apbtch;
 Tapbtchd apbtchd;
 Tapdist  apdist;
 Tapjsum  apjsum;

 Tapcash  apcash;
 Tapcashd apcashd;
 TSpread   *List;
 TPostBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
	strcpy(szBatchno,"batchno");
 };
 ~TPostBatch()
 {
	delete List;
 }
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Close          (RTMessage Msg) = [CM_FIRST + IDCANCEL]{Cancel(Msg);}
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void HasChanged     (RTMessage Msg) = [ID_FIRST + 1001];
 virtual void Query          (RTMessage Msg) = [ID_FIRST + ID_VIEW];
 virtual void GLRedraw       (RTMessage Msg) = [CM_FIRST + ID_REDRAW]{GLRedraw();}
 virtual void ErrorClose(LPSTR szFile);
 // General Ledger Entries
 //
 void GL_PostAll(); // Formally the okay button;
 BOOL UpdateGL(Field4 *account, Field4 *dept);
 void PostGLSlip    (long lBatchno);
 void PostAdjustment(long lBatchno);
 BOOL ValidateGLAccounts();
 BOOL ValidateGLAccount(long lBatchno);
 BOOL CheckGLBalance(long lBatchno);
 void GLRedraw();

 // Accounts Payable Entries
 void AP_PostAll ();
 void AP_AppendAPDist(char *szInvoice);
 void AP_AppendGLDist(PTapbtch apbtch);
 void AP_AppendAPSummary(long lBatchno);
 void AP_CreateGLSlip(void);
 void AP_AccountUpdate(void);
 void AP_AppendPriorPeriod(long lBatchno);
 BOOL AP_UpdateGL(Field4 *account, Field4 *dept,int iPeriod);
 double AP_dCurrentYearTotal();
 BOOL AP_CheckAPBalance(long lBatchno);
 void AP_UpdateRetainedEarnings(double dPrevious12);
 void APRedraw();

 // Accounts Receivable Entries
 void AR_PostAll (); // Formally the Ok button;
 void AR_AppendDist(char *szInvoice);
 void AR_AppendGLDist(PTarbtch arbtch);
 void AR_AppendSummary(long lBatchno);
 void AR_CreateGLSlip(void);
 void ARAccountUpdate(void);

 BOOL AR_CheckBalance(long lBatchno,double dPayable,char *szARAccount);
 void AR_AppendPriorPeriod(long lBatchno);
 BOOL AR_UpdateGL(Field4 *account, Field4 *dept,int iPeriod);
 void AR_UpdateRetainedEarnings(double dPrevious12);
 double AR_dCurrentYearTotal();
 void ARRedraw();

 // Cash Entries
 void CASH_PostAll(); // Formally Ok button for cash entries.
 void CASH_CreateGLSlip(void);
 BOOL CASH_CheckBalance(long lBatchno);
 void CASH_AppendSummary(long lBatchno);
 void CashRedraw();

 char *GetQuery(BOOL bPost = FALSE);
 char *GetValidateQuery();

 double dCurrentYearTotal();
};
#endif