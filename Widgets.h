#ifndef WIDGETS_H
#define WIDGETS_H
/***************************************************************************\
          (c) 1992-4, Simple Software, Inc., All rights reserved

  WIDGETS.H

  The WinWidgets' Exported Header

\***************************************************************************/
/***************************** Revision History ****************************/
/*1 WIDGETS.H 30-Jan-94,15:08:54,`HL' WinWidgets Exported Header           */
/*2 WIDGETS.H 30-Jan-94,18:06:40,`HL' Added OwnderDraw Style               */
/*     Added HXTbl Declarations                                            */
/*3 WIDGETS.H 04-Feb-94,17:25:58,`HL' Rearranged and removed HXTbl styles  */
/*4 WIDGETS.H 07-Feb-94,11:56:44,`JS' HX styles                            */
/*5 WIDGETS.H 14-Feb-94,15:44:18,`HL' FAR PASCAL -> SS_EXPORT              */
/*6 WIDGETS.H 16-Feb-94,13:26:00,`HL' Defined LPSHORT, ROW, COL, FLD, REC  */
/*     Integrated WIN32                                                    */
/*     WORD->UINT avoids constant warnings                                 */
/*7 WIDGETS.H 22-Feb-94,12:53:32,`HL' Added HGS_STANDARD                   */
/*8 WIDGETS.H 28-Feb-94,12:33:28,`HL' Changed HXTbl Styles                 */
/*9 WIDGETS.H 01-Mar-94,20:16:34,`HL' Defined HXCTL_STD                    */
/*10 WIDGETS.H 17-Mar-94,12:35:50,`HL' Updated State Flag definitions      */
/*11 WIDGETS.H 21-Mar-94,18:43:58,`HL' Added HATab control                 */
/*12 WIDGETS.H 06-Apr-94,18:28:42,`HL' Added HGSetCellText declaration     */
/*13 WIDGETS.H 26-Apr-94,11:14:34,`HL' Discontiguous rows, HTS_NOSTRETCH   */
/*     [MERGED_12_+_1.4,_base=1]                                           */
/*14 WIDGETS.H 01-May-94,04:20:12,`HL' Added HasCtlChanged macros          */
/*15 WIDGETS.H 11-May-94,12:56:22,`JS' HC_ATM added                        */
/*16 WIDGETS.H 19-May-94,11:58:56,`JS' Spin Messages, Styles, etc and      */
/*     new Edit messages GET/SETMAX/MIN                                    */
/*17 WIDGETS.H 26-May-94,20:56:56,`JS' HPS_READONLY, HPS_GRABBAR           */
/*18 WIDGETS.H 06-Jun-94,12:53:26,`HL' Moved XTable stuff to HXAPI.H       */
/*19 WIDGETS.H 06-Jun-94,12:56:02,`PKC' BOOL->int for HLDeleteItems        */
/*     HLSelectItems and HCDeleteItems                                     */
/*20 WIDGETS.H 09-Jun-94,17:17:06,`PKC' added HM_ISNULL standard message   */
/*21 WIDGETS.h 23-Jun-94,14:55:00,`HL' Spin control defs                   */
/*22 WIDGETS.H 27-Jun-94,15:01:56,`JS' HPM_SETGRABBAR defined              */
/*23 WIDGETS.H 12-Jul-94,13:50:52,`JS' Defined WW_VERSION_1 and _2         */
/*24 WIDGETS.H 14-Jul-94,15:24:26,`HL' HAM_GET/SETTABS for GET/SETTABCOUNT */
/*25 WIDGETS.H 26-Jul-94,23:44:32,`HL' Added HLM_GETCOLUMNWIDTH            */
/*26 WIDGETS.H 06-Aug-94,14:47:42,`JS' added HAM_GETCARDRECT and 2         */
/*     reserved messages                                                   */
/*27 WIDGETS.H 15-Aug-94,20:23:28,`HL' Added IsTabDlgMessage               */
/*28 WIDGETS.H 21-Aug-94,12:57:20,`HL' Remove obsolete and unsupported     */
/*29 WIDGETS.H 31-Aug-94,10:48:52,`JS' Added HEM_GRAB_FOCUS message        */
/*30 WIDGETS.H 08-Sep-94,22:14:12,`HL' Added WW_VERSION_DESIGN             */
/*     [MERGED_29_+_28.1,_base=28]                                         */
/*31 WIDGETS.H 09-Sep-94,10:46:14,`HL' Moved HERR def from Widgeint.h      */
/***************************** Revision History ****************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef WWAPI
#ifdef 	_WIN32
#define WWAPI 		__stdcall
#else//!_WIN32
#define WWAPI 		FAR PASCAL
#endif//_WIN32
#endif

void WWAPI WidgetsInit (void);

/***************************************************************************\
                             Common Messages
\***************************************************************************/
#define HM_FIRST            (WM_USER + 100)
#define HM_GETBKGNDBRUSH    (HM_FIRST + 0)
#define HM_SETBKGNDBRUSH    (HM_FIRST + 1)
#define HM_GETFORMAT        (HM_FIRST + 2)
#define HM_SETFORMAT        (HM_FIRST + 3)
#define HM_GETHILITEBRUSH   (HM_FIRST + 4)
#define HM_SETHILITEBRUSH   (HM_FIRST + 5)
#define HM_GETTEXTCOLOR     (HM_FIRST + 6)
#define HM_SETTEXTCOLOR     (HM_FIRST + 7)
#define HM_GETDATACLASS     (HM_FIRST + 8)
#define HM_GETDATASIZE      (HM_FIRST + 9)
#define HM_GETDATATYPE      (HM_FIRST + 10)
#define HM_GETDATALINK      (HM_FIRST + 11)
#define HM_SETDATALINK      (HM_FIRST + 12)
#define HM_GETFONT          (WM_GETFONT)
#define HM_SETFONT          (WM_SETFONT)
#define HM_HASCHANGED       (HM_FIRST + 13)
#define HM_SETCHANGED       (HM_FIRST + 14)
#define HM_GETSTATE         (HM_FIRST + 17)
#define HM_SETSTATE         (HM_FIRST + 18)
#define HM_GETTEXT          (WM_GETTEXT)
#define HM_SETTEXT          (WM_SETTEXT)
#define HM_BEQUIET          (HM_FIRST + 19)
#define HM_ISQUIET          (HM_FIRST + 20)
#define HM_SETFOCUSCHILD    (HM_FIRST + 21)
#define HM_KILLFOCUSCHILD   (HM_FIRST + 22)
#define HM_RESERVED1				(HM_FIRST + 23)
#define HM_RESERVED2				(HM_FIRST + 24)
#define HM_GETNOTIFY        (HM_FIRST + 25)
#define HM_SETNOTIFY        (HM_FIRST + 26)
#define HM_GETSTYLE	        (HM_FIRST + 27)
#define HM_SETSTYLE	        (HM_FIRST + 28)
#define HM_RESERVED3       	(HM_FIRST + 29)
#define HM_RESERVED4       	(HM_FIRST + 30)
#define HM_CANCEL						(HM_FIRST + 31)
#define HM_ISNULL						(HM_FIRST + 32)

/***************************************************************************\

                           Miscellaneous Macros

\***************************************************************************/
#define DLP(p)            ((DWORD)(LPSTR)(p))
#define SM                SendMessage
#define PM                PostMessage
#define SIM               SendDlgItemMessage
#define ISKEYPRESSED(vk)  (HIBYTE (GetKeyState (vk)))

#define	HasCtlChanged(h,id) 			(BOOL)SIM(h,id,HM_HASCHANGED,0,0L)
#define	CtlHasChanged(h,id) 			(BOOL)SIM(h,id,HM_SETCHANGED,TRUE,0L)
#define	CtlHasNotChanged(h,id) 		(BOOL)SIM(h,id,HM_SETCHANGED,FALSE,0L)

//  Sound resource type
#define HWAVERES          400
#define RT_HWAVERES       MAKEINTRESOURCE(HWAVERES)  

//	Error return type
typedef int HERR;

/***************************************************************************\

											Version Definitions
											
\***************************************************************************/  

#define WW_VERSION_1 3  //Major revision number
#define WW_VERSION_2 0  //Minor revision number 
#define WW_VERSION_DESIGN 1 //Flag for design-only builds

/***************************************************************************\
  Type definitions used by the DataEngine and other Widget controls.
\***************************************************************************/

typedef unsigned char       BYTE,   FAR *LPBYTE;
typedef short int           SHORT,  FAR *LPSHORT;
typedef unsigned short int  WORD,   FAR *LPWORD;
typedef int                 INT,    FAR *LPINT;
typedef unsigned int        UINT,   FAR *LPUINT;
typedef long int                    FAR *LPLONG;
typedef unsigned long int   DWORD,  FAR *LPDWORD;
typedef float               FLOAT,  FAR *LPFLOAT;
typedef double              DOUBLE, FAR *LPDOUBLE;

/***************************************************************************\

                               Data Classes

\***************************************************************************/
#define HC_CHAR             'h'
#define HC_NUMBER           'n'
#define HC_CURRENCY         'c'
#define HC_DATE             'd'
#define HC_TIME             't'
#define HC_DATETIME         'a'
#define HC_CLOCK            'k'
#define HC_MASK             'm'
#define HC_BOOL             'b'
#define HC_STRING           's' 
#define HC_ATM							'z'

/***************************************************************************\

                               Data Types

\***************************************************************************/
// Types for HC_CHAR
#define HT_CHAR             'h'

// Types for HC_BOOL, HC_NUMBER and HC_CURRENCY
#define HT_BYTE             'b'
#define HT_SHORT            's'
#define HT_WORD             'w'
#define HT_INT              'i'
#define HT_UINT             'u'
#define HT_LONG             'l'
#define HT_DWORD            'W'

// Types for HC_NUMBER and HC_CURRENCY
#define HT_FLOAT            'f'
#define HT_DOUBLE           'd'
#define HT_BIGMONEY         'M'

// Types for HC_MASK and HC_STRING
#define HT_STRING           's'

// Types for HC_DATETIME and HC_CLOCK
#define HT_DATETIME         'D'
#define HT_ODBCTIMESTAMP    's'
#define HT_SQLDATETIME4     'q'
#define HT_SQLDATETIME      'Q'
#define HT_TM               'm'
#define HT_TIME_T           'T'
#define HT_DOUBLEDATE       'u'

// Types for HC_DATE
#define HT_ODBCDATE         'd'
#define HT_DOSDATE          'a'
#define HT_DOSFILEDATE      'A'
#define HT_LONGDATE         'l'

// Types for HC_TIME
#define HT_ODBCTIME         't'
#define HT_DOSTIME          'i'
#define HT_DOSFILETIME      'I'


