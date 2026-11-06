
extern Code4 cb;

_CLASSDEF(TMainDialog)
class TMainDialog : public TDialog
{
 public:
  TMainDialog (PTWindowsObject AParent, LPSTR ATitle):TDialog(AParent,ATitle){};
  virtual void SetupWindow();
  virtual void Add         (RTMessage Msg) = [ID_FIRST + ID_ADD];
  virtual void Top         (RTMessage Msg) = [ID_FIRST + ID_TOP];
  virtual void Bottom      (RTMessage Msg) = [ID_FIRST + ID_BOTTOM];
  virtual void Forward     (RTMessage Msg) = [ID_FIRST + ID_FORWARD];
  virtual void Back        (RTMessage Msg) = [ID_FIRST + ID_BACK];
  virtual void Delete      (RTMessage Msg) = [ID_FIRST + ID_DELETE];
  virtual void Cancel      (RTMessage Msg) = [ID_FIRST + IDCANCEL];
  virtual void Modify      (RTMessage Msg) = [ID_FIRST + ID_MODIFY];
  virtual void CreateDbf(char *DbfName, FIELD4INFO *fields, TAG4INFO *tag);
  virtual void UpdateWindow(void);
  virtual BOOL SetField   (int ID, Field4 *field);
  virtual BOOL SetField   (int ID, char *);
  virtual char *GetField(int ID);
  virtual double dGetField(int ID);
  virtual void ClearScreen(void);
  virtual void EnterError(int ID);
  virtual void CheckError(int ID);
  virtual int DlgMsg(int ID,int ID_MSG,UINT ICON);
};
