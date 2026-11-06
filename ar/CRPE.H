/*
** File:    CRPE.h
**
** Author:  Rick Cameron, Ron Hayter
** Date:    13 Nov 91
**
** Purpose: This file presents the API to the Crystal Reports
**          Print Engine DLL.
**
** Copyright (c) 1991  Crystal Services
*/

#if !defined (CRPE_H)
#define CRPE_H

#include <Print.h>

#if defined (__cplusplus)
extern "C"
{
#endif

#define PE_SF_MAX_NAME_LENGTH 50

#define PE_SF_DESCENDING 0
#define PE_SF_ASCENDING  1

#define PE_ERR_NOERROR              0

#define PE_ERR_NOTENOUGHMEMORY      500
#define PE_ERR_INVALIDJOBNO         501
#define PE_ERR_INVALIDHANDLE        502
#define PE_ERR_STRINGTOOLONG        503
#define PE_ERR_NOSUCHREPORT         504
#define PE_ERR_NODESTINATION        505
#define PE_ERR_BADFILENUMBER        506
#define PE_ERR_BADFILENAME          507
#define PE_ERR_BADFIELDNUMBER       508
#define PE_ERR_BADFIELDNAME         509
#define PE_ERR_BADFORMULANAME       510
#define PE_ERR_BADSORTDIRECTION     511
#define PE_ERR_ENGINENOTOPEN        512
#define PE_ERR_INVALIDPRINTER       513
#define PE_ERR_PRINTFILEEXISTS      514
#define PE_ERR_BADFORMULATEXT       515
#define PE_ERR_BADGROUPSECTION      516
#define PE_ERR_ENGINEBUSY           517
#define PE_ERR_BADSECTION           518
#define PE_ERR_NOPRINTWINDOW        519
#define PE_ERR_JOBALREADYSTARTED    520
#define PE_ERR_BADSUMMARYFIELD      521
#define PE_ERR_NOTENOUGHSYSRES      522
#define PE_ERR_BADGROUPCONDITION    523
#define PE_ERR_JOBBUSY              524
#define PE_ERR_BADREPORTFILE        525
#define PE_ERR_NODEFAULTPRINTER     526
#define PE_ERR_SQLSERVERERROR       527
#define PE_ERR_BADLINENUMBER        528
#define PE_ERR_DISKFULL             529
#define PE_ERR_FILEERROR            530
#define PE_ERR_INCORRECTPASSWORD    531
#define PE_ERR_BADDATABASEDLL       532
#define PE_ERR_BADDATABASEFILE      533
#define PE_ERR_ERRORINDATABASEDLL   534
#define PE_ERR_DATABASESESSION      535
#define PE_ERR_DATABASELOGON        536
#define PE_ERR_DATABASELOCATION     537
#define PE_ERR_BADSTRUCTSIZE        538
#define PE_ERR_BADDATE              539
#define PE_ERR_BADEXPORTDLL         540
#define PE_ERR_ERRORINEXPORTDLL     541
#define PE_ERR_PREVATFIRSTPAGE      542
#define PE_ERR_NEXTATLASTPAGE       543
#define PE_ERR_CANNOTACCESSREPORT   544
#define PE_ERR_USERCANCELLED        545
#define PE_ERR_OLE2NOTLOADED        546
#define PE_ERR_BADCROSSTABGROUP     547
#define PE_ERR_NOCTSUMMARIZEDFIELD  548
#define PE_ERR_DESTINATIONNOTEXPORT 549

#define PE_ERR_NOTIMPLEMENTED       999

short FAR PASCAL PEPrintReport (char FAR *reportFilePath,
                                BOOL  toDefaultPrinter,
                                BOOL  toWindow, char FAR *title,
                                                short left,
                                                short top,
                                                short width,
                                                short height,
                                                long  style,
                                                HWND parentWindow);

BOOL FAR PASCAL PEOpenEngine (void);
void FAR PASCAL PECloseEngine (void);

#define PE_GV_DLL     100
#define PE_GV_ENGINE  200

short FAR PASCAL PEGetVersion (short versionRequested);

short FAR PASCAL PEOpenPrintJob (char FAR *reportFilePath);
BOOL FAR PASCAL PEClosePrintJob (short printJob);
BOOL FAR PASCAL PEStartPrintJob (short printJob,
                                 BOOL waitUntilDone);
void FAR PASCAL PECancelPrintJob (short printJob);

BOOL FAR PASCAL PEShowNextPage (short printJob);
BOOL FAR PASCAL PEShowFirstPage (short printJob);
BOOL FAR PASCAL PEShowPreviousPage (short printJob);
BOOL FAR PASCAL PEShowLastPage (short printJob);
BOOL FAR PASCAL PEShowPrintControls (short printJob,
                                     BOOL showPrintControls);

BOOL FAR PASCAL PEIsPrintJobFinished (short printJob);

#define PE_JOBNOTSTARTED 1
#define PE_JOBINPROGRESS 2
#define PE_JOBCOMPLETED  3
#define PE_JOBFAILED     4
#define PE_JOBCANCELLED  5

struct PEJobInfo
{
    WORD StructSize;    // initialize to sizeof (PEJobInfo)

    DWORD NumRecordsRead,
          NumRecordsSelected,
          NumRecordsPrinted;
    WORD  DisplayPageN, // the page being displayed in window
          LatestPageN,  // the page being generated
          StartPageN;   // user opted, default to 1
    BOOL  printEnded;   // full report print completed?
};

short FAR PASCAL PEGetJobStatus (short printJob,
                                 struct PEJobInfo FAR *jobInfo);

BOOL FAR PASCAL PESetPrintDate (short printJob,
                                short year,
                                short month,
                                short day);

// Controlling database(s) accessed in a report
// --------------------------------------------
//
// The following functions allow retrieving and updating database info
// in an opened report, so that a report can be printed using different
// session, server, database, user and/or table location settings.  Any
// changes made to the report via these functions are not permanent, and
// only last as long as the report is open.
//
// The following database functions (except for PELogOnServer and
// PELogOffServer) must be called after PEOpenPrintJob and before
// PEStartPrintJob.

// The function PEGetNTables is called to fetch the number of tables in
// the report.  This includes all PC databases (e.g. Paradox, xBase)
// as well as SQL databases (e.g. SQL Server, Oracle, Netware).

short FAR PASCAL PEGetNTables (short printJob);


// The function PEGetNthTableType allows the application to determine the
// type of each table.  The application can test DBType (equal to
// PE_DT_STANDARD or PE_DT_SQL), or test the database DLL name used to
// create the report.  DLL names have the following naming convention:
//     - PDB*.DLL for standard (non-SQL) databases,
//     - PDS*.DLL for SQL databases.
//
// In the case of ODBC (PDSODBC.DLL) the DescriptiveName includes the
// ODBC data source name.

#define PE_WORD_LEN          2
#define PE_DLL_NAME_LEN      64
#define PE_FULL_NAME_LEN     256

#define PE_SIZEOF_TABLE_TYPE PE_WORD_LEN + \
                             PE_DLL_NAME_LEN + \
                             PE_FULL_NAME_LEN + \
                             PE_WORD_LEN

#define PE_DT_STANDARD  1
#define PE_DT_SQL       2

struct PETableType
{
    // Initialize to PE_SIZEOF_TABLE_TYPE.
    WORD StructSize;

    // All strings are null-terminated.
    char DLLName [PE_DLL_NAME_LEN];
    char DescriptiveName [PE_FULL_NAME_LEN];

    WORD DBType;
};

BOOL  FAR PASCAL PEGetNthTableType (short printJob,
                                    short tableN,
                                    struct PETableType FAR *tableType);


// RESERVED: The functions PEGetNthTableSessionInfo and
// PESetNthTableSessionInfo are reserved for MS Access DLLs.

#define PE_LONGPTR_LEN       4
#define PE_SESS_USERID_LEN   128
#define PE_SESS_PASSWORD_LEN 128

#define PE_SIZEOF_SESSION_INFO PE_WORD_LEN + \
                               PE_SESS_USERID_LEN + \
                               PE_SESS_PASSWORD_LEN + \
                               PE_LONGPTR_LEN

struct PESessionInfo
{
    // Initialize to PE_SIZEOF_SESSION_INFO.
    WORD StructSize;

    // All strings are null-terminated.
    char UserID [PE_SESS_PASSWORD_LEN];

    // Password is undefined when getting information from report.
    char Password [PE_SESS_PASSWORD_LEN];

    // SessionHandle is undefined when getting information from report.
    // When setting information, if it is = 0 the UserID and Password
    // settings are used, otherwise the SessionHandle is used.
    DWORD SessionHandle;
};

BOOL  FAR PASCAL PEGetNthTableSessionInfo (short printJob,
                                           short tableN,
                                           struct PESessionInfo FAR *sessionInfo);
BOOL  FAR PASCAL PESetNthTableSessionInfo (short printJob,
                                           short tableN,
                                           struct PESessionInfo FAR *sessionInfo,
                                           BOOL propagateAcrossTables);


// Logging on is performed when printing the report, but the correct
// log on information must first be set using PESetNthTableLogOnInfo.
// Only the password is required, but the server, database, and
// user names may optionally be overriden as well.
//
// If the parameter propagateAcrossTables is TRUE, the new log on info
// is also applied to any other tables in this report that had the
// same original server and database names as this table.  If FALSE
// only this table is updated.
//
// Logging off is performed automatically when the print job is closed.

#define PE_SERVERNAME_LEN    128
#define PE_DATABASENAME_LEN  128
#define PE_USERID_LEN        128
#define PE_PASSWORD_LEN      128

#define PE_SIZEOF_LOGON_INFO PE_WORD_LEN + \
                             PE_SERVERNAME_LEN + \
                             PE_DATABASENAME_LEN + \
                             PE_USERID_LEN + \
                             PE_PASSWORD_LEN

struct PELogOnInfo
{
    // Initialize to PE_SIZEOF_LOGON_INFO.
    WORD StructSize;

    // For any of the following values an empty string ("") means to use
    // the value already set in the report.  To override a value in the
    // report use a non-empty string (e.g. "Server A").  All strings are
    // null-terminated.
    //
    // For Netware SQL, pass the dictionary path name in ServerName and
    // data path name in DatabaseName.
    char ServerName [PE_SERVERNAME_LEN];
    char DatabaseName [PE_DATABASENAME_LEN];
    char UserID [PE_USERID_LEN];

    // Password is undefined when getting information from report.
    char Password [PE_PASSWORD_LEN];
};

BOOL  FAR PASCAL PEGetNthTableLogOnInfo (short printJob,
                                         short tableN,
                                         struct PELogOnInfo FAR *logOnInfo);
BOOL  FAR PASCAL PESetNthTableLogOnInfo (short printJob,
                                         short tableN,
                                         struct PELogOnInfo FAR *logOnInfo,
                                         BOOL propagateAcrossTables);


// A table's location is fetched and set using PEGetNthTableLocation and
// PESetNthTableLocation.  This name is database-dependent, and must be
// formatted correctly for the expected database.  For example:
//     - Paradox: "c:\crw\ORDERS.DB"
//     - SQL Server: "publications.dbo.authors"

#define PE_TABLE_LOCATION_LEN      256

#define PE_SIZEOF_TABLE_LOCATION PE_WORD_LEN + \
                                 PE_TABLE_LOCATION_LEN

struct PETableLocation
{
    // Initialize to PE_SIZEOF_TABLE_LOCATION.
    WORD StructSize;

    // String is null-terminated.
    char Location [PE_TABLE_LOCATION_LEN];
};

BOOL  FAR PASCAL PEGetNthTableLocation (short printJob,
                                        short tableN,
                                        struct PETableLocation FAR *location);
BOOL  FAR PASCAL PESetNthTableLocation (short printJob,
                                        short tableN,
                                        struct PETableLocation FAR *location);


// The function PETestNthTableConnectivity tests whether a database
// table's settings are valid and ready to be reported on.  It returns
// true if the database session, log on, and location info is all
// correct.
//
// This is useful, for example, in prompting the user and testing a
// server password before printing begins.
//
// This function may require a significant amount of time to complete,
// since it will first open a user session (if required), then log onto
// the database server (if required), and then open the appropriate
// database table (to test that it exists).  It does not read any data,
// and closes the table immediately once successful.  Logging off is
// performed when the print job is closed.
//
// If it fails in any of these steps, the error code set indicates
// which database info needs to be updated using functions above:
//    - If it is unable to begin a session, PE_ERR_DATABASESESSION is set,
//      and the application should update with PESetNthTableSessionInfo.
//    - If it is unable to log onto a server, PE_ERR_DATABASELOGON is set,
//      and the application should update with PESetNthTableLogOnInfo.
//    - If it is unable open the table, PE_ERR_DATABASELOCATION is set,
//      and the application should update with PESetNthTableLocation.

BOOL  FAR PASCAL PETestNthTableConnectivity (short printJob,
                                             short tableN);


// PELogOnServer and PELogOffServer can be called at any time to log on
// and off of a database server.  These functions are not required if
// function PESetNthTableLogOnInfo above was already used to set the
// password for a table.
//
// These functions require a database DLL name, which can be retrieved
// using PEGetNthTableType above.
//
// This function can also be used for non-SQL tables, such as password-
// protected Paradox tables.  Call this function to set the password
// for the Paradox DLL before beginning printing.
//
// Note: When printing using PEStartPrintJob the ServerName passed in
// PELogOnServer must agree exactly with the server name stored in the
// report.  If this is not true use PESetNthTableLogOnInfo to perform
// logging on instead.

BOOL FAR PASCAL PELogOnServer (char FAR *dllName,
                               struct PELogOnInfo FAR *logOnInfo);
BOOL FAR PASCAL PELogOffServer (char FAR *dllName,
                                struct PELogOnInfo FAR *logOnInfo);


BOOL FAR PASCAL PEGetReportTitle (short printJob,
                                  HANDLE FAR *titleHandle,
                                  short  FAR *titleLength);
BOOL FAR PASCAL PESetReportTitle (short printJob,
                                  char FAR *title);

short FAR PASCAL PEGetNFiles (short printJob);
BOOL  FAR PASCAL PEGetNthFileName (short printJob,
                                   short fileN,
                                   HANDLE FAR *nameHandle,
                                   short  FAR *nameLength);
BOOL  FAR PASCAL PESetNthFileName (short printJob,
                                   short fileN,
                                   char FAR *name);

BOOL FAR PASCAL PESelectPrinter (short printJob,
                                 char FAR *driverName,
                                 char FAR *printerName,
                                 char FAR *portName,
                                 DEVMODE FAR *mode);

#define PE_SM_DEFAULT  0x8000

BOOL FAR PASCAL PESetMargins (short printJob,
                              short left,
                              short right,
                              short top,
                              short bottom);

// PESetRefreshData () is used when saved data is stored with the report
// or the report is printed more than once in a print job (since data is
// saved during the first print).
//
// If refreshData is FALSE the saved data is used when printing the report,
// if refreshData is TRUE saved data is ignored and new data retrieved.
//
// This setting is FALSE by default.  This function should be called
// each time when printing more than once in a print job.
BOOL FAR PASCAL PESetRefreshData (short printJob,
                                  BOOL refreshData);

BOOL FAR PASCAL PEOutputToDefaultPrinter (short printJob,
                                          short nCopies);

BOOL FAR PASCAL PEOutputToPrinter (short printJob,
                                   short nCopies);

BOOL FAR PASCAL PEOutputToWindow (short printJob,
                                  char FAR *title,
                                  short left,
                                  short top,
                                  short width,
                                  short height,
                                  long  style,
                                  HWND parentWindow);

#define PE_FT_RECORD              0
#define PE_FT_TABSEPARATED        1
#define PE_FT_TEXT                2
#define PE_FT_DIF                 3
#define PE_FT_CSV                 4
#define PE_FT_CHARSEPARATED       5
#define PE_FT_TABFORMATTED        6

// Use for all types except PE_FT_CHARSEPARATED
struct PEPrintFileOptions
{
    WORD StructSize;        // initialize to sizeof (PEPrintFileOptions)
    BOOL UseReportNumberFmt,
         UseReportDateFormat;
};

#define PE_FIELDDELIMLEN  16

// Use for PE_FT_CHARSEPARATED
struct PECharSepFileOptions
{
    WORD StructSize;        // initialize to sizeof (PECharSepFileOptions)
    BOOL UseReportNumberFmt,
         UseReportDateFormat;
    char StringDelimiter;
    char FieldDelimiter [PE_FIELDDELIMLEN + 1];
};

BOOL FAR PASCAL PEOutputToFile (short printJob,
                                char FAR *outputFilePath,
                                short type,
                                void FAR *options);

struct PEExportOptions
{
    WORD StructSize;               // initialize to sizeof (PEExportOptions)

    char formatDLLName [PE_DLL_NAME_LEN];
    DWORD formatType;
    void FAR *formatOptions;
    char destinationDLLName [PE_DLL_NAME_LEN];
    DWORD destinationType;
    void FAR *destinationOptions;
    WORD nFormatOptionsBytes;      // Set by 'PEGetExportOptions', 
                                   // ignored by 'PEExportTo'.
    WORD nDestinationOptionsBytes; // Set by 'PEGetExportOptions',
                                   // ignored by 'PEExportTo'.
};

BOOL FAR PASCAL PEGetExportOptions (short printJob,
                                    struct PEExportOptions FAR *options);

BOOL FAR PASCAL PEExportTo (short printJob,
                            struct PEExportOptions FAR *options);


#define PE_MAXPAGEN  65535

#define PE_UNCOLLATED       0
#define PE_COLLATED         1
#define PE_DEFAULTCOLLATION 2

struct PEPrintOptions
{
    WORD StructSize;            // initialize to sizeof (PEPrintOptions)

    // page and copy numbers are 1-origin
    // use 0 to preserve the existing settings
    unsigned short startPageN,
                   stopPageN;

    unsigned short nReportCopies;
    unsigned short collation;
};

BOOL FAR PASCAL PESetPrintOptions (short printJob,
                                   struct PEPrintOptions FAR *options);

HWND FAR PASCAL PEGetWindowHandle (short printJob);

BOOL FAR PASCAL PEPrintWindow (short printJob,
                               BOOL waitUntilDone);
void FAR PASCAL PECloseWindow (short printJob);

short FAR PASCAL PEGetNSortFields (short printJob);

BOOL FAR PASCAL PEGetNthSortField (short printJob,
                                   short sortFieldN,
                                   HANDLE FAR *nameHandle,
                                   short  FAR *nameLength,
                                   short  FAR *direction);

BOOL FAR PASCAL PESetNthSortField (short printJob,
                                   short sortFieldN,
                                   char FAR *name,
                                   short direction);

BOOL FAR PASCAL PEDeleteNthSortField (short printJob,
                                      short sortFieldN);

short FAR PASCAL PEGetNGroupSortFields (short printJob);

BOOL FAR PASCAL PEGetNthGroupSortField (short printJob,
                                        short sortFieldN,
                                        HANDLE FAR *nameHandle,
                                        short  FAR *nameLength,
                                        short  FAR *direction);

BOOL FAR PASCAL PESetNthGroupSortField (short printJob,
                                        short sortFieldN,
                                        char FAR *name,
                                        short direction);

BOOL FAR PASCAL PEDeleteNthGroupSortField (short printJob,
                                           short sortFieldN);

BOOL FAR PASCAL PEGetFormula (short printJob,
                              char FAR *formulaName,
                              HANDLE FAR *textHandle,
                              short  FAR *textLength);

BOOL FAR PASCAL PESetFormula (short printJob,
                              char FAR *formulaName,
                              char FAR *formulaString);

BOOL FAR PASCAL PEGetSelectionFormula (short printJob,
                                       HANDLE FAR *textHandle,
                                       short  FAR *textLength);

BOOL FAR PASCAL PESetSelectionFormula (short printJob,
                                       char FAR *formulaString);

BOOL FAR PASCAL PEGetGroupSelectionFormula (short printJob,
                                            HANDLE FAR *textHandle,
                                            short  FAR *textLength);

BOOL FAR PASCAL PESetGroupSelectionFormula (short printJob,
                                            char FAR *formulaString);

#define PE_ALLSECTIONS           0
#define PE_HEADERSECTION      2000
#define PE_GROUPHEADER        3000
#define PE_DETAILSECTION      4000
#define PE_GROUPFOOTER        5000
#define PE_GRANDTOTALSECTION  6000
#define PE_FOOTERSECTION      7000

BOOL FAR PASCAL PESetMinimumSectionHeight (short printJob,
                                           short sectionCode,
                                           short minimumHeight);  // in twips

struct PESectionOptions
{
    WORD StructSize;            // initialize to sizeof (PESectionOptions)

    short visible,              // BOOLEAN values, except use PE_UNCHANGED
          newPageBefore,        // to preserve the existing settings
          newPageAfter,
          keepTogether,
          suppressBlankLines,
          resetPageNAfter,
          printAtBottomOfPage;
};

BOOL FAR PASCAL PESetSectionFormat (short printJob,
                                    short sectionCode,
                                    struct PESectionOptions FAR *options);

#define PE_ALLLINES  -1

BOOL FAR PASCAL PESetLineHeight (short printJob,
                                 short sectionCode,
                                 short lineN,      // use PE_ALLLINES for all lines
                                 short height,     // in twips
                                 short ascent);    // in twips


#define PE_FIELDS    0x0001
#define PE_TEXT      0x0002

#define PE_UNCHANGED -1

BOOL FAR PASCAL PESetFont (short printJob,
                           short sectionCode,
                           short scopeCode,
                           char FAR *faceName,  // 0 for no change
                           short fontFamily,    // FF_DONTCARE for no change
                           short fontPitch,     // DEFAULT_PITCH for no change
                           short charSet,       // DEFAULT_CHARSET for no change
                           short pointSize,     // 0 for no change
                           short isItalic,      // PE_UNCHANGED for no change
                           short isUnderlined,  // ditto
                           short isStruckOut,   // ditto
                           short weight);       // 0 for no change

BOOL FAR PASCAL PESetNDetailCopies (short printJob,
                                    short nDetailCopies);

// use PE_ANYCHANGE for all field types except Date and Boolean
#define PE_GC_ANYCHANGE    0

// use these constants for Date fields
#define PE_GC_DAILY        0
#define PE_GC_WEEKLY       1
#define PE_GC_BIWEEKLY     2
#define PE_GC_SEMIMONTHLY  3
#define PE_GC_MONTHLY      4
#define PE_GC_QUARTERLY    5
#define PE_GC_SEMIANNUALLY 6
#define PE_GC_ANNUALLY     7

// use these constants for Boolean fields
#define PE_GC_TOYES        1
#define PE_GC_TONO         2
#define PE_GC_EVERYYES     3
#define PE_GC_EVERYNO      4
#define PE_GC_NEXTISYES    5
#define PE_GC_NEXTISNO     6

BOOL FAR PASCAL PESetGroupCondition (short printJob,
                                     short sectionCode,
                                     char FAR *conditionField,
                                     short condition,       // a PE_GC_ constant
                                     short sortDirection);  // a PE_SF_ constant

short FAR PASCAL PEGetErrorCode (short printJob);
BOOL  FAR PASCAL PEGetErrorText (short printJob,
                                 HANDLE FAR *textHandle,
                                 short  FAR *textLength);

BOOL  FAR PASCAL PEGetHandleString (HANDLE textHandle,
                                    char FAR *buffer,
                                    short bufferLength);

#if defined (__cplusplus)
}
#endif

#endif // CRPE_H