/***************************************************************************\

                         DATE and TIME DATA TYPES
                         ========================

  The HT_DATETIME data type is used internally by the Data Engine for all
  date and time conversions.

\***************************************************************************/
//  HT_DATETIME:
typedef struct tagDATETIME
  {
  short   year;     //  (i.e. 1970)
  BYTE    month;    //  1-12
  BYTE    day;      //  1-31
  BYTE    hour;     //  0-23
  BYTE    minute;   //  0-59
  BYTE    second;   //  0-59
  WORD    msec;     //  0-999
  }
  DATETIME,
  FAR *LPDATETIME;

/***************************************************************************\

  HT_ODBCDATE, HT_ODBCTIME and HT_ODBCTIMESTAMP data types

  USE:      SQL databases
  DEFINED:  Microsoft's ODBC (Open Database Connectivity) API

\***************************************************************************/

//  HT_ODBCDATE data type
typedef struct tagODBCDATE
  {
  short   year;
  WORD    month;
  WORD    day;
  }
  ODBCDATE,
  FAR *LPODBCDATE;

//  HT_ODBCTIME:
typedef struct tagODBCTIME
  {
  WORD    hour;
  WORD    minute;
  WORD    second;
  }
  ODBCTIME,
  FAR *LPODBCTIME;

//  HT_ODBCTIMESTAMP:
typedef struct tagODBCTIMESTAMP
  {
  short   year;
  WORD    month;
  WORD    day;
  WORD    hour;
  WORD    minute;
  WORD    second;
  DWORD   fraction;
  }
  ODBCTIMESTAMP,
  FAR *LPODBCTIMESTAMP;


/***************************************************************************\

  HT_SQLDATE, HT_SQLTIME and HT_SQLTIMESTAMP data types

  USE:      SQL databases
  DEFINED:  Microsoft's DBLib interface to SQL Server

\***************************************************************************/

//  HT_SQLDATETIME4
typedef struct tagSQLDATETIME4
  {
  WORD   days;    //  Julian days since 1/1/1900 up to June 6 2079
  WORD   time;    //  minutes since midnight
  }
  SQLDATETIME4,
  FAR *LPSQLDATETIME4;

//  HT_SQLDATETIME
typedef struct tagSQLDATETIME
  {               //  Some countries (UK) adopted Gregorian cal 9/1752
  long    days;   //  Julian days since 1/1/1900 from 1/1/1753-12/31/9999
  long    time;   //  milliseconds since midnight
  }
  SQLDATETIME,
  FAR *LPSQLDATETIME;


/***************************************************************************\

  HT_TM and HT_TIME_T data types

  USE:      standard C time functions
  DEFINED:  time.h

\***************************************************************************/
#include <time.h>

//  HT_TIME_T data type
typedef time_t TIME_T,        // the number of seconds since Jan 1, 1970
               FAR *LPTIME_T; // at 00:00:00

//  HT_TM data type
/*
typedef struct tagTM
  {
  short   tm_sec;     //  seconds after the minute (0-61?)
  short   tm_min;     //  minutes after the hour (0-59)
  short   tm_hour;    //  hours since midnight (0-23)
  short   tm_mday;    //  day of the month (1-31)
  short   tm_mon;     //  months since January (0-11)
  short   tm_year;    //  years since 1900
  short   tm_wday;    //  days since Sunday (0-6)
  short   tm_yday;    //  days since January 1 (0-365)
  short   tm_isdst;   //  Daylight Saving Time flag, >0 if DST, or 0
  }
*/
typedef struct tm TM,
                  FAR * LPTM;


/***************************************************************************\

  HT_DOSDATE and HT_DOSTIME data types

  USE:      _dos_getdate/_dossetdate
            _dos_gettime/_dos_settime

  DEFINED:  dos.h

\***************************************************************************/

//  HT_DOSDATE data type
typedef struct tagDOSDATE
  {
  BYTE    day;        // 1-31
  BYTE    month;      // 1-12
  WORD    year;       // 1980-2099
  BYTE    dayofweek;  // 0-6, 0=Sunday
  }
  DOSDATE,
  FAR * LPDOSDATE;

//  HT_DOSTIME data type
typedef struct tagDOSTIME
  {
  BYTE    hour;       // 0-23
  BYTE    minute;     // 0-59
  BYTE    second;     // 0-59
  BYTE    hsecond;    // 0-99
  }
  DOSTIME,
  FAR * LPDOSTIME;


/***************************************************************************\

  HT_DOSFILEDATE and HT_DOSFILETIME data types

  USE:      _dos_findfirst/_dos_findnext

\***************************************************************************/

//  HT_DOSFILEDATE data type
typedef struct tagDOSFILEDATE
  {
  WORD    Year  : 7;  //  years since 1980
  WORD    Month : 4;  //  month (1-12)
  WORD    Day   : 5;  //  day (1-31)
  }
  DOSFILEDATE,
  FAR * LPDOSFILEDATE;

//  HT_DOSFILETIME data type
typedef struct tagDOSFILETIME
  {
  WORD    Hour  : 5;  //  hour (0-23)
  WORD    Min   : 6;  //  minutes (0-59)
  WORD    Sec   : 5;  //  second divided by 2 (10 here means 20)
  }
  DOSFILETIME,
  FAR * LPDOSFILETIME;


/***************************************************************************\

  HT_DOUBLEDATE and HT_LONGDATE data types

  USE:      spreadsheets, misc.

\***************************************************************************/

//  HT_DOUBLEDATE data type
typedef double  DOUBLEDATE,         //  integral and partial days since
                FAR * LPDOUBLEDATE; //  December 30, 1899, midnight **

/***************************************************************************\
  ** Excel, and perhaps other spreadsheets, claim that their date represents
  the number of days from December 31, 1899 - making January 1, 1900
  day 1.  However, they neglect the fact that 1900 was NOT a leap year!
  There was no 2/29/1900, which is a valid date in Excel.  In addition,
  SQL uses the number of days SINCE January 1, 1900 - making 1/1/1900
  day 0.  The net effect is that the integral portion of a DOUBLEDATE
  will be 2 greater than the days element of the SQL types.
\***************************************************************************/

//  HT_LONGDATE data type
typedef long  LONGDATE,           //  year * 10000 + month * 100 + day
              FAR * LPLONGDATE;   //  e.g. 12/30/1968 -> 19681230



/***************************************************************************\

                               MONEY TYPES
                         ========================

  In most cases, money values are identical to other numeric
  types in terms of storage and formatting options.  However, some
  databases use a 64-bit signed integer to represent large money values
  without the rounding errors of doubles.  The actual money value is
  the integer value divided by 10,000.  Special routines are provided
  for converting this data type to and from a character string.

\***************************************************************************/

//  HT_BIGMONEY
typedef struct
  {
  DWORD  bm[2];   // bm[0] is the more significant DWORD
  }
  BIGMONEY,
  FAR * LPBIGMONEY;

typedef struct
  {
  WORD bmw[4];    // bmw[3] is the most significant WORD
  }
  BMWORDS,
  FAR *LPBMWORDS;


/***************************************************************************\

                           Miscellaneous Types

\***************************************************************************/

typedef const char FAR *LPCSTR;

typedef BOOL    (WWAPI* VALIDATEPROC)(HWND, UINT, LPVOID);  

#define	HGB_RETRIEVE    0
#define	HGB_UPDATE			1
#define	HGB_INSERT  	  2
#define	HGB_DELETE			3
#define	HGB_CHANGE			4
#define	HGB_VALIDATE	  5
#define	HGB_ADD	  			6

#define HG_ADDNEW				((BOOL)-1)

typedef BOOL    (WWAPI* BUFFERPROC)(HWND, UINT, LONG, LPVOID);  

#define HP_BITMAP  	0x0001
#define HP_ICON			0x0002
#define HP_DIB			0x0003


/***************************************************************************\

                           International Functions

\***************************************************************************/
void WWAPI GetIntlSettings (void);

/***************************************************************************\

                              Utility Functions

\***************************************************************************/
void WWAPI RCtoS (HWND hwnd, LPRECT lpRect);
void WWAPI RStoC (HWND hwnd, LPRECT lpRect);
WNDPROC WWAPI SubclassWW (HWND hwnd, WNDPROC wpNew);
void WWAPI CenterWindow (HWND hwnd, int top);
int  WWAPI WindowWidth (HWND hwnd);
int  WWAPI WindowHeight (HWND hwnd);
BOOL WWAPI SetOverwriteMode (BOOL bSetTo);
BOOL WWAPI GetOverwriteMode (void);
BOOL WWAPI SetDecimalSep (char cNewSep);
BOOL WWAPI SetThousandsSep (char cNewSep);
BOOL WWAPI SetDateSep (char cNewSep);
BOOL WWAPI SetTimeSep (char cNewSep);
BOOL WWAPI SetDefDateFormat (LPCSTR lpNewFmt);
BOOL WWAPI SetDefTimeFormat (LPCSTR lpNewFmt);
BOOL WWAPI IsTabDlgMessage (HWND hTabDlg, HWND hPage, WORD idTabCtl, LPMSG msg);

#define WINVER30  0x0300
#define WINVER31  0x030a
WORD WWAPI GetWindowsVersion (void);

//  Returns the handle of the first child control with WS_TABSTOP set
HWND WWAPI CreateDialogInWin
(
HANDLE      hInstance,      //  Instance containing the dialog resource
LPCSTR      lpTemplateName, //  Name of the dialog resource
HWND        hParent,        //  Window in which to create the child controls
LPRECT      lpDlgRect,      //  If not NULL, rtns dlg size in pixels
HFONT FAR * lphfont         //  If not NULL, rtns dlg font, user must delete
);

BOOL WWAPI ClearDialog (HWND hwnd);

//  Additional Notification codes
#define HN_RETURN      0x0101
#define HN_ESCAPE      0x0102
#define HN_LEFT        0x0103
#define HN_RIGHT       0x0104
#define HN_UP          0x0105
#define HN_DOWN        0x0106
#define HN_TAB         0x0107
#define HN_BACKTAB     0x0108
#define HN_ALTARROW    0x0109
#define HN_NEXT        0x010a
#define HN_PRIOR       0x010b
#define HN_DBLCLK      0x010c 
#define HN_HELP   		 0x010d
#define HN_F1PRESSED   HN_HELP

/***************************************************************************\
  The HBUTT control
\***************************************************************************/

