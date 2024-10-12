/* d4defs.h   (c)Copyright Sequiter Software Inc., 1994. All rights reserved. */

#ifndef S4MDX
#ifndef S4FOX
#ifndef S4CLIPPER
#ifndef S4NDX
   error - Must compile with one of the indexing options (S4MDX, S4FOX, S4CLIPPER, OR S4NDX)
#endif
#endif
#endif
#endif

#ifdef S4VBASIC
#ifdef S4WINDOWS
   #define WINVER 0x0300
#endif
#endif

#ifdef S4MDX
#ifdef S4FOX
   error - Both S4MDX and S4FOX switches set - only one is allowed.
#endif
#ifdef S4CLIPPER
   error - Both S4MDX and S4CLIPPER switches set - only one is allowed.
#endif
#ifdef S4NDX
   error - Both S4MDX and S4NDX switches set - only one is allowed.
#endif
#endif

#ifdef S4FOX
#ifdef S4CLIPPER
   error - Both S4FOX and S4CLIPPER switches set - only one is allowed.
#endif
#ifdef S4NDX
   error - Both S4FOX and S4NDX switches set - only one is allowed.
#endif
#endif

#ifdef S4CLIPPER
#ifdef S4NDX
   error - Both S4CLIPPER and S4NDX switches set - only one is allowed.
#endif
#endif
/*   OS2 2.0 SUPPORT */
#ifdef __OS2__       /* Watcom 386, Borland C++ for OS/2 */
   #define S4OS2
#endif
#ifdef __IBMC__  /* IBM C SET/2 */
   #define S4OS2
   #define S4IBMOS2
#endif
#ifdef __IBMCPP__  /* IBM C++ SET/2 */
   #define S4OS2
   #define S4IBMOS2
#endif

#ifdef __OS2__
   #ifndef S4OS2
      #define S4OS2
   #endif
   typedef unsigned HANDLE;
   #if __BORLANDC__ == 0x400
      #define S4MEMCMP
   #endif
#endif

#ifdef S4OS2
#ifndef __WATCOMC__
   #define INCL_DOSSEMAPHORES
   #ifndef S4OS2PM
      #define INCL_NOPMAPI    /* don't include PM.H */
   #endif
   #include <os2.h>
#endif
#endif

#ifndef S4OS2
#ifndef S4UNIX
#ifndef _SIZE_T_DEFINED_     /* Watcom */
#ifndef _SIZE_T_DEFINED      /* Microsoft, Metaware */
#ifndef _SIZE_T              /* Borland  */
   typedef unsigned size_t ;
   #define _SIZE_T_DEFINED_           /* Used to resolve Watcom 386 warning */
   #define _SIZE_T
#endif
#endif
#endif
#endif
#endif

#ifndef _A_RDONLY
   #define _A_RDONLY 0x01                  /* Read-only attribute  */
#endif

#ifdef __HIGHC__           /* align structures to word alignment, Metaware */
  #pragma Align_members(2)
  #pragma Off(Char_default_unsigned)
#endif

#ifdef __SC__
   #pragma SC align 2      /* align structures to word alignment, Symantec */
#endif

#ifdef __WATCOMC__
   #pragma off(unreferenced)
#endif
                           /* Foreign language support */
#ifdef S4GERMAN
   #define S4LANGUAGE
   #ifdef S4FOX
      #define u4memcmp memcmp
   #endif
#endif

#ifdef S4FRENCH
   #define S4LANGUAGE
   #define S4VMAP
#endif

#ifdef S4SWEDISH
   #define S4LANGUAGE
   #define S4VMAP
#endif

#ifdef S4FINNISH
   #define S4LANGUAGE
   #define S4VMAP
#endif

#ifdef S4ANSI
   #define S4VMAP
#endif

#ifdef S4MEMCMP
   #ifdef __cplusplus
      extern "C" {
   #endif
      int c4memcmp(void *, void *, size_t) ;
   #ifdef __cplusplus
      }
   #endif
   #define u4memcmp c4memcmp
#else
#define c4memcmp memcmp
#ifndef S4LANGUAGE
   #define u4memcmp memcmp
