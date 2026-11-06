// StatBar message handler declarations

BOOL StatBar_Register(HINSTANCE hInstance);

LRESULT CALLBACK StatBar_WndProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

VOID StatBar_OnCommand(HWND hwnd, int id, HWND hwndCtl, WORD codeNotify);
VOID StatBar_OnPaint(HWND hwnd);
BOOL StatBar_OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct);
VOID StatBar_OnDestroy(HWND hwnd);
VOID StatBar_OnSize(HWND hwnd, WORD state, int cx, int cy);
VOID StatBar_OnGetMinMaxInfo(HWND hwnd, MINMAXINFO FAR* lpMinMaxInfo);
VOID StatBar_OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, WORD flags);
DWORD StatBar_OnMenuChar(HWND hwnd, WORD ch, WORD flags, HMENU hmenu);
VOID StatBar_OnEnterIdle(HWND hwnd, WORD source, HWND hwndSource);
VOID Statbar_OnDestroy(HWND hwnd);
VOID Statbar_OnKeyUp(HWND hwnd, WORD vk, BOOL fDown, int cRepeat, WORD flags);
VOID Statbar_OnKeyDown(HWND hwnd, WORD vk, BOOL fDown, int cRepeat, WORD flags);
VOID Statbar_OnSetFocus(HWND hwnd, HWND hwndOldFocus);

// Initilize Status Bar
void InitStatBar(void);