// Messages 
#define HBM_FIRST							(WM_USER + 30)
#define HBM_GETBKGNDBRUSH     (HM_GETBKGNDBRUSH)  // Returns HiliteBrush.
#define HBM_SETBKGNDBRUSH     (HM_SETBKGNDBRUSH)  // Sets HiliteBrush.
#define HBM_GETPALIGN         (HBM_FIRST + 0)
#define HBM_GETBALIGN         HBM_GETPALIGN
#define HBM_GETPIC            (HBM_FIRST + 1)
#define HBM_GETBITMAP         HBM_GETPIC
#define HBM_GETCOUNT          (HBM_FIRST + 2)
#define HBM_GETFONT       		(HM_GETFONT)
#define HBM_GETMASKCOLOR      (HBM_FIRST + 3)
#define HBM_GETRALIGN         (HBM_FIRST + 4)
#define HBM_GETSOUND          (HBM_FIRST + 5)
#define HBM_GETSTATE      		(BM_GETCHECK)
#define HBM_GETTALIGN         (HBM_FIRST + 6)
#define HBM_GETTEXT       		(HM_GETTEXT)
#define HBM_GETTEXTCOLOR  		(HM_GETTEXTCOLOR)
#define HBM_GETTYPE           (HBM_FIRST + 9)

#define HBM_CLICK             (WM_USER + 99)
#define HBM_ISPRESSED     		(BM_GETSTATE)
#define HBM_PRESS         		(BM_SETSTATE)

#define HBM_SETPALIGN         (HBM_FIRST + 20)
#define HBM_SETBALIGN     		HBM_SETPALIGN				// For compatibility
#define HBM_SETPIC            (HBM_FIRST + 21)
#define HBM_SETBITMAP     		HBM_SETPIC        	// For compatibility
#define HBM_SETFONT       		(HM_SETFONT)
#define HBM_SETMASKCOLOR      (HBM_FIRST + 22)
#define HBM_SETRALIGN         (HBM_FIRST + 23)
#define HBM_SETSOUND          (HBM_FIRST + 24)
#define HBM_SETSTATE      		(BM_SETCHECK)
#define HBM_SETTALIGN         (HBM_FIRST + 25)
#define HBM_SETTEXT       		(HM_SETTEXT)
#define HBM_SETTEXTCOLOR  		(HM_SETTEXTCOLOR)
#define HBM_SETTYPE       		(BM_SETSTYLE)
#define HBM_GETDATA           (HBM_FIRST + 26)
#define HBM_SETDATA           (HBM_FIRST + 27)
#define HBM_GETDATALINK       (HBM_FIRST + 28)
#define HBM_SETDATALINK       (HBM_FIRST + 29)
#define HBM_BEQUIET       		(HM_BEQUIET)
#define HBM_ISQUIET       		(HM_ISQUIET)
#define HBM_GETDATASIZE       (HBM_FIRST + 30)
#define HBM_GETSTATECOUNT     (HBM_FIRST + 31)
#define HBM_ISLASTINGROUP     (HBM_FIRST + 32)
#define HBM_SETCOUNT 			    (HBM_FIRST + 33)
#define HBM_GETPALETTE		    (HBM_FIRST + 34)

// Styles
#define HBS_PUSHBUTTON          0x00L
#define HBS_DEFPUSHBUTTON       0x01L
#define HBS_CHECKBOX            0x02L
#define HBS_RADIOBUTTON         0x03L
#define HBS_3STATE              0x04L
#define HBS_GROUPPUSH           0x06L

#define HBS_TRANSPARENT         0x0010L
#define HBS_LEFTTEXT            0x0020L
#define HBS_LJUST               0x0020L
#define HBS_RJUST               0x0040L
#define HBS_NOFOCUS             0x0080L
#define HBS_OWNERDRAW           0x0100L
#define HBS_DOWNPICS            0x0400L
#define HBS_AUTOADVANCE         0x0800L
#define HBS_NOBUTTON            0x1000L
#define HBS_TEXTINDENT          0x2000L
#define HBS_SQUARED             0x4000L
#define HBS_ASYNC               0x8000L

#define HBS_ICONS               0L						//  For Compatibility - was 0x0100L
#define HBS_DOWNMAPS            HBS_DOWNPICS  //	For Compatibility

// Notification Codes
#define HBN_CLICKED        BN_CLICKED
#define HBN_DOUBLECLICKED  BN_DOUBLECLICKED

// Error Codes
#define HBERR              -1

/***************************************************************************\
  The HEdit control
\***************************************************************************/

//  Messages
#define HEM_FIRST             (WM_USER + 40)
#define HEM_GETBKGNDBRUSH     (HM_GETBKGNDBRUSH)  // Returns HiliteBrush.
#define HEM_SETBKGNDBRUSH     (HM_SETBKGNDBRUSH)  // Sets HiliteBrush.
#define HEM_GETDATA           (HEM_FIRST + 0)     // Retrieves Data.
#define HEM_SETDATA           (HEM_FIRST + 1)     // Sets Data.
#define HEM_GETDATACLASS      (HM_GETDATACLASS)   // Returns DataType
#define HEM_GETDATALINK       (HM_GETDATALINK)    // Returns DataLink.
#define HEM_SETDATALINK       (HM_SETDATALINK)    // Sets DataLink.
#define HEM_GETDATASIZE       (HM_GETDATASIZE)    // Returns DataSize
#define HEM_GETDATATYPE       (HM_GETDATATYPE)    // Returns DataType
#define HEM_GETFONT           (HM_GETFONT)        // Returns the Font handle.
#define HEM_SETFONT           (HM_SETFONT)        // Sets the Font handle.
#define HEM_GETFORMAT         (HM_GETFORMAT)      // Retrieves Format string.
#define HEM_SETFORMAT         (HM_SETFORMAT)      // Sets Format string.
#define HEM_GETFORMATRECT     (HEM_FIRST + 2)     // Retrieves FormatRect
#define HEM_SETFORMATRECT     (HEM_FIRST + 3)     // Sets FormatRect.
#define HEM_GETHILITEBRUSH    (HM_GETHILITEBRUSH) // Returns HiliteBrush.
#define HEM_SETHILITEBRUSH    (HM_SETHILITEBRUSH) // Sets HiliteBrush.
#define HEM_GETMAXTEXTLEN     (HEM_FIRST + 4)     // Returns TextMaxLen.
#define HEM_SETMAXTEXTLEN     (EM_LIMITTEXT)      // Sets TextMaxLen.
#define HEM_GETOVERWRITEMODE  (HEM_FIRST + 5)     // Returns overwrite/insert status.
#define HEM_SETOVERWRITEMODE  (HEM_FIRST + 6)     // Toggles overwrite/insert modes.
#define HEM_GETPASSWORDCHAR   (HEM_FIRST + 7)     // Returns PasswordChar.
#define HEM_SETPASSWORDCHAR   (HEM_FIRST + 8)     // Sets PasswordChar.
#define HEM_GETSCROLLPOS      (HEM_FIRST + 9)     // Retrieves ScrollPos.
#define HEM_SETSCROLLPOS      (HEM_FIRST + 10)    // Sets ScrollPos.
#define HEM_GETSEL            (EM_GETSEL)         // Retrieves range of selected text.
#define HEM_SETSEL            (EM_SETSEL)         // Sets range of selected text.
#define HEM_GETSTATE          (HM_GETSTATE)       // Returns set State flags.
#define HEM_SETSTATE          (HM_SETSTATE)       // Sets one of State flags.
#define HEM_GETTEXT           (HM_GETTEXT)        // Retrieves formatted Text.
#define HEM_SETTEXT           (HM_SETTEXT)        // Sets Data for String Class.
#define HEM_GETTEXTCOLOR      (HM_GETTEXTCOLOR)   // Returns TextColor.
#define HEM_SETTEXTCOLOR      (HM_SETTEXTCOLOR)   // Sets TextColor.
#define HEM_GETTEXTLEN        (WM_GETTEXTLENGTH)  // Returns TextLen.
#define HEM_UPDATE            (HEM_FIRST + 13)    // Updates Text using Data or vice-versa.
#define HEM_HASCHANGED        (HM_HASCHANGED)     // Returns the status of the Changed flag
#define HEM_SETCHANGED        (HM_SETCHANGED)     // Sets the status of the Changed flag
#define HEM_GETVALIDATE       (HEM_FIRST + 14)    // Returns a pointer to the validation function
#define HEM_SETVALIDATE       (HEM_FIRST + 15)    // Sets the validation function
#define HEM_VALIDATE          (HEM_FIRST + 16)    // Calls the validation function
#define HEM_BEQUIET           (HM_BEQUIET)        // Disables Notification
#define HEM_ISQUIET           (HM_ISQUIET)        // Tests the state of quiet flag
#define HEM_GETSELTEXT        (HEM_FIRST + 17)
#define HEM_REPLACESEL        (HEM_FIRST + 18)
#define HEM_GETMAX						(HEM_FIRST + 19)
#define HEM_SETMAX						(HEM_FIRST + 20)
#define HEM_GETMIN						(HEM_FIRST + 21)
#define HEM_SETMIN						(HEM_FIRST + 22)
#define HEM_GRAB_FOCUS				(HEM_FIRST + 23)		//MJR
#define HEM_LAST              (HEM_FIRST + 23)

// Macros
#define HESetData(h,p)      SendMessage(h,HEM_SETDATA,    0,(DWORD)(LPSTR)p)
#define HEGetData(h,m,p)    SendMessage(h,HEM_GETDATA,    m,(DWORD)(LPSTR)p)
#define HESetDataLink(h,p,b) SendMessage(h,HEM_SETDATALINK,b,(DWORD)(LPSTR)p)
#define HEIsChanged(h)      SendMessage(h,HEM_HASCHANGED, 0,0L)

// State flags
#define HEF_DISPLAYMODE 0x0002  //  Control is in Display mode
#define HEF_RIGHT       0x0004  //  Text is right justified in current mode
#define HEF_CENTER      0x0008  //  Text is centered in the current mode
#define HEF_CHANGED     0x2000  //  Data has changed since last HEM_SETDATA

// Styles
#define HES_DISPLAYLEFT     0x0000L
#define HES_DISPLAYCENTER   0x0001L
#define HES_DISPLAYRIGHT    0x0002L
#define HES_UPPERCASE       0x0008L
#define HES_LOWERCASE       0x0010L
#define HES_PASSWORD        0x0020L
#define HES_AUTOHSCROLL     0x0080L
#define HES_NOHIDESEL       0x0100L

#define HES_BORDER3D        0x0200L              // 3-D indented border
#define HES_INDENT          0x0200L              // 3-D indented border
#define HES_EXTRUDE        (0x0200L | WS_BORDER) // 3-D extruded border
#define HES_HILITE          0x0400L              // changes background color on SETFOCUS

#define HES_EDITLEFT        0x0000L
#define HES_EDITRIGHT       0x0800L
#define HES_HUNGRY          0x1000L
#define HES_READONLY				0x2000L

