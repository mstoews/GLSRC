/* GLDBF.DLL Header File */

typedef BOOL (FAR PASCAL _export * FPSTRCB) (LPSTR,LPVOID);

extern "C" {
BOOL FAR * PASCAL __export ValidateGL(LPSTR lpAccount,LPSTR lpDepart);
int FAR PASCAL _export WEP(int);
}