#endif
#endif

#ifdef S4WIN32
   #define S4NO_FILELENGTH
   #ifdef __cplusplus
      #define S4CLASS
   #endif
#else
   #ifdef __TURBOC__
      #ifdef S4DLL
         #define S4CLASS huge
         #define S4FUNCTION far pascal
      #endif
   #endif

   #ifdef _MSC_VER
      #if _MSC_VER == 600
         #define S4NO_NEGATIVE_LOCK
      #endif
      #ifdef S4DLL
         #ifdef __cplusplus
            #define S4CLASS
         #else
            #define S4CLASS huge
         #endif
         #define S4FUNCTION far pascal
      #endif
   #endif

   #ifdef __ZTC__
      #ifdef S4DLL
         #ifdef __cplusplus
            #define S4CLASS
         #else
            #define S4CLASS huge
         #endif
         #define S4FUNCTION far pascal
      #endif
   #endif
#endif

#ifdef S4PASCAL_DOS
   #define S4PASCAL
#endif

#ifdef S4PASCAL_WIN
   #define S4PASCAL
#endif

#ifdef S4DLL_BUILD
   #ifndef S4PASCAL_DOS
      #define S4WINDOWS
   #endif
   #define __DLL__ 1
#endif

#ifdef __DLL__
   #ifdef S4WIN32
      #define S4DLL
      #define S4CLASS _declspec(dllexport)
      #define S4FUNCTION
   #else
      #ifdef S4OS2
         #define S4CLASS _export
         #define S4FUNCTION _export
      #else
         #define S4DLL
         #define S4CLASS _export
         #define S4FUNCTION far pascal _export
      #endif
   #endif
#endif

#ifndef S4CLASS
   #define S4CLASS
#endif
#ifndef S4FUNCTION
   #define S4FUNCTION
#endif

#ifdef S4DLL
   #ifndef S4WINDOWS
      #ifndef S4PASCAL_DOS
         #define S4WINDOWS
      #endif
   #endif
   #ifdef S4WIN32
      #define S4CALL _cdecl
   #else
      #ifdef S4OS2
         #define S4CALL
      #else
         #define S4CALL far _cdecl _export
      #endif
   #endif
#else
   #ifdef _MSC_VER
      #define S4CALL _cdecl S4FUNCTION
   #else
      #ifdef __ZTC__
         #define S4CALL _cdecl S4FUNCTION
      #else
         #define S4CALL S4FUNCTION
      #endif
   #endif
#endif

#ifdef S4WIN32
   #define S4NO_FILELENGTH
   #define INVALID_HANDLE_VALUE (HANDLE)-1    /* for Windows NT CreateFile */
#endif

#ifdef S4WINDOWS
   typedef unsigned int UINT ;
#endif

#ifdef S4WINDOWS
   #ifdef __TURBOC__
      #if __TURBOC__ == 0x297         /* Borland C++ 2.0 */
         #define M4PRINT sprintf
      #else
         #define M4PRINT wsprintf
      #endif
   #else
      #define M4PRINT wsprintf
   #endif
#else
   #define M4PRINT sprintf            /* DOS */
#endif

#ifndef S4WIN32
   #ifdef S4DLL
      #ifndef S4OS2
         #define S4PTR far
      #endif
   #endif
#endif

#ifndef S4PTR
   #define S4PTR
#endif

#ifdef S4NDX
   #define N4OTHER
   /* default is to use dBASE III+/Clipper memo file formats if using .NDX index file formats */
   #define S4MNDX
#endif

#ifdef S4CLIPPER
   #define N4OTHER
   /* default is to use dBASE III+/Clipper memo file formats if using .NTX index file formats */
   #define S4MNDX
   #define S4HAS_DESCENDING
#endif

#ifdef S4FOX
   /* default is to use foxpro memo file formats if using foxpro index file formats */
   #define S4MFOX
   #define S4HAS_DESCENDING
#endif

#ifdef S4UNIX
   #ifdef N4OTHER
      force error:  Clipper or dBase III support not allowed.
   #endif
