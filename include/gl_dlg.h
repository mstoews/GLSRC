

#ifndef __GL_DLG_H
#define __GL_DLG_H
_CLASSDEF(TMainDialog)
class TMainDialog : public TDialog
{
 public:
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
  virtual void CreateDbf(char *DbfName, FIELD4INFO *fields, TAG4INFO *tag);
  virtual char *GetString(int ID);
  virtual void UpdateWindow(void);
  virtual void DatabaseError();
  virtual BOOL ValidGLAccount(char *szAccount);
  virtual BOOL SetField   (int ID, Field4 *field);
  virtual BOOL SetField   (int ID, char *);
  virtual BOOL SetField   (int ID, double);
  virtual BOOL SetField   (int ID, int);
  virtual void SetBitmap(int ID,char *szBitmap);
  virtual void SetBitmap(UINT ID);
  virtual void SetupList(void);
  virtual char *GetField(int ID);
  virtual char *GetDate(int ID);
  virtual double dGetField(int ID);
  virtual long   lGetField(int ID);
  virtual void ClearScreen(void);
  virtual BOOL szEnterError(int ID);
  virtual BOOL iEnterError(int ID);
  virtual BOOL dEnterError(int ID);
  virtual void CheckError(int ID);
  virtual int DlgMsg(int ID,int ID_MSG,UINT ICON);
  virtual BOOL SetPosition(char *szWinName);
  virtual BOOL GetPosition(char *szWinName);
};
#endif
