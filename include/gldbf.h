/* GLDBF.DLL Header File */

typedef BOOL (FAR PASCAL _export * FPSTRCB) (LPSTR,LPVOID);

#define VATACCT   1
#define SALESACCT 2
#define RETAINED  3
#define AR        4
#define AP        5

extern "C" {
BOOL FAR  PASCAL __export ValidateGL(LPSTR lpAccount,LPSTR lpDepart);
void FAR PASCAL __export SetPeriods(HWND hWnd);
char FAR * PASCAL __export GetAccount(int ID);
int FAR PASCAL _export WEP(int);
int FAR PASCAL __export GetPeriod(HWND hWnd);
int FAR  PASCAL __export iGetPeriod();
}