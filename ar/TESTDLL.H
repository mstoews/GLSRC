/*
 *    TESTDLL.H   
 *
 *    
 *
 */
     
#define DLLENTRY FAR PASCAL     
     
/*************** FUNCTION PROTOTYPES ************************/
// All functions in DLL's that are to be accessed from outside the DLL MUST
// be declarled as FAR PASCAL (DLLENTRY) and exported in the 
// applications definition file

void  DLLENTRY       ClearAllEdits     (HWND);

/**************** LIBRARY GLOBALS **************************/

extern   HANDLE   hDynamicInst;
