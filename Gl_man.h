#ifndef __GL_MAN_H
#define __GL_MAN_H
#include "gl_manil.h"
#define ID_APBATCH 4044
#define ID_ADDVENDOR 4045
#define ID_DELVENDOR 4046
#define ID_MODVENDOR 4047
#define ID_PREPOSTING 8136
#define ID_AR_CUSTSUMMARY 8137

class TTooltip : public TDialog
{
  public:
  TTooltip(PTWindowsObject AParent,UINT ID):TDialog(AParent,ID){}
  virtual void  WMPaint(RTMessage Msg) = [WM_FIRST + WM_PAINT];
};


_CLASSDEF(TManilaGL)
class TManilaGL : public TGlWindow
{
 public:
 Code4 *cb;
 TManilaGL(LPSTR ATitle,LPSTR MenuName);
 virtual void SetupWindow();
 WORD wSelection;
 UINT timer;
 void  DisplayToolTip(UINT ID);
 TTooltip *Tooltip;
 virtual void WMInitMenuPopup (RTMessage Msg) = [WM_FIRST + WM_INITMENUPOPUP];
 virtual void GLSysAccess     (RTMessage Msg) = [CM_FIRST + ID_SYSACCESS];
 virtual void GLAccessMain    (RTMessage Msg) = [CM_FIRST + ID_ACCESS];
 virtual void GLUserAccess    (RTMessage Msg) = [CM_FIRST + ID_USERACCESS];
 virtual void GLFixedAdd      (RTMessage Msg) = [CM_FIRST + ID_FAADD];
 virtual void GLFixedTypes    (RTMessage Msg) = [CM_FIRST + ID_FATYPE];
 virtual void GLFixedSlip     (RTMessage Msg) = [CM_FIRST + ID_FASLIP];
 virtual void GLFixedDisposal (RTMessage Msg) = [CM_FIRST + ID_DISPOSAL];
 virtual void GLMedicalReps   (RTMessage Msg) = [CM_FIRST + ID_MEDREPS];
 virtual void GLProductCode   (RTMessage Msg) = [CM_FIRST + ID_PRODCODE];
 virtual void GLMedRepsList   (RTMessage Msg) = [CM_FIRST + ID_REPLIST];
 virtual void GLProductList   (RTMessage Msg) = [CM_FIRST + ID_PRODUCTLIST];
 virtual void GLDoctor        (RTMessage Msg) = [CM_FIRST + ID_DOCTOR];
 virtual void GLDoctorRpt     (RTMessage Msg) = [CM_FIRST + ID_DOCTOR_RPT];
 virtual void GLExternal      (RTMessage Msg) = [CM_FIRST + ID_EXTBATCH];
 virtual void GLPrePosting    (RTMessage Msg) = [CM_FIRST + ID_PREPOSTING];
 virtual void GLCreateCustomReport (RTMessage Msg) = [CM_FIRST + ID_CREATE_CUSTOM];
 virtual void WMTimer         (RTMessage Msg) = [WM_FIRST + WM_TIMER];

 // Accounts Payable Programs
 virtual void APBatch         (RTMessage Msg) = [CM_FIRST + ID_APBATCH];
 virtual void APCash          (RTMessage Msg) = [CM_FIRST + ID_APCASH];

