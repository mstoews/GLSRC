/* p4port.h (c)Copyright Sequiter Software Inc., 1990-1992.  All rights reserved.

   Declarations for non-standard C runtime library functions.
*/
#ifdef S4UNIX

   #ifdef S4MACINTOSH
      #define S4SINGLE
      #define S4OPTIMIZE_OFF
      #define S4BYTEORDER_3210
      #define S4NO_ATOF
      #define S4NO_FCVT
      #define S4NO_ECVT
   #else
      #define S4LOCKF          /* use function lockf() for locking  */
      #define S4NO_MEMMOVE     /* use if memmove() not found        */
   #endif

/* Universal CodeBase defines */
   #define S4PORTABLE

/* CodeBase Multi-Platform defines */
   #define S4ERRNO          /* use global variable, 'errno'      */
   #define S4NO_CHSIZE      /* use if chsize() not found         */
   #define S4NO_FILELENGTH  /* use if filelength() not found     */
   #define S4NO_POW         /* use if pow() not found            */
   #define S4NO_REMOVE      /* use if remove() not found         */
   #define S4NO_RENAME      /* use if rename() not found         */
   #define S4NO_STRLWR      /* use if strlwr() not found         */
   #define S4NO_STRNICMP    /* use if strnicmp() not found       */
   #define S4NO_STRUPR      /* use if strupr() not found         */

/* #define S4BYTEORDER_2301  */ /* use for 2301 systems */
/* #define S4BYTEORDER_3210  */ /* use for 3210 systems */
/* #define S4BYTE_SWAP       */ /* use for swapping bytes on 3210 or 2301 systems */
/* #define S4MEMCMP          */ /* use if memcmp() uses signed byte comparison */
/* #define S4NO_ATOF         */ /* use if atof() not found */
/* #define S4NO_ECVT         */ /* use if ecvt() not found */
/* #define S4NO_FCVT         */ /* use if fcvt() not found */
/* #define S4NO_SIZE_T       */ /* use if variable size_t not found */
#endif

#ifdef S4UNIX
   #define  S4CMP_PARM  void *  
   #ifndef S4MACINTOSH
      extern  long   filelength() ;
      extern  int    atoi() ;      /* These functions are usually declared  */ 
      extern  long   atol() ;      /* in other system header files, and     */ 
      extern  int    chsize() ;    /* therefore most of these declarations  */
      extern  int    close() ;     /* are redundant.  Sometimes, however,   */
      extern  char  *ecvt() ;      /* the declaration is in a header file   */
      extern  char  *fcvt() ;      /* not included in the compile.  In that */
      extern  void   exit() ;      /* case, compile and/or run-time errors  */
      extern  int    lock() ;      /* will be avoided because of these      */
      extern  int    locking() ;   /* declarations.                         */
      extern  long   lseek() ;
      extern  void  *malloc() ;  /* malloc(), calloc() and realloc() may */
      extern  void  *calloc() ;  /* return a 'char *' in some compilers. */
      extern  void  *realloc() ; /* if so, change 'void *' to 'char *'   */
      extern  void   free() ;
      extern  int    read() ; 
      extern  long   time() ;
      extern  int    unlink() ; 
      extern  int    write() ; 
   #else 
      long filelength(int) ;
      long MAClseek(int, long, int) ;
          
      #include <errors.h>
      #include <files.h>
   #endif

   #define USHRT_MAX      0xFFFF
   #define INT_MAX        0x7FFF
   #define UINT_MAX       0xFFFF
#endif

#ifdef S4LOCKF
   #define S4LOCKING
#endif

#ifdef S4NO_SIZE_T
   typedef unsigned size_t ;
#endif

#ifdef S4NO_STRNICMP
   int strnicmp() ;
#endif

#ifdef S4NO_MEMCMP
   int memcmp() ;
#endif

#ifdef S4NO_MEMMOVE
   void *memmove() ;
#endif

#ifdef S4NO_FCVT 
   char *fcvt() ;
#endif

#ifdef S4NO_ECVT 
   char *ecvt() ;
#endif

#ifdef S4NO_ATOF
   double c4atof() ;
#endif

#ifdef S4BYTEORDER_3210
   #define S4DO_BYTEORDER
#endif

#ifdef S4BYTEORDER_2301
   #define S4DO_BYTEORDER
#endif