// Notification codes
#define HEN_SETFOCUS    0x0100
#define HEN_KILLFOCUS   0x0200
#define HEN_CHANGE      0x0300
#define HEN_UPDATE      0x0400
#define HEN_ERRSPACE    0x0500
#define HEN_MAXTEXT     0x0501
#define HEN_HSCROLL     0x0601
#define HEN_INVALID     0x0602
#define HEN_MIN					0x0603
#define HEN_MAX					0x0604
#define HEN_LAST				0x0604

// Error codes
#define HEERR           -1  //  A general error occurred.
#define HEERR_GENERAL   -1  //  A general error occurred.
#define HEERR_BADPTR    -2  //  A pointer was found to be NULL unexpectedly.
#define HEERR_BADVAL    -3  //  A value was not in the expected group or range.
#define HEERR_NOSEL     -4  //  There is no selection.
#define HEERR_SPACE     -5  //  The control was unable to allocate memory.


/***************************************************************************\
  The HList control
\***************************************************************************/

// Messages
#define HLM_FIRST               (WM_USER + 150)

// multiple item messages
#define HLM_GETCOUNT            (HLM_FIRST + 0)   //  get number of items
#define HLM_RESETCONTENT        (HLM_FIRST + 1)   //  Delete all items

// index messages
#define HLM_DELETEITEM          (HLM_FIRST + 10)  //  Delete item by index
#define HLM_SELECTITEM          (HLM_FIRST + 11)  //  Select item by index
#define HLM_GETDATA             (HLM_FIRST + 12)  //  get item data by index
#define HLM_GETCODE             (HLM_FIRST + 13)  //  get item code by index
#define HLM_GETTEXT             (HLM_FIRST + 14)  //  get item text by index
#define HLM_GETTEXTLEN          (HLM_FIRST + 15)  //  get item textlen by index
#define HLM_SETCODE             (HLM_FIRST + 16)  //  get item code by index

// data messages
#define HLM_FINDSTRING          (HLM_FIRST + 18)
#define HLM_SELECTSTRING        (HLM_FIRST + 19)
#define HLM_GETDATACLASS        (HM_GETDATACLASS) //  get data class
#define HLM_GETDATALINK         (HM_GETDATALINK)  //  pointer to cursel data
#define HLM_SETDATALINK         (HM_SETDATALINK)  //
#define HLM_GETDATASIZE         (HM_GETDATASIZE)  //  get data size in bytes
#define HLM_GETDATATYPE         (HM_GETDATATYPE)  //  get data type
#define HLM_ADDITEM             (HLM_FIRST + 25)  //  add item without codes
#define HLM_INSERTITEM          (HLM_FIRST + 26)  //  insert item without codes
#define HLM_FINDDATA            (HLM_FIRST + 27)  //  find item by data
#define HLM_SELECTDATA          (HLM_FIRST + 28)  //  select item by data
#define HLM_GETCURDATA          (HLM_FIRST + 29)  //  current item's Data

// code messages
#define HLM_GETCODECLASS        (HLM_FIRST + 30)  //  get code class
#define HLM_GETCODELINK         (HLM_FIRST + 31)  //  hot-link to current selection
#define HLM_SETCODELINK         (HLM_FIRST + 32)  //  hot-link to current selection
#define HLM_GETCODESIZE         (HLM_FIRST + 33)  //  get code size in bytes
#define HLM_GETCODETYPE         (HLM_FIRST + 34)  //  get data type
#define HLM_FINDCODE            (HLM_FIRST + 35)  //  find item by code
#define HLM_SELECTCODE          (HLM_FIRST + 36)  //  select item by data
#define HLM_GETCURCODE          (HLM_FIRST + 37)  //  current item's Code

// painting messages
#define HLM_UPDATE              (HLM_FIRST + 50)  //  repaint the list
#define HLM_GETBKGNDBRUSH       (HM_GETBKGNDBRUSH)  //  background color
#define HLM_SETBKGNDBRUSH       (HM_SETBKGNDBRUSH)
#define HLM_GETHILITEBRUSH      (HM_GETHILITEBRUSH) //  background color with focus
#define HLM_SETHILITEBRUSH      (HM_SETHILITEBRUSH)
#define HLM_GETTEXTCOLOR        (HM_GETTEXTCOLOR) //  text color
#define HLM_SETTEXTCOLOR        (HM_SETTEXTCOLOR)

// selection messages
#define HLM_GETCURSEL           (HLM_FIRST + 60)  //  currently selected item
#define HLM_SETCURSEL           (HLM_FIRST + 61)
#define HLM_GETSEL              (HLM_FIRST + 62)  //  selection state of item
#define HLM_SETSEL              (HLM_FIRST + 63)
#define HLM_GETSELCOUNT         (HLM_FIRST + 64)  //  number of selected items
#define HLM_SELECTRANGE         (HLM_FIRST + 65)  //  select a range
#define HLM_GETSELITEMS         (HLM_FIRST + 66)  //  get array of indices

// dimension messages
#define HLM_SETCOLUMNWIDTH      (HLM_FIRST + 70)  //  set the column width
#define HLM_GETHEXTENT          (HLM_FIRST + 71)  //  get the total width of list
#define HLM_GETITEMRECT         (HLM_FIRST + 72)  //  get an item rectangle
#define HLM_GETCOLUMNWIDTH      (HLM_FIRST + 73)  //  get the column width

// miscellany
#define HLM_GETTOPINDEX         (HLM_FIRST + 80)  //  index of first visible item
#define HLM_SETTOPINDEX         (HLM_FIRST + 81)
#define HLM_GETSCROLLPOS        (HLM_GETTOPINDEX) //  index of first visible item
#define HLM_SETSCROLLPOS        (HLM_SETTOPINDEX)
#define HLM_SCROLL              (HLM_FIRST + 83)  //  scroll the list box
#define HLM_SETTABSTOPS         (HLM_FIRST + 84)  //  set tab stops
#define HLM_SETREDRAW           (HLM_FIRST + 87)  //  set redraw on/off
#define HLM_HASCHANGED          (HM_HASCHANGED)   //  has selection changed since focus
#define HLM_SETCHANGED          (HM_SETCHANGED)   //  has selection changed since focus
#define HLM_GETSTATE            (HLM_FIRST + 91)  //  get state flags
#define HLM_SETSTATE            (HLM_FIRST + 92)  //  set state flag
#define HLM_GETFORMAT           (HM_GETFORMAT)    //  get format string
#define HLM_SETFORMAT           (HM_SETFORMAT)    //  set format string
#define HLM_GETFONT             (HM_GETFONT)
#define HLM_SETFONT             (HM_SETFONT)
#define HLM_BEQUIET             (HM_BEQUIET)
#define HLM_ISQUIET             (HM_ISQUIET)
#define HLM_ADDSAMPLES          (HLM_FIRST + 93)
#define HLM_GETSTYLE            (HLM_FIRST + 94)
#define HLM_LAST                (HLM_FIRST + 150)

// Styles
#define HLS_SINGLELINE        0x0001L
#define HLS_SORTBYDATA        0x0002L
#define HLS_SORTBYCODE        0x0004L
#define HLS_MULTISEL          0x0008L
#define HLS_BORDER3D          0x0010L
#define HLS_HILITE            0x0020L
#define HLS_OWNERDRAW					0x0040L

#define HLS_USETABS           0x0080L
#define HLS_NONINTHEIGHT      0x0100L
#define HLS_MULTICOL          0x0200L
#define HLS_EXTENDEDSEL       0x0800L
#define HLS_HUNGRY            0x1000L
#define HLS_NOSCROLL          0x4000L

#define HLS_INDENT           (HLS_BORDER3D)
#define HLS_EXTRUDE          (HLS_BORDER3D | WS_BORDER)
#define HLS_STANDARD         (WS_VSCROLL | HLS_INDENT | HLS_HILITE)

// Notification Codes
#define HLN_ERRSPACE    (-2)
#define HLN_SELCHANGE   1
#define HLN_DBLCLK      2
#define HLN_SELCANCEL   3
#define HLN_SETFOCUS    4
#define HLN_KILLFOCUS   5

// Error Codes
#define HLERR               (-1)
#define HLERR_GENERAL       (-1)
#define HLERR_SPACE         (-2)
#define HLERR_EMPTY         (-3)
#define HLERR_BADVAL        (-4)
#define HLERR_BADPTR        (-5)
#define HLERR_NOCODES       (-6)
#define HLERR_NOTFOUND      (-1)

// Search options
#define HL_ALL          0x0000
#define HL_DATA         0x0001
#define HL_CODE         0x0002
#define HL_INDEX        0x0004
#define HL_SELECTED     0x0008
#define HL_UNSELECTED   0x0010
#define HL_TEXT         0x0020

// Action options
#define HL_DESELECT     0x0000
#define HL_SELECT       0x0001
#define HL_TOGGLE       0x0002

// State flags
#define HLF_HASCODES    0x0080
#define HLF_CHANGED     0x2000

// Function prototypes
BOOL 	WWAPI HLEmptyList 		(HWND hwnd);
int 	WWAPI HLAddItems 		(HWND hwnd, int iCount, LPVOID lpData);
int 	WWAPI HLAddItemsEx 	(HWND hwnd, int iCount, LPVOID lpData, LPVOID lpCode);
int 	WWAPI HLAddItem 			(HWND hwnd, LPVOID lpData);
int 	WWAPI HLAddItemEx 		(HWND hwnd, LPVOID lpData, LPVOID lpCode);
int 	WWAPI HLInsertItems 	(HWND hwnd, int iPos, int iCount, LPVOID lpData);
int 	WWAPI HLInsertItemsEx(HWND hwnd, int iPos, int iCount, LPVOID lpData, LPVOID lpCode);
int 	WWAPI HLInsertItem 	(HWND hwnd, int iPos, LPVOID lpData);
int 	WWAPI HLInsertItemEx (HWND hwnd, int iPos, LPVOID lpData, LPVOID lpCode);
int 	WWAPI HLInsertStringEx (HWND hwnd, int iPos, LPSTR lpDataStr, LPSTR lpCodeStr);
//int 	WWAPI HLMarkItems 		(HWND hwnd, UINT uiSearch, int iCount, LPVOID lpInfo);
int 	WWAPI HLDeleteItems 	(HWND hwnd, UINT uiSearch, int iCount, LPVOID lpInfo);
int 	WWAPI HLSelectItems 	(HWND hwnd, UINT uiAction, UINT uiSearch, int iCount, LPVOID lpInfo);
int 	WWAPI HLGetItems 		(HWND hwnd, int iCount, UINT uiReturn, LPVOID lpReturn, UINT uiSearch, LPVOID lpInfo);
int 	WWAPI HLFindData 		(HWND hwnd, int iStart, LPVOID lpData);
int 	WWAPI HLFindCode 		(HWND hwnd, int iStart, LPVOID lpCode);
BOOL 	WWAPI HLGetData 			(HWND hwnd, int iIndex, LPVOID lpData);
BOOL 	WWAPI HLGetCode 			(HWND hwnd, int iIndex, LPVOID lpCode);
BOOL 	WWAPI HLSetCode 			(HWND hwnd, int iIndex, LPVOID lpCode);
BOOL 	WWAPI HLDeleteItem 	(HWND hwnd, int iIndex);
BOOL 	WWAPI HLSelectItem 	(HWND hwnd, int iIndex, UINT uiAction);
BOOL 	WWAPI HLSelectData 	(HWND hwnd, LPVOID lpData, UINT uiAction);
BOOL 	WWAPI HLSelectCode 	(HWND hwnd, LPVOID lpCode, UINT uiAction);
BOOL 	WWAPI HLIsSelected 	(HWND hwnd, int iIndex);
BOOL 	WWAPI HLSetDataLink 	(HWND hwnd, LPVOID lpData, BOOL bSelect);
BOOL 	WWAPI HLSetCodeLink 	(HWND hwnd, LPVOID lpCode, BOOL bSelect);


