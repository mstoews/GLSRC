#ifndef __AR_POSTB_B
#define __AR_POSTB_B
_CLASSDEF(TARPostBatch)
class TARPostBatch : public TMainDialog
{
 public:
 char szBatchno[8];
 Tarbtch  arbtch;
 Tarbtchd arbtchd;
 Tglaccts glaccts;
 Tardist  ardist;
 Tarjsum  arjsum;
 //Tarctrl  arctrl;
 double dAPControl;
 PTListArray ListArray;
 TARPostBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
  ListArray = new TListArray(1500, 0, 50);
  strcpy(szBatchno,"batchno");
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 void AppendARDist(char *szInvoice);
 void AppendGLDist(PTarbtch arbtch);
 void AppendARSummary(long lBatchno);
 void CreateGLSlip(void);
 void ARAccountUpdate(void);
 BOOL CheckARBalance(long lBatchno);
 void AppendPriorPeriod(long lBatchno);
 BOOL UpdateGL(Field4 *account, Field4 *dept,int iPeriod);
 void UpdateRetainedEarnings(double dPrevious12);
 double dCurrentYearTotal();
 ~TARPostBatch(){delete ListArray;};
};

#endif
