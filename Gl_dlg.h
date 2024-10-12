

#ifndef __GL_DLG_H
#define __GL_DLG_H
_CLASSDEF(TMainDialog)
class TMainDialog : public TDialog
{
 public:
  Code4 *cb;
  HWND hCalculator;
  TMainDialog (PTWindowsObject AParent, LPSTR ATitle):TDialog(AParent,ATitle){};
  TMainDialog (PTWindowsObject AParent, int ResourceId):TDialog(AParent,ResourceId){};
  virtual void SetupWindow();
  virtual void Add         (RTMessage Msg) = [ID_FIRST + ID_ADD];
  virtual void Top         (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom      (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Forward     (RTMessage Msg) = [ID_FIRST + ID_FORWARD];
  virtual void Back        (RTMessage Msg) = [ID_FIRST + ID_BACK];
  virtual void Delete      (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void Cancel      (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Modify      (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
  virtual void GetCalc     (RTMessage Msg) = [WM_FIRST + WM_RBUTTONDOWN];
  virtual void CMExit      (RTMessage Msg) = [CM_FIRST + CM_EXIT]{ Cancel(Msg); }
  void CreateDbf(char *DbfName, FIELD4INFO *fields, TAG4INFO *tag);
  char *GetString(int ID);
  void UpdateWindow(void);
  BOOL CheckPeriod(Date4 dDate,int iPeriod);
  void DatabaseError();
  void         ErrorClose(LPSTR szFile);
  BOOL ValidGLAccount(char *szAccount);
  BOOL SetField   (int ID, Field4 *field);
  BOOL SetField   (int ID, char *);
  BOOL SetField   (int ID, double);
  BOOL SetField   (int ID, int);
  BOOL SetField(int ID, Field4& field);
  BOOL SetField(int ID, string* field);
  void SetBitmap(int ID,char *szBitmap);
  void SetBitmap(UINT ID);
  void SetupList(void);
  char *GetField(int ID);
  char *GetField(HWND hWnd, int ID);
  double dGetField(HWND hWnd,int ID);
  char *GetDate(int ID);
  char *GetDate(char *date);
  double dGetField(int ID);
  long   lGetField(int ID);
  void ClearScreen(void);
  BOOL szEnterError(int ID);
  BOOL iEnterError(int ID);
  BOOL dEnterError(int ID);
  void CheckError(int ID);
  int DlgMsg(int ID,int ID_MSG,UINT ICON);
  BOOL SetPosition(char *szWinName);
  BOOL GetPosition(char *szWinName);
  void MsgBox(string strMessage,UINT ID);
};
#endif
