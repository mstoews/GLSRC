#include "tbowl.h"
#include "d4all.hpp"

#include "ar.h"
#include "utildlg.h"


void TGlWindow::APVenLists      (RTMessage){PrintReport("ar_vend.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APOpenAccts     (RTMessage){PrintReport("ar_detl.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APPrintBatch    (RTMessage){PrintReport("arbtchsm.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APPrintCash     (RTMessage){PrintReport("ar_cashb.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APPrintAdj      (RTMessage){PrintReport("ar_adjst.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APSummary       (RTMessage){PrintReport("ar_bala.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APOverDue       (RTMessage){PrintReport("ar_agedc.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APAgedCash      (RTMessage){PrintReport("ar_agedc.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APAgedRetain    (RTMessage){PrintReport("ar_agedr.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APBatchStatus   (RTMessage){PrintReport("ar_bstat.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APPostJournal   (RTMessage){PrintReport("ar_postj.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APRebateSummary (RTMessage){PrintReport("ar_reb.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APVatAPReport   (RTMessage){PrintReport("ar_vatap.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APVatGLReport   (RTMessage){PrintReport("ar_vatgl.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APChequeAP	(RTMessage){PrintReport("ar_cheqa.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APChequeCash	(RTMessage){PrintReport("ar_cheqc.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APJournalCash	(RTMessage){PrintReport("ar_jsuma.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APJournalAP	(RTMessage){PrintReport("ar_jsumc.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APBatchCash	(RTMessage){PrintReport("ar_cashd.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APBatchAP	(RTMessage){PrintReport("ar_btchd.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APGLSummary	(RTMessage){PrintReport("ar_jsum.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APGLDetail	(RTMessage){PrintReport("ar_jsumd.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APAdjBatchPrt 	(RTMessage){PrintReport("ar_adjbt.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APPrePayment 	(RTMessage){PrintReport("ar_prepy.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APRequestSummary(RTMessage){PrintReport("ar_reqst.rpt",GetApplication()->MainWindow->HWindow);}
void TGlWindow::APRequestDetail (RTMessage){PrintReport("ar_reqsd.rpt",GetApplication()->MainWindow->HWindow);}


