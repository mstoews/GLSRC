/* GLDBF.DLL Header File */

typedef BOOL (FAR PASCAL _export * FPSTRCB) (LPSTR,LPVOID);

extern "C" {
double FAR * PASCAL __export CB(LPSTR lpAccount,LPSTR lpDepart);
double FAR * PASCAL __export QUERY(LPSTR lpQuery);
double FAR * PASCAL __export CBR(LPSTR lpAccount,LPSTR lpEnd);
double FAR  * PASCAL __export OPEN(LPSTR lpAccount,LPSTR lpDepart);
double FAR * PASCAL __export PD(LPSTR lpAccount,LPSTR lpDepart,int iPeriod);
double FAR * PASCAL __export BG(LPSTR lpAccount,LPSTR lpDepart,int iPeriod);
double FAR * PASCAL __export PR(LPSTR lpAccount,LPSTR lpDepart,int iPeriod);
char FAR  * PASCAL __export DESC(LPSTR lpAccount,LPSTR lpDepart);

int FAR PASCAL _export WEP(int);

}