/***************************************************************************\
  The HComb control
\***************************************************************************/

// Messages
#define HCM_FIRST             (WM_USER + 250)
#define HCM_GETFONT           (WM_GETFONT)            // Returns Font.
#define HCM_SETFONT           (WM_SETFONT)            // Sets Font.
#define HCM_GETSTATE          (HCM_FIRST + 21)        // Returns set State flags.
#define HCM_SETSTATE          (HCM_FIRST + 22)        // Sets one of State flags.
#define HCM_HASCHANGED        (HM_HASCHANGED)         // Returns the status of the Changed flag
#define HCM_SETCHANGED        (HM_SETCHANGED)         // Sets the status of the Changed flag
#define HCM_GETDROPHEIGHT			(HCM_FIRST + 23)
#define HCM_SETDROPHEIGHT			(HCM_FIRST + 24)
#define HCM_GETDROPWIDTH 			(HCM_FIRST + 25)
#define HCM_SETDROPWIDTH 			(HCM_FIRST + 26)

// Apply to both edit and list controls
#define HCM_GETBKGNDBRUSH     (HM_GETBKGNDBRUSH)      // Returns HiliteBrush.
#define HCM_GETDATACLASS      (HM_GETDATACLASS)       // Returns DataType
#define HCM_GETDATASIZE       (HM_GETDATASIZE)        // Returns DataSize
#define HCM_GETDATATYPE       (HM_GETDATATYPE)        // Returns DataType
#define HCM_GETDATALINK       (HM_GETDATALINK)        // Returns DataLink.
#define HCM_SETDATALINK       (HM_SETDATALINK)        // Sets DataLink.
#define HCM_GETFORMAT         (HM_GETFORMAT)          // Retrieves Format string.
#define HCM_GETHILITEBRUSH    (HM_GETHILITEBRUSH)     // Returns HiliteBrush.
#define HCM_GETTEXTCOLOR      (HM_GETTEXTCOLOR)       // text color

#define HCM_SETBKGNDBRUSH     (HM_SETBKGNDBRUSH)      // Sets HiliteBrush.
#define HCM_SETFORMAT         (HM_SETFORMAT)          // Sets Format string.
#define HCM_SETHILITEBRUSH    (HM_SETHILITEBRUSH)     // Sets HiliteBrush.
#define HCM_SETTEXTCOLOR      (HM_SETTEXTCOLOR)

// Edit Control messages
#define HCM_GETEDITDATA       (HEM_GETDATA)           // Retrieves Data.
#define HCM_SETEDITDATA       (HEM_SETDATA)           // Sets Data.
#define HCM_GETEDITSCROLLPOS  (HEM_GETSCROLLPOS)      // Retrieves ScrollPos.
#define HCM_SETEDITSCROLLPOS  (HEM_SETSCROLLPOS)      // Sets ScrollPos.
#define HCM_GETEDITSEL        (HEM_GETSEL)            // Retrieves range of selected text.
#define HCM_SETEDITSEL        (HEM_SETSEL)            // Sets range of selected text.
#define HCM_GETEDITTEXT       (HEM_GETTEXT)           // Retrieves formatted Text.
#define HCM_SETEDITTEXT       (HEM_SETTEXT)           // Sets Data for String Class.
#define HCM_GETEDITTEXTLEN    (HEM_GETTEXTLEN)        // Returns TextLen.
#define HCM_GETEDITMAXTEXTLEN (HEM_GETMAXTEXTLEN)     // Returns TextMaxLen.
#define HCM_SETEDITMAXTEXTLEN (HEM_SETMAXTEXTLEN)     // Sets TextMaxLen.
#define HCM_GETOVERWRITEMODE  (HEM_GETOVERWRITEMODE)  // Returns overwrite/insert status.
#define HCM_SETOVERWRITEMODE  (HEM_SETOVERWRITEMODE)  // Toggles overwrite/insert modes.
#define HCM_UPDATE            (HEM_UPDATE)            // Updates Text using Data or vice-versa.

// List Control messages
#define HCM_ADDITEM           (HLM_ADDITEM)           // add item without codes
#define HCM_DELETEITEM        (HLM_DELETEITEM)        // Delete item by index
#define HCM_FINDCODE          (HLM_FINDCODE)          // find item by code
#define HCM_FINDDATA          (HLM_FINDDATA)          // find item by data
#define HCM_FINDSTRING        (HLM_FINDSTRING)        // ONLY for HT_STRING data type
#define HCM_GETCODECLASS      (HLM_GETCODECLASS)      // get code class
#define HCM_GETCODELINK       (HLM_GETCODELINK)       // hot-link to current selection
#define HCM_GETCODESIZE       (HLM_GETCODESIZE)       // get code size in bytes
#define HCM_GETCODETYPE       (HLM_GETCODETYPE)       // get data type
#define HCM_GETCOUNT          (HLM_GETCOUNT)          // get number of items
#define HCM_GETCURCODE        (HLM_GETCURCODE)        // current item's Code
#define HCM_GETCURDATA        (HLM_GETCURDATA)        // current item's Data
#define HCM_GETCURSEL         (HLM_GETCURSEL)         // currently selected item
#define HCM_GETCODE           (HLM_GETCODE)           // get item code by index
#define HCM_SETCODE           (HLM_SETCODE)           // set item code by index
#define HCM_GETDATA           (HLM_GETDATA)           // get item data by index
#define HCM_GETSCROLLPOS      (HLM_GETSCROLLPOS)      // of first visible item
#define HCM_GETTEXT           (HLM_GETTEXT)           // get item text by index
#define HCM_GETTEXTLEN        (HLM_GETTEXTLEN)        // get item textlen by index
#define HCM_INSERTITEM        (HLM_INSERTITEM)        // insert item without codes
#define HCM_RESETCONTENT      (HLM_RESETCONTENT)      // Delete all items
#define HCM_SELECTCODE        (HLM_SELECTCODE)        // select item by data
#define HCM_SELECTDATA        (HLM_SELECTDATA)        // select item by data
#define HCM_SELECTSTRING      (HLM_SELECTSTRING)
#define HCM_SETCODELINK       (HLM_SETCODELINK)       // hot-link to current selection
#define HCM_SETCURSEL         (HLM_SETCURSEL)
#define HCM_SETSCROLLPOS      (HLM_SETSCROLLPOS)
#define HCM_SCROLL            (HLM_SCROLL)            // scroll the list box
#define HCM_SETTABSTOPS       (HLM_SETTABSTOPS)       // set tab stops
#define HCM_BEQUIET           (HM_BEQUIET)
#define HCM_ISQUIET           (HM_ISQUIET)

// Styles
#define HCS_DROPDOWN          0x0001L
#define HCS_SORTBYDATA        0x0002L
#define HCS_SORTBYCODE        0x0004L
#define HCS_HASEDIT           0x0008L
#define HCS_BORDER3D          0x0010L
#define HCS_HILITE            0x0020L
#define HCS_OWNERDRAW					0x0040L
#define HCS_USETABS           0x0080L
#define HCS_NONINTHEIGHT      0x0100L
#define HCS_WANTKEYS          0x0400L
#define HCS_HUNGRY            0x1000L

#define HCS_INDENT           (HCS_BORDER3D)
#define HCS_EXTRUDE          (HCS_BORDER3D | WS_BORDER)
#define HCS_STANDARD         (HCS_SORTBYDATA | WS_VSCROLL | HCS_INDENT | HCS_HILITE)

// Notification Codes
#define HCN_ERRSPACE    (-2)
#define HCN_SELCHANGE   1
#define HCN_DBLCLK      2
#define HCN_SELCANCEL   3
#define HCN_SETFOCUS    4
#define HCN_KILLFOCUS   5
#define HCN_EDITUPDATE  6
#define HCN_DROPDOWN    7
#define HCN_EDITCHANGE  8

// Error Codes
#define HCOKAY              0
#define HCERR               (-1)
#define HCERR_GENERAL       (-1)
#define HCERR_SPACE         (-2)
#define HCERR_EMPTY         (-3)
#define HCERR_BADVAL        (-4)
#define HCERR_BADPTR        (-5)
#define HCERR_NOCODES       (-6)
#define HCERR_NOTFOUND      (-1)

// Search options
#define HC_ALL          0x0000
#define HC_DATA         0x0001
#define HC_CODE         0x0002
#define HC_INDEX        0x0004
#define HC_TEXT         0x0020

// Listbox State flags
#define HCF_HASCODES    0x0080
#define HCF_CHANGED     0x2000

// Function prototypes
int  WWAPI HCAddItemEx     (HWND hwnd, LPVOID lpData, LPVOID lpCode);
int  WWAPI HCAddItems      (HWND hwnd, int iCount, LPVOID lpData);
int  WWAPI HCAddItemsEx    (HWND hwnd, int iCount, LPVOID lpData, LPVOID lpCode);
int  WWAPI HCDeleteItems   (HWND hwnd, UINT uiSearch, int iCount, LPVOID lpInfo);
int  WWAPI HCGetItems      (HWND hwnd, int iCount, UINT uiReturn, LPVOID lpReturn, UINT uiSearch, LPVOID lpInfo);
int  WWAPI HCInsertItemEx  (HWND hwnd, int iPos, LPVOID lpData, LPVOID lpCode);
int  WWAPI HCInsertItems   (HWND hwnd, int iPos, int iCount, LPVOID lpData);
int  WWAPI HCInsertItemsEx (HWND hwnd, int iPos, int iCount, LPVOID lpData, LPVOID lpCode);

