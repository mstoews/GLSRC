#include "std_inc.h"
#include "db_rpt.h"

class TReportFormat : public TMainDialog
{
  public:
  TGl_rpt gl_rpt;
  TGl_sub gl_sub;
  TGl_cat gl_cat;

  TSpread* SubList;
  TSpread* CatList;

  TReportFormat(TWindowsObject* parent,int ID = 5001):TMainDialog(parent,ID)
  {
	SubList = new TSpread(this,ID_DISTLIST);
	CatList = new TSpread(this,IDC_SUB);
  }
  virtual void SetupWindow();
  virtual void Cancel          (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Ok              (RTMessage Msg) = [ID_FIRST + IDOK];

};

void TReportFormat::SetupWindow()
{
  TMainDialog::SetupWindow();
  gl_rpt.open(((TAcctModule *)GetApplication())->cb);
  gl_sub.open(((TAcctModule *)GetApplication())->cb);
  gl_cat.open(((TAcctModule *)GetApplication())->cb);

  for(gl_rpt.top();!gl_rpt.eof();gl_rpt.skip())
  {
	 SendDlgItemMsg(IDC_REPORTFILE,CB_ADDSTRING,NULL,(LPARAM)(LPSTR)gl_rpt.desc.str());
  }

  long Indx =  SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);

  HFONT hFont1 = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
  HFONT hFont2 = CreateFontIndirect((LPLOGFONT)((TAcctModule *)GetApplication())->GetFontStyle());
  CatList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont1, TRUE);
  SubList->SetFont(SS_ALLCOLS, SS_ALLROWS,hFont2, TRUE);
  DeleteObject(hFont1);
  DeleteObject(hFont2);


  SubList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
  SubList->SetBool(SSB_PROTECT,TRUE);
  SubList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
  SubList->SetBool(SSB_NOBEEP,TRUE);
  SubList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
  SubList->SetBool(SSB_VERTSCROLLBAR,TRUE);

  CatList->SetBool(SSB_ALLOWUSERSELBLOCK,FALSE);
  CatList->SetBool(SSB_PROTECT,TRUE);
  CatList->SetBool(SSB_SCROLLBAREXTMODE,TRUE);
  CatList->SetBool(SSB_NOBEEP,TRUE);
  CatList->SetBool(SSB_MOVEACTIVEONFOCUS, FALSE);
  CatList->SetBool(SSB_VERTSCROLLBAR,TRUE);

  SubList->SetData(1,SS_HEADER, "Account");
  SubList->SetData(2,SS_HEADER, "Dept");
  SubList->SetData(3,SS_HEADER, "Depart");
  SubList->SetData(4,SS_HEADER, "Seqno");
  SubList->SetData(5,SS_HEADER, "Field");

  SubList->SetMaxCols(5);
  SubList->SetColWidth(1,12);
  SubList->SetColWidth(2,12);
  SubList->SetColWidth(3,12);
  SubList->SetColWidth(4,12);
  short icolumn1,icolumn2,icolumn3,icolumn4,icolumn5;
  SubList->GetColWidthInPixels(1,(LPSHORT)&icolumn1);
  SubList->GetColWidthInPixels(2,(LPSHORT)&icolumn2);
  SubList->GetColWidthInPixels(3,(LPSHORT)&icolumn3);
  SubList->GetColWidthInPixels(4,(LPSHORT)&icolumn4);

  RECT rc;
  SubList->GetClientRect(&rc);
  short width = rc.right - rc.left;
  icolumn5 = width -  ( icolumn1+icolumn2+icolumn3+icolumn4 );
  SubList->SetColWidthInPixels(5,icolumn5);

  CatList->SetData(1,SS_HEADER, "Report");
  CatList->SetData(2,SS_HEADER, "Sub");
  CatList->SetData(3,SS_HEADER, "Cat");
  CatList->SetData(4,SS_HEADER, "Description");

  CatList->SetMaxCols(4);
  CatList->SetColWidth(1,12);
  CatList->SetColWidth(2,12);
  CatList->SetColWidth(3,12);

  short icol1,icol2,icol3,icol4;
  CatList->GetColWidthInPixels(1,(LPSHORT)&icol1);
  CatList->GetColWidthInPixels(2,(LPSHORT)&icol2);
  CatList->GetColWidthInPixels(3,(LPSHORT)&icol3);
  CatList->GetClientRect(&rc);
  width = rc.right - rc.left;
  icol4 = width -  ( icol1+icol2+icol3);
  CatList->SetColWidthInPixels(4,icol4);


  /*
  if(Msg.LP.Hi == CBN_SELCHANGE)
  {
	long Indx =  SendDlgItemMsg(ID_DESCRIPT,CB_GETCURSEL,0,0);
  if(Indx != CB_ERR)
	{
	 SendDlgItemMsg(ID_DESCRIPT,CB_GETLBTEXT, (WPARAM)Indx, (LPARAM) ((LPSTR) szBankAccount));
	 if(gl_setat.seek(szBankAccount) == 0)
	  {
		SetField(ID_ACCOUNTS,gl_setat.account.str());
		SetField(ID_DEPART,gl_setat.dept.str());
	  }
	  SetFocus(GetDlgItem(HWindow,2017));
	}
  }
 */
  

}

void TReportFormat::Cancel(RTMessage)
{
  gl_rpt.close();
  gl_sub.close();
  gl_cat.close();
  ShutDownWindow();
}

void TReportFormat::Ok (RTMessage)
{
  MessageBox(HWindow,"OK","",MB_OK);
}

void TManilaGL::GLCreateCustomReport (RTMessage)
{
	GetApplication()->ExecDialog(new TReportFormat(this));
}

