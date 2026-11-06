/*
** File:    UFFuncs.h
** Author:  Rex Benning
** Date:
**
** Purpose: Declaration of the entry points a user-defined function DLL
**          must provide.
*/


#if !defined (UFFUNCS_H)
#define UFFUNCS_H

ExternC UFError FAR _export PASCAL UFInitialize (void);

ExternC UFError FAR _export PASCAL UFTerminate (void);

ExternC UFFunctionDefStringList FAR * FAR _export PASCAL UFGetFunctionPrototypes (void);


ExternC UFFunctionTemplateList FAR * FAR _export PASCAL UFGetFunctionTemplates (void);


ExternC UFFunctionExampleList FAR * FAR _export PASCAL UFGetFunctionExamples (void);

ExternC UFError FAR _export PASCAL UFCallFunction (UFTInt16s i,
                                                   UFParamBlock FAR *paramPtr);

ExternC char FAR * FAR _export PASCAL UFErrorRecovery (UFParamBlock FAR *paramPtr,
                                                   UFError errN);

ExternC short FAR _export PASCAL UFGetVersion (void);

ExternC UFFunctionDefStringList FAR * FAR _export PASCAL UFGetFunctionDefStrings (void);

ExternC void FAR _export PASCAL UFStartJob (UFTInt32u jobId);

ExternC void FAR _export PASCAL UFEndJob (UFTInt32u jobId);

#endif // !defined (UFFUNCS_H)
