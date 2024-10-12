/* d4all.h   (c)Copyright Sequiter Software Inc., 1991-1994. All rights reserved. */

/**********************************************************************/
/**********            USER SWITCH SETTINGS AREA            ***********/

/* Index File compatibility options */
//#define S4FOX
/* #define S4CLIPPER */
#define S4MDX
/* #define S4NDX     */

/* Specify Operating Environment or Screen Output Option - Default is DOS */
/* #define S4DLL          */
/* #define S4WINDOWS      */
/* #define S4WIN32           */
/* #define S4OS2          */
/* #define S4OS2DLL       */ /* OS/2 PRESENTATION MANAGER DLL LIBRARY */
/* #define S4OS2PM        */ /* OS/2 PRESENTATION MANAGER STATIC LIBRARY */
/* #define S4UNIX         */ /* requires Multi-Platform Version */
/* #define S4MACINTOSH    */ /* requires Multi-Platform Version */
/* #define S4CODE_SCREENS */
/* #define S4CONTROLS     */ /* for use with CodeControls DLL */
/* #define S4VBX          */ /* for use with CodeControls VBX */
/* #define S4PASCAL_DOS   */ /* Borland Pascal for DOS DLL */
/* #define S4PASCAL_WIN   */ /* Borland Pascal for Windows DLL */

/* General Configuration Options */
/* #define S4DEBUG        */
/* #define S4DEBUG_DEV    */
/* #define S4ERROR_HOOK   */
/* #define S4LOCK_CHECK   */
/* #define S4LOCK_HOOK    */
/* #define S4MAX          */
/* #define S4MEMO_OFF     */
/* #define S4OFF_ERROR    */
/* #define S4OFF_INDEX    */
/* #define S4OFF_MEMO     */
/* #define S4OFF_MULTI    */
/* #define S4OFF_OPTIMIZE */
/* #define S4OFF_REPORT   */
/* #define S4OFF_WRITE    */
/* #define S4OLD_CODE     */
/* #define S4OPTIMIZE_OFF */
/* #define S4SAFE         */
/* #define S4SINGLE       */

/**********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef S4MACINTOSH
   #define S4UNIX
#endif

#ifdef S4UNIX
   #include "p4port.h"
#else
   #include <limits.h>
   #include <io.h>
   #include <stdarg.h>
#endif

#ifdef S4VB_DOS
   #include "v4names.h"
#endif

#include "d4defs.h"
#include "d4data.h"
#include "d4declar.h"
#include "f4flag.h"
#include "e4expr.h"
#include "s4sort.h"
#include "e4error.h"

#include "o4opt.h"

#ifdef S4WINDOWS
   #include <windows.h>
#else
   #ifdef S4WIN32
      #include <windows.h>
   #endif
#endif

#include "r4relate.h"

#ifdef S4VBX
   #ifndef S4CONTROLS
      #define S4CONTROLS
   #endif
#endif

#ifdef S4VB_DOS
   #include "v4dos.h"
#endif

#ifndef S4OFF_REPORT
   #include "r4report.h"
#endif

#ifdef S4CODE_SCREENS
   #include "w4.h"
#endif

#define S4VERSION  5104