 virtual void APPrintBatch  (RTMessage Msg)   = [CM_FIRST + ID_APPRINTBATCH];
 virtual void APPrintCash   (RTMessage Msg)   = [CM_FIRST + ID_APPRINTCASH];
 virtual void APPrintAdj    (RTMessage Msg)   = [CM_FIRST + ID_APPRINTADJ];
 virtual void APPostBatch   (RTMessage Msg)   = [CM_FIRST + ID_APPOSTBATCH];
 virtual void APPostCash    (RTMessage Msg)   = [CM_FIRST + ID_APPOSTCASH];
 virtual void APPostAdj     (RTMessage Msg)   = [CM_FIRST + ID_APPOSTADJ];
 virtual void APCancelBatch (RTMessage Msg)   = [CM_FIRST + ID_BATCHCANCEL];
 virtual void APCancelCash  (RTMessage Msg)   = [CM_FIRST + ID_CASHCANCEL];
 virtual void APPostJournal (RTMessage Msg)   = [CM_FIRST + ID_APPOSTJOURNAL];
 virtual void APAdjBatchPrt (RTMessage Msg)   = [CM_FIRST + ID_ADJBATCHPRINT];
 virtual void APPrePayment  (RTMessage Msg)   = [CM_FIRST + ID_PREPAYMENT];
 virtual void APCreateGLSlip(RTMessage Msg)   = [CM_FIRST + ID_GLSLIP];
 virtual void APPayment     (RTMessage Msg)   = [CM_FIRST + ID_PAYMENT];
 virtual void APVenLists    (RTMessage Msg)   = [CM_FIRST + ID_VENLISTS];
 virtual void APOpenAccts   (RTMessage Msg)   = [CM_FIRST + ID_OPENACCTS];
 virtual void APSummary     (RTMessage Msg)   = [CM_FIRST + CM_SUMMARY];
 virtual void APOverDue     (RTMessage Msg)   = [CM_FIRST + ID_OVERDUE];
 virtual void APAgedCash    (RTMessage Msg)   = [CM_FIRST + ID_AGEDCASH];
 virtual void APAgedRetain  (RTMessage Msg)   = [CM_FIRST + ID_AGEDRETAIN];
 virtual void APBatchStatus (RTMessage Msg)   = [CM_FIRST + ID_BATCHSTATUS];
 virtual void APVatAPReport   (RTMessage Msg) = [CM_FIRST + CM_VATAP_REPORT];
 virtual void APVatGLReport   (RTMessage Msg) = [CM_FIRST + CM_VATGL_REPORT];
 virtual void APChequeCash   (RTMessage Msg) = [CM_FIRST + CM_CHEQUE_CASH];
 virtual void APChequeAP     (RTMessage Msg) = [CM_FIRST + CM_CHEQUE_AP];
 virtual void APJournalAP    (RTMessage Msg) = [CM_FIRST + CM_AP_JSUM];
 virtual void APJournalCash  (RTMessage Msg) = [CM_FIRST + CM_CASH_JSUM];
 virtual void APBatchAP      (RTMessage Msg) = [CM_FIRST + CM_AP_BTCHD];
 virtual void APBatchCash    (RTMessage Msg) = [CM_FIRST + CM_AP_CASHD];
 virtual void APGLSummary    (RTMessage Msg) = [CM_FIRST + CM_GLSUMMARY];
 virtual void APGLDetail      (RTMessage Msg) = [CM_FIRST + CM_GLDETAIL];
 virtual void APRebateSummary (RTMessage Msg) = [CM_FIRST + CM_REBATE_REPORT];
 virtual void APPurgeAccts  (RTMessage Msg)   = [CM_FIRST + ID_PURGEACCTS];
 virtual void APRequestSummary (RTMessage Msg) = [CM_FIRST + ID_REQUESTSUM];
 virtual void APRequestDetail (RTMessage Msg)  = [CM_FIRST + ID_REQUESTDETAIL];
 virtual void APAddVendors    (RTMessage Msg) = [CM_FIRST + ID_ADDVENDOR];
 virtual void APDelVendors    (RTMessage Msg) = [CM_FIRST + ID_DELVENDOR];
 virtual void APModVendors    (RTMessage Msg) = [CM_FIRST + ID_MODVENDOR];
 virtual void APChequeSummary (RTMessage Msg) = [CM_FIRST + ID_CHEQUESUMMARY];
 virtual void APPriorPeriod  (RTMessage Msg) = [CM_FIRST + CM_APPRIOR];

 // Accounts Receivable Programs
 virtual void ARAddVendors  (RTMessage Msg)   = [CM_FIRST + ID_AR_CUSTOMER];
 virtual void ARBatch       (RTMessage Msg)   = [CM_FIRST + ID_ARBATCH];
 virtual void ARReceipt     (RTMessage Msg)   = [CM_FIRST + ID_ARRECEIPT];
 virtual void ARPrintBatch  (RTMessage Msg)   = [CM_FIRST + ID_ARPRINTBATCH];
 virtual void ARPostBatch   (RTMessage Msg)   = [CM_FIRST + ID_ARPOST];
 virtual void ARCancelBatch (RTMessage Msg)   = [CM_FIRST + ID_ARCANCEL];
 virtual void ARCreateSlip  (RTMessage Msg)   = [CM_FIRST + ID_ARPAYMENT];
 virtual void ARVenLists    (RTMessage Msg)   = [CM_FIRST + ID_ARCUSTOMERLIST];
 virtual void AROpenAccts   (RTMessage Msg)   = [CM_FIRST + ID_OPENAR];
 virtual void ARSummary     (RTMessage Msg)   = [CM_FIRST + ID_AR_SUMMARY];
 virtual void AROverDue     (RTMessage Msg)   = [CM_FIRST + ID_OVERDUE_AR];
 virtual void ARAgedCash    (RTMessage Msg)   = [CM_FIRST + ID_ARAGED];
 virtual void ARAgedRetain  (RTMessage Msg)   = [CM_FIRST + ID_ARRETAINED];
 virtual void ARBatchStatus (RTMessage Msg)   = [CM_FIRST + ID_ARSTATUS];
 virtual void ARVatARReport (RTMessage Msg)   = [CM_FIRST + ID_VATAR_REPORT];
 virtual void ARVatGLReport (RTMessage Msg)   = [CM_FIRST + ID_VATGL_REPORT];
 virtual void ARBatchAR     (RTMessage Msg)   = [CM_FIRST + ID_AR_BTCHD];
 virtual void ARCustSummary (RTMessage Msg)   = [CM_FIRST + ID_AR_CUSTSUMMARY];
 virtual void ARStatements   (RTMessage Msg)   = [CM_FIRST + ID_AR_STATEMENT];
 virtual void ARStatementDet (RTMessage Msg)   = [CM_FIRST + ID_AR_STATEMENTDETAIL];
 virtual void ARPriorPeriod  (RTMessage Msg)   = [CM_FIRST + CM_ARPRIOR];

 virtual void ARGLSummary   (RTMessage Msg)   = [CM_FIRST + ID_GLSUMMARY];
 virtual void ARGLDetail    (RTMessage Msg)   = [CM_FIRST + ID_GLDETAIL];
 virtual void ARPurgeAccts  (RTMessage Msg)   = [CM_FIRST + ID_ARPURGE];
 virtual void CMExit        (RTMessage Msg)   = [CM_FIRST + CM_EXIT];
 void SetPosition();
 void ShowButton(int ID, BOOL bShow);
};

#endif
