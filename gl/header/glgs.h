// Dialog support functions

LRESULT DefDialogProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

// AboutDlg functions

VOID	AboutDlg_Do(HWND hwndOwner);
VOID	NotImpDlg_Do(HWND hwndOwner);

BOOL CALLBACK SimpleDlg_OldDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

LRESULT SimpleDlg_DlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);
VOID	SimpleDlg_OnCommand(HWND hwnd, WORD id, HWND hwndCtl, WORD codeNotify);
