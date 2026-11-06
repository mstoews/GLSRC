/*
** File:    UFMain.h
** Author:  Rick Cameron, Rex Benning
** Date:    7 Mar 93
**
** Purpose: Declarations of standard functions
**          for a user-defined function DLL.
*/

#if !defined (UFMAIN_H)

#define UFMAIN_H

extern HINSTANCE UFInstance;

extern UFParamListElement FAR *GetParam (UFParamBlock FAR *param,
                                         UFTInt16s paramNumber);

extern UFMemCalcParam FAR *GetMemCalcParam (UFMemCalcBlock FAR *param,
                                            UFTInt16s paramNumber);
extern UFMemCalcParam FAR *GetMemCalcArrayElement (UFMemCalcParam FAR *param,
                                                   UFTInt16s paramNumber);
extern UFParamListElement FAR *GetMemCalcRunParam (UFMemCalcBlock FAR *param,
                                                   UFTInt16s paramNumber);

#endif // !defined (UFMAIN_H)