// Useful macros
#define HCAddItem(h,p)          ((int)SM (h, HCM_ADDITEM, 0, DLP(p)))
#define HCDeleteItem(h,i)       ((BOOL)SM (h, HCM_DELETEITEM, i, 0L))
#define HCEmptyList(h)          ((BOOL)SM (h, HCM_RESETCONTENT, 0, 0L))
#define HCFindCode(h,i,p)       ((int)SM (h, HCM_FINDCODE, i, DLP(p)))
#define HCFindData(h,i,p)       ((int)SM (h, HCM_FINDDATA, i, DLP(p)))
#define HCGetCode(h,i,p)        ((int)SM (h, HCM_GETCODE, i, DLP(p)))
#define HCSetCode(h,i,p)        ((int)SM (h, HCM_SETCODE, i, DLP(p)))
#define HCGetData(h,i,p)        ((int)SM (h, HCM_GETDATA, i, DLP(p)))
#define HCInsertItem(h,i,p)     ((int)SM (h, HCM_INSERTITEM, i, DLP(p)))
#define HCIsChanged(h)          ((BOOL)SM (h, HCM_HASCHANGED, 0, 0L))
#define HCIsSelected(h,i)       ((int)SM (h, HCM_GETCURSEL, 0, 0L) == i)
#define HCSelectCode(h,p)       ((BOOL)SM (h, HCM_SELECTCODE, TRUE, DLP(p)))
#define HCSelectData(h,p)       ((BOOL)SM (h, HCM_SELECTDATA, TRUE, DLP(p)))
#define HCSetCodeLink(h,p)      ((int)SM (h, HCM_SETCODELINK, TRUE, DLP(p)))
#define HCSetDataLink(h,p,b)    ((int)SM (h, HCM_SETDATALINK, b, DLP(p)))
#define HCSetCurSel(h,i)        ((int)SM (h, HCM_SETCURSEL, i, 0L))

#define HCSetEditData(h,p)      SM (h, HCM_SETEDITDATA, 0, DLP(p))
#define HCGetEditData(h,m,p)    SM (h, HCM_GETEDITDATA, m, DLP(p))


/***************************************************************************\
  The HSTat control
\***************************************************************************/

// Messages
#define HSM_FIRST               (WM_USER + 400)
#define HSM_GETALIGN            (HSM_FIRST + 0)
#define HSM_SETALIGN            (HSM_FIRST + 1)
#define HSM_GETBKGNDCOLOR       (HSM_FIRST + 2)
#define HSM_SETBKGNDCOLOR       (HSM_FIRST + 3)
#define HSM_GETFRGNDCOLOR       (HSM_FIRST + 4)
#define HSM_SETFRGNDCOLOR       (HSM_FIRST + 5)
#define HSM_GETPIC              (HSM_FIRST + 6)
#define HSM_SETPIC              (HSM_FIRST + 7)
#define HSM_GETIMAGE            HSM_GETPIC					//	Obsolete
#define HSM_SETIMAGE            HSM_SETPIC          //	Obsolete
#define HSM_GETTEXT             (HM_GETTEXT)
#define HSM_SETTEXT             (HM_SETTEXT)
#define HSM_GETTYPE             (HSM_FIRST + 8)
#define HSM_SETTYPE             (HSM_FIRST + 9)
#define HSM_GETPALETTE					(HSM_FIRST + 10)

// Constants
#define HS_LEFT                 0
#define HS_CENTER               1
#define HS_RIGHT                2

// Styles
#define HST_TEXT                0x00
#define HST_GROUP               0x01
#define HST_FRAME               0x02
#define HST_PIC                 0x03
#define HST_HLINE               0x04
#define HST_VLINE               0x05

#define HST_BITMAP              0x03		//	Obsolete - replaced by HST_PIC
#define HST_ICON                0x03		//	Obsolete - replaced by HST_PIC

#define HSS_TEXT                0x00L
#define HSS_GROUP               0x01L
#define HSS_FRAME               0x02L
#define HSS_PIC                 0x03L
#define HSS_HLINE               0x04L
#define HSS_VLINE               0x05L

#define HSS_BITMAP              0x03L		//	Obsolete - replaced by HSS_PIC
#define HSS_ICON                0x03L		//	Obsolete - replaced by HSS_PIC

#define HSS_BORDER3D            0x0010L
#define HSS_BUMP                0x0020L
#define HSS_LEFT                0x0040L
#define HSS_RIGHT               0x0080L 
#define HSS_TEXTINDENT          0x0100L
#define HSS_TRANSPARENT         0x0200L

#define HSS_EXTRUDE             (HSS_BORDER3D | WS_BORDER)
#define HSS_INDENT              (HSS_BORDER3D)

/***************************************************************************\
  The HTool control
\***************************************************************************/

// Messages
#define HTM_GETSTYLE      (HM_GETSTYLE)
#define HTM_SETSTYLE      (HM_SETSTYLE)
#define HTM_GETBRUSH      (WM_USER + 12)
#define HTM_SETBRUSH      (WM_USER + 13)
#define HTM_GETNOTIFY     (WM_USER + 14)
#define HTM_SETNOTIFY     (WM_USER + 15)
#define HTM_GETCAPTION    (WM_GETTEXT)
#define HTM_SETCAPTION    (WM_SETTEXT)
#define HTM_GETTHICK      (WM_USER + 16)
#define HTM_SETTHICK      (WM_USER + 17)
#define HTM_BEQUIET       (HM_BEQUIET)
#define HTM_ISQUIET       (HM_ISQUIET)

// Styles
#define HTS_FLOAT      WS_POPUP
#define HTS_TOP        0x0000L
#define HTS_BOTTOM     0x0001L
#define HTS_LEFT       0x0002L
#define HTS_RIGHT      0x0004L
#define HTS_NOSTRETCH  0x0008L

#define HTS_RIBBON     (WS_CHILD | WS_VISIBLE)
#define HTS_STATUS     (WS_CHILD | WS_VISIBLE | HTS_BOTTOM)
#define HTS_PALETTE    (HTS_FLOAT | WS_CAPTION | WS_VISIBLE)

// Function Prototypes
HWND    WWAPI HToolCreate
(
HANDLE  hInstance,          //  The instance that contains the dialog template
LPCSTR  lpTemplate,         //  The name of the dialog template
HWND    hwndParent,         //  The parent window of the toolbar
HWND    hwndNotify,         //  The window the toolbar is to notify
DWORD   dwStyle,            //  The toolbar style
WORD    wID,                //  The toolbar ID (only for attached toolbars)
int     iXPos,              //  The toolbar's initial X and Y screen positions
int     iYPos               //    (only for floating toolbars)
);

void    WWAPI HToolUpdate (HWND);


/***************************************************************************\
  The HGrid control
\***************************************************************************/

// HGrid Resource type constant
#define RT_HGRID      	300		
#define CTLCOLOR_GRID   21

// Messages
#define HGM_FIRST                 (WM_USER + 300)
#define HGM_GETFONT               (HM_GETFONT)
#define HGM_SETFONT               (HM_SETFONT)
#define HGM_GETBKGNDBRUSH         (HM_GETBKGNDBRUSH)
#define HGM_SETBKGNDBRUSH         (HM_SETBKGNDBRUSH)

#define HGM_SETGRID               (HGM_FIRST + 0)
#define HGM_GETGRID               (HGM_FIRST + 1)  
#define HGM_RESETCONTENT          (HGM_FIRST + 2)
#define HGM_UPDATE                (HGM_FIRST + 5)
#define HGM_GETBTNHEIGHT          (HGM_FIRST + 10)
#define HGM_SETBTNHEIGHT          (HGM_FIRST + 11)
#define HGM_GETBTNWIDTH           (HGM_FIRST + 12)
#define HGM_SETBTNWIDTH           (HGM_FIRST + 13)
#define HGM_GETROWHEIGHT          (HGM_FIRST + 14)
#define HGM_SETROWHEIGHT          (HGM_FIRST + 15)
#define HGM_GETCOLCOUNT           (HGM_FIRST + 16)
#define HGM_GETROWCOUNT           (HGM_FIRST + 17)
#define HGM_GETFIRSTCOL           (HGM_FIRST + 18)
#define HGM_SETFIRSTCOL           (HGM_FIRST + 19)
#define HGM_GETFIRSTREC           (HGM_FIRST + 20)
#define HGM_SETFIRSTREC           (HGM_FIRST + 21)
#define HGM_GETFROZENCOLS         (HGM_FIRST + 22)
#define HGM_SETFROZENCOLS         (HGM_FIRST + 23)
#define HGM_GETSELANCHOR          (HGM_FIRST + 24)
#define HGM_SETSELANCHOR          (HGM_FIRST + 25)
#define HGM_GETSELEXTENT          (HGM_FIRST + 26)
#define HGM_SETSELEXTENT          (HGM_FIRST + 27)
#define HGM_GETSTATE              (HGM_FIRST + 28)
#define HGM_SETSTATE              (HGM_FIRST + 29)
#define HGM_GETTITLE              (HGM_FIRST + 30)
#define HGM_SETTITLE              (HGM_FIRST + 31)
#define HGM_ADDFLD                (HGM_FIRST + 32)
#define HGM_DELETEFLD             (HGM_FIRST + 33)
#define HGM_INSERTFLD             (HGM_FIRST + 34)
#define HGM_MOVECOL               (HGM_FIRST + 35)
#define HGM_ADDREC                (HGM_FIRST + 36)
#define HGM_DELETEREC             (HGM_FIRST + 37)
#define HGM_INSERTREC             (HGM_FIRST + 38)
#define HGM_MOVEROW            		(HGM_FIRST + 39)
#define HGM_GETBASEREC            (HGM_FIRST + 40)
#define HGM_SETBASEREC            (HGM_FIRST + 41)
#define HGM_GETCURREC             (HGM_FIRST + 44)
#define HGM_FINDFLD               (HGM_FIRST + 46)
#define HGM_GETCOLMAP             (HGM_FIRST + 47)
#define HGM_SETCOLMAP             (HGM_FIRST + 48)
#define HGM_BEQUIET               (HM_BEQUIET)
#define HGM_ISQUIET               (HM_ISQUIET)
#define HGM_GETBUFFERPROC         (HGM_FIRST + 49)
#define HGM_SETBUFFERPROC         (HGM_FIRST + 50)
#define HGM_GETMAXREC             (HGM_FIRST + 51)
#define HGM_SETMAXREC             (HGM_FIRST + 52)
#define HGM_GETBUFFERSIZE         (HGM_FIRST + 53)
#define HGM_SETBUFFERSIZE         (HGM_FIRST + 54)
#define HGM_GETSTYLE              (HM_GETSTYLE)
#define HGM_SETSTYLE              (HGM_FIRST + 56) // Allows setting of individual bits
#define HGM_GETFLD		            (HGM_FIRST + 57)
#define HGM_GETCOL           			(HGM_FIRST + 58)
#define HGM_GETROW           			(HGM_FIRST + 59)
#define HGM_GETMARKER             (HGM_FIRST + 60)
#define HGM_SETMARKER             (HGM_FIRST + 61)
#define HGM_GETHSCROLLPOS         (HGM_FIRST + 62)
#define HGM_GETVSCROLLPOS         (HGM_FIRST + 63)
#define HGM_SETHSCROLLPOS         (HGM_FIRST + 64)
#define HGM_SETVSCROLLPOS         (HGM_FIRST + 65) 
#define HGM_GETVSCROLLRANGE				(HGM_FIRST + 66)
#define HGM_GETHSCROLLRANGE				(HGM_FIRST + 67)
#define HGM_GETSELROWS     				(HGM_FIRST + 68)
#define HGM_GETSELCOUNT    				(HGM_FIRST + 69)
#define HGM_SELECTROW							(HGM_FIRST + 70)

