
/*****************************************************************************
* FarPoint Technologies  (C) Copyright 1991, 1992, 1993  All Rights Reserved
* portions (C) Copyright 1991, 1992 Applegate Sotware
*****************************************************************************/

#ifndef TOOLBOX_H
#define TOOLBOX_H

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /* RC_INVOKED */

#if defined( __cplusplus )
extern "C" {
#endif

/*
#define SS_USE16BITCOORDS
*/

/***************************************
* Include for Drover Toolbox libraries
***************************************/

#ifndef RC_INVOKED

/*
#define  DRV_NOCLASS
#define  DRV_NODIALOG
#define  DRV_NOGAUGE
#define  DRV_NOPATTERN
#define  DRV_NOSORT
#define  DRV_NOSYS

#define  TB_NOSPREAD
*/

#undef  GHND
#define GHND       (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE)

#ifndef WINAPI
#define WINAPI _far _pascal
#endif

#ifndef CALLBACK
#define CALLBACK _far _pascal
#endif

typedef HPALETTE FAR *LPHPALETTE;
typedef HBITMAP FAR *LPHBITMAP;
typedef HFONT FAR *LPHFONT;

#if (WINVER < 0x030a)
typedef unsigned int UINT;
typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;
typedef HANDLE HINSTANCE;
typedef const char FAR* LPCSTR;
typedef FARPROC DLGPROC;
typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
#endif

#define DLL_TBPRO2 0x01
#define DLL_TBPRO3 0x02
#define DLL_TBPRO4 0x04
#define DLL_TBPRO5 0x08
#define DLL_TBPRO6 0x10

#define DRIVELEN   3                 /* max length of drive in file name */
#define DIRLEN     130               /* max length of directory */
#define NAMELEN    10                /* max length of name in file name */
#define EXTLEN     5                 /* max length of extn in file name */
#define FILELEN    (NAMELEN+EXTLEN)  /* max length of file name */
#define PATHLEN    (DRIVELEN+DIRLEN+FILELEN) /* max full file name path */

#define ERRLEN     100               /* ERROR length */
#define INTLEN     7                 /* max integer string length */
#define LONGLEN    11                /* max long string length */
#define DATELEN    12                /* Length of date string: dd-mmm-[yy]yy */
#define LIBVERSION 30                /* LIBRARY VERSION */

#define TB_INT_MAX        (32767)        /* max integer supported - rdw */
#define TB_INT_MIN       (-32767)        /* min integer supported - rdw */
#define TB_INT_OVERFLOW  (TB_INT_MIN-1)  /* StringToInt overflow  - rdw */
#define TB_LONG_MAX       (2147483647)   /* max long supported    - rdw */
#define TB_LONG_MIN      (-2147483647)   /* min long supported    - rdw */
#define TB_LONG_OVERFLOW (TB_LONG_MIN-1) /* StringToLong overflow - rdw */

#define DLLENTRY      FAR PASCAL
#define WINENTRY LONG FAR PASCAL
#define DLGENTRY BOOL FAR PASCAL

#if WINVER != 0x030a
typedef GLOBALHANDLE     HGLOBAL;
typedef LOCALHANDLE      HLOCAL;
#endif

typedef VOID huge       *HPVOID;
typedef char huge       *HPSTR;
typedef float FAR       *LPFLOAT;
typedef short FAR       *LPSHORT;
typedef BOOL FAR        *LPBOOL;
typedef double FAR      *LPDOUBLE;
typedef long double FAR *LPLDOUBLE;
typedef COLORREF FAR    *LPCOLORREF;
typedef unsigned short   USHORT;
typedef unsigned long    ULONG;

#define MAKEWORD(a, b)  ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))

#define SEGMENT(lp)     (*((unsigned *)&(lp) + 1))
#define OFFSET(lp)      (*((unsigned *)&(lp)))

typedef struct tagWINDEVICE
   {
   char DeviceName[50];
   char DriverName[9];
   char Port[40];
   } WINDEVICE, FAR *LPWINDEVICE;

/*************************************************************
* Passed to date functions to set date parameters Structures
* for holding the actual values for the Date and Time values
*************************************************************/

typedef struct tagDATE
   {
   int  nDay;      /* Day of the Month  1..31 */
   int  nMonth;    /* Month of the Year 1..12 */
   int  nYear;     /* Year */
   } DATE, FAR *LPDATE;

typedef struct tagTIME
   {
   int  nHour;     /* Hour 0..23 */
   int  nMinute;   /* Minutes 0..59 */
   int  nSecond;   /* Seconds 0..59 */
   } TIME, FAR *LPTIME;

/****************************************
* Structures for describing the way the
* dates and times are to be displayed
****************************************/

typedef struct tagDATEFORMAT 
   {
   BOOL   bCentury;
   char   cSeparator;
   int    nFormat;
   BOOL   bSpin;
   } DATEFORMAT, FAR *LPDATEFORMAT;

typedef struct tagTIMEFORMAT 
   {
   BOOL   b24Hour;
   BOOL   bSeconds;
   char   cSeparator;
   BOOL   bSpin;
   } TIMEFORMAT, FAR *LPTIMEFORMAT;

typedef struct tagFLOATFORMAT
   {
   char   cCurrencySign;
   char   cDecimalSign;
   char   cSeparator;
   } FLOATFORMAT, FAR *LPFLOATFORMAT;

/**************************
* DATE FORMAT IDENTIFIERS
**************************/

#define  IDF_DDMONYY    100
#define  IDF_DDMMYY     101
#define  IDF_MMDDYY     102
#define  IDF_YYMMDD     103

/*********************************************************
* User callable functions to support ToolBox edit fields
*********************************************************/
 
LPDATE  DLLENTRY DateAddDays       (LPDATE, LONG);
LONG    DLLENTRY DateDMYToJulian   (LPDATE);
LPSTR   DLLENTRY DateDMYToString   (LPDATE, LPSTR, LPDATEFORMAT);
BOOL    DLLENTRY DateGetFormat     (HWND, LPDATEFORMAT);
LPSTR   DLLENTRY DateGetWeekday    (LPDATE, LPSTR);
BOOL    DLLENTRY DateIntIsValid    (LPDATE);
LPDATE  DLLENTRY DateJulianToDMY   (LONG, LPDATE);
BOOL    DLLENTRY DateSetFormat     (HWND, LPDATEFORMAT);
BOOL    DLLENTRY DateSetRange      (HWND, LPDATE, LPDATE);
BOOL    DLLENTRY DateStringIsValid (LPSTR, LPDATEFORMAT);
LPDATE  DLLENTRY DateStringToDMY   (LPSTR, LPDATE, LPDATEFORMAT);
LONG    DLLENTRY DateSubtractDates (LPDATE, LPDATE);
void    DLLENTRY DateGetCurrentDate(LPSTR lpszDate, LPDATEFORMAT lpdf);
BOOL    DLLENTRY DateSetDMY        (HWND hWnd, LPDATE lpDate);
BOOL    DLLENTRY DateGetDMY        (HWND hWnd, LPDATE lpDate);
#define DateGetDlgItemFormat(h,i,fmt)    DateGetFormat(GetDlgItem((h),(i)),(fmt))
#define DateSetDlgItemFormat(h,i,fmt)    DateSetFormat(GetDlgItem((h),(i)),(fmt))
#define DateSetDlgItemRange(h,i,min,max) DateSetRange(GetDlgItem((h),(i)),(min),(max))
#define DateGetDlgItemDMY(h,i,d)         DateGetDMY(GetDlgItem((h), (i)), (d))
#define DateSetDlgItemDMY(h,i,d)         DateSetDMY(GetDlgItem((h), (i)), (d))

extern void DLLENTRY DateSetJulianStart(short dYear);
extern short DLLENTRY DateGetJulianStart(void);

BOOL    DLLENTRY PopupCalendar     (HWND hTBDateWnd);
LONG    DLLENTRY Calendar          (HWND hWndParent, LPSTR lpszDateText,
                                   LPDATEFORMAT lpDateFormat, LPDATE lpDateMin,
                                   LPDATE lpDateMax);
void    DLLENTRY CalSetNames       (LPSTR lpszDays, LPSTR lpszMonths);
void    DLLENTRY DateWinFormat     (LPSTR, LPDATE, LPSTR);
void    DLLENTRY DateSetNames      (LPSTR, LPSTR, LPSTR, LPSTR);

BOOL    DLLENTRY TimeGetFormat     (HWND, LPTIMEFORMAT);
LONG    DLLENTRY TimeHMSToSeconds  (LPTIME);
LPSTR   DLLENTRY TimeHMSToString   (LPTIME, LPSTR, LPTIMEFORMAT);
LPTIME  DLLENTRY TimeSecondsToHMS  (LONG, LPTIME);
BOOL    DLLENTRY TimeSetFormat     (HWND, LPTIMEFORMAT);
BOOL    DLLENTRY TimeSetRange      (HWND, LPTIME, LPTIME);
BOOL    DLLENTRY TimeStringIsValid (LPSTR, LPTIMEFORMAT);
LPTIME  DLLENTRY TimeStringToHMS   (LPSTR, LPTIME, LPTIMEFORMAT);
void    DLLENTRY TimeGetCurrentTime(LPSTR lpszTime, LPTIMEFORMAT lptf);
#define TimeGetDlgItemFormat(h,i,fmt)    TimeGetFormat(GetDlgItem((h),(i)),(fmt))
#define TimeSetDlgItemFormat(h,i,fmt)    TimeSetFormat(GetDlgItem((h),(i)),(fmt))
#define TimeSetDlgItemRange(h,i,min,max) TimeSetRange(GetDlgItem((h),(i)),(min),(max))

BOOL    DLLENTRY IntSetRange       (HWND hWnd, LONG lMin, LONG lMax);
BOOL    DLLENTRY IntGetRange       (HWND hWnd, LPLONG lpMin, LPLONG lpMax);
BOOL    DLLENTRY IntGetValue       (HWND hWnd, LPLONG lpLong);
BOOL    DLLENTRY IntSetValue       (HWND hWnd, long lLong);
BOOL    DLLENTRY IntSetMask        (HWND hWnd, LPSTR szMask);
BOOL    DLLENTRY IntGetMask        (HWND hWnd, LPSTR szMask);
BOOL    DLLENTRY IntSetSpin        (HWND hWnd, BOOL fSpin, BOOL fSpinWrap,
                                    long lSpinInc);
BOOL    DLLENTRY IntGetSpin        (HWND hWnd, LPBOOL lpfSpinWrap,
                                    LPLONG lplSpinInc);
#define IntSetDlgItemRange(h,i,min,max) IntSetRange(GetDlgItem((h),(i)),(min),(max))
#define IntGetDlgItemRange(h,i,min,max) IntGetRange(GetDlgItem((h),(i)),(min),(max))
#define IntSetDlgItemValue(h,i,Val)     IntSetValue(GetDlgItem((h),(i)),(Val))
#define IntGetDlgItemValue(h,i,Val)     IntGetValue(GetDlgItem((h),(i)),(Val))

BOOL    DLLENTRY FloatGetValue     (HWND hWnd, LPDOUBLE lpDouble);
BOOL    DLLENTRY FloatSetValue     (HWND hWnd, double dfDouble);
BOOL    DLLENTRY FloatSetRange     (HWND hWnd, double dMin, double dMax);
BOOL    DLLENTRY FloatGetRange     (HWND hWnd, LPDOUBLE lpMin,
                                    LPDOUBLE lpMax);
BOOL    DLLENTRY FloatSetFormat    (HWND hWnd, LPFLOATFORMAT ff);
BOOL    DLLENTRY FloatGetFormat    (HWND hWnd, LPFLOATFORMAT ff);
void    DLLENTRY FloatSetStyle     (HWND hWnd, long Style);
LPSTR   DLLENTRY FloatFormatString (double Val, LPSTR szBuffer,
                                    BOOL fSeparator, BOOL fMoney,
                                    short iDecimalPlaces);
LPSTR   DLLENTRY FloatFormatStringExt(double Val, LPSTR szBuffer,
                                      LPFLOATFORMAT lpFormat,
                                      short iDecimalPlaces);
BOOL    DLLENTRY FloatSetMask      (HWND hWnd, LPSTR szMask);
BOOL    DLLENTRY FloatGetMask      (HWND hWnd, LPSTR szMask);
#define FloatSetDlgItemRange(h,i,min,max) FloatSetRange(GetDlgItem((h),(i)),(min),(max))
#define FloatGetDlgItemRange(h,i,min,max) FloatGetRange(GetDlgItem((h),(i)),(min),(max))
#define FloatGetDlgItemValue(h,i,val)    FloatGetValue(GetDlgItem((h),(i)),(val))
#define FloatSetDlgItemValue(h,i,val)    FloatSetValue(GetDlgItem((h),(i)),(val))

BOOL    DLLENTRY GetToolBoxGlobal  (int nID, LPLONG lplValue);
BOOL    DLLENTRY SetToolBoxGlobal  (int nID, LONG lValue);

#define DlgCoordToPixelX(x) (short)((x * LOWORD(GetDialogBaseUnits())) / 4)
#define DlgCoordToPixelY(y) (short)((y * HIWORD(GetDialogBaseUnits())) / 8)

/***************************************
* ToolBox Global Parameter Identifiers
***************************************/

#define TBG_CURRENTDATETIME   1
#define TBG_POPUPCALENDAR     2
#define TBG_CLEARFIELD        3

#define  ERR_AUTO    100

#ifndef  DRV_NOPATTERN
typedef  HGLOBAL     HPATTERN;
#define  PATTERNMAX  999
#define  PATTERNBRUSHMAX   4
typedef  enum tagPATTERNTYPE {SCREEN, PRINTER, PLOTTER} PATTERNTYPE;

VOID     DLLENTRY PatternPie        (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, LPRECT Rect, int x1, int y1, int x2, int y2);
VOID     DLLENTRY PatternPoly       (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, LPPOINT Poly, int Size);
VOID     DLLENTRY PatternRect       (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, LPRECT Rect);
VOID     DLLENTRY PatternRgn        (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, HRGN hRgn);
int      DLLENTRY PatternCreateBrush(HPATTERN hPat, HDC hDC, PATTERNTYPE Type, int nPattern, HBRUSH FAR * lphBrush);

VOID     DLLENTRY _PatternPie       (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, LPRECT Rect, int x1, int y1, int x2, int y2);
VOID     DLLENTRY _PatternPoly      (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, LPPOINT Poly, int Size);
VOID     DLLENTRY _PatternRect      (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, LPRECT Rect);
VOID     DLLENTRY _PatternRgn       (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, HRGN hRgn);
int      DLLENTRY _PatternCreateBrush(HDC hDC, PATTERNTYPE Type, LPVOID pPattern, HBRUSH FAR * lphBrush);

WORD     DLLENTRY PatternGetItemCount(HPATTERN hPat);
DWORD    DLLENTRY PatternGetScreenColor(int iIndex);
DWORD    DLLENTRY PatternGetPlotColor(int iIndex);
int      DLLENTRY PatternGetPlotPattern(int iIndex);
WORD     DLLENTRY PatternGetStructSize(PATTERNTYPE iType);

HPATTERN DLLENTRY PatternEdit       (LPSTR szFileName, int iColorNum);
HPATTERN DLLENTRY PatternLoad       (LPSTR szFileName);
BOOL     DLLENTRY PatternSave       (HPATTERN hPat);
LPVOID   DLLENTRY PatternLock       (HPATTERN hPat, PATTERNTYPE iType, int iIndex);
BOOL     DLLENTRY PatternUnlock     (HPATTERN hPat, PATTERNTYPE Type);
BOOL     DLLENTRY PatternFree       (HPATTERN hPat);
#endif   /* DRV_NOPATTERN */

#ifndef DRV_NOSYS /* no system functions */
#define FI_NORMAL       0x0000   /* Normal file - No read/write restrictions */
#define FI_RDONLY       0x0001   /* Read only file */
#define FI_HIDDEN       0x0002   /* Hidden file */
#define FI_SYSTEM       0x0004   /* System file */
#define FI_VOLID        0x0008   /* Volume ID file */
#define FI_SUBDIR       0x0010   /* Subdirectory */
#define FI_ARCH         0x0020   /* Archive file */


/* The following parameter is used by SysFindFirst() and EnumFiles() */
/* to match ONLY the file mode specified:                            */

#define FI_EXCLUSIVE    0x8000   /* only those selected by above attributes */

/* The following structure is used only by SysFindFirst() and SysFindNext() */

typedef struct tagFILEINFO
   {
   char  reserved[ 21];          /* Reserved for subsequent SysFindNext */
   char  attrib;                 /* Attribute of this file              */
   int   time;                   /* Time of last modification           */
   int   date;                   /* Date of last modification           */
   LONG  fileSize;               /* Size of File                        */
   char  fileName[ 13];          /* File name                           */
   } FILEINFO, FAR *LPFILEINFO, NEAR *NPFILEINFO, *PFILEINFO;

/*********************************************************
* The following structure is used only by SysDiskInfo():
*********************************************************/

typedef struct tagDISKINFO
   {
   WORD  bytesPerSector;         /* Bytes in a Sector (normally 512) */
   WORD  clustersPerDrive;       /* Clusters per Device              */
   WORD  sectorsPerCluster;      /* Sectors in a Cluster             */
   WORD  availableClusters;      /* Number of free Clusters          */
   } DISKINFO, FAR *LPDISKINFO, NEAR *NPDISKINFO, *PDISKINFO;

typedef struct tagPARAMBLK
   {
   WORD  wSegmentEnvBlock;       /* segment pointer to environment block */
   WORD  wOffSetCmdLine;         /* offset of command line               */
   WORD  wSegmentCmdLine;        /* segment of command line              */
   WORD  wOffSetFCB1;            /* etc.                                 */
   WORD  wSegmentFCB1;
   WORD  wOffSetFCB2;
   WORD  wSegmentFCB2;
   } PARAMBLK, FAR *LPPARAMBLK;

/**********************************************
* System and File-Related Function Prototypes
**********************************************/

int      DLLENTRY GetFileString     (LPSTR, LPSTR, LPSTR, int, LPSTR);
int      DLLENTRY SysCreateSubDir   (LPSTR szNewSubDir);
int      DLLENTRY SysDeleteFile     (LPSTR szExistingFile);
int      DLLENTRY SysDeleteSubDir   (LPSTR szExistingDir);
DWORD    DLLENTRY SysDiskBytesFree  (int iDrive);
DWORD    DLLENTRY SysDiskBytesTotal (int iDrive);
BOOL     DLLENTRY SysDiskInfo       (int iDrive, LPDISKINFO di);
BOOL     DLLENTRY SysFileSpec       (LPSTR , LPSTR);
int      DLLENTRY SysFindFirst      (int iAttrib, LPSTR szPath, LPFILEINFO fi);
int      DLLENTRY SysFindNext       (LPFILEINFO);
BOOL     DLLENTRY SysFixedDrive     (int iDrive);
int      DLLENTRY SysGetCurrentDir  (int iDrive, LPSTR szCurrentDir);
LPVOID   DLLENTRY SysGetDTA         (VOID);
VOID     DLLENTRY SysGetDate        (LPINT lpDay, LPINT lpMonth, LPINT lpYear);
int      DLLENTRY SysGetDefDrive    (VOID);
int      DLLENTRY SysGetLogicalDriveCount(VOID);
VOID     DLLENTRY SysGetTime        (LPINT lpHour, LPINT lpMinute, LPINT lpSeconds);
BOOL     DLLENTRY SysGetVolumeName  (int iDrive, LPSTR szVolumeName);
LPSTR    DLLENTRY SysMakePath       (LPSTR, LPSTR, LPSTR, LPSTR, LPSTR);
BOOL     DLLENTRY SysNetworkDrive   (int iDrive);
int      DLLENTRY SysRead           (int iHandle, LPVOID buff, unsigned size, LPWORD read);
int      DLLENTRY SysRenameFile     (LPSTR oldFile, LPSTR newFile);
int      DLLENTRY SysSetCurrentDir  (char FAR *szNewDir);
VOID     DLLENTRY SysSetDTA         (LPVOID);
BOOL     DLLENTRY SysSetDate        (int bDay, int bMonth, int bYear);
int      DLLENTRY SysSetDefDrive    (int iDrive);
BOOL     DLLENTRY SysSetTime        (int bHour, int bMinute, int bSeconds);
BOOL     DLLENTRY SysSetVolumeName  (int iDrive, LPSTR szNewVolumeName);
LPSTR    DLLENTRY SysSplitPath      (LPSTR, LPSTR, LPSTR, LPSTR, LPSTR);
BOOL     DLLENTRY SysValidDrive     (int iDrive);
int      DLLENTRY SysWrite          (int iHandle, LPVOID buff, unsigned size, LPWORD written);
int      DLLENTRY SysXCreateFile    (int attrib, LPSTR filename, LPWORD errorcode);
int      DLLENTRY SysSetFileAttr    (LPSTR path, WORD attrib);
int      DLLENTRY SysGetFileAttr    (LPSTR path, LPWORD attrib);

#ifndef O_RDONLY
#define O_RDONLY       0x0000
#define O_WRONLY       0x0001
#define O_RDWR         0x0002
#define O_APPEND       0x0008
#define O_CREAT        0x0100
#define O_TRUNC        0x0200
#define O_EXCL         0x0400
#define O_TEXT         0x4000
#define O_BINARY       0x8000
#endif

#ifndef S_IREAD
#define S_IREAD        0000400
#define S_IWRITE       0000200
#endif

BOOL DLLENTRY WriteFileString(LPSTR, LPSTR, LPSTR);

typedef enum {CPU86 = 0, CPU186 = 1, CPU286 = 2, CPU386 = 3} CPUID;

CPUID DLLENTRY GetCPUType(void);
#endif   /* DRV_NOSYS */

/* auto messagebox functions */
#define DRVMSG_BEEP  0x0001
#define DRVMSG_LOG   0x0010

#define StrPrintf STRPRINTF
#define StrScanf  STRSCANF
#define Debug     DEBUG
#define MsgDisp   MSGDISP

BOOL     DLLENTRY InitToolBox        (HANDLE);
BOOL     DLLENTRY ToolBoxLoadDlls    (short dDlls);
void     DLLENTRY ToolBoxFreeDlls    (short dDlls);
void     DLLENTRY ToolBoxFreeAllDlls (void);
WORD     DLLENTRY GetToolBoxVersion  (void);

BOOL     DLLENTRY InitTbPro1         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro2         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro3         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro4         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro5         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro6         (HANDLE hInst);

/* HOUR GLASS FUNCTIONS */
WORD     DLLENTRY HourGlassCount    (VOID);
VOID     DLLENTRY HourGlassOff      (VOID);
VOID     DLLENTRY HourGlassOn       (VOID);

/* STRING FUNCTIONS */
LPSTR    DLLENTRY StrCat            (LPSTR, LPSTR);
LPSTR    DLLENTRY StrChk            (LPSTR);
LPSTR    DLLENTRY StrChr            (LPSTR, char);
int      DLLENTRY StrCmp            (LPSTR, LPSTR);
LPSTR    DLLENTRY StrCpy            (LPSTR, LPSTR);
LPSTR    DLLENTRY StrDeleteChar     (LPSTR lpszString, int iStart);
LPSTR    DLLENTRY StrEnd            (LPSTR);
LPSTR    DLLENTRY StrFormat         (LPSTR);
LPSTR    DLLENTRY StrInsertChar     (char cChar, LPSTR lpszString, int iInsertPos);
LPSTR    DLLENTRY StrIntCat         (LPSTR, int);
LPSTR    DLLENTRY StrIntnCat        (LPSTR, int, int);
LPSTR    DLLENTRY StrIntnCpy        (LPSTR, int, int);
LPSTR    DLLENTRY StrLTrim          (LPSTR);
LPSTR    DLLENTRY StrLast           (LPSTR);
int      DLLENTRY StrLen            (LPSTR);
LPSTR    DLLENTRY StrLongCat        (LPSTR, LONG);
LPSTR    DLLENTRY StrLongnCat       (LPSTR, LONG, int);
LPSTR    DLLENTRY StrLongnCpy       (LPSTR, LONG, int);
LPSTR    DLLENTRY StrLwr            (LPSTR);
LPSTR    DLLENTRY StrPad            (LPSTR, int);
int      FAR cdecl StrPrintf         (LPSTR lpBuffer, LPSTR lpFormat,...);
LPSTR    DLLENTRY StrRTrim          (LPSTR);
LPSTR    DLLENTRY StrRemove         (LPSTR lpszString, int iFrom, int iTo);
LPSTR    DLLENTRY StrReplace        (LPSTR, LPSTR);
LPSTR    DLLENTRY StrRev            (LPSTR);
int      FAR cdecl StrScanf          (LPSTR lpStr, LPSTR lpFormat, ...);
LPSTR    DLLENTRY StrStr            (LPSTR, LPSTR);
LPSTR    DLLENTRY StrTok            (LPSTR, LPSTR);
LPSTR    DLLENTRY StrTrim           (LPSTR);
LPSTR    DLLENTRY StrUpr            (LPSTR);
int      DLLENTRY StriCmp           (LPSTR, LPSTR);
LPSTR    DLLENTRY StriStr           (LPSTR, LPSTR);
LPSTR    DLLENTRY Strip             (LPSTR, LPSTR);
LPSTR    DLLENTRY StrnCat           (LPSTR, LPSTR, int);
int      DLLENTRY StrnCmp           (LPSTR, LPSTR, int);
LPSTR    DLLENTRY StrnCpy           (LPSTR, LPSTR, int);
int      DLLENTRY StrniCmp          (LPSTR, LPSTR, int);
LPSTR    DLLENTRY StrnnCat          (LPSTR, LPSTR, int);
LPSTR    DLLENTRY StrnnCpy          (LPSTR, LPSTR, int);
LPSTR    DLLENTRY StrpBrk           (LPSTR, LPSTR);
LPSTR    DLLENTRY StrpCpy           (LPSTR, LPSTR, int);
LPSTR    DLLENTRY StrrChr           (LPSTR, char);
#define  StrEqu(s1,s2)              (!StrCmp (s1, s2))
#define  StrSize(s)                 (StrLen (s)+1)
#define  StriEqu(s1,s2)             (!StriCmp (s1, s2))
#define  StrnEqu(s1,s2,n)           (!StrnCmp (s1, s2, n))
#define  StrniEqu(s1,s2,n)          (!StrniCmp (s1, s2, n))
#define  StrIntCpy(s,i)             IntToString (i, s)
#define  StrLongCpy(s,l)            LongToString (l, s)
                                    
/* CONVERSION FUNCTIONS */
LPSTR    DLLENTRY FloatToEString (LPSTR szBuff, double value, int ndigit, LPINT decpt, LPINT sign);
LPSTR    DLLENTRY FloatToFString (LPSTR szBuff, double value, int ndec, LPINT decpt, LPINT sign);
LPSTR    DLLENTRY IntToString       (int, LPSTR);
LPSTR    DLLENTRY LongToString      (LONG, LPSTR);
void     DLLENTRY StringToFloat     (LPSTR, LPDOUBLE);
int      DLLENTRY StringToInt       (LPSTR);
LONG     DLLENTRY StringToLong      (LPSTR);
                                    
/* These macros added to assure backwards compatibility */

#define FloatToEStr(a, b, c, d, e)  FloatToEString(a, b, c, d, e)
#define FloatToFStr(a, b, c, d, e)  FloatToFString(a, b, c, d, e)
#define _SysGetDefDrive()           (SysGetDefDrive() + 1)
#define _SysSetDefDrive(drive)      (SysSetDefDrive(drive-1))
                                    
/* FILE FUNCTIONS */                        
int      DLLENTRY ErrorNo           (void);
BOOL     DLLENTRY FileClose         (int File);
BOOL     DLLENTRY FileExists        (LPSTR);
BOOL     DLLENTRY FileGetc          (LPSTR, int);
BOOL     DLLENTRY FileGetf          (LPFLOAT, int);
BOOL     DLLENTRY FileGetl          (LPLONG, int);
BOOL     DLLENTRY FileGets          (LPSTR, WORD, int);
BOOL     DLLENTRY FileGetw          (LPINT, int);
BOOL     DLLENTRY FileOpen          (LPSTR Name, int OFlag, int PMode, LPINT File);
BOOL     DLLENTRY FilePutc          (char, int);
BOOL     DLLENTRY FilePutf          (double, int);
BOOL     DLLENTRY FilePutl          (LONG,int);
BOOL     DLLENTRY FilePuts          (LPSTR,int);
BOOL     DLLENTRY FilePutw          (int,int);
WORD     DLLENTRY FileRead          (LPVOID lpbuff, WORD wItemSize, WORD wItemCount, int iFH);
BOOL     DLLENTRY FileRemove        (LPSTR);
BOOL     DLLENTRY FileSeek          (int, LONG, WORD);
WORD     DLLENTRY FileWrite         (LPVOID lpbuff, WORD wItemSize, WORD wItemCount, int iFH);
BOOL     DLLENTRY FileCopy(LPSTR buff, int size, LPSTR src, LPSTR dst);
BOOL     DLLENTRY FileMultipleCopy(LPSTR src, LPSTR dst);
HANDLE   DLLENTRY FileFindFirst(LPSTR szBuff, LPINT lpAttrib, LPSTR szSpec, int iAttrib);
HANDLE   DLLENTRY FileFindNext(LPSTR szBuff, LPINT lpAttrib, HANDLE  hFInfo);
BOOL     DLLENTRY FileSetDateTime(int fh, int time, int date);
BOOL     DLLENTRY FileGetDateTime(int fh, LPINT time, LPINT date);
void     DLLENTRY FileDateConvert(int nDate, LPDATE lpDate);
void     DLLENTRY FileTimeConvert(int nTime, LPTIME lpTime);

/* LIST BOX FUNCTIONS */
BOOL     DLLENTRY ListAddStr        (HWND,LPSTR);
BOOL     DLLENTRY ListDeleteStr     (HWND,int);
BOOL     DLLENTRY ListFindDeleteStr (HWND,LPSTR);
int      DLLENTRY ListFindStr       (HWND,LPSTR,int);
int      DLLENTRY ListGetCount      (HWND);
BOOL     DLLENTRY ListGetText       (HWND,LPSTR,int,int);
BOOL     DLLENTRY ListGetTextByIndex(HWND,LPSTR,int,int,int);
BOOL     DLLENTRY ListHiLiteFirstItem(HWND);
BOOL     DLLENTRY ListReset         (HWND);
BOOL     DLLENTRY ListSelectStr     (HWND,LPSTR);

/* FAR POINTER MEMORY FUNCTIONS (+ ALLOCATION ) */
int      DLLENTRY MemCmp            (const LPVOID, const LPVOID, WORD);
LPVOID   DLLENTRY MemcCpy           (LPVOID, LPVOID, char, WORD);
LPVOID   DLLENTRY MemCpy            (LPVOID, LPVOID, WORD);
LPVOID   DLLENTRY MemMove           (LPVOID, LPVOID, WORD);
LPVOID   DLLENTRY MemSet            (LPVOID, char, WORD);
LPVOID   DLLENTRY MemChr            (LPVOID, char, WORD);
int      DLLENTRY MemiCmp           (LPVOID, LPVOID, WORD);
#define  Zero(struct)               MemSet(&struct,'\0',sizeof(struct))
#define  pZero(ptr)                 MemSet(ptr,'\0',sizeof(*ptr))

/* huge POINTER MEMORY FUNCTIONS */
int      DLLENTRY MemHugeCmp        (HPVOID,HPVOID, WORD);
HPVOID   DLLENTRY MemHugeCpy        (HPVOID,HPVOID, WORD);
HPVOID   DLLENTRY MemHugeSet        (HPVOID,char, WORD);
HPVOID   DLLENTRY MemHugecCpy       (HPVOID,HPVOID,char, WORD);
HPVOID   DLLENTRY MemHugeChr        (HPVOID, char, WORD);

/* MESSAGE DISPLAY */
VOID     FAR cdecl Debug             (LPSTR,...);
int      FAR cdecl MsgDisp           (HANDLE,int /* string ID format */, ...);
int      DLLENTRY MsgDoDisp         (LPSTR,LPSTR, int, int);
LPSTR    DLLENTRY MsgLoad           (HANDLE hInst, int id, LPSTR buff, LPSTR Title, LPINT mflags, LPINT iflags);

BOOL     DLLENTRY DlgFileOpenOne    (HWND,LPSTR,LPSTR,int);
BOOL     DLLENTRY DlgFileOpenTwo    (HWND,LPSTR,LPSTR,int);
BOOL     DLLENTRY DlgFontSelect     (HWND,LPLOGFONT,LPINT);
BOOL     DLLENTRY DlgPrinterSetup   (HWND);
int      DLLENTRY EnumFiles         (LPSTR, int, int (FAR PASCAL *)(LPFILEINFO, LPSTR, LPVOID), LPVOID );
VOID     DLLENTRY ExitNow           (VOID);
int      DLLENTRY FullMkDir         (LPSTR);

BOOL     DLLENTRY GaugeGetPos       (HWND,LPINT);
BOOL     DLLENTRY GaugeGetRange     (HWND,LPINT,LPINT);
BOOL     DLLENTRY GaugeSetColors    (HWND,DWORD,DWORD,DWORD,DWORD);
BOOL     DLLENTRY GaugeSetPos       (HWND,int);
BOOL     DLLENTRY GaugeSetRange     (HWND,int,int);
LPSTR    DLLENTRY GaugeSetTitle     (HWND,LPSTR);

VOID     DLLENTRY GetChildSize      (HWND, LPRECT);
VOID     DLLENTRY Line              (HDC, short, short, short, short);
int      DLLENTRY MsgBox            (LPSTR,LPSTR,WORD);
VOID     DLLENTRY MsgLineMenuHelp   (HWND,WORD);
VOID     DLLENTRY MsgLinePaint      (HWND,HDC);
VOID     DLLENTRY MsgLineTextOut    (HWND,LPSTR);
VOID     DLLENTRY OLRect            (HDC, LPRECT);
VOID	   DLLENTRY OLLine            (HDC, int, int, int, int, int);
HDC      DLLENTRY PrinterGetDC      (VOID);
HDC      DLLENTRY PrinterGetIC      (VOID);
BOOL     DLLENTRY PrinterGetDetails (LPWINDEVICE);
int      DLLENTRY RmTmp             (VOID);
void     DLLENTRY TBFocusRect       (HDC,LPRECT);
LPSTR    DLLENTRY TempNam           (LPSTR, LPSTR);
VOID     DLLENTRY Tile              (HWND,HWND);
int      DLLENTRY TmpFile           (VOID);
LPSTR    DLLENTRY TmpNam            (LPSTR);
int      DLLENTRY WinExecProg       (LPSTR szPath, LPSTR lpCmdLine, int nCmdShow);

#ifndef   DRV_NOSORT
VOID     DLLENTRY QuickSort         (LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (DLLENTRY *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
BOOL     DLLENTRY HeapSort          (LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (DLLENTRY *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
LPVOID   DLLENTRY BinarySearch      (LPVOID lpKey, LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (DLLENTRY *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
HPVOID   DLLENTRY BinarySearchHuge  (HPVOID hpKey, HPVOID hpBase, LONG wTotal, WORD wItemWidth, int (DLLENTRY *lpfnCompare)(HPVOID lpItem1, HPVOID lpItem2));
LPVOID   DLLENTRY LinearFind        (LPVOID lpKey, LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (DLLENTRY *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
HPVOID   DLLENTRY LinearFindHuge    (HPVOID hpKey, HPVOID hpBase, LONG lTotal, WORD wItemWidth, int (DLLENTRY *lpfnCompare)(HPVOID hpItem1, HPVOID hpItem2));

extern void DLLENTRY QuickSortHuge(HPSTR lpBase, WORD Num, WORD Width,
                                   long lUserData,
                                   int (FAR PASCAL *lpCompare)(HPVOID,
                                   HPVOID, long));

#endif   /* DRV_NOSORT */

#ifndef  DRV_NODIALOG
HWND     DLLENTRY DialogNew         (HWND hWndParent, LPSTR szDialogName, HANDLE hInst);
int      DLLENTRY DialogShow        (HWND hWndParent, HWND hDlg);
#endif   /* DRV_NODIALOG */

/* these two functions are undocumented Windows function */
int DLLENTRY KillSystemTimer(HWND hWnd, int nIDEvent);
int DLLENTRY SetSystemTimer(HWND hWnd, int nIDEvent, WORD wElapse, FARPROC lpTimerFunc);

#endif   /* RC_INVOKED */


/******************
* Custom Controls
******************/

#ifndef DRV_NOCLASS

/****************
* Edit controls
****************/

#define LBN_ESCAPE        30       /* the user pressed the escape key */
#define LBN_RETURN        31       /* the user pressed the return key */

#define LBN_LBUTTONDBLCLK 2         /* our own notification */
#define LBN_TOPITEM       1000
#define LBN_RBUTTONDBLCLK 1001      /* our own notification */
#define LBN_MBUTTONDBLCLK 1002      /* our own notification */

/* LB_DIR attributes */

#define LBDA_NORMAL       0x0000
#define LBDA_READONLY     0x0001
#define LBDA_HIDDEN       0x0002
#define LBDA_SYSTEM       0x0004
#define LBDA_SUBDIR       0x0010
#define LBDA_ARCHIVE      0x0020

/* Windows places the messages generated by DlgDirList in the application */
/* queue; otherwise sent directly to the dialog function                  */

#define LBDA_DIRFLAG      0x2000
#define LBDA_DRIVE        0x4000
#define LBDA_EXCLUSIVE    0x8000

#ifndef LB_MSGMAX
#define LB_MSGMAX           (WM_USER+33)
#endif

#define LB_SETSELECTIONMODE (LB_MSGMAX+0)
#define LB_GETTEXTEXTENT    (LB_MSGMAX+1)
#define LB_GETDBLCLKHPOS    (LB_MSGMAX+2)
#define LB_GETDBLCLKITEM    (LB_MSGMAX+3)

/****************
* Edit controls
****************/

typedef struct tagEDIT3DFORMAT
   {
   WORD     wStyle;               /* 3D Styles                            */
   short    dShadowSize;          /* The size of the shadow in pixels     */
   COLORREF ColorShadow;          /* The color of the dark portion        */
   COLORREF ColorHighlight;       /* The color the light portion          */
   COLORREF ColorInsideShadow;    /* The color of the inside dark portion */
   COLORREF ColorInsideHighlight; /* The color the inside light portion   */
   } EDIT3DFORMAT, FAR *LPEDIT3DFORMAT;

#define EDIT3D_OUT          0x01  /* Project the 3D out     */
#define EDIT3D_INSIDEBORDER 0x02  /* Add a 3D inside border */

#ifndef EM_MSGMAX
#define EM_MSGMAX         (WM_USER+30)
#endif

#define EM_VALIDATE       (EM_MSGMAX + 1)
#define EM_GETCARETPOS    (EM_MSGMAX + 2)
#define EM_SETCARETPOS    (EM_MSGMAX + 3)
#define EM_CARETPOS       (EM_MSGMAX + 4)
#define EM_SETRANGE       (EM_MSGMAX + 5)
#define EM_SETFORMAT      (EM_MSGMAX + 6)
#define EM_GETFORMAT      (EM_MSGMAX + 7)
#define EPM_GETMASK       (EM_MSGMAX + 8)
#define EPM_SETMASK       (EM_MSGMAX + 9)
#define EPM_SETCALLBACK   (EM_MSGMAX + 10)
#define EM_GET3D          (EM_MSGMAX + 11)
#define EM_SET3D          (EM_MSGMAX + 12)
#define EPM_GETVALUE      (EM_MSGMAX + 13)
#define EPM_SETVALUE      (EM_MSGMAX + 14)
#define EM_TOOLBOXUSERMAX (EPM_SETVALUE)

#define  EN_INVALIDDATA    0x1000
#define  EN_FIELDNONEMPTY  0x1001
#define  EN_FIELDEMPTY     0x1002
#define  EN_INVALIDSETTEXT 0x1003
#define  EN_INVALIDCHAR   (0x2003)

FARPROC DLLENTRY GetTBEditClassProc(VOID);

extern BOOL DLLENTRY PicFormatText(HWND hWnd, LPSTR lpszDefText,
                                   LPSTR lpszMask, LPSTR lpszText,
                                   LPSTR lpszTextFmt, BOOL fTextUnformatted);

/****************
* Gauge control
****************/

#ifndef DRV_NOGAUGE
#define GS_TITLE       0x00000001L
#define GS_VALUE       0x00000010L
#endif  /* DRV_NOGAUGE */

/*****************
* General styles
*****************/

#define ES_STATIC      0x0080L
#define ES_OVERSTRIKE  0x0800L
#define ES_LEFTALIGN   0x0200L
#define ES_3D          0x0400L

#ifndef ES_UPPERCASE
#define ES_UPPERCASE   0x0008L
#endif

#ifndef ES_LOWERCASE
#define ES_LOWERCASE   0x0010L
#endif

/**************
* Time fields
**************/

#define  TS_12HOUR      0x1000L
#define  TS_24HOUR      0x2000L
#define  TS_SECONDS     0x4000L
#define  TS_SPIN        0x8000L

/**************
* Date fields
**************/

#define  DS_SPIN        0x0040L
#define  DS_DEFCENTURY  0x0004L
#define  DS_CENTURY     0x1000L
#define  DS_DDMONYY     0x2000L
#define  DS_DDMMYY      0x4000L
#define  DS_MMDDYY      0x6000L
#define  DS_YYMMDD      0x8000L

/****************
* Integer Style
****************/

#define IS_SPIN        0x1000L
#define IS_SPINWRAP    0x2000L

/**************
* Float Style
**************/

#define FS_MONEY       0x1000L
#define FS_SEPARATOR   0x2000L

/********************************
* Define WM_CTLCOLOR parameters
********************************/

#define CTLCOLOR_TBEDIT     (101)
#define CTLCOLOR_TBPROGRESS (102)
#define CTLCOLOR_TBVIEWPICT (106)
#define CTLCOLOR_TBCHECKBOX (107)
#define CTLCOLOR_TBRADIOBTN (108)

/**********
* DirFile
**********/

#define DFM_GETSIZE             (WM_USER + 30)
#define DFM_SETSIZE             (WM_USER + 31)

#define DFN_SETFOCUS            30
#define DFN_KILLFOCUS           31
#define DFN_SIZECHANGE          32

#define DFS_DIRRIGHT            0x0001L
#define DFS_NOUPDATEFILELIST    0x0002L
#define DFS_CLOSEALLCHILDREN    0x0004L
#define DFS_OPENROOTCHILDREN    0x0008L
#define DFS_FOLDEROPENDIR       0x0010L
#define DFS_FILELIST_NOSHOWICON 0x0020L
#define DFS_FILELIST_VERT       0x0040L
#define DFS_FILELIST_HORZ       0x0080L
#define DFS_FILELIST_SHOW_SIZE  0x0100L
#define DFS_FILELIST_SHOW_DATE  0x0200L
#define DFS_FILELIST_SHOW_TIME  0x0400L
#define DFS_FILELIST_SHOW_ATTR  0x0800L
#define DFS_MULTISEL            0x1000L

/**********
* DirList
**********/

#define DLM_SETDRIVE              (WM_USER + 0)
#define DLM_READDRIVE             (WM_USER + 1)
#define DLM_GETSELDIRENTRY        (WM_USER + 2)
#define DLM_GETSELDIR             (WM_USER + 3)
#define DLM_GETDIRCNT             (WM_USER + 4)
#define DLM_SETSELDIR             (WM_USER + 5)
#define DLM_SETCOLOR              (WM_USER + 6)
#define DLM_OPENCHILDREN          (WM_USER + 7)
#define DLM_GETCOLOR              (WM_USER + 8)
#define DLM_SETSORTMETHOD         (WM_USER + 9)
#define DLM_SETSORTORDER          (WM_USER + 40)
#define DLM_GETCURDIR             (WM_USER + 41)
#define DLM_SETCURDIR             (WM_USER + 42)
#define DLM_GETSEL                (WM_USER + 43)
#define DLM_GETDRIVEHANDLE        (WM_USER + 44)
#define DLM_GETCURDIRENTRY        (WM_USER + 45)

#define DLN_SETFOCUS              10
#define DLN_KILLFOCUS             11
#define DLN_SELCHANGE             12
#define DLN_DBLCLK                13
#define DLN_TAG                   14

#define DLS_CLOSEALLCHILDREN      0x0001L
#define DLS_OPENROOTCHILDREN      0x0002L
#define DLS_FOLDEROPENDIR         0x0004L
#define DLS_SORT_NONE             0x0008L
#define DLS_SORT_NAME             0x0010L
#define DLS_SORT_DATE             0x0020L
#define DLS_SORT_EXT              0x0040L
#define DLS_SORT_DESC             0x0080L
#define DLS_SORT_SIZE             0x0100L
#define DLS_MULTISEL              0x0200L

#define DIRLIST_SORTORDER_ASC     0
#define DIRLIST_SORTORDER_DESC    1

#define DIRLIST_CHILDREN_OPENALL  0
#define DIRLIST_CHILDREN_OPENROOT 1
#define DIRLIST_CHILDREN_CLOSEALL 2
#define DIRLIST_CHILDREN_OPENCUR  3
#define DIRLIST_CHILDREN_CLOSECUR 4
#define DIRLIST_CHILDREN_FLIPCUR  5

typedef struct dirlistcolor
   {
   COLORREF ColorBack;
   COLORREF ColorText;
   COLORREF ColorLines;
   COLORREF ColorIconShadow;
   COLORREF ColorIconBack;
   COLORREF ColorIconFore;
   } DIRLISTCOLOR, FAR *LPDIRLISTCOLOR;

/************
* DriveList
************/

#define DVLM_GETDRIVECOUNT        (WM_USER + 0)
#define DVLM_GETSELDRIVE          (WM_USER + 1)
#define DVLM_SETSELDRIVE          (WM_USER + 2)
#define DVLM_SETCOLOR             (WM_USER + 3)
#define DVLM_GETCOLOR             (WM_USER + 4)

#define DVLN_SETFOCUS             10   /* Used Internally */
#define DVLN_KILLFOCUS            11
#define DVLN_SELCHANGE            12   /* Used Internally */
#define DVLN_DBLCLK               13
#define DVLN_DRIVECHANGE          14

#define DVLS_DRIVELIST_FIXED      0x01
#define DVLS_DRIVELIST_REMOTE     0x02
#define DVLS_DRIVELIST_REMOVABLE  0x04
#define DVLS_DRIVELIST_ALL        0x07
#define DVLS_DRIVELIST_LISTBOX    0x08
#define DVLS_DRIVELIST_COMBOBOX   0x10
#define DVLS_DRIVELIST_NOSHOWICON 0x20

typedef struct drivelistcolor
   {
   COLORREF ColorBack;
   COLORREF ColorText;
   COLORREF ColorIconShadowBright;
   COLORREF ColorIconShadowDark;
   COLORREF ColorIconBack;
   COLORREF ColorIconFore;
   COLORREF ColorIconLight;
   } DRIVELISTCOLOR, FAR *LPDRIVELISTCOLOR;

/***********
* FileList
***********/

#define FLM_SETDIRECTORY        (WM_USER + 10)   /* Used Internally */
#define FLM_READDIRECTORY       (WM_USER + 11)
#define FLM_SETVIEWTYPE         (WM_USER + 12)
#define FLM_SETSORTMETHOD       (WM_USER + 13)
#define FLM_SETSORTORDER        (WM_USER + 14)
#define FLM_SETCOLOR            (WM_USER + 15)
#define FLM_GETFILECOUNT        (WM_USER + 16)
#define FLM_GETSELFILE          (WM_USER + 17)
#define FLM_SETSELFILE          (WM_USER + 18)
#define FLM_SETFILESPEC         (WM_USER + 19)
#define FLM_GETCOLOR            (WM_USER + 20)
#define FLM_GETSELCOUNT         (WM_USER + 21)
#define FLM_SELECTALL           (WM_USER + 22)
#define FLM_GETCURFILE          (WM_USER + 23)
#define FLM_SETCURFILE          (WM_USER + 24)
#define FLM_GETSEL              (WM_USER + 25)

#define FLN_SETFOCUS            20
#define FLN_KILLFOCUS           21
#define FLN_SELCHANGE           22
#define FLN_DBLCLK              23
#define FLN_TAG                 24

#define FLS_FILELIST_SORT_NONE  0x0001L
#define FLS_FILELIST_SORT_NAME  0x0002L
#define FLS_FILELIST_SORT_DATE  0x0004L
#define FLS_FILELIST_SORT_SIZE  0x0008L
#define FLS_FILELIST_SORT_EXT   0x0010L
#define FLS_FILELIST_SORT_DESC  0x0020L
#define FLS_FILELIST_NOSHOWICON 0x0040L
#define FLS_FILELIST_VERT       0x0080L
#define FLS_FILELIST_HORZ       0x0100L
#define FLS_FILELIST_SHOW_SIZE  0x0200L
#define FLS_FILELIST_SHOW_DATE  0x0400L
#define FLS_FILELIST_SHOW_TIME  0x0800L
#define FLS_FILELIST_SHOW_ATTR  0x1000L
#define FLS_FILELIST_MULTISEL   0x2000L

#define FILELIST_SORTORDER_ASC  0
#define FILELIST_SORTORDER_DESC 1

typedef struct filelistcolor
   {
   COLORREF ColorBack;
   COLORREF ColorText;
   COLORREF ColorIconShadow;
   COLORREF ColorIconExeBack;
   COLORREF ColorIconExeFore;
   COLORREF ColorIconExeUpper;
   COLORREF ColorIconDocBack;
   COLORREF ColorIconDocFore;
   COLORREF ColorIconDocUpper;
   } FILELISTCOLOR, FAR *LPFILELISTCOLOR;

/**********
* Imprint
**********/

#define IS_IMPRINT_OUT            0x0001L
#define IS_IMPRINT_IN             0x0002L
#define IS_FRAME_OUT              0x0004L
#define IS_FRAME_IN               0x0008L
#define IS_INSIDEBORDER           0x0010L
#define IS_OUTSIDEBORDER          0x0020L
#define IS_GROUPBOX               0x0040L
#define IS_CLEAR                  0x0080L
#define IS_TEXTBOTTOM             0x0100L
#define IS_TEXTCENTER             0x0200L
#define IS_TEXTLEFT               0x0400L
#define IS_TEXTRIGHT              0x0800L
#define IS_TEXTTOP                0x1000L
#define IS_TEXTVCENTER            0x2000L
#define IS_TEXTBORDER             0x4000L
#define IS_SHADOWSIZE1            0x8000L

#define IM_GETFRAMESIZE           (WM_USER + 0)
#define IM_GETLINECNT             (WM_USER + 1)
#define IM_GETLINESIZE            (WM_USER + 2)
#define IM_GETSHADOWSIZE          (WM_USER + 3)
#define IM_SETCOLOR               (WM_USER + 4)
#define IM_SETFRAMESIZE           (WM_USER + 5)
#define IM_SETLINECNT             (WM_USER + 6)
#define IM_SETLINESIZE            (WM_USER + 7)
#define IM_SETSHADOWSIZE          (WM_USER + 8)
#define IM_GETCOLOR               (WM_USER + 9)

typedef struct imprintcolor
   {
   COLORREF Color;
   COLORREF ColorFrame;
   COLORREF ColorShadow;
   COLORREF ColorHighlight;
   COLORREF ColorInsideBorder;
   COLORREF ColorOutsideBorder;
   COLORREF ColorText;
   COLORREF ColorLines;
   } IMPRINTCOLOR, FAR *LPIMPRINTCOLOR;

/***********
* Progress
***********/

#define PS_VERTICAL       0x01
#define PS_HORIZONTAL     0x02
#define PS_HIDEPERCENTAGE 0x04

#define PM_GETRANGE       (WM_USER + 0)
#define PM_GETPOS         (WM_USER + 1)
#define PM_SETRANGE       (WM_USER + 2)
#define PM_SETPOS         (WM_USER + 3)

/**********
* SpinBtn
**********/

#define SBNS_UPARROW      0x001L
#define SBNS_DOWNARROW    0x002L
#define SBNS_UPDOWNARROW  0x003L
#define SBNS_HORIZONTAL   0x004L
#define SBNS_VERTICAL     0x008L
#define SBNS_BORDERNONE   0x010L
#define SBNS_BORDERLEFT   0x020L
#define SBNS_BORDERRIGHT  0x040L
#define SBNS_BORDERTOP    0x080L
#define SBNS_BORDERBOTTOM 0x100L

/************
* Check Box
************/

#define BM_SETPICT               (WM_USER + 51)

#define BT_NONE                  0x0000
#define BT_ICON                  0x0001
#define BT_BITMAP                0x0002
#define BT_HANDLE                0x0004
#define BT_PICT_UP               0x0010
#define BT_PICT_DOWN             0x0020
#define BT_PICT_FOCUSUP          0x0040
#define BT_PICT_FOCUSDOWN        0x0080
#define BT_PICT_DISABLEDUP       0x0100
#define BT_PICT_DISABLEDDOWN     0x0200

#define BS_CENTER                0x0800

extern BOOL DLLENTRY tbCheckBoxLoadDef(HANDLE hInst, WORD wButtonType,
                                       long lPict);
extern BOOL DLLENTRY tbCheckBoxUnloadDefs(HANDLE hInst);

/***********
* RadioBtn
***********/

extern BOOL DLLENTRY tbRadioBtnLoadDef(HANDLE hInst, WORD wButtonType,
                                       long lPict);
extern BOOL DLLENTRY tbRadioBtnUnloadDefs(HANDLE hInst);

/***********
* SuperBtn
***********/

#define SBS_DEFPUSHBUTTON        0x0001L
#define SBS_TEXTTOP_PICTBOT      0x0002L
#define SBS_TEXTBOT_PICTTOP      0x0004L
#define SBS_TEXTLEFT_PICTRIGHT   0x0008L
#define SBS_TEXTRIGHT_PICTLEFT   0x0010L
#define SBS_BTNFRAMEIN           0x0020L
#define SBS_BTNFRAMEOUT          0x0040L
#define SBS_PICTFRAMEIN          0x0080L
#define SBS_PICTFRAMEOUT         0x0100L
#define SBS_TEXTFRAMEIN          0x0200L
#define SBS_TEXTFRAMEOUT         0x0400L
#define SBS_NOINVERTFRAME        0x0800L
#define SBS_TEXTICON             0x1000L
#define SBS_TEXTBITMAP           0x2000L
#define SBS_TEXTTEXT             0x4000L
#define SBS_NOPOINTERFOCUS       0x8000L

#define SBM_SETSTYLE             BM_SETSTYLE
#define SBM_SETCOLOR             (WM_USER + 50)
#define SBM_SETPICT              BM_SETPICT
#define SBM_SETPICT_DISABLED     (WM_USER + 52)
#define SBM_SETPICT_BTNDOWN      (WM_USER + 53)
#define SBM_GETCOLOR             (WM_USER + 54)
#define SBM_SETBUTTONTYPE        (WM_USER + 55)
#define SBM_GETBUTTONSTATE       (WM_USER + 56)
#define SBM_SETBUTTONSTATE       (WM_USER + 57)
#define SBM_SETSHADOWSIZE        (WM_USER + 58)
#define SBM_GETSHADOWSIZE        (WM_USER + 59)
#define SBM_SETSTRETCH           (WM_USER + 60)
#define SBM_GETSTRETCH           (WM_USER + 61)

#define SBN_CLICKED              BN_CLICKED

#define SUPERBTN_UP              0
#define SUPERBTN_DOWN            1

#define SUPERBTN_TYPE_NORMAL     0
#define SUPERBTN_TYPE_2STATE     1

#define SUPERBTN_PICT_NONE       0
#define SUPERBTN_PICT_ICON       BT_ICON
#define SUPERBTN_PICT_BITMAP     BT_BITMAP
#define SUPERBTN_PICT_HANDLE     BT_HANDLE

typedef struct superbtncolor
   {
   COLORREF Color;
   COLORREF ColorBorder;
   COLORREF ColorShadow;
   COLORREF ColorHighlight;
   COLORREF ColorText;
   } SUPERBTNCOLOR, FAR *LPSUPERBTNCOLOR;

/******************
* ToolBox/ToolBar
******************/

#define TBXS_SINGLESEL    0x01L
#define TBXS_MULTISEL     0x02L
#define TBXS_NOSTATE      0x04L

#define TBX_NOGROUP       0

#define TBX_NOSEL         0
#define TBX_SELECTED      1
#define TBX_DISABLED      2

#define TBX_RESIZE        -1
#define TBX_MAXSIZE       -2

#define TBXM_SETCOLOR     (WM_USER + 50)
#define TBXM_GETCOLOR     (WM_USER + 51)
#define TBXM_GETITEMSTATE (WM_USER + 52)
#define TBXM_SETITEMSTATE (WM_USER + 53)
#define TBXM_GETITEMCNT   (WM_USER + 54)
#define TBXM_ADDITEM      (WM_USER + 55)
#define TBXM_INSERTITEM   (WM_USER + 56)
#define TBXM_DELETEITEM   (WM_USER + 57)
#define TBXM_SETGROUP     (WM_USER + 58)
#define TBXM_GETITEMRECT  (WM_USER + 59)
#define TBXM_CHANGEITEM   (WM_USER + 60)

#define TBXN_CLICKED      1

#define WM_TBXBUTTON      0x0600
#define WM_TBXDESTROY     0x0601

/***************************
* Button Message Structure
***************************/

typedef struct tbx_button
   {
   short dItemID;               /* ID of clicked on item                     */
   short xPos;                  /* Mouse x coordinate                        */
   short yPos;                  /* Mouse y coordinate                        */
   WORD  wKeys;                 /* Other keys that are down                  */
                                /*    MK_CONTROL                             */
                                /*    MK_LBUTTON                             */
                                /*    MK_MBUTTON                             */
                                /*    MK_RBUTTON                             */
                                /*    MK_SHIFT                               */
   WORD  wButtonType;           /* Button Type. May be one of the following: */
                                /*    TBX_BUTTONTYPE_LDOWN                   */
                                /*    TBX_BUTTONTYPE_LUP                     */
                                /*    TBX_BUTTONTYPE_LDBLCLK                 */
                                /*    TBX_BUTTONTYPE_RDOWN                   */
                                /*    TBX_BUTTONTYPE_RUP                     */
                                /*    TBX_BUTTONTYPE_RDBLCLK                 */
   } TBX_BUTTON, FAR *LPTBX_BUTTON;

#define TBX_BUTTONTYPE_LDOWN   1
#define TBX_BUTTONTYPE_LUP     2
#define TBX_BUTTONTYPE_LDBLCLK 3
#define TBX_BUTTONTYPE_RDOWN   4
#define TBX_BUTTONTYPE_RUP     5
#define TBX_BUTTONTYPE_RDBLCLK 6

#define TBXS_TEXTTOP_PICTBOT      SBS_TEXTTOP_PICTBOT
#define TBXS_TEXTBOT_PICTTOP      SBS_TEXTBOT_PICTTOP
#define TBXS_TEXTLEFT_PICTRIGHT   SBS_TEXTLEFT_PICTRIGHT
#define TBXS_TEXTRIGHT_PICTLEFT   SBS_TEXTRIGHT_PICTLEFT
#define TBXS_BTNFRAMEIN           SBS_BTNFRAMEIN
#define TBXS_BTNFRAMEOUT          SBS_BTNFRAMEOUT
#define TBXS_PICTFRAMEIN          SBS_PICTFRAMEIN
#define TBXS_PICTFRAMEOUT         SBS_PICTFRAMEOUT
#define TBXS_TEXTFRAMEIN          SBS_TEXTFRAMEIN
#define TBXS_TEXTFRAMEOUT         SBS_TEXTFRAMEOUT
#define TBXS_NOINVERTFRAME        SBS_NOINVERTFRAME
#define TBXS_STRETCH              0x1000

typedef struct toolboxcolor
   {
   COLORREF Background;
   COLORREF Border;
   COLORREF ButtonBorder;
   COLORREF ButtonUpColor;
   COLORREF ButtonUpShadow;
   COLORREF ButtonUpHighlight;
   COLORREF ButtonUpText;
   COLORREF ButtonDownColor;
   COLORREF ButtonDownShadow;
   COLORREF ButtonDownHighlight;
   COLORREF ButtonDownText;
   } TOOLBOXCOLOR, FAR *LPTOOLBOXCOLOR;

typedef struct toolboxitem
   {
   short Id;
   LPSTR lpPictUpName;
   short dPictUpType;
   LPSTR lpPictDownName;
   short dPictDownType;
   LPSTR lpPictDisabledName;
   short dPictDisabledType;
   LPSTR lpText;
   short dColWidth;
   short dRowHeight;
   long  lStyle;
   } TOOLBOXITEM, FAR *LPTOOLBOXITEM;

HWND FAR PASCAL tbCreateToolBox(HANDLE hInstance, HWND hWndParent,
                                DWORD lStyle, short x, short y,
                                short dMinWidth, short dMinHeight,
                                short dColCnt, short dRowCnt,
                                short dBorderSize, LPSTR lpszTitle);

/************
* StatusBar
************/

#define SBRS_TEXTLEFT         0x0001
#define SBRS_TEXTCENTER       0x0002
#define SBRS_TEXTRIGHT        0x0004
#define SBRS_DRAWTOPLINE      0x0008
#define SBRS_PROGRESS         0x0010
#define SBRS_PIXELS           0x0020
#define SBRS_PERCENTSCREEN    0x0040
#define SBRS_PERCENT          0x0080
#define SBRS_TWIPS            0x0100
#define SBRS_DLGUNITS         0x0200
#define SBRS_OWNERDRAW        0x0400
#define SBRS_NO3D             0x0800
#define SBRS_3DOUT            0x1000

#define SBRM_SETCOLOR         (WM_USER + 50)
#define SBRM_GETCOLOR         (WM_USER + 51)
#define SBRM_SETITEMS         (WM_USER + 52)
#define SBRM_SETITEMTEXT      (WM_USER + 53)
#define SBRM_GETITEMTEXT      (WM_USER + 54)
#define SBRM_GETITEMRECT      (WM_USER + 55)
#define SBRM_SETPROGRESSPOS   (WM_USER + 56)
#define SBRM_SETPROGRESSRANGE (WM_USER + 57)

#define WM_SBRBUTTON          0x0570

#define SBR_AUTOSIZE          -1

#define ODT_TBSTATUSBAR       500

/*******************************
* Status Bar Message Structure
*******************************/

typedef struct sbr_button
   {
   short dItemID;               /* ID of clicked on item                     */
   short xPos;                  /* Mouse x coordinate                        */
   short yPos;                  /* Mouse y coordinate                        */
   WORD  wKeys;                 /* Other keys that are down                  */
                                /*    MK_CONTROL                             */
                                /*    MK_LBUTTON                             */
                                /*    MK_MBUTTON                             */
                                /*    MK_RBUTTON                             */
                                /*    MK_SHIFT                               */
   WORD  wButtonType;           /* Button Type. May be one of the following: */
                                /*    SBR_BUTTONTYPE_LDOWN                   */
                                /*    SBR_BUTTONTYPE_LUP                     */
                                /*    SBR_BUTTONTYPE_LDBLCLK                 */
                                /*    SBR_BUTTONTYPE_RDOWN                   */
                                /*    SBR_BUTTONTYPE_RUP                     */
                                /*    SBR_BUTTONTYPE_RDBLCLK                 */
   } SBR_BUTTON, FAR *LPSBR_BUTTON;

#define SBR_BUTTONTYPE_LDOWN   1
#define SBR_BUTTONTYPE_LUP     2
#define SBR_BUTTONTYPE_LDBLCLK 3
#define SBR_BUTTONTYPE_RDOWN   4
#define SBR_BUTTONTYPE_RUP     5
#define SBR_BUTTONTYPE_RDBLCLK 6

typedef struct statusbarcolor
   {
   COLORREF Line;
   COLORREF Background;
   COLORREF Shadow;
   COLORREF Highlight;
   COLORREF Text;
   COLORREF Progress;
   } STATUSBARCOLOR, FAR *LPSTATUSBARCOLOR;

typedef struct statusbarprogress
   {
   short dMin;
   short dMax;
   BOOL  fShowPercentage;
   } STATUSBARPROGRESS, FAR *LPSTATUSBARPROGRESS;

typedef struct statusbaritem
   {
   WORD  ItemID;
   long  lOffset;
   long  lWidth;
   LPSTR lpText;
   long  lItemStyle;
   } STATUSBARITEM, FAR *LPSTATUSBARITEM;

HWND FAR PASCAL tbCreateStatusBar(HWND hWndParent, DWORD lStyle, short x,
                                  short y, short dWidth, short dHeight,
                                  LPSTATUSBARITEM lpItems, short dItemCnt);

/***********
* ViewText
***********/

#define VTS_OWNERSUPPLYTEXT       0x01L
#define VTS_NOHSCROLLBAR          0x02L
#define VTS_NOVSCROLLBAR          0x04L

#define VTM_ADDSTRING             (WM_USER + 0)
#define VTM_CLEAR                 (WM_USER + 1)
#define VTM_DELETESTRING          (WM_USER + 2)
#define VTM_GETLINECOUNT          (WM_USER + 3)
#define VTM_GETSTRING             (WM_USER + 4)
#define VTM_GETSTRINGLEN          (WM_USER + 5)
#define VTM_GETTOPINDEX           (WM_USER + 6)
#define VTM_HIGHLIGHTTEXT         (WM_USER + 7)
#define VTM_INSERTSTRING          (WM_USER + 8)
#define VTM_REDRAW                (WM_USER + 9)
#define VTM_SEARCH                (WM_USER + 10)
#define VTM_SEARCHREPEAT          (WM_USER + 11)
#define VTM_SETCOLHEADERS         (WM_USER + 12)
#define VTM_SETCOLOR              (WM_USER + 13)
#define VTM_GETCOLOR              (WM_USER + 14)
#define VTM_SETHEADER             (WM_USER + 15)
#define VTM_SETROWHEADERS         (WM_USER + 16)
#define VTM_SETTOPINDEX           (WM_USER + 17)

#define VTM_VIEWSIZE              0x0510
#define VTM_RETRIEVETEXT          0x0511

#define VTN_KILLFOCUS             1
#define VTN_SETFOCUS              2

#define VIEWTEXT_SEARCH_CASE      0x0001
#define VIEWTEXT_SEARCH_NOCASE    0x0002

#define VIEWTEXT_SEARCH_CURRENT   0x0004
#define VIEWTEXT_SEARCH_BEGIN     0x0008
#define VIEWTEXT_SEARCH_END       0x0010

#define VIEWTEXT_SEARCH_FORWARD   0x0020
#define VIEWTEXT_SEARCH_BACKWARD  0x0040

#define VIEWTEXT_SEARCH_SUBSTRING 0x0080
#define VIEWTEXT_SEARCH_WHOLEWORD 0x0100

typedef struct viewtextcolor
   {
   COLORREF BackGround;
   COLORREF MainText;
   COLORREF Header;
   COLORREF ColTitles;
   COLORREF RowTitles;
   COLORREF HighLightText;
   COLORREF HighLightBk;
   } VIEWTEXTCOLOR, FAR *LPVIEWTEXTCOLOR;

typedef struct viewsize
   {
   short WidthMax;
   short ItemCnt;
   } VIEWSIZE, FAR *LPVIEWSIZE;

typedef struct viewretrievetext
   {
   short ItemNum;
   LPSTR ItemText;
   } VIEWRETRIEVETEXT, FAR *LPVIEWRETRIEVETEXT;

/***********
* ViewPict
***********/

#define VPS_NORMAL            0x0000L
#define VPS_CENTER            0x0001L
#define VPS_STRETCH           0x0002L
#define VPS_MAINTAINSCALE     0x0004L
#define VPS_SCROLL            0x0008L
#define VPS_RESOURCE          0x0010L
#define VPS_BMP               0x0020L
#define VPS_PCX               0x0040L
#define VPS_GIF               0x0080L
#define VPS_FREEZE            0x0100L
#define VPS_SHOWFRAMEONCE     0x0200L
#define VPS_STATIC            0x0400L
#define VPS_HANDLE            0x0800L
#define VPS_ICON              0x1000L

/********************
* ViewPict Messages
********************/

#define VPN_KILLFOCUS         1
#define VPN_SETFOCUS          2
#define VPN_BUTTONDOWN        3
#define VPN_DBLCLK            4

#define VPM_SETPICTURE        (WM_USER + 0)
#define VPM_CLEAR             (WM_USER + 1)
#define VPM_SETSTYLE          (WM_USER + 2)
#define VPM_GETPOINTERPOS     (WM_USER + 3)
#define VPM_SETPALETTE        (WM_USER + 9)
#define VPM_GETPALETTE        (WM_USER + 10)

/*************************
* Messages for animation
*************************/

#define VPM_SETFRAMES         (WM_USER + 4)
#define VPM_FREEZEANIMATION   (WM_USER + 5)
#define VPM_STARTANIMATION    (WM_USER + 6)
#define VPM_SHOWNEXTFRAME     (WM_USER + 7)
#define VPM_SHOWPREVFRAME     (WM_USER + 8)

/**************************
* ViewPict Error Messages
**************************/

#define VIEWPICT_ERR_OK       0
#define VIEWPICT_ERR_NOTFOUND 1
#define VIEWPICT_ERR_MEMORY   2
#define VIEWPICT_ERR_FILEERR  3
#define VIEWPICT_ERR_BADPICT  4

#define VIEWPICT_END          -1
#define VIEWPICT_DEFDELAY     -1

typedef struct viewpict_animatehdr
   {
   short PictureCnt;
   short TimeDelay;
   } VIEWPICT_ANIMATEHDR, FAR *LPVIEWPICT_ANIMATEHDR;

typedef struct viewpict_animatepict
   {
   LPSTR PictName;
   short PictStyle;
   short x;
   short y;
   short TimeDelay;
   } VIEWPICT_ANIMATEPICT, FAR *LPVIEWPICT_ANIMATEPICT;

typedef struct viewpict_animate
   {
   VIEWPICT_ANIMATEHDR    AnimateHdr;
   LPVIEWPICT_ANIMATEPICT AnimatePict;
   } VIEWPICT_ANIMATE, FAR *LPVIEWPICT_ANIMATE;

/**************
* SpreadSheet
**************/

#ifndef TB_NOSPREAD

typedef ULONG TBGLOBALHANDLE, FAR *LPTBGLOBALHANDLE;

/******************
* Notify Messages
******************/

#define SSM_DBLCLK             0x0501
#define SSM_EDITMODEON         0x0502
#define SSM_EDITMODEOFF        0x0503
#define SSM_CELLLOCKED         0x0504
#define SSM_ROWRESTRICTED      0x0505
#define SSM_COLRESTRICTED      0x0506
#define SSM_COLROWRESTRICTED   0x0507
#define SSM_DATACHANGE         0x0508
#define SSM_LEAVECELL          0x0509
#define SSM_CLICK              0x050A
#define SSM_KEYDOWN            0x050B
#define SSM_LEAVEROW           0x050C
#define SSM_RBUTTON            0x050D
#define SSM_ENTERROW           0x050E
#define SSM_COLWIDTHCHANGE     0x050F
#define SSM_ROWHEIGHTCHANGE    0x0510
#define SSM_DRAWITEM           0x0511
#define SSM_VQUERYDATA         0x0512
#define SSM_DRAGDROP           0x0513

#define SSN_SETFOCUS           51
#define SSN_KILLFOCUS          52
#define SSN_INVALIDDATA        53
#define SSN_BLOCKSELECTED      54
#define SSN_BLOCKCHANGED       55
#define SSN_USERFORMULA        56
#define SSN_EXITNEXT           57
#define SSN_EXITPREV           58
#define SSN_BUTTONDOWN         59
#define SSN_BUTTONUP           60
#define SSN_TOPROWCHANGE       61
#define SSN_LEFTCOLCHANGE      62
#define SSN_TOPLEFTCHANGE      63
#define SSN_BLOCKDESELECTED    64

#define SSS_RESTRICTROWS       0x001L
#define SSS_RESTRICTCOLS       0x002L
#define SSS_PROTECT            0x004L
#define SSS_LOCKALL            0x008L
#define SSS_NORESIZE           0x010L
#define SSS_NOSELECTBLOCK      0x020L
#define SSS_NOGRIDLINES        0x040L
#define SSS_ALLOWUSERFORMULAS  0x080L
#define SSS_HIDECOLHEADERS     0x100L
#define SSS_HIDEROWHEADERS     0x200L
#define SSS_ALLOWCELLOVERFLOW  0x400L
#define SSS_SCROLLBAREXTMODE   0x800L

#define SS_GWL_USERDATA        4

#ifdef SS_USE16BITCOORDS
typedef short SS_COORD;
#else
typedef long SS_COORD;
#endif

typedef SS_COORD FAR *LPSS_COORD;
typedef short SS_FONTID;
typedef short SS_COLORID;

/*********************************
* Define Calculation definitions
*********************************/

#define SS_CALC_MAXPARAMS      20
#define SS_CALC_VARPARAMS      -1

/******************************
* Calculation Value Structure
******************************/

typedef struct ss_value
   {
   char Status;
   char Type;
   union
      {
      GLOBALHANDLE hValStr;
      long         ValLong;
      double       ValDouble;
      } Val;
   } SS_VALUE, FAR *LPSS_VALUE;

typedef FARPROC SS_CALCFUNC;

/*********************************
* Define Calculation definitions
*********************************/

#define SS_VALUE_STATUS_OK     0
#define SS_VALUE_STATUS_ERROR  1
#define SS_VALUE_STATUS_EMPTY  2
#define SS_VALUE_STATUS_CLEAR  3
#define SS_VALUE_STATUS_NONE   4
        
#define SS_VALUE_TYPE_LONG     0
#define SS_VALUE_TYPE_DOUBLE   1
#define SS_VALUE_TYPE_STR      2

/***************
* Print Format
***************/

#define SS_PRINTCMD_HEADERLEN   0
#define SS_PRINTCMD_FOOTERLEN   1
#define SS_PRINTCMD_PRINTHEADER 2
#define SS_PRINTCMD_PRINTFOOTER 3

typedef short (FAR PASCAL * SS_PRINTFUNC)(HDC hDC, short dCommand,
               short nPageNum, LONG lAppData);

#define SS_PRINTTYPE_CURRENTPAGE 0x01
#define SS_PRINTTYPE_CELLRANGE   0x02
#define SS_PRINTTYPE_PAGERANGE   0x04

typedef struct ss_printformat
   {
   BOOL     fDrawBorder;
   BOOL     fDrawShadows;
   BOOL     fDrawColors;
   BOOL     fShowGrid;
   BOOL     fShowColHeaders;
   BOOL     fShowRowHeaders;
   BOOL     fUseDataMax;
   short    dPrintType;
   SS_COORD x1CellRange;
   SS_COORD y1CellRange;
   SS_COORD x2CellRange;
   SS_COORD y2CellRange;
   short    nPageStart;
   short    nPageEnd;
   float    fMarginLeft;
   float    fMarginTop;
   float    fMarginRight;
   float    fMarginBottom;
   HDC      hDCPrinter;
   char     Filler[20];
   } SS_PRINTFORMAT, FAR *LPSS_PRINTFORMAT;

/******************
* Cell Coordinate
******************/

typedef struct ss_cellcoord
   {
   SS_COORD Col;
   SS_COORD Row;
   } SS_CELLCOORD, FAR *LPSS_CELLCOORD;

/*******************
* Coordinate Range
*******************/

typedef struct ss_coordrange
   {
   SS_COORD Coord1;
   SS_COORD Coord2;
   } SS_COORDRANGE, FAR *LPSS_COORDRANGE;

/*************************
* EditModeOff Coordinate
*************************/

typedef struct ss_editmodeoff
   {
   SS_COORD Col;
   SS_COORD Row;
   BOOL     fChangeMade;
   } SS_EDITMODEOFF, FAR *LPSS_EDITMODEOFF;

/*************
* Leave Cell
*************/

typedef struct ss_leavecell
   {
   SS_COORD ColCurrent;
   SS_COORD RowCurrent;
   SS_COORD ColNew;
   SS_COORD RowNew;
   } SS_LEAVECELL, FAR *LPSS_LEAVECELL;

/************
* Enter Row
************/

typedef struct ss_enterrow
   {
   SS_COORD Row;
   BOOL     fNewRow;
   } SS_ENTERROW, FAR *LPSS_ENTERROW;

/************
* Leave Row
************/

typedef struct ss_leaverow
   {
   SS_COORD RowCurrent;               /* Current selected row                */
   SS_COORD RowNew;                   /* New row being entered               */
   BOOL     fRowEdited;               /* The user entered edit mode within   */
                                      /* the current row                     */
   BOOL     fRowChangeMade;           /* The user made a change to the       */
                                      /* current row                         */
   BOOL     fRowNewBeyondLastDataRow; /* The new row the user is clicking    */
                                      /* into is beyond the row below        */
                                      /* the last row containing data.  The  */
                                      /* purpose of this flag is so the      */
                                      /* application can limit the user to   */
                                      /* entering data one line at a time    */
                                      /* and not skipping rows.              */
   BOOL     fAllCellsHaveData;        /* All cells within the current row    */
                                      /* have data                           */
   BOOL     fAddRow;                  /* The current row that was just edited*/
                                      /* should be treated as a new row.     */
   BOOL     fCancel;                  /* If the application chooses to       */
                                      /* prevent the user from entering      */
                                      /* the new row, this value should be   */
                                      /* set to TRUE upon exiting the        */
                                      /* SSM_LEAVEROW message.               */
   } SS_LEAVEROW, FAR *LPSS_LEAVEROW;

/********************
* Right Button Down
********************/

typedef struct ss_rbutton
   {
   SS_COORD Col;                /* Column that was clicked on                */
   SS_COORD Row;                /* Row that was clicked on                   */
   short    xPos;               /* Mouse x coordinate                        */
   short    yPos;               /* Mouse y coordinate                        */
   short    RButtonType;        /* Button Type. May be one of the following: */
                                /*    RBUTTONTYPE_DOWN                       */
                                /*    RBUTTONTYPE_UP                         */
                                /*    RBUTTONTYPE_DBLCLK                     */
   } SS_RBUTTON, FAR *LPSS_RBUTTON;

#define RBUTTONTYPE_DOWN   0
#define RBUTTONTYPE_UP     1
#define RBUTTONTYPE_DBLCLK 2

/***********************
* Cell Type definition
***********************/

typedef struct ss_celltype
   {
   char  Type;
   short ControlID;
   long  Style;
   union
      {
      struct
         {
         short ChrSet;
         short Len;
         char  ChrCase;
         } Edit;
      struct
         {
         long  Min;
         long  Max;
         BOOL  fSpinWrap;
         long  lSpinInc;
         } Integer;
      struct
         {
         short       Left;
         short       Right;
         double      Min;
         double      Max;
         BOOL        fSetFormat;
         FLOATFORMAT Format;
         } Float;
      struct
         {
         short          dItemCnt;
         TBGLOBALHANDLE hItems;
         } ComboBox;
      struct
         {
         BOOL       SetFormat;
         BOOL       SetRange;
         TIMEFORMAT Format;
         TIME       Min;
         TIME       Max;
         } Time;
      struct
         {
         BOOL       SetFormat;
         BOOL       SetRange;
         DATEFORMAT Format;
         DATE       Min;
         DATE       Max;
         } Date;
      struct
         {
         TBGLOBALHANDLE hMask;
         } Pic;
      struct
         {
         TBGLOBALHANDLE hPictName;
         HPALETTE       hPal;
#ifdef SS_VB
         HANDLE         hPic;
#endif
         BOOL           fDeleteHandle;
         } ViewPict;
      struct
         {
         GLOBALHANDLE  hText;
         GLOBALHANDLE  hPictName;
         GLOBALHANDLE  hPictDownName;
         short         nPictureType;
         short         nPictureDownType;
         char          ButtonType;
         char          ShadowSize;
         char          fDelHandle;
         char          fDelDownHandle;
         SUPERBTNCOLOR Color;
#ifdef SS_VB
         HANDLE         hPic;
         HANDLE         hPicDown;
#endif
         } Button;
      struct
         {
         GLOBALHANDLE   hText;
         TBGLOBALHANDLE hPictUpName;
         TBGLOBALHANDLE hPictDownName;
         TBGLOBALHANDLE hPictFocusUpName;
         TBGLOBALHANDLE hPictFocusDownName;
         TBGLOBALHANDLE hPictDisabledUpName;
         TBGLOBALHANDLE hPictDisabledDownName;
         BYTE           bPictUpType;
         BYTE           bPictDownType;
         BYTE           bPictFocusUpType;
         BYTE           bPictFocusDownType;
         BYTE           bPictDisabledUpType;
         BYTE           bPictDisabledDownType;
         char           fDelUpHandle;
         char           fDelDownHandle;
         char           fDelFocusUpHandle;
         char           fDelFocusDownHandle;
         char           fDelDisabledUpHandle;
         char           fDelDisabledDownHandle;
#ifdef SS_VB
         HANDLE         hPicUp;
         HANDLE         hPicDown;
         HANDLE         hPicFocusUp;
         HANDLE         hPicFocusDown;
         HANDLE         hPicDisabledUp;
         HANDLE         hPicDisabledDown;
#endif
         } CheckBox;

      char Dummy[34];
      /*
      char Dummy[45];
      */
      } Spec;
   } SS_CELLTYPE, FAR *LPSS_CELLTYPE;

/*************************************************
* Define character sets for the string data type
*************************************************/

#define SS_CHRSET_CHR          0
#define SS_CHRSET_ALPHA        1
#define SS_CHRSET_ALPHANUM     2
#define SS_CHRSET_NUM          3

/***************************************
* Define case for the string data type
***************************************/

#define SS_CASE_LCASE          0
#define SS_CASE_NOCASE         1
#define SS_CASE_UCASE          2

/**************************************************
* Define Plus and Minus for the numeric data type
**************************************************/

#define SS_NUM_PLUSMINUS       0
#define SS_NUM_PLUS            1

/*************
* Cell Types
*************/

#define SS_TYPE_DATE           1
#define SS_TYPE_EDIT           2
#define SS_TYPE_PIC            3
#define SS_TYPE_FLOAT          4
#define SS_TYPE_INTEGER        5
#define SS_TYPE_STATICTEXT     6
#define SS_TYPE_TIME           7
#define SS_TYPE_PICTURE        8
#define SS_TYPE_BUTTON         10
#define SS_TYPE_COMBOBOX       11
#define SS_TYPE_CHECKBOX       12
#define SS_TYPE_OWNERDRAW      13

/**************
* Text Styles
**************/

#define SS_TEXT_LEFT           0x001
#define SS_TEXT_RIGHT          0x002
#define SS_TEXT_CENTER         0x004
#define SS_TEXT_VCENTER        0x008
#define SS_TEXT_TOP            0x010
#define SS_TEXT_BOTTOM         0x020
#define SS_TEXT_WORDWRAP       0x040
#define SS_TEXT_SHADOW         0x080
#define SS_TEXT_PREFIX         0x100
#define SS_TEXT_SHADOWIN       0x200

#define SS_FONT_NONE           -1

#define SS_ROWHEIGHT_DEFAULT   0
#define SS_COLWIDTH_DEFAULT    0

#ifdef SS_USE16BITCOORDS
#define SS_MAXCOORD            32000
#else
#define SS_MAXCOORD            2000000000L
#endif

#define SS_FIRST               -1
#define SS_LAST                -2
#define SS_HEADER              -32767

#define SS_VISIBLE_PARTIAL     0
#define SS_VISIBLE_ALL         1

#define SS_SCROLLAREA_ALL      0
#define SS_SCROLLAREA_COLS     1
#define SS_SCROLLAREA_ROWS     2
#define SS_SCROLLAREA_CELLS    3

#define SS_ALLCOLS             -1
#define SS_ALLROWS             -1

/********************************************************************
* SpreadSheet boolean options to be used by SSSetBool and SSGetBool
********************************************************************/

#define SSB_PROTECT            0
#define SSB_RESTRICTROWS       1
#define SSB_RESTRICTCOLS       2
#define SSB_SHOWCOLHEADERS     3
#define SSB_SHOWROWHEADERS     4
#define SSB_ALLOWUSERRESIZE    5
#define SSB_ALLOWUSERSELBLOCK  6
#define SSB_AUTOCALC           7
#define SSB_REDRAW             8
#define SSB_GRIDLINES          9
#define SSB_HORZSCROLLBAR      10
#define SSB_VERTSCROLLBAR      11
#define SSB_ALLOWUSERFORMULAS  12
#define SSB_EDITMODEPERMANENT  13
#define SSB_AUTOSIZE           14
#define SSB_SCROLLBAREXTMODE   15
#define SSB_SCROLLBARSHOWMAX   16
#define SSB_SCROLLBARMAXALIGN  17
#define SSB_PROCESSTAB         18
#define SSB_ARROWSEXITEDITMODE 19
#define SSB_MOVEACTIVEONFOCUS  20
#define SSB_RETAINSELBLOCK     21
#define SSB_EDITMODEREPLACE    22
#define SSB_NOBORDER           23
#define SSB_NOBEEP             24
#define SSB_AUTOCLIPBOARD      25
#define SSB_ALLOWMULTIBLOCKS   26
#define SSB_ALLOWCELLOVERFLOW  27
#define SSB_VIRTUALMODE        28
#define SSB_ALLOWDRAGDROP      29
#define SSB_CHANGEMADE         30

#define SS_SHOW_TOPLEFT        0
#define SS_SHOW_TOPCENTER      1
#define SS_SHOW_TOPRIGHT       2
#define SS_SHOW_CENTERLEFT     3
#define SS_SHOW_CENTER         4
#define SS_SHOW_CENTERRIGHT    5
#define SS_SHOW_BOTTOMLEFT     6
#define SS_SHOW_BOTTOMCENTER   7
#define SS_SHOW_BOTTOMRIGHT    8
#define SS_SHOW_NEAREST        9

#define SS_ENTERACTION_NONE    0
#define SS_ENTERACTION_UP      1
#define SS_ENTERACTION_DOWN    2
#define SS_ENTERACTION_LEFT    3
#define SS_ENTERACTION_RIGHT   4
#define SS_ENTERACTION_NEXT    5
#define SS_ENTERACTION_PREV    6
#define SS_ENTERACTION_SAME    7

#define SS_UNITTYPE_NORMAL     0
#define SS_UNITTYPE_DEFAULT    1
#define SS_UNITTYPE_TWIPS      2

#define SS_OPMODE_NORMAL       0
#define SS_OPMODE_READONLY     1
#define SS_OPMODE_ROWMODE      2
#define SS_OPMODE_SINGLESEL    3
#define SS_OPMODE_MULTISEL     4

#define SS_BDM_ALWAYS          0x00
#define SS_BDM_CURRENTCELL     0x01
#define SS_BDM_CURRENTCOL      0x02
#define SS_BDM_CURRENTROW      0x04

#define SS_CB_DROPDOWN         0x02

#define SS_CBM_ADDSTRING       (WM_USER+3)
#define SS_CBM_DELETESTRING    (WM_USER+4)
#define SS_CBM_GETCOUNT        (WM_USER+6)
#define SS_CBM_GETCURSEL       (WM_USER+7)
#define SS_CBM_GETLBTEXT       (WM_USER+8)
#define SS_CBM_GETLBTEXTLEN    (WM_USER+9)
#define SS_CBM_INSERTSTRING    (WM_USER+10)
#define SS_CBM_RESETCONTENT    (WM_USER+11)
#define SS_CBM_SETCURSEL       (WM_USER+14)

#define SS_GRID_HORIZONTAL     0x01
#define SS_GRID_VERTICAL       0x02
#define SS_GRID_SOLID          0x04

#define SS_USERRESIZE_COL      0x01
#define SS_USERRESIZE_ROW      0x02

#define SS_RESIZE_DEFAULT      0
#define SS_RESIZE_ON           1
#define SS_RESIZE_OFF          2

/*****************************
* SpreadSheet Sort structure
*****************************/

typedef struct ss_sort
   {
   WORD     wSortBy;                   /* Sort by rows or columns. Use       */
                                       /* one of the following:              */
                                       /*    SS_SORT_ROW                     */
                                       /*    SS_SORT_COL                     */
   SS_COORD Key1Reference;             /* Column or row of first key.  Key1  */
                                       /* must be specified.                 */
   WORD     wKey1Order;                /* The sort order of Key1.  One of the*/
                                       /* following may be used:             */
                                       /*    SS_SORT_ASCENDING               */
                                       /*    SS_SORT_DESCENDING              */
   SS_COORD Key2Reference;             /* Column or row of second key.  Key2 */
                                       /* must be -1 if only 1 key is desired*/
   WORD     wKey2Order;                /* The sort order of Key2.  One of the*/
                                       /* following may be used:             */
                                       /*    SS_SORT_NONE - No Key2          */
                                       /*    SS_SORT_ASCENDING               */
                                       /*    SS_SORT_DESCENDING              */
   SS_COORD Key3Reference;             /* Column or row of second key.  Key3 */
                                       /* must be -1 if 2 keys are desired.  */
   WORD     wKey3Order;                /* The sort order of Key3.  One of the*/
                                       /* following may be used:             */
                                       /*    SS_SORT_NONE - No Key3          */
                                       /*    SS_SORT_ASCENDING               */
                                       /*    SS_SORT_DESCENDING              */
   } SS_SORT, FAR *LPSS_SORT;

#define SS_SORT_ROW            0
#define SS_SORT_COL            1

#define SS_SORT_NONE           0
#define SS_SORT_ASCENDING      1
#define SS_SORT_DESCENDING     2

typedef int (DLLENTRY *SS_SORTCOMPARE)(HPVOID, HPVOID, long);

/*****************
* Cursor options
*****************/

#define SS_CURSORTYPE_DEFAULT    0x01
#define SS_CURSORTYPE_COLRESIZE  0x02
#define SS_CURSORTYPE_ROWRESIZE  0x04
#define SS_CURSORTYPE_BUTTON     0x08
#define SS_CURSORTYPE_GRAYAREA   0x10
#define SS_CURSORTYPE_LOCKEDCELL 0x20
#define SS_CURSORTYPE_COLHEADER  0x40
#define SS_CURSORTYPE_ROWHEADER  0x80

#define SS_CURSOR_DEFAULT        0
#define SS_CURSOR_ARROW          (WORD)-1
#define SS_CURSOR_DEFCOLRESIZE   (WORD)-2
#define SS_CURSOR_DEFROWRESIZE   (WORD)-3

#define SS_SELMODE_CLEARSEL     1
#define SS_SELMODE_GETCURSEL    2
#define SS_SELMODE_GETSEL       3
#define SS_SELMODE_GETSELCOUNT  4
#define SS_SELMODE_GETSELITEMS  5
#define SS_SELMODE_SELITEMRANGE 6
#define SS_SELMODE_SETCURSEL    7
#define SS_SELMODE_SETSEL       8

/*********************************
* Define Border types and styles
*********************************/

#define SS_BORDERTYPE_NONE         0x00
#define SS_BORDERTYPE_LEFT         0x01
#define SS_BORDERTYPE_RIGHT        0x02
#define SS_BORDERTYPE_TOP          0x04
#define SS_BORDERTYPE_BOTTOM       0x08
#define SS_BORDERTYPE_OUTLINE      0x10

#define SS_BORDERSTYLE_NONE        0
#define SS_BORDERSTYLE_SOLID       1
#define SS_BORDERSTYLE_DASH        2
#define SS_BORDERSTYLE_DOT         3
#define SS_BORDERSTYLE_DASHDOT     4
#define SS_BORDERSTYLE_DASHDOTDOT  5
#define SS_BORDERSTYLE_BLANK       6

#define SS_SELBLOCK_COLS           0x01
#define SS_SELBLOCK_ROWS           0x02
#define SS_SELBLOCK_BLOCKS         0x04
#define SS_SELBLOCK_ALL            0x08

#define SS_HEADERDISPLAY_BLANK   0
#define SS_HEADERDISPLAY_NUMBERS 1
#define SS_HEADERDISPLAY_LETTERS 2

/***********************************
* SpreadSheet Owner draw structure
***********************************/

typedef struct ss_drawitemstruct
   {
   WORD     CtlID;      /* ID of spreadsheet control               */
   SS_COORD Col;        /* Column of cell to be drawn              */
   SS_COORD Row;        /* Row of cell to be drawn                 */
   HWND     hWnd;       /* Spreadsheet window handle               */
   HDC      hDC;        /* Spreadsheet device context to be used   */
   RECT     Rect;       /* Rectangle of cell.  All drawing must be */
                        /* be done within this rectangle.          */
   long     lStyle;     /* User defined style                      */
   LPSTR    lpData;     /* Cell data                               */
   } SS_DRAWITEMSTRUCT, FAR *LPSS_DRAWITEMSTRUCT;

/**************************
* Define a block of cells
**************************/

typedef struct ss_selblock
   {
   SS_CELLCOORD UL;
   SS_CELLCOORD LR;
   } SS_SELBLOCK, FAR *LPSS_SELBLOCK;

/******************************************
* Define the Virtual Query Data structure
******************************************/

typedef struct ss_vquerydata
   {
   SS_COORD Row;                       /* Virtual top row to begin          */
                                       /* assignment of data                */
   SS_COORD RowsNeeded;                /* The number of rows requested      */
   SS_COORD RowsLoaded;                /* The application should set the    */
                                       /* number of rows actually loaded    */
                                       /* before returning                  */
   WORD     wDirection;                /* Direction to retrieve data        */
                                       /*    SS_VDOWN                       */
                                       /*    SS_VUP                         */
                                       /*    SS_VTOP                        */
                                       /*    SS_VBOTTOM                     */
                                       /*    SS_VREFRESH                    */
   BOOL     fAtTop;                    /* The application should set this   */
                                       /* flag to TRUE if the top of the    */
                                       /* data has been reached             */
   BOOL     fAtBottom;                 /* The application should set this   */
                                       /* flag to TRUE if the bottom of the */
                                       /* data has been reached             */
   } SS_VQUERYDATA, FAR *LPSS_VQUERYDATA;

#define SS_VDOWN   1
#define SS_VUP     2
#define SS_VTOP    3
#define SS_VBOTTOM 4
#define SS_VREFRESH 5

#define SSV_SCROLLBARSHOWBUFFER 0x01
#define SSV_NOROWNUMBERS        0x02

#define SS_VSCROLL_NOHOMEEND  0x01
#define SS_VSCROLL_NOPAGEUPDN 0x02
#define SS_VSCROLL_NOUPDN     0x04

/********************************************************
* Define the structure used when the user uses dragdrop
********************************************************/

typedef struct ss_dragdrop
   {
   SS_SELBLOCK BlockPrev;              /* Previous block                     */
   SS_SELBLOCK BlockNew;               /* New block being draged to          */
   BOOL        fDataBeingOverwritten;  /* TRUE if data is being overwritten  */
   BOOL        fCopy;                  /* TRUE if the data is being copied.  */
                                       /* FALSE if the data is being moved.  */
   BOOL        fDataOnly;              /* The application should return TRUE */
                                       /* if only the data should be copied, */
                                       /* otherwise all formatting attributes*/
                                       /* will be transfered.                */
   BOOL        fCancel;                /* The application should return TRUE */
                                       /* if the drag/drop operation should  */
                                       /* be canceled.                       */
   } SS_DRAGDROP, FAR *LPSS_DRAGDROP;

/**********************
* Function Prototypes
**********************/

extern BOOL FAR PASCAL          SSShowCell(HWND hwnd, SS_COORD Col,
                                           SS_COORD Row, short Position);
extern BOOL FAR PASCAL          SSShowActiveCell(HWND hwnd, short Position);

extern BOOL FAR PASCAL          SSClear(HWND hWnd, SS_COORD Col, SS_COORD Row);
extern BOOL FAR PASCAL          SSClearRange(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row, SS_COORD Col2,
                                             SS_COORD Row2);
extern BOOL FAR PASCAL          SSGetVisible(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row, short Visible);

extern BOOL FAR PASCAL          SSSetFont(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row, HFONT hFont,
                                          BOOL fDeleteFont);
extern BOOL FAR PASCAL          SSSetFontRange(HWND hWnd, SS_COORD Col,
                                               SS_COORD Row, SS_COORD Col2,
                                               SS_COORD Row2, HFONT hFont,
                                               BOOL fDeleteFont);
extern HFONT FAR PASCAL         SSGetFont(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row);
extern BOOL FAR PASCAL          SSSetCellType(HWND hWnd, SS_COORD Col,
                                              SS_COORD Row,
                                              LPSS_CELLTYPE CellType);
extern BOOL FAR PASCAL          SSSetCellTypeRange(HWND hWnd, SS_COORD Col,
                                                  SS_COORD Row,
                                                   SS_COORD Col2,
                                                   SS_COORD Row2,
                                                   LPSS_CELLTYPE CellType);
extern BOOL FAR PASCAL          SSGetCellType(HWND hWnd, SS_COORD Col,
                                              SS_COORD Row,
                                              LPSS_CELLTYPE CellType);
extern BOOL FAR PASCAL          SSSetColor(HWND hWnd, SS_COORD Col,
                                           SS_COORD Row,
                                           COLORREF Background,
                                           COLORREF Foreground);
extern BOOL FAR PASCAL          SSSetColorRange(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row,
                                                SS_COORD Col2,
                                                SS_COORD Row2,
                                                COLORREF Background,
                                                COLORREF Foreground);
extern BOOL FAR PASCAL          SSGetColor(HWND hWnd, SS_COORD Col,
                                           SS_COORD Row,
                                           LPCOLORREF lpBackground,
                                           LPCOLORREF lpForeground);

extern BOOL FAR PASCAL          SSSetLock(HWND hWnd, SS_COORD Col,
                                         SS_COORD Row, BOOL Lock);
extern BOOL FAR PASCAL          SSSetLockRange(HWND hWnd, SS_COORD Col,
                                               SS_COORD Row, SS_COORD Col2,
                                               SS_COORD Row2, BOOL Lock);
extern BOOL FAR PASCAL          SSGetLock(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row);

extern BOOL FAR PASCAL          SSSetRowHeight(HWND hWnd, SS_COORD Row,
                                               double dfHeight);
#ifndef SS_USE16BITCOORDS
extern BOOL FAR PASCAL          SSGetRowHeight(HWND hWnd, SS_COORD Row,
                                               LPDOUBLE lpdfRowHeight);
#endif
extern BOOL FAR PASCAL          SSInsRow(HWND hWnd, SS_COORD Row);
extern BOOL FAR PASCAL          SSDelRow(HWND hWnd, SS_COORD Row);

extern BOOL FAR PASCAL          SSSetColWidth(HWND hWnd, SS_COORD Col,
                                              double dfWidth);
#ifndef SS_USE16BITCOORDS
extern BOOL FAR PASCAL          SSGetColWidth(HWND hWnd, SS_COORD Col,
                                              LPDOUBLE lpdfColWidth);
#endif
extern BOOL FAR PASCAL          SSDelCol(HWND hWnd, SS_COORD Col);
extern BOOL FAR PASCAL          SSInsCol(HWND hWnd, SS_COORD Col);

extern BOOL FAR PASCAL          SSSetData(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row, LPSTR lpData);
extern BOOL FAR PASCAL          SSSetDataRange(HWND hWnd, SS_COORD Col,
                                               SS_COORD Row, SS_COORD Col2,
                                               SS_COORD Row2, LPSTR lpData);
extern short FAR PASCAL         SSGetData(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row, LPSTR Data);
extern short FAR PASCAL         SSGetDataLen(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row);

extern BOOL FAR PASCAL          SSSetEditMode(HWND hWnd, BOOL fEditModeOn);
extern BOOL FAR PASCAL          SSGetEditMode(HWND hWnd);
extern BOOL FAR PASCAL          SSGetActiveCell(HWND hWnd, LPSS_COORD Col,
                                                LPSS_COORD Row);
extern BOOL FAR PASCAL          SSSetActiveCell(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row);
extern void FAR PASCAL          SSSetMaxCols(HWND hWnd, SS_COORD MaxCols);
extern SS_COORD FAR PASCAL      SSGetMaxCols(HWND hWnd);
extern void FAR PASCAL          SSSetMaxRows(HWND hWnd, SS_COORD MaxRows);
extern SS_COORD FAR PASCAL      SSGetMaxRows(HWND hWnd);

extern BOOL FAR PASCAL          SSGetSelectBlock(HWND hWnd,
                                                 LPSS_CELLCOORD CellUL,
                                                 LPSS_CELLCOORD CellLR);
extern BOOL FAR PASCAL          SSSetSelectBlock(HWND hWnd,
                                                 LPSS_CELLCOORD CellUL,
                                                 LPSS_CELLCOORD CellLR);
void FAR PASCAL                 SSGetShadowColor(HWND hWnd,
                                                 LPCOLORREF lpShadowColor,
                                                 LPCOLORREF lpShadowText,
                                                 LPCOLORREF lpShadowDark,
                                                 LPCOLORREF lpShadowLight);
void FAR PASCAL                 SSSetShadowColor(HWND hWnd,
                                                 COLORREF ShadowColor,
                                                 COLORREF ShadowText,
                                                 COLORREF ShadowDark,
                                                 COLORREF ShadowLight);
void FAR PASCAL                 SSGetGrayAreaColor(HWND hWnd,
                                                   LPCOLORREF lpBackground,
                                                   LPCOLORREF lpForeground);
void FAR PASCAL                 SSSetGrayAreaColor(HWND hWnd,
                                                   COLORREF Background,
                                                   COLORREF Foreground);

extern LPSS_CELLTYPE FAR PASCAL SSSetTypeEdit(HWND hWnd,
                                              LPSS_CELLTYPE CellType,
                                              long Style, short Len,
                                              short ChrSet, char ChrCase);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypePic(HWND hWnd, LPSS_CELLTYPE CellType,
                                             long Style, LPSTR Mask);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeStaticText(HWND hWnd,
                                                    LPSS_CELLTYPE CellType,
                                                    short TextStyle);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeDate(HWND hWnd,
                                              LPSS_CELLTYPE CellType,
                                              long Style,
                                              LPDATEFORMAT lpFormat,
                                              LPDATE lpMin, LPDATE lpMax);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeTime(HWND hWnd,
                                              LPSS_CELLTYPE CellType,
                                              long Style,
                                              LPTIMEFORMAT lpFormat,
                                              LPTIME lpMin, LPTIME lpMax);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeInteger(HWND hWnd,
                                                 LPSS_CELLTYPE CellType,
                                                 long Min, long Max);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeFloat(HWND hWnd,
                                               LPSS_CELLTYPE CellType,
                                               long Style, short Left,
                                               short Right, double Min,
                                               double Max);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeFloatExt(HWND hWnd,
                                                  LPSS_CELLTYPE CellType,
                                                  long Style, short Left,
                                                  short Right, double Min,
                                                  double Max,
                                                  LPFLOATFORMAT lpFormat);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypePicture(HWND hWnd,
                                                 LPSS_CELLTYPE CellType,
                                                 long Style, LPSTR PictName);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeButton(HWND hWnd,
                                                LPSS_CELLTYPE CellType,
                                                long Style, LPSTR Text,
                                                LPSTR Picture,
                                                short nPictureType,
                                                LPSTR PictureDown,
                                                short nPictureDownType,
                                                short ButtonType,
                                                short nShadowSize,
                                                LPSUPERBTNCOLOR lpColor);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeComboBox(HWND hWnd,
                                                  LPSS_CELLTYPE CellType,
                                                  long Style, LPSTR lpItems);

extern BOOL FAR PASCAL          SSSaveToFile(HWND hWnd, LPSTR lpFileName,
                                             BOOL fDataOnly);
extern GLOBALHANDLE FAR PASCAL  SSSaveToBuffer(HWND hWnd,
                                               long far *lpBufferLen,
                                               BOOL fDataOnly);
extern BOOL FAR PASCAL          SSLoadFromFile(HWND hWnd, LPSTR lpFileName);
extern BOOL FAR PASCAL          SSLoadFromBuffer(HWND hWnd,
                                                 GLOBALHANDLE hBuffer,
                                                 long lBufferLen);

extern BOOL FAR PASCAL          SSBuildDependencies(HWND hWnd);
extern BOOL FAR PASCAL          SSAddCustomFunction(HWND hWnd,
                                                    LPSTR FunctionName,
                                                    short ParameterCnt,
                                                    FARPROC lpfnFunctionProc);

extern BOOL FAR PASCAL          SSSetFormula(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row, LPSTR Formula,
                                             BOOL BuildDependencies);
extern BOOL FAR PASCAL          SSSetFormulaRange(HWND hWnd, SS_COORD Col,
                                                  SS_COORD Row, SS_COORD Col2,
                                                  SS_COORD Row2, LPSTR Formula,
                                                  BOOL BuildDependencies);
extern short FAR PASCAL         SSGetFormula(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row, LPSTR lpFormula);
extern short FAR PASCAL         SSGetFormulaLen(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row);
extern BOOL FAR PASCAL          SSSetBool(HWND hWnd, short nIndex,
                                          BOOL bNewVal);
extern BOOL FAR PASCAL          SSGetBool(HWND hWnd, short nIndex);
extern GLOBALHANDLE FAR PASCAL  SSClipOut(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row, SS_COORD Col2,
                                          SS_COORD Row2);
extern BOOL FAR PASCAL          SSClipIn(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                         SS_COORD Col2, SS_COORD Row2,
                                         HPSTR Buffer, long BufferLen);
extern GLOBALHANDLE FAR PASCAL  SSClipValueOut(HWND hWnd, SS_COORD Col,
                                               SS_COORD Row, SS_COORD Col2,
                                               SS_COORD Row2);
extern BOOL FAR PASCAL          SSClipValueIn(HWND hWnd, SS_COORD Col,
                                              SS_COORD Row, SS_COORD Col2,
                                              SS_COORD Row2, HPSTR Buffer,
                                              long BufferLen);

extern BOOL FAR PASCAL          SSSetSel(HWND hWnd, short SelStart,
                                         short SelEnd);
extern BOOL FAR PASCAL          SSGetSel(HWND hWnd, LPSHORT SelStart,
                                         LPSHORT SelEnd);
extern BOOL FAR PASCAL          SSReplaceSel(HWND hWnd, LPSTR lpText);
extern GLOBALHANDLE FAR PASCAL  SSGetSelText(HWND hWnd);
extern BOOL FAR PASCAL          SSGetCurrSelBlockPos(HWND hWnd,
                                                     LPSS_COORD CurrentCol,
                                                     LPSS_COORD CurrentRow);

extern BOOL DLLENTRY            SSValidateFormula(HWND hWnd,
                                                  LPSTR lpszFormula);
#ifndef SS_USE16BITCOORDS
extern void DLLENTRY            SSLogUnitsToColWidth(HWND hWnd, short dUnits,
                                                     LPDOUBLE lpdfColWidth);
extern void DLLENTRY            SSLogUnitsToRowHeight(HWND hWnd, SS_COORD Row,
                                                      short dUnits,
                                                      LPDOUBLE lpdfRowHeight);
#endif
extern short DLLENTRY           SSColWidthToLogUnits(HWND hWnd,
                                                     double dfColWidth);
extern short DLLENTRY           SSRowHeightToLogUnits(HWND hWnd, SS_COORD Row,
                                                      double dfRowHeight);
extern BOOL DLLENTRY            SSReCalc(HWND hWnd);
extern BOOL FAR PASCAL          SSDeSelectBlock(HWND hWnd);

extern BOOL FAR PASCAL          SSSetValue(HWND hWnd, SS_COORD Col,
                                           SS_COORD Row, LPSTR lpData);
extern BOOL FAR PASCAL          SSSetValueRange(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row, SS_COORD Col2,
                                                SS_COORD Row2, LPSTR lpData);
extern short FAR PASCAL         SSGetValue(HWND hWnd, SS_COORD Col,
                                           SS_COORD Row, LPSTR lpBuffer);
extern short FAR PASCAL         SSGetValueLen(HWND hWnd, SS_COORD Col,
                                              SS_COORD Row);
extern BOOL FAR PASCAL          SSClearData(HWND hWnd, SS_COORD Col,
                                            SS_COORD Row);
extern BOOL FAR PASCAL          SSClearDataRange(HWND hWnd, SS_COORD Col,
                                                 SS_COORD Row, SS_COORD Col2,
                                                 SS_COORD Row2);
extern void FAR PASCAL          SSGetLockColor(HWND hWnd,
                                               LPCOLORREF lpBackground,
                                               LPCOLORREF lpForeground);
extern void FAR PASCAL          SSSetLockColor(HWND hWnd, COLORREF Background,
                                               COLORREF Foreground);
extern BOOL FAR PASCAL          SSPrint(HWND hWnd, LPSTR lpszPrintJobName,
                                        LPSS_PRINTFORMAT lpPrintFormat,
                                        FARPROC lpfnPrintProc, LONG lAppData,
                                        FARPROC lpfnAbortProc);
extern WORD FAR PASCAL          SSSetEditEnterAction(HWND hWnd, WORD wAction);
extern WORD FAR PASCAL          SSGetEditEnterAction(HWND hWnd);

extern BOOL FAR PASCAL          SSSetFloat(HWND hWnd, SS_COORD Col,
                                           SS_COORD Row, double dfValue);
extern BOOL FAR PASCAL          SSSetFloatRange(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row, SS_COORD Col2,
                                                SS_COORD Row2, double dfValue);
extern BOOL FAR PASCAL          SSGetFloat(HWND hWnd, SS_COORD Col,
                                           SS_COORD Row, LPDOUBLE lpdfValue);

extern BOOL FAR PASCAL          SSSetInteger(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row, long lValue);
extern BOOL FAR PASCAL          SSSetIntegerRange(HWND hWnd, SS_COORD Col,
                                                  SS_COORD Row, SS_COORD Col2,
                                                  SS_COORD Row2, long lValue);
extern BOOL FAR PASCAL          SSGetInteger(HWND hWnd, SS_COORD Col,
                                             SS_COORD Row, LPLONG lplValue);

extern WORD FAR PASCAL          SSSetUnitType(HWND hWnd, WORD wUnitType);
extern WORD FAR PASCAL          SSGetUnitType(HWND hWnd);

extern BOOL FAR PASCAL          SSClipboardCopy(HWND hWnd);
extern BOOL FAR PASCAL          SSClipboardCut(HWND hWnd);
extern BOOL FAR PASCAL          SSClipboardPaste(HWND hWnd);

extern void FAR PASCAL          SSFloatFormat(HWND hWnd,
                                              LPSS_CELLTYPE lpCellType,
                                              double dfVal, LPSTR lpszText,
                                              BOOL fValue);

extern WORD FAR PASCAL          SSSetOperationMode(HWND hWnd, WORD wMode);
extern WORD FAR PASCAL          SSGetOperationMode(HWND hWnd);

extern LONG FAR PASCAL          SSSetUserData(HWND hWnd, LONG lUserData);
extern LONG FAR PASCAL          SSGetUserData(HWND hWnd);

extern HWND FAR PASCAL          SSSetOwner(HWND hWnd, HWND hWndOwner);
extern HWND FAR PASCAL          SSGetOwner(HWND hWnd);
extern WNDPROC FAR PASCAL       SSSetCallBack(HWND hWnd, WNDPROC lpfnCallBack);
extern WNDPROC FAR PASCAL       SSGetCallBack(HWND hWnd);
extern void FAR PASCAL          SSGetCellSendingMsg(HWND hWnd,
                                                    LPSS_COORD lpCol,
                                                    LPSS_COORD lpRow);

extern WORD FAR PASCAL          SSSetButtonDrawMode(HWND hWnd, WORD wMode);
extern WORD FAR PASCAL          SSGetButtonDrawMode(HWND hWnd);

extern BOOL FAR PASCAL          SSSetRowUserData(HWND hWnd, SS_COORD Row,
                                                 long lUserData);
extern BOOL FAR PASCAL          SSGetRowUserData(HWND hWnd, SS_COORD Row,
                                                 LPLONG lplUserData);

extern WORD FAR PASCAL          SSSetGridType(HWND hWnd, WORD wGridType);
extern WORD FAR PASCAL          SSGetGridType(HWND hWnd);
extern COLORREF FAR PASCAL      SSSetGridColor(HWND hWnd, COLORREF Color);
extern COLORREF FAR PASCAL      SSGetGridColor(HWND hWnd);

extern BOOL FAR PASCAL          SSGetDataCnt(HWND hWnd, LPSS_COORD lpColCnt,
                                             LPSS_COORD lpRowCnt);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypeIntegerExt(HWND hWnd,
                                                    LPSS_CELLTYPE CellType,
                                                    long lStyle, long lMin,
                                                    long lMax, BOOL fSpinWrap,
                                                    long lSpinInc);
extern BOOL FAR PASCAL          SSInsColRange(HWND hWnd, SS_COORD Col,
                                              SS_COORD Col2);
extern BOOL FAR PASCAL          SSDelColRange(HWND hWnd, SS_COORD Col,
                                              SS_COORD Col2);
extern BOOL FAR PASCAL          SSInsRowRange(HWND hWnd, SS_COORD Row,
                                              SS_COORD Row2);
extern BOOL FAR PASCAL          SSDelRowRange(HWND hWnd, SS_COORD Row,
                                              SS_COORD Row2);
extern BOOL FAR PASCAL          SSCopyRange(HWND hWnd, SS_COORD Col,
                                            SS_COORD Row, SS_COORD Col2,
                                            SS_COORD Row2, SS_COORD ColDest,
                                            SS_COORD RowDest);
extern BOOL FAR PASCAL          SSSwapRange(HWND hWnd, SS_COORD Col,
                                            SS_COORD Row, SS_COORD Col2,
                                            SS_COORD Row2, SS_COORD ColDest,
                                            SS_COORD RowDest);

extern BOOL FAR PASCAL          SSSort(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                       SS_COORD Col2, SS_COORD Row2,
                                       LPSS_SORT lpSort);

extern LPSS_CELLTYPE FAR PASCAL SSSetTypeCheckBox(HWND hWnd,
                                                  LPSS_CELLTYPE lpCellType,
                                                  long lStyle, LPSTR lpText,
                                                  LPSTR lpPictUp,
                                                  WORD wPictUpType,
                                                  LPSTR lpPictDown,
                                                  WORD wPictDownType,
                                                  LPSTR lpPictFocusUp,
                                                  WORD wPictFocusUpType,
                                                  LPSTR lpPictFocusDown,
                                                  WORD wPictFocusDownType,
                                                  LPSTR lpPictDisabledUp,
                                                  WORD wPictDisabledUpType,
                                                  LPSTR lpPictDisabledDown,
                                                  WORD wPictDisabledDownType);

extern BOOL FAR PASCAL          SSSetCursor(HWND hWnd, WORD wType,
                                            HCURSOR hCursor);
extern HCURSOR FAR PASCAL       SSGetCursor(HWND hWnd, WORD wType);
extern short FAR PASCAL         SSSetUserResizeCol(HWND hWnd, SS_COORD Col,
                                                   short dOption);
extern short FAR PASCAL         SSGetUserResizeCol(HWND hWnd, SS_COORD Col);
extern short FAR PASCAL         SSSetUserResizeRow(HWND hWnd, SS_COORD Row,
                                                   short dOption);
extern short FAR PASCAL         SSGetUserResizeRow(HWND hWnd, SS_COORD Row);

extern LPSS_CELLTYPE FAR PASCAL SSSetTypeOwnerDraw(HWND hWnd,
                                                   LPSS_CELLTYPE lpCellType,
                                                   long lStyle);
extern BOOL FAR PASCAL          SSGetBorder(HWND hWnd, SS_COORD Col,
                                            SS_COORD Row, LPWORD lpwBorderType,
                                            LPWORD lpwBorderStyle,
                                            LPCOLORREF lpColor);
extern BOOL FAR PASCAL          SSSetBorder(HWND hWnd, SS_COORD Col,
                                            SS_COORD Row, WORD wBorderType,
                                            WORD wBorderStyle, COLORREF Color);
extern BOOL FAR PASCAL          SSSetBorderRange(HWND hWnd, SS_COORD Col,
                                                 SS_COORD Row, SS_COORD Col2,
                                                 SS_COORD Row2,
                                                 WORD wBorderType,
                                                 WORD wBorderStyle,
                                                 COLORREF Color);
extern BOOL FAR PASCAL          SSGetMaxTextColWidth(HWND hWnd, SS_COORD Col,
                                                     LPDOUBLE lpdfColWidth);
extern BOOL FAR PASCAL          SSGetMaxTextRowHeight(HWND hWnd, SS_COORD Row,
                                                      LPDOUBLE lpdfRowHeight);
extern BOOL FAR PASCAL          SSGetMaxTextCellSize(HWND hWnd, SS_COORD Col,
                                                     SS_COORD Row,
                                                     LPDOUBLE lpdfWidth,
                                                     LPDOUBLE lpdfHeight);
extern BOOL FAR PASCAL          SSSetMultiSelBlocks(HWND hWnd,
                                                    LPSS_SELBLOCK lpSelBlockList,
                                                    short dSelBlockCnt);
extern BOOL FAR PASCAL          SSAddMultiSelBlocks(HWND hWnd,
                                                    LPSS_SELBLOCK lpSelBlock);
extern GLOBALHANDLE PASCAL      SSGetMultiSelBlocks(HWND hWnd,
                                                    LPSHORT lpdSelBlockCnt);
extern WORD FAR PASCAL          SSGetSelBlockOptions(HWND hWnd);
extern WORD FAR PASCAL          SSSetSelBlockOptions(HWND hWnd, WORD wOption);

extern BOOL FAR PASCAL          SSGetCellRect(HWND hWnd, SS_COORD Col,
                                              SS_COORD Row, LPRECT lpRect);
extern WORD FAR PASCAL          SSSetUserResize(HWND hWnd, WORD wUserResize);
extern WORD FAR PASCAL          SSGetUserResize(HWND hWnd);
extern LPSS_CELLTYPE FAR PASCAL SSSetTypePictureHandle(HWND hWnd,
                                                       LPSS_CELLTYPE CellType,
                                                       long Style,
                                                       HANDLE hPict,
                                                       HPALETTE hPal,
                                                       BOOL fDeleteHandle);

extern BOOL DLLENTRY            SSShowRow(HWND hWnd, SS_COORD Row, BOOL fShow);
extern BOOL DLLENTRY            SSShowCol(HWND hWnd, SS_COORD Col, BOOL fShow);

extern BOOL DLLENTRY            SSVScrollSetSpecial(HWND hWnd,
                                                    BOOL fUseSpecialVScroll,
                                                    WORD wOptions);
extern BOOL DLLENTRY            SSVScrollGetSpecial(HWND hWnd,
                                                    LPWORD lpwOptions);
extern LONG DLLENTRY            SSVSetStyle(HWND hWnd, LONG lStyle);
extern LONG DLLENTRY            SSVGetStyle(HWND hWnd);
extern void DLLENTRY            SSVSetBufferSize(HWND hWnd,
                                                 SS_COORD dBufferSize,
                                                 SS_COORD dOverlap);
extern void DLLENTRY            SSVGetBufferSize(HWND hWnd,
                                                 LPSS_COORD lpdBufferSize,
                                                 LPSS_COORD lpdOverlap);
extern SS_COORD DLLENTRY        SSVGetPhysBufferTop(HWND hWnd);
extern SS_COORD DLLENTRY        SSVGetPhysBufferSize(HWND hWnd);
extern SS_COORD DLLENTRY        SSVSetMax(HWND hWnd, SS_COORD Max);
extern SS_COORD DLLENTRY        SSVGetMax(HWND hWnd);
extern BOOL DLLENTRY            SSSetDefFloatFormat(HWND hWnd,
                                                  LPFLOATFORMAT lpFloatFormat);
extern BOOL DLLENTRY            SSGetDefFloatFormat(HWND hWnd,
                                                  LPFLOATFORMAT lpFloatFormat);

extern void DLLENTRY            SSGetClientRect(HWND hWnd, LPRECT lpRect);
extern BOOL DLLENTRY            SSSetButtonText(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row, LPSTR lpText);
extern BOOL DLLENTRY            SSGetButtonText(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row, LPSTR lpText);
extern BOOL FAR PASCAL          SSMoveRange(HWND hWnd, SS_COORD Col,
                                            SS_COORD Row, SS_COORD Col2,
                                            SS_COORD Row2, SS_COORD ColDest,
                                            SS_COORD RowDest);
extern BOOL DLLENTRY            SSIsCellInSelection(HWND hWnd, SS_COORD Col,
                                                    SS_COORD Row);
extern BOOL FAR PASCAL          SSSetColWidthInPixels(HWND hWnd, SS_COORD Col,
                                                      short dWidth);
extern BOOL FAR PASCAL          SSGetColWidthInPixels(HWND hWnd, SS_COORD Col,
                                                      LPSHORT lpdWidthPixels);
extern BOOL FAR PASCAL          SSSetRowHeightInPixels(HWND hWnd, SS_COORD Row,
                                                       short dHeight);
extern BOOL FAR PASCAL          SSGetRowHeightInPixels(HWND hWnd, SS_COORD Row,
                                                      LPSHORT lpdHeightPixels);
extern BOOL FAR PASCAL          SSSetColUserData(HWND hWnd, SS_COORD Col,
                                                 long lUserData);
extern BOOL FAR PASCAL          SSGetColUserData(HWND hWnd, SS_COORD Col,
                                                 LPLONG lplUserData);


extern long FAR PASCAL          SSSelModeSendMessage(HWND hWnd, WORD wMsg,
                                                     long lParam1,
                                                     long lParam2,
                                                     long lParam3);

extern long FAR PASCAL          SSComboBoxSendMessage(HWND hWnd, SS_COORD Col,
                                                      SS_COORD Row, WORD Msg,
                                                      UINT wParam,
                                                      LONG lParam);
extern BOOL FAR PASCAL          SSSetFreeze(HWND hWnd, SS_COORD ColsFrozen,
                                            SS_COORD RowsFrozen);
extern BOOL FAR PASCAL          SSGetFreeze(HWND hWnd, LPSS_COORD lpColsFrozen,
                                            LPSS_COORD lpRowsFrozen);
extern void FAR PASCAL          SSSetAutoSizeVisible(HWND hWnd,
                                                     SS_COORD VisibleCols,
                                                     SS_COORD VisibleRows);
extern void FAR PASCAL          SSGetAutoSizeVisible(HWND hWnd,
                                                     LPSS_COORD lpVisibleCols,
                                                     LPSS_COORD lpVisibleRows);

extern BOOL FAR PASCAL          SSGetCellFromPixel(HWND hWnd, LPSS_COORD lpCol,
                                                   LPSS_COORD lpRow,
                                                   short MouseX, short MouseY);

#ifdef SS_USE16BITCOORDS
extern LONG FAR PASCAL          SSGetTopLeftCell(HWND hWnd);
extern LONG FAR PASCAL          SSGetBottomRightCell(HWND hWnd);
#else
extern BOOL DLLENTRY            SSGetTopLeftCell(HWND hWnd, LPSS_COORD lpCol,
                                                 LPSS_COORD lpRow);
extern BOOL DLLENTRY            SSGetBottomRightCell(HWND hWnd,
                                                     LPSS_COORD lpCol,
                                                     LPSS_COORD lpRow);

extern void DLLENTRY            SSGetTopLeftPrev(HWND hWnd, LPSS_COORD lpCol,
                                                 LPSS_COORD lpRow);

extern void DLLENTRY            SSSetStartingNumbers(HWND hWnd,
                                                   SS_COORD StartingColNumber,
                                                   SS_COORD StartingRowNumber);
extern void DLLENTRY            SSGetStartingNumbers(HWND hWnd,
                                               LPSS_COORD lpStartingColNumber,
                                               LPSS_COORD lpStartingRowNumber);

extern WORD DLLENTRY            SSSetColHeaderDisplay(HWND hWnd,
                                                      WORD wDisplay);
extern WORD DLLENTRY            SSGetColHeaderDisplay(HWND hWnd);
extern WORD DLLENTRY            SSSetRowHeaderDisplay(HWND hWnd,
                                                      WORD wDisplay);
extern WORD DLLENTRY            SSGetRowHeaderDisplay(HWND hWnd);

extern BOOL DLLENTRY            SSVRefreshBuffer(HWND hWnd);

extern BOOL FAR PASCAL          SSGetFirstValidCell(HWND hWnd,
                                                    LPSS_COORD lpCol,
                                                    LPSS_COORD lpRow);
extern BOOL FAR PASCAL          SSGetLastValidCell(HWND hWnd, LPSS_COORD lpCol,
                                                   LPSS_COORD lpRow);

extern BOOL FAR PASCAL          SSSetColPageBreak(HWND hWnd, SS_COORD Col,
                                                  BOOL fPageBreak);
extern BOOL FAR PASCAL          SSGetColPageBreak(HWND hWnd, SS_COORD Col);
extern BOOL FAR PASCAL          SSSetRowPageBreak(HWND hWnd, SS_COORD Row,
                                                  BOOL fPageBreak);
extern BOOL FAR PASCAL          SSGetRowPageBreak(HWND hWnd, SS_COORD Row);

#endif

#endif   /* TB_NOSPREAD */

#endif   /* DRV_NOCLASS */

/*********
* Colors
*********/

#define RGBCOLOR_DEFAULT   (COLORREF)-1L
#define RGBCOLOR_WHITE     RGB(255, 255, 255)
#define RGBCOLOR_BLUE      RGB(0, 0, 255)
#define RGBCOLOR_RED       RGB(255, 0, 0)
#define RGBCOLOR_PINK      RGB(255, 0, 255)
#define RGBCOLOR_GREEN     RGB(0, 255, 0)
#define RGBCOLOR_CYAN      RGB(0, 255, 255)
#define RGBCOLOR_YELLOW    RGB(255, 255, 0)
#define RGBCOLOR_BLACK     RGB(0, 0, 0)
#define RGBCOLOR_DARKGRAY  RGB(128, 128, 128)
#define RGBCOLOR_DARKBLUE  RGB(0, 0, 128)
#define RGBCOLOR_DARKRED   RGB(128, 0, 0)
#define RGBCOLOR_DARKPINK  RGB(255, 0, 128)
#define RGBCOLOR_DARKGREEN RGB(0, 128, 0)
#define RGBCOLOR_DARKCYAN  RGB(0, 128, 128)
#define RGBCOLOR_BROWN     RGB(128, 128, 64)
#define RGBCOLOR_PALEGRAY  RGB(192, 192, 192)

/*****************
* Play functions
*****************/

extern HANDLE DLLENTRY Play(HWND hWndParent, LPSTR lpPlayString, BYTE Speed);
extern BOOL   DLLENTRY PlayChangeSpeed(HANDLE hPlayInst, BYTE Speed);
extern BOOL   DLLENTRY PlayStop(HANDLE hPlayInst);
extern BOOL   DLLENTRY PlayFreeze(HANDLE hPlayInst);
extern BOOL   DLLENTRY PlayResume(HANDLE hPlayInst);
extern BOOL   DLLENTRY WhiteNoise(WORD FreqLow, WORD FreqHigh, WORD Duration);
extern void FAR PASCAL ToneInit(void);
extern void FAR PASCAL ToneOn(WORD);
extern void FAR PASCAL ToneOff(void);
extern long FAR PASCAL TickCount(void);

extern void FAR PASCAL       TBDelay(DWORD ms);
extern BOOL DLLENTRY _export PrintWin(HWND, LPRECT, BOOL);
extern void DLLENTRY         DlgBoxCenter(HWND hWnd);

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#if defined( __cplusplus )
}
#endif

#endif
