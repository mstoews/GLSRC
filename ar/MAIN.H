// Globals

extern HINSTANCE g_hinst;
extern HMODULE	 g_hmod;
extern HINSTANCE g_hinstPrev;
extern LPCSTR g_lpCmdLine;
extern int    g_nCmdShow;
extern HWND   g_hwndFrame;

// Application Functions

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

BOOL App_Initialize(VOID);
int  App_Main(VOID);
