/* GLDBF.DLL Header File */

typedef BOOL (FAR PASCAL _export * FPSTRCB) (LPSTR,LPVOID);

#define VATACCT   1
#define SALESACCT 2
#define RETAINED  3
#define AR        4
#define AP        5
#define COMPANY   6
#define INDIV     7

/*
extern "C" {
BOOL FAR  PASCAL __export ValidateGL(LPSTR lpAccount,LPSTR lpDepart);
void FAR PASCAL __export SetPeriods(HWND hWnd);
char FAR * PASCAL __export GetAccount(int ID);
int FAR PASCAL _export WEP(int);
int FAR PASCAL __export GetPeriod(HWND hWnd);
char FAR * PASCAL __export GetCompany();
char FAR * PASCAL __export szGetPeriod();
int FAR PASCAL __export iGetPeriod();
void FAR PASCAL __export PrintReport(char *szReportName,HWND HWindow);
BOOL FAR PASCAL __export CheckFile(LPSTR szFileName,HWND hWnd);
BOOL FAR PASCAL __export InitFiles();
}
*/

BOOL  ValidateGL(LPSTR lpAccount,LPSTR lpDepart);
void  SetPeriods(HWND hWnd);
char* GetAccount(int ID);
long  GetPeriod(HWND hWnd);
char* GetCompany();
char* szGetPeriod();
int   iGetPeriod();
void  PrintReport(char *szReportName,HWND HWindow);
BOOL  CheckFile(LPSTR szFileName,HWND hWnd);
BOOL  InitFiles();