#endif

#ifdef S4MDX
   #define S4MMDX
#endif

#ifdef S4MNDX
   #define   MEMO4SIZE   0x200
#endif

#ifndef S4UNIX
   typedef  const void S4PTR *  S4CMP_PARM ;
#endif

#ifdef S4OFF_OPTIMIZE
   #define S4OPTIMIZE_OFF
#endif

#ifdef S4OFF_INDEX
   #define S4INDEX_OFF
#endif

#ifdef S4OFF_MEMO
   #ifndef S4MEMO_OFF
      #define S4MEMO_OFF
   #endif
#endif

#ifdef S4MEMO_OFF
   #ifndef S4OFF_MEMO
      #define S4OFF_MEMO
   #endif
#endif

#ifdef S4OFF_MULTI
    #define S4SINGLE
#endif

#ifdef S4TEST
   #ifndef S4DEBUG_DEV
      #ifndef S4UNIX
      #ifndef S4MACINTOSH
      #ifndef S4WINDOWS
      #ifdef S4SINGLE
         #define S4DEBUG_DEV
      #endif
      #endif
      #endif
      #endif
   #endif
   #ifndef S4TESTING
      #define S4TESTING
   #endif
   #ifndef S4ERROR_HOOK
      #define S4ERROR_HOOK
   #endif
   #ifndef S4NO_OUT
      #define S4NO_OUT
   #endif
#endif

#ifdef S4DEBUG_DEV
   #ifndef S4INDEX_VERIFY
      #define S4INDEX_VERIFY
   #endif
#endif

#ifdef S4DLL
   #define sort4assignCmp(s4,f)  (s4)->cmp = (S4CMP_FUNCTION S4PTR *) MakeProcInstance((FARPROC) f, (s4)->code_base->hInst)
#else
   #define sort4assignCmp(s4,f)  (s4)->cmp = (S4CMP_FUNCTION S4PTR *) (f)
#endif

/* Integer Return Codes */
#define r4success     0
#define r4found       1       /* Primary Key Match */
#define r4after       2
#define r4eof         3
#define r4bof         4
#define r4entry       5       /* No index file entry or no record (go) */
#define r4descending 10
#define r4unique     20       /* Key is not unique, do not write/append */
#define r4uniqueContinue  25 /* Key is not unique, write/append anyway */
#define r4ignore     40
#define r4keep       45
#define r4locked     50
#define r4noCreate  60       /* Could not create file */
#define r4noOpen    70       /* Could not open file */
#define r4no_tag     80       /* DataIndex::seek, with no default tag */
#define r4terminate  90       /* no relation match with terminate set */

/* General Disk Access Errors */
#define e4close   -10
#define e4create  -20
#define e4len     -30
#define e4lenSet -40
#define e4lock    -50
#define e4open    -60
#define e4read    -70
#define e4remove  -80
#define e4rename  -90
#define e4seek   -100
#define e4unlock -110
#define e4write  -120

/* Database Specific Errors */
#define e4data  -200
#define e4fieldName -210     /* Invalid field name */
#define e4fieldType -220
#define e4recordLen -230

/* Index File Specific Errors */
#define e4entry      -300     /* Tag entry not located */
#define e4index      -310
#define e4tagName   -330
#define e4unique     -340     /* Key is not unique */

/* Expression Errors */
#define e4commaExpected -400
#define e4complete       -410
#define e4dataName      -420
#define e4lengthErr     -422
#define e4notConstant   -425
#define e4numParms      -430
#define e4overflow       -440 /* Overflow while evaluating expression */
#define e4rightMissing  -450
#define e4typeSub       -460
#define e4unrecFunction -470
#define e4unrecOperator -480
#define e4unrecValue    -490
#define e4unterminated   -500

/* Optimization Errors */
#define e4opt         -610
#define e4optSuspend -620
#define e4optFlush   -630

/* Relation Errors */
#define e4lookupErr  -710
#define e4relate      -720