#define HGRM_GETSTATE             (HGM_FIRST + 80)
#define HGRM_SETSTATE             (HGM_FIRST + 81)
#define HGRM_GETDATA              (HGM_FIRST + 82)
#define HGRM_SETDATA              (HGM_FIRST + 83)
#define HGRM_GETLINK              (HGM_FIRST + 84)
#define HGRM_SETLINK              (HGM_FIRST + 85)
#define HGRM_GETSIZE              (HGM_FIRST + 86)
#define HGRM_ISSELECTED  					(HGM_FIRST + 87)

#define HGFM_GETHCTL              (HGM_FIRST + 100)
#define HGFM_GETCTLSTYLE          (HGM_FIRST + 101)
#define HGFM_GETCTLTYPE           (HGM_FIRST + 102)
#define HGFM_GETCODECLASS         (HGM_FIRST + 103)
#define HGFM_GETCODETYPE          (HGM_FIRST + 104)
#define HGFM_GETDATACLASS         (HGM_FIRST + 105)
#define HGFM_GETDATASIZE          (HGM_FIRST + 106)
#define HGFM_GETDATATYPE          (HGM_FIRST + 107)
#define HGFM_GETFORMAT            (HGM_FIRST + 108)
#define HGFM_SETFORMAT            (HGM_FIRST + 109)
#define HGFM_GETFORMATLEN         (HGM_FIRST + 110)
#define HGFM_GETNAME              (HGM_FIRST + 111)
#define HGFM_GETNAMELEN           (HGM_FIRST + 112)
#define HGFM_SETNAME              (HGM_FIRST + 113)
#define HGFM_GETCOLWIDTH          (HGM_FIRST + 114)
#define HGFM_SETCOLWIDTH          (HGM_FIRST + 115)
#define HGFM_GETDROPHEIGHT        (HGM_FIRST + 116)
#define HGFM_SETDROPHEIGHT        (HGM_FIRST + 117)
#define HGFM_GETSTATE             (HGM_FIRST + 118)
#define HGFM_SETSTATE             (HGM_FIRST + 119)
#define HGFM_GETOFFSET            (HGM_FIRST + 122)
#define HGFM_FREEHCTL             (HGM_FIRST + 123)

#define HGM_CTLCOLORGRID					(HGM_FIRST + 124) //_WIN32 


// Notification Codes
#define HGN_SETFOCUS    0x0001
#define HGN_KILLFOCUS   0x0002
#define HGN_SELCHANGE   0x0003
#define HGN_DBLCLK      0x0004
#define HGN_DESTROY     0x0005
#define HGN_ERRSPACE    0x000D
#define HGN_MAXTEXT     0x000E
#define HGN_TOP         0x000F
#define HGN_BOTTOM      0x0010
#define HGN_RECNEW      0x0011
#define HGN_RECSWITCH   0x0012
#define HGN_RECCHANGED  0x0013
#define HGN_RECDELETE   0x0014
#define HGN_COLMOVED    0x0015
#define HGN_ROWSIZED    0x0016
#define HGN_COLSIZED    0x0017
#define HGN_SELCHANGING 	0x0018
#define HGN_SELEXTENDING	0x0019
#define HGN_HSCROLL 		  0x0020
#define HGN_VSCROLL			  0x0021
#define HGN_HSCROLLRANGE 	0x0022
#define HGN_VSCROLLRANGE	0x0023


// Error Codes
#define HGERR           -1
#define HGERR_NOTFOUND  -1

// Styles
#define HGS_BROWSE          0x0001L
#define HGS_NOLINES         0x0002L
#define HGS_MDICHILD        0x0004L
#define HGS_INPLACE         0x0008L
#define HGS_AUTOEXTEND      0x0010L
#define HGS_RESIZEROWS      0x0020L
#define HGS_RESIZECOLS      0x0040L
#define HGS_ROWBUTTONS      0x0080L
#define HGS_COLBUTTONS      0x0100L
#define HGS_KEYBDDELINS     0x0200L
#define HGS_LEAVEONTAB      0x0400L
#define HGS_NOHIDESEL       0x0800L
#define HGS_DRAGCOLS        0x1000L
#define HGS_WHOLEROWS       0x2000L
#define HGS_SINGLESELECT    0x4000L
#define HGS_DISABLENOSCROLL 0x8000L
#define HGS_STANDARD			 (HGS_INPLACE | HGS_RESIZEROWS | HGS_RESIZECOLS | HGS_ROWBUTTONS | \
														HGS_COLBUTTONS | HGS_KEYBDDELINS | HGS_DISABLENOSCROLL | \
														WS_VSCROLL | WS_HSCROLL | WS_BORDER)
// State Flags
#define HGF_CHANGED         0x2000

// Field State Flags
#define HGFF_HIDDEN         0x0001
#define HGFF_BROWSE         0x0004
#define HGFF_USECODE        0x0008

// Record State Flags
#define HGRF_CHANGED        0x0001
#define HGRF_NEW            0x0002

// Cell State Flags
#define HGCF_NULL        		0x0001

// Field Control Types
#define HGCTL_COMBO         'c'
#define HGCTL_EDIT          'e'
#define HGCTL_LIST          'l'
#define HGCTL_DROP          'd'
#define HGCTL_CHECK         'k'

// Typedefs
typedef LPVOID  HHGRID;
typedef LPVOID  HFLD;
typedef LPSTR   LPRECORD;

typedef short int		ROW;
typedef long				REC;
typedef short int		COL;
typedef short int		FLD;


#ifdef WW_DEFS_V12

#define HGM_GETRECHEIGHT          HGM_GETROWHEIGHT	// Obsolete
#define HGM_SETRECHEIGHT          HGM_SETROWHEIGHT	// Obsolete
#define HGM_GETFLDCOUNT           HGM_GETCOLCOUNT		// Obsolete
#define HGM_GETRECCOUNT           HGM_GETROWCOUNT   // Obsolete
#define HGM_GETFIRSTFLD           HGM_GETFIRSTCOL		// Obsolete
#define HGM_SETFIRSTFLD           HGM_SETFIRSTCOL		// Obsolete
#define HGM_GETFROZENFLDS         HGM_GETFROZENCOLS	// Obsolete
#define HGM_SETFROZENFLDS         HGM_SETFROZENCOLS	// Obsolete
#define HGM_MOVEFLD               HGM_MOVECOL				// Obsolete
#define HGM_MOVEREC            		HGM_MOVEROW				// Obsolete
#define HGM_GETFLDMAP             HGM_GETCOLMAP			// Obsolete
#define HGM_SETFLDMAP             HGM_SETCOLMAP     // Obsolete
#define HGM_GETRECLINK            HGRM_GETLINK      // Obsolete
#define HGM_SETRECLINK            HGRM_SETLINK      // Obsolete
#define HGM_GETRECSIZE            HGRM_GETSIZE      // Obsolete

#define HGFM_GETWIDTH             HGFM_GETCOLWIDTH		// Obsolete
#define HGFM_SETWIDTH             HGFM_SETCOLWIDTH		// Obsolete
#define HGFM_GETHEIGHT        		HGFM_GETDROPHEIGHT	// Obsolete
#define HGFM_SETHEIGHT        		HGFM_SETDROPHEIGHT	// Obsolete
#define HGFM_GETRECPOS            HGM_GETFLD					// Obsolete
#define HGFM_GETGRIDPOS           HGM_GETCOL					// Obsolete

#define HGN_FLDMOVED    HGN_COLMOVED                // Obsolete

typedef struct
  {
  COL		iFld;
  ROW		iRec;
  }
  CELL,
  FAR *LPCELL;
  
#define SETCELL(pc,f,r)		 	(((LPCELL)pc)->iFld=(f),((LPCELL)pc)->iRec=(r))
#else

typedef struct
  {
  COL		iCol;
  ROW		iRow;
  }
  CELL,
  FAR *LPCELL;

#define SETCELL(pc,c,r)		 	(((LPCELL)pc)->iCol=(c),((LPCELL)pc)->iRow=(r))
#endif //	WW_DEFS_V12

// Macros
#define MAKECELL(l)        	(*((LPCELL)&(l)))
#define LONGCELL(c)        	(*((long FAR *)&(c)))
#define CELLROW(l)         	((ROW)HIWORD(l))
#define CELLCOL(l)         	((COL)LOWORD(l))
#define CELLREC         		CELLROW
#define CELLFLD         		CELLCOL
#define HGGetCurSel(h) 		 	(SM(h, HGM_GETSELEXTENT, 0, NULL))
#define HGSetCurSel(h,c,r) 	((BOOL)SM(h, HGM_SETSELEXTENT, 0, MAKELONG (c,r)))
#define HGColToFld(h,c)			((FLD)SM(h, HGM_GETFLD, c, 0L))
#define HGFldToCol(h,f)			((COL)SM(h, HGM_GETCOL, f, 0L))
#define HGRowToRec(h,row)		((REC)row + (REC)SM(h, HGM_GETBASEREC, 0, 0L))
#define HGRecToRow(h,rec)		((ROW)SM(h, HGM_GETROW, 0, rec))

// Function Prototypes
BOOL WWAPI HGGetCellData (HWND hCtl, COL iCol, ROW iRow, LPVOID lpDest);
BOOL WWAPI HGSetCellData (HWND hCtl, COL iCol, ROW iRow, LPVOID lpSrc);
BOOL WWAPI HGSetCellString (HWND hCtl, COL iCol, ROW iRow, LPSTR lpSrc);
WORD WWAPI HGGetCellText (HWND hCtl, COL iCol, ROW iRow, LPSTR lpText, int iMax);
BOOL WWAPI HGSetCellText (HWND hwnd, COL iCol, ROW iRow, LPSTR lpText);
BOOL WWAPI HGridInvalidateRange(HWND hwnd, COL iCol1, ROW iRow1, COL iCol2, ROW iRow2);
BOOL WWAPI HGridInvalidateCell (HWND hwnd, COL iCol, ROW iRow);
LPSTR WWAPI HGOffsetPtr (HWND hCtl, COL iCol, LPVOID lpRec);

#ifndef WIDGEINT_H

HFLD WWAPI HGFieldCreate
(
LPCSTR  	lpName,
WORD    	wState,
short int iColWidth,
short int iDropHeight,
char    	cDataClass,
char    	cDataType,
char    	cCodeClass,
char    	cCodeType,
short int iSize,
char    	cCtlType,
long    	lCtlStyle,
LPCSTR  	lpFmt
);

#define HGFieldCreateCheck(n,s,w,t)                 HGFieldCreate(n,s,w,0,HC_BOOL,t,0,0,0,HGCTL_CHECK,0L,NULL)
#define HGFieldCreateComb(n,s,w,h,c,t,z,y,f)        HGFieldCreate(n,s,w,h,c,t,0,0,z,HGCTL_COMBO,y,f)
#define HGFieldCreateDrop(n,s,w,h,c,t,cc,ct,z,y,f)  HGFieldCreate(n,s,w,h,c,t,cc,ct,z,HGCTL_DROP,y,f)
#define HGFieldCreateEdit(n,s,w,c,t,z,y,f)          HGFieldCreate(n,s,w,0,c,t,0,0,z,HGCTL_EDIT,y,f)
#define HGFieldCreateList(n,s,w,c,t,cc,ct,z,y,f)    HGFieldCreate(n,s,w,0,c,t,cc,ct,z,HGCTL_LIST,y,f)

BOOL    WWAPI HGFieldDestroy (HFLD lpFld);

#endif // WIDGEINT_H

/***************************************************************************\
  The HTabs control
\***************************************************************************/

// Messages
#define HAM_FIRST								(WM_USER + 10)
#define HAM_REFRESH   					(HAM_FIRST + 0)
#define HAM_GETACTIVE 					(HAM_FIRST + 1)
#define HAM_SETACTIVE 					(HAM_FIRST + 2)
#define HAM_GETTABS     				(HAM_FIRST + 3)
#define HAM_SETTABS     				(HAM_FIRST + 4)
#define HAM_INSERTTAB  	  			(HAM_FIRST + 5)
#define HAM_DELETETAB     			(HAM_FIRST + 6)
#define HAM_ISTABENABLED		  	(HAM_FIRST + 7)
#define HAM_ENABLETAB      			(HAM_FIRST + 8)
#define HAM_ISTABVISIBLE  			(HAM_FIRST + 9)
#define HAM_SHOWTAB      				(HAM_FIRST + 10)
#define HAM_GETALIGNMENT				(HAM_FIRST + 11)
#define HAM_SETALIGNMENT				(HAM_FIRST + 12)
#define HAM_GETBACKCOLOR				(HAM_FIRST + 13)
#define HAM_SETBACKCOLOR				(HAM_FIRST + 14)
#define HAM_SETCAPTION					(HAM_FIRST + 15)
#define HAM_GETCAPTIONLEN				(HAM_FIRST + 16)
#define HAM_GETCAPTION					(HAM_FIRST + 17)
#define HAM_GETFORECOLOR				(HAM_FIRST + 18)
#define HAM_SETFORECOLOR				(HAM_FIRST + 19)
#define HAM_GETCARDCOLOR				(HAM_FIRST + 20)
#define HAM_SETCARDCOLOR				(HAM_FIRST + 21)
#define HAM_ISINITIALIZED  			(HAM_FIRST + 22)
#define HAM_GETTEXTINDENT				(HAM_FIRST + 23)    
#define HAM_SETTEXTINDENT				(HAM_FIRST + 24)    

#define HAM_RESERVED1						(HAM_FIRST + 40)
#define HAM_RESERVED2						(HAM_FIRST + 41)
#define HAM_GETCARDRECT 				(HAM_FIRST + 42)

// Macros
#define HAGetActive(hwnd)							((UINT)SM((hwnd), HAM_GETACTIVE, 0, 0L))
#define HASetActive(hwnd,tab)					(SM((hwnd), HAM_SETACTIVE, (WPARAM)(tab), 0L) != 0L)
#define HAClickTab(hwnd,tab)					(SM((hwnd), HAM_SETACTIVE, (WPARAM)(tab), TRUE) != 0L)
#define HAInsertTab(hwnd,pos,caption)	(SM((hwnd), HAM_INSERTTAB, (WPARAM)(pos), DLP(caption)) != 0L)
#define HARemoveTab(hwnd,pos)					(SM((hwnd), HAM_DELETETAB, (WPARAM)(pos), 0L) != 0L)
#define HARefresh(hwnd)								(SM((hwnd), HAM_REFRESH, 0, 0L))

// Constants
#define ALLTABS									-1
#define TAB_ALIGN_CENTER				0
#define TAB_ALIGN_LEFT					1
#define TAB_ALIGN_RIGHT					2

// Notification Codes


#define HAN_INITIALIZE					1
#define HAN_ACTIVATE						2
#define HAN_DEACTIVATE					3
#define HAN_INITTABPROPS				4


/***************************************************************************\
  The HSpin control
\***************************************************************************/
#define HPM_FIRST									(WM_USER+450)
#define HPM_INC										(HPM_FIRST+0)
#define HPM_DEC       						(HPM_FIRST+1)
#define HPM_GETINC								(HPM_FIRST+2)
#define HPM_SETINC								(HPM_FIRST+3)
#define HPM_SETGRABBAR            (HPM_FIRST+4)
#define HPM_GETBKGNDBRUSH         HEM_GETBKGNDBRUSH     
#define HPM_SETBKGNDBRUSH         HEM_SETBKGNDBRUSH     
#define HPM_GETDATA               HEM_GETDATA           
#define HPM_SETDATA               HEM_SETDATA           
#define HPM_GETDATACLASS          HEM_GETDATACLASS      
#define HPM_GETDATALINK           HEM_GETDATALINK       
#define HPM_SETDATALINK           HEM_SETDATALINK       
#define HPM_GETDATASIZE           HEM_GETDATASIZE       
#define HPM_GETDATATYPE           HEM_GETDATATYPE       
#define HPM_GETFONT               HEM_GETFONT           
#define HPM_SETFONT               HEM_SETFONT           
#define HPM_GETFORMAT             HEM_GETFORMAT         
#define HPM_SETFORMAT             HEM_SETFORMAT         
#define HPM_GETFORMATRECT         HEM_GETFORMATRECT     
#define HPM_SETFORMATRECT         HEM_SETFORMATRECT     
#define HPM_GETHILITEBRUSH        HEM_GETHILITEBRUSH    
#define HPM_SETHILITEBRUSH        HEM_SETHILITEBRUSH    
#define HPM_GETMAXTEXTLEN         HEM_GETMAXTEXTLEN     
#define HPM_SETMAXTEXTLEN         HEM_SETMAXTEXTLEN     
#define HPM_GETOVERWRITEMODE      HEM_GETOVERWRITEMODE  
#define HPM_SETOVERWRITEMODE      HEM_SETOVERWRITEMODE  
#define HPM_GETSCROLLPOS          HEM_GETSCROLLPOS      
#define HPM_SETSCROLLPOS          HEM_SETSCROLLPOS      
#define HPM_GETSEL                HEM_GETSEL            
#define HPM_SETSEL                HEM_SETSEL            
#define HPM_GETSTATE              HEM_GETSTATE          
#define HPM_SETSTATE              HEM_SETSTATE          
#define HPM_GETTEXT               HEM_GETTEXT           
#define HPM_SETTEXT               HEM_SETTEXT           
#define HPM_GETTEXTCOLOR          HEM_GETTEXTCOLOR      
#define HPM_SETTEXTCOLOR          HEM_SETTEXTCOLOR      
#define HPM_GETTEXTLEN            HEM_GETTEXTLEN        
#define HPM_UPDATE                HEM_UPDATE            
#define HPM_HASCHANGED            HEM_HASCHANGED        
#define HPM_SETCHANGED            HEM_SETCHANGED        
#define HPM_GETVALIDATE           HEM_GETVALIDATE       
#define HPM_SETVALIDATE           HEM_SETVALIDATE       
#define HPM_VALIDATE              HEM_VALIDATE          
#define HPM_BEQUIET               HEM_BEQUIET           
#define HPM_ISQUIET               HEM_ISQUIET           
#define HPM_GETSELTEXT            HEM_GETSELTEXT        
#define HPM_REPLACESEL            HEM_REPLACESEL        
#define HPM_GETMAX					      HEM_GETMAX						
#define HPM_SETMAX					      HEM_SETMAX						
#define HPM_GETMIN					      HEM_GETMIN						
#define HPM_SETMIN					      HEM_SETMIN						

#define HPN_INC       HEN_LAST + 0x0001
#define HPN_DEC       HEN_LAST + 0x0002
#define HPN_MIN 			HEN_MIN
#define HPN_MAX 			HEN_MAX 

//STYLES
#define HPS_DISPLAYLEFT    	HES_DISPLAYLEFT   
#define HPS_DISPLAYCENTER  	HES_DISPLAYCENTER 
#define HPS_DISPLAYRIGHT   	HES_DISPLAYRIGHT  
#define HPS_UPPERCASE      	HES_UPPERCASE     
#define HPS_LOWERCASE      	HES_LOWERCASE     
#define HPS_AUTOHSCROLL    	HES_AUTOHSCROLL   
#define HPS_NOHIDESEL      	HES_NOHIDESEL     

#define HPS_BORDER3D       	HES_BORDER3D       // 3-D indented border
#define HPS_INDENT         	HES_INDENT         // 3-D indented border
#define HPS_EXTRUDE        	HES_EXTRUDE        // 3-D extruded border
#define HPS_HILITE         	HES_HILITE         // changes background color on SETFOCUS

#define HPS_EDITLEFT       	HES_EDITLEFT      
#define HPS_EDITRIGHT      	HES_EDITRIGHT     
#define HPS_HUNGRY         	HES_HUNGRY        
#define HPS_READONLY			 	HES_READONLY
#define HPS_GRABBAR					HES_PASSWORD 				//OVERLOADED STYLE BIT !!!				


/***************************************************************************\
  The XTable control
\***************************************************************************/


#ifdef __cplusplus
  }
#endif

#endif // WIDGETS_H