/* Report Errors */
#define e4report           -810
#define e4style_create     -811
#define e4style_select     -812
#define e4style_index      -813
#define e4area_create      -814
#define e4group_create     -815
#define e4group_expr       -816
#define e4total_create     -817
#define e4obj_create       -818
#define e4rep_win          -819
#define e4rep_out          -820
#define e4rep_save         -821
#define e4rep_ret          -822
#define e4rep_data         -823

/* Critical Errors */
#define e4info     -910  /* Unexpected information in internal variable */
#define e4memory   -920  /* Out of memory */
#define e4parm     -930  /* Unexpected parameter */
#define e4demo     -940  /* Exceeded maximum record number for demo */
#define e4result   -950  /* Unexpected result */

/* Library Errors */
/* call to library function calls not supported */
#define e4notIndex   -1010  /* S4OFF_INDEX */
#define e4notMemo    -1020  /* S4OFF_MEMO */
#define e4notRename  -1030  /* S4NO_RENAME */
#define e4notWrite   -1040  /* S4OFF_WRITE */
#define e4not_clipper -1050  /* S4CLIPPER */
#define e4not_lock    -1060  /* S4LOCK_HOOK */
#define e4not_hook    -1070  /* S4ERROR_HOOK */

#define E4DEMO_MAX 200

/* garbage between expression and filter is length: */
#ifdef  S4FOX
   #define I4MULTIPLY       1
   #define B4DO_BACK_LEVEL  3
   #define I4MAX_KEY_SIZE 240
   #define F4MAX_NUMERIC   20
   #define F4MAX_DECIMAL   19
   #define F4DECIMAL_OFFSET 1
#else
   #ifdef N4OTHER
      #define F4MAX_NUMERIC   19
      #define F4MAX_DECIMAL   15
      #define F4DECIMAL_OFFSET 2
   #endif
   #ifdef S4MDX
      #define F4MAX_NUMERIC   20
      #define F4MAX_DECIMAL   18
      #define F4DECIMAL_OFFSET 2
   #endif

   #define I4MULTIPLY     512

   #ifdef S4NDX
      #define I4MAX_KEY_SIZE 100
   #else
      #ifdef S4CLIPPER
         #define I4MAX_KEY_SIZE 338
      #else
         #define I4MAX_KEY_SIZE 102
      #endif
   #endif
#endif

#define D4GARBAGE_LEN  518
#define E4ACCURACY     1.0e-13
#define E4ACCURACY_DIGITS  15


/* if S4NO_NEGATIVE_LOCK is defined, there is no dBASE IV compatibility */

#ifdef N4OTHER
   #define L4LOCK_POS     1000000000L
#endif
#ifdef S4FOX
   #ifdef S4NO_NEGATIVE_LOCK
      #define L4LOCK_POS_OLD 1L
      #define L4LOCK_POS     2000000000L
   #else
      #define L4LOCK_POS_OLD 0x40000000L
      #define L4LOCK_POS     0x7FFFFFFEL
   #endif
#endif
#ifdef S4MDX
   #ifdef S4NO_NEGATIVE_LOCK
      #define L4LOCK_POS_OLD 2000000000L
      #define L4LOCK_POS     2000000000L
   #else
      #define L4LOCK_POS_OLD 0x40000000L
      #define L4LOCK_POS     0xEFFFFFFFL
   #endif
#endif

#ifdef S4MDX
   #define I4MAX_EXPR_SIZE 220
#endif
#ifdef S4NDX
   #define I4MAX_EXPR_SIZE 220
#endif
#ifdef S4FOX
   #define I4MAX_EXPR_SIZE 255
#endif
#ifdef S4CLIPPER
   #define I4MAX_EXPR_SIZE 255
#endif

#ifdef N4OTHER
   #ifdef S4NDX
      #define B4BLOCK_SIZE 512
   #else
      #ifdef S4CLIPPER
         #define B4BLOCK_SIZE 1024
      #endif
   #endif
#endif

#ifdef S4FOX
/*#define VERSION_POS 498L*/
/* next is the # of bytes of important info for T4HEADER */
#define T4HEADER_WR_LEN 0x10
/* block_size is 512 for foxpro */
#define B4BLOCK_SIZE 512
#endif
