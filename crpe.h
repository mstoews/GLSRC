/*
** File:    CRPE.h
**
** Authors: Seagate Software Information Management Group, Inc.
** Date:    13 Nov 91
**
** Purpose: This file presents the API to the Crystal Reports
**          Print Engine DLL.
**
** Copyright (c) 1991-1996  Seagate Software Information Management Group, Inc.
*/

#if !defined (CRPE_H)
#define CRPE_H

// Set 1-byte structure alignment
#if defined (__BORLANDC__)               // Borland C/C++
  #pragma option -a-
  #pragma comment (lib, "crpe32.lib")    // Point to Borland Lib File
#elif defined (_MSC_VER)                 // Microsoft Visual C++
  #if _MSC_VER >= 900                    // MSVC 2.x and later
    #pragma pack (push)
    #pragma comment (lib, "crpe32m.lib") // Point to Microsoft Lib File
  #endif
  #pragma pack (1)
#endif

#if !defined (WIN32)
  #include <Print.h>
#endif

#if defined (__cplusplus)
extern "C"
{
#endif

#define PE_SF_MAX_NAME_LENGTH 50

#define PE_SF_DESCENDING 0
#define PE_SF_ASCENDING  1

#define PE_UNCHANGED       -1
#define PE_UNCHANGED_COLOR (COLORREF) -2

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
#define PE_ERR_INVALIDPAGENUMBER    550
#define PE_ERR_NOTSTOREDPROCEDURE   552
#define PE_ERR_INVALIDPARAMETER     553
#define PE_ERR_GRAPHNOTFOUND        554
#define PE_ERR_INVALIDGRAPHTYPE     555
#define PE_ERR_INVALIDGRAPHDATA     556
#define PE_ERR_CANNOTMOVEGRAPH      557
#define PE_ERR_INVALIDGRAPHTEXT     558
#define PE_ERR_INVALIDGRAPHOPT      559
#define PE_ERR_BADSECTIONHEIGHT     560
#define PE_ERR_BADVALUETYPE         561
#define PE_ERR_INVALIDSUBREPORTNAME 562
#define PE_ERR_NOPARENTWINDOW       564 // dialog parent window
#define PE_ERR_INVALIDZOOMFACTOR    565 // zoom factor
#define PE_ERR_PAGESIZEOVERFLOW     567
#define PE_ERR_LOWSYSTEMRESOURCES   568

#define PE_ERR_NOTIMPLEMENTED       999
                
#if defined (WIN32)
  #if defined (_WINDLL)
    #define CRPE_API /* __declspec(dllexport) */ WINAPI
  #else
    #define CRPE_API WINAPI
  #endif
#else
  #define CRPE_API FAR PASCAL _export
#endif


// Open, print and close report (used when no changes needed to report)
// --------------------------------------------------------------------

short CRPE_API PEPrintReport (char FAR *reportFilePath,
                              BOOL toDefaultPrinter,
                              BOOL toWindow, char FAR *title,
                                             int left,
                                             int top,
                                             int width,
                                             int height,
                                             DWORD style,
                                             HWND parentWindow);


// Open and close print engine
// ---------------------------

BOOL CRPE_API PEOpenEngine (void);
void CRPE_API PECloseEngine (void);
BOOL CRPE_API PECanCloseEngine (void);


// Get version info
// ----------------

#define PE_GV_DLL     100
#define PE_GV_ENGINE  200

short CRPE_API PEGetVersion (short versionRequested);


// Open and close print job (i.e. report)
// --------------------------------------

short CRPE_API PEOpenPrintJob (char FAR *reportFilePath);

BOOL CRPE_API PEClosePrintJob (short printJob);


// Start and cancel print job (i.e. print the report, usually after changing report)
// ---------------------------------------------------------------------------------

BOOL CRPE_API PEStartPrintJob (short printJob,
                               BOOL waitUntilDone);
void CRPE_API PECancelPrintJob (short printJob);


// Print job status
// ----------------

BOOL CRPE_API PEIsPrintJobFinished (short printJob);

#define PE_JOBNOTSTARTED 1
#define PE_JOBINPROGRESS 2
#define PE_JOBCOMPLETED  3
#define PE_JOBFAILED     4  // an error occurred
#define PE_JOBCANCELLED  5  // by user
#define PE_JOBHALTED     6  // too many records or too much time

typedef struct PEJobInfo
{
    WORD StructSize;    // initialize to PE_SIZEOF_JOB_INFO

    DWORD NumRecordsRead,
          NumRecordsSelected,
          NumRecordsPrinted;
    WORD  DisplayPageN, // the page being displayed in window
          LatestPageN,  // the page being generated
          StartPageN;   // user opted, default to 1
    BOOL  printEnded;   // full report print completed?
}
    PEJobInfo;
#define PE_SIZEOF_JOB_INFO (sizeof (PEJobInfo))

short CRPE_API PEGetJobStatus (short printJob,
                               PEJobInfo FAR *jobInfo);


// Controlling dialogs
// -------------------

BOOL CRPE_API PESetDialogParentWindow (short printJob,
                                       HWND parentWindow);

BOOL CRPE_API PEEnableProgressDialog (short printJob,
                                      BOOL enable);


// Print job error codes and messages
// ----------------------------------

short CRPE_API PEGetErrorCode (short printJob);
BOOL  CRPE_API PEGetErrorText (short printJob,
                               HANDLE FAR *textHandle,
                               short  FAR *textLength);

BOOL  CRPE_API PEGetHandleString (HANDLE textHandle,
                                  char FAR *buffer,
                                  short bufferLength);


// Setting the print date
// ----------------------

BOOL CRPE_API PEGetPrintDate (short printJob,
                              short FAR *year,
                              short FAR *month,
                              short FAR *day);

BOOL CRPE_API PESetPrintDate (short printJob,
                              short year,
                              short month,
                              short day);


// Encoding and Decoding Section Codes
// -----------------------------------

#define PE_ALLSECTIONS           0

// A macro to create section codes:
// (This representation allows up to 25 groups and 40 sections of a given
// type, although Crystal Reports itself has no such limitations.)
#define PE_SECTION_CODE(sectionType,groupN,sectionN) \
    (((sectionType) * 1000) + ((groupN) % 25) + (((sectionN) % 40) * 25))

// Section types:
#define PE_SECT_PAGE_HEADER      2
#define PE_SECT_PAGE_FOOTER      7
#define PE_SECT_REPORT_HEADER    1
#define PE_SECT_REPORT_FOOTER    8
#define PE_SECT_GROUP_HEADER     3
#define PE_SECT_GROUP_FOOTER     5
#define PE_SECT_DETAIL           4

// Macros to decode section codes:
#define PE_SECTION_TYPE(sectionCode) ((sectionCode) / 1000)
#define PE_GROUP_N(sectionCode)      ((sectionCode) % 25)
#define PE_SECTION_N(sectionCode)    (((sectionCode) / 25) % 40)

// The old section constants redefined in terms of the new:
// (Note that PE_GRANDTOTALSECTION and PE_SUMMARYSECTION both map
// to PE_SECT_REPORT_FOOTER.)
#define PE_HEADERSECTION      PE_SECTION_CODE (PE_SECT_PAGE_HEADER,   0, 0)
#define PE_FOOTERSECTION      PE_SECTION_CODE (PE_SECT_PAGE_FOOTER,   0, 0)
#define PE_TITLESECTION       PE_SECTION_CODE (PE_SECT_REPORT_HEADER, 0, 0)
#define PE_SUMMARYSECTION     PE_SECTION_CODE (PE_SECT_REPORT_FOOTER, 0, 0)
#define PE_GROUPHEADER        PE_SECTION_CODE (PE_SECT_GROUP_HEADER,  0, 0)
#define PE_GROUPFOOTER        PE_SECTION_CODE (PE_SECT_GROUP_FOOTER,  0, 0)
#define PE_DETAILSECTION      PE_SECTION_CODE (PE_SECT_DETAIL,        0, 0)
#define PE_GRANDTOTALSECTION  PE_SUMMARYSECTION


// Controlling group conditions (i.e. group breaks)
// ------------------------------------------------

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

BOOL CRPE_API PESetGroupCondition (short printJob,
                                   short sectionCode,
                                   char FAR *conditionField,
                                   short condition,       // a PE_GC_ constant
                                   short sortDirection);  // a PE_SF_ constant

short CRPE_API PEGetNGroups (short printJob);

// for PEGetGroupCondition, condition encodes both
// the condition and the type of the condition field
#define PE_GC_CONDITIONMASK   0x00FF
#define PE_GC_TYPEMASK        0x0F00

#define PE_GC_TYPEOTHER       0x0000
#define PE_GC_TYPEDATE        0x0200
#define PE_GC_TYPEBOOLEAN     0x0400

BOOL CRPE_API PEGetGroupCondition (short printJob,
                                   short sectionCode,
                                   HANDLE FAR *conditionFieldHandle,
                                   short  FAR *conditionFieldLength,
                                   short  FAR *condition,
                                   short  FAR *sortDirection);

// Controlling formulas, selection formulas and group selection formulas
// ---------------------------------------------------------------------

short CRPE_API PEGetNFormulas (short printJob);

BOOL CRPE_API PEGetNthFormula (short printJob,
                               short formulaN,
                               HANDLE FAR *nameHandle,
                               short FAR *nameLength,
                               HANDLE FAR *textHandle,
                               short FAR *textLength);

BOOL CRPE_API PEGetFormula (short printJob,
                            char FAR *formulaName,
                            HANDLE FAR *textHandle,
                            short FAR *textLength);

BOOL CRPE_API PESetFormula (short printJob,
                            char FAR *formulaName,
                            char FAR *formulaString);

BOOL CRPE_API PECheckFormula (short printJob,
                              char FAR *formulaName);

BOOL CRPE_API PEGetSelectionFormula (short printJob,
                                     HANDLE FAR *textHandle,
                                     short FAR *textLength);

BOOL CRPE_API PESetSelectionFormula (short printJob,
                                     char FAR *formulaString);

BOOL CRPE_API PECheckSelectionFormula (short printJob);

BOOL CRPE_API PEGetGroupSelectionFormula (short printJob,
                                          HANDLE FAR *textHandle,
                                          short FAR *textLength);

BOOL CRPE_API PESetGroupSelectionFormula (short printJob,
                                          char FAR *formulaString);

BOOL CRPE_API PECheckGroupSelectionFormula (short printJob);


// Controlling parameter fields
// -------------------------------

#define PE_WORD_LEN           2
#define PE_PF_NAME_LEN        256
#define PE_PF_PROMPT_LEN      256
#define PE_PF_VALUE_LEN       256

#define PE_PF_NUMBER          0
#define PE_PF_CURRENCY        1
#define PE_PF_BOOLEAN         2
#define PE_PF_DATE            3
#define PE_PF_STRING          4

typedef struct PEParameterFieldInfo
{
    // Initialize to PE_SIZEOF_PARAMETER_FIELD_INFO.
    WORD StructSize;

    // PE_PF_ constant
    WORD ValueType;

    // Indicate the default value is set in PEParameterFieldInfo.
    WORD DefaultValueSet;

    // Indicate the current value is set in PEParameterFieldInfo.
    WORD CurrentValueSet;

    // All strings are null-terminated.
    char Name [PE_PF_NAME_LEN];
    char Prompt [PE_PF_PROMPT_LEN];

    // Could be Number, Currency, Boolean, Date, DateTime, Time, or String
    char DefaultValue [PE_PF_VALUE_LEN];
    char CurrentValue [PE_PF_VALUE_LEN];
}
    PEParameterFieldInfo;
#define PE_SIZEOF_PARAMETER_FIELD_INFO (sizeof (PEParameterFieldInfo))

short CRPE_API PEGetNParameterFields (short printJob);

BOOL CRPE_API PEGetNthParameterField (short printJob,
                                      short parameterN,
                                      PEParameterFieldInfo FAR *parameterInfo);

BOOL CRPE_API PESetNthParameterField (short printJob,
                                      short parameterN,
                                      PEParameterFieldInfo FAR *parameterInfo);

// Controlling sort order and group sort order
// -------------------------------------------

short CRPE_API PEGetNSortFields (short printJob);

BOOL CRPE_API PEGetNthSortField (short printJob,
                                 short sortFieldN,
                                 HANDLE FAR *nameHandle,
                                 short  FAR *nameLength,
                                 short  FAR *direction);

BOOL CRPE_API PESetNthSortField (short printJob,
                                 short sortFieldN,
                                 char FAR *name,
                                 short direction);

BOOL CRPE_API PEDeleteNthSortField (short printJob,
                                    short sortFieldN);

short CRPE_API PEGetNGroupSortFields (short printJob);

BOOL CRPE_API PEGetNthGroupSortField (short printJob,
                                      short sortFieldN,
                                      HANDLE FAR *nameHandle,
                                      short  FAR *nameLength,
                                      short  FAR *direction);

BOOL CRPE_API PESetNthGroupSortField (short printJob,
                                      short sortFieldN,
                                      char FAR *name,
                                      short direction);

BOOL CRPE_API PEDeleteNthGroupSortField (short printJob,
                                         short sortFieldN);


// Controlling databases
// ---------------------
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

short CRPE_API PEGetNTables (short printJob);

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

#define PE_DT_STANDARD  1
#define PE_DT_SQL       2

typedef struct PETableType
{
    // Initialize to PE_SIZEOF_TABLE_TYPE.
    WORD StructSize;

    // All strings are null-terminated.
    char DLLName [PE_DLL_NAME_LEN];
    char DescriptiveName [PE_FULL_NAME_LEN];

    WORD DBType;
}
    PETableType;
#define PE_SIZEOF_TABLE_TYPE (sizeof (PETableType))


BOOL  CRPE_API PEGetNthTableType (short printJob,
                                  short tableN,
                                  PETableType FAR *tableType);


// The functions PEGetNthTableSessionInfo and PESetNthTableSessionInfo
// are only used when connecting to MS Access databases (which require a
// session to be opened first)

#define PE_LONGPTR_LEN       4
#define PE_SESS_USERID_LEN   128
#define PE_SESS_PASSWORD_LEN 128

typedef struct PESessionInfo
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
}
    PESessionInfo;
#define PE_SIZEOF_SESSION_INFO (sizeof (PESessionInfo))


BOOL  CRPE_API PEGetNthTableSessionInfo (short printJob,
                                         short tableN,
                                         PESessionInfo FAR *sessionInfo);
BOOL  CRPE_API PESetNthTableSessionInfo (short printJob,
                                         short tableN,
                                         PESessionInfo FAR *sessionInfo,
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

typedef struct PELogOnInfo
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
}
    PELogOnInfo;
#define PE_SIZEOF_LOGON_INFO (sizeof (PELogOnInfo))


BOOL  CRPE_API PEGetNthTableLogOnInfo (short printJob,
                                       short tableN,
                                       PELogOnInfo FAR *logOnInfo);
BOOL  CRPE_API PESetNthTableLogOnInfo (short printJob,
                                       short tableN,
                                       PELogOnInfo FAR *logOnInfo,
                                       BOOL propagateAcrossTables);


// A table's location is fetched and set using PEGetNthTableLocation and
// PESetNthTableLocation.  This name is database-dependent, and must be
// formatted correctly for the expected database.  For example:
//     - Paradox: "c:\crw\ORDERS.DB"
//     - SQL Server: "publications.dbo.authors"

#define PE_TABLE_LOCATION_LEN      256

typedef struct PETableLocation
{
    // Initialize to PE_SIZEOF_TABLE_LOCATION.
    WORD StructSize;

    // String is null-terminated.
    char Location [PE_TABLE_LOCATION_LEN];
}
    PETableLocation;
#define PE_SIZEOF_TABLE_LOCATION (sizeof (PETableLocation))


BOOL  CRPE_API PEGetNthTableLocation (short printJob,
                                      short tableN,
                                      PETableLocation FAR *location);
BOOL  CRPE_API PESetNthTableLocation (short printJob,
                                      short tableN,
                                      PETableLocation FAR *location);

// If report based on a SQL Stored Procedure, use PEGetNParams to fetch the
// number of parameters, and PEGetNthParam and PESetNthParam to fetch and
// set individual parameters.  All parameter values are encoded as strings.

#define PE_PARAMETER_NAME_LEN    128

typedef struct PEParameterInfo
{
    // Initialize to PE_SIZEOF_PARAMETER_INFO.
    WORD StructSize;

    WORD Type;
    // String is null-terminated.
    char Name [PE_PARAMETER_NAME_LEN];
}
    PEParameterInfo;
#define PE_SIZEOF_PARAMETER_INFO (sizeof (PEParameterInfo))


short CRPE_API PEGetNParams (short printJob);         // Returns number of input parameters.

BOOL  CRPE_API PEGetNthParam (short printJob,
                              short paramN,           // 0 indexed parameter number.
                              HANDLE FAR *textHandle, // Address of a texthandle variable.
                              short  FAR *textLength);// Number of characters in string
                                                        // (not including null terminator).
BOOL  CRPE_API PEGetNthParamInfo (short printJob,
                                  short paramN,
                                  PEParameterInfo FAR *paramInfo);

BOOL  CRPE_API PESetNthParam (short printJob,
                              short paramN,                  // 0 indexed parameter number.
                              const char FAR *szParamValue); // String representation of parameter value.


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

BOOL  CRPE_API PETestNthTableConnectivity (short printJob,
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

BOOL CRPE_API PELogOnServer (char FAR *dllName,
                             PELogOnInfo FAR *logOnInfo);
BOOL CRPE_API PELogOffServer (char FAR *dllName,
                              PELogOnInfo FAR *logOnInfo);

BOOL CRPE_API PELogOnSQLServerWithPrivateInfo (char FAR *dllName,
                                               void FAR *privateInfo);


// Overriding SQL query in report
// ------------------------------
//
// PEGetSQLQuery () returns the same query as appears in the Show SQL Query
// dialog in CRW, in syntax specific to the database driver you are using.
//
// PESetSQLQuery () is mostly useful for reports with SQL queries that
// were explicitly edited in the Show SQL Query dialog in CRW, i.e. those
// reports that needed database-specific selection criteria or joins.
// (Otherwise it is usually best to continue using function calls such as
// PESetSelectionFormula () and let CRW build the SQL query automatically.)
//
// PESetSQLQuery () has the same restrictions as editing in the Show SQL
// Query dialog; in particular that changes are accepted in the FROM and
// WHERE clauses but ignored in the SELECT list of fields.

BOOL CRPE_API PEGetSQLQuery (short printJob,
                             HANDLE FAR *textHandle,
                             short  FAR *textLength);

BOOL CRPE_API PESetSQLQuery (short printJob,
                             char FAR *queryString);


// Saved data
// ----------
//
// Use PEHasSavedData() to find out if a report currently has saved data
// associated with it.  This may or may not be TRUE when a print job is
// first opened from a report file.  Since data is saved during a print,
// this will always be TRUE immediately after a report is printed.
//
// Use PEDiscardSavedData() to release the saved data associated with a
// report.  The next time the report is printed, it will get current data
// from the database.
//
// The default behavior is for a report to use its saved data, rather than
// refresh its data from the database when printing a report.

BOOL CRPE_API PEHasSavedData (short printJob,
                              BOOL FAR *hasSavedData);
BOOL CRPE_API PEDiscardSavedData (short printJob);

// Report title
// ------------

BOOL CRPE_API PEGetReportTitle (short printJob,
                                HANDLE FAR *titleHandle,
                                short  FAR *titleLength);
BOOL CRPE_API PESetReportTitle (short printJob,
                                char FAR *title);


// Controlling print to window
// ---------------------------

BOOL CRPE_API PEOutputToWindow (short printJob,
                                char FAR *title,
                                int left,
                                int top,
                                int width,
                                int height,
                                DWORD style,
                                HWND parentWindow);

HWND CRPE_API PEGetWindowHandle (short printJob);

void CRPE_API PECloseWindow (short printJob);


// Controlling printed pages
// -------------------------

BOOL CRPE_API PEShowNextPage (short printJob);
BOOL CRPE_API PEShowFirstPage (short printJob);
BOOL CRPE_API PEShowPreviousPage (short printJob);
BOOL CRPE_API PEShowLastPage (short printJob);
short CRPE_API PEGetNPages (short printJob);
BOOL CRPE_API PEShowNthPage (short printJob,
                             short pageN);

#define PE_ZOOM_FULL_SIZE           0
#define PE_ZOOM_SIZE_FIT_ONE_SIDE   1
#define PE_ZOOM_SIZE_FIT_BOTH_SIDES 2

BOOL CRPE_API PEZoomPreviewWindow (short printJob,
                                   short level); // a percent from 25 to 400
                                                 // or a PE_ZOOM_ constant


// Controlling print window when print control buttons hidden
// ----------------------------------------------------------

BOOL CRPE_API PEShowPrintControls (short printJob,
                                   BOOL showPrintControls);
BOOL CRPE_API PEPrintControlsShowing (short printJob,
                                      BOOL FAR *controlsShowing);

BOOL CRPE_API PEPrintWindow (short printJob,
                             BOOL waitUntilDone);

BOOL CRPE_API PEExportPrintWindow (short printJob,
                                   BOOL toMail,
                                   BOOL waitUntilDone);

BOOL CRPE_API PENextPrintWindowMagnification (short printJob);


// Changing printer selection
// --------------------------

BOOL CRPE_API PESelectPrinter (short printJob,
                               char FAR *driverName,
                               char FAR *printerName,
                               char FAR *portName,
                               DEVMODE FAR *mode);

BOOL CRPE_API PEGetSelectedPrinter (short printJob,
                                    HANDLE  FAR *driverHandle,
                                    short   FAR *driverLength,
                                    HANDLE  FAR *printerHandle,
                                    short   FAR *printerLength,
                                    HANDLE  FAR *portHandle,
                                    short   FAR *portLength,
                                    DEVMODE FAR * FAR *mode);


// Controlling print to printer
// ----------------------------

BOOL CRPE_API PEOutputToPrinter (short printJob,
                                 short nCopies);

BOOL CRPE_API PESetNDetailCopies (short printJob,
                                  short nDetailCopies);

BOOL CRPE_API PEGetNDetailCopies (short printJob,
                                  short FAR *nDetailCopies);

// Extension to PESetPrintOptions function: If the 2nd parameter
// (pointer to PEPrintOptions) is set to 0 (null) the function prompts
// the user for these options.
//
// With this change, you can get the behaviour of the print-to-printer
// button in the print window by calling PESetPrintOptions with a
// null pointer and then calling PEPrintWindow.

#define PE_MAXPAGEN  65535

#define PE_UNCOLLATED       0
#define PE_COLLATED         1
#define PE_DEFAULTCOLLATION 2

typedef struct PEPrintOptions
{
    WORD StructSize;            // initialize to PE_SIZEOF_PRINT_OPTIONS

    // page and copy numbers are 1-origin
    // use 0 to preserve the existing settings
    unsigned short startPageN,
                   stopPageN;

    unsigned short nReportCopies;
    unsigned short collation;
}
    PEPrintOptions;
#define PE_SIZEOF_PRINT_OPTIONS (sizeof (PEPrintOptions))

BOOL CRPE_API PESetPrintOptions (short printJob,
                                 PEPrintOptions FAR *options);

BOOL CRPE_API PEGetPrintOptions (short printJob,
                                 PEPrintOptions FAR *options);


// Controlling print to file and export
// ------------------------------------

typedef struct PEExportOptions
{
    WORD StructSize;               // initialize to PE_SIZEOF_EXPORT_OPTIONS

    char formatDLLName [PE_DLL_NAME_LEN];
    DWORD formatType;
    void FAR *formatOptions;
    char destinationDLLName [PE_DLL_NAME_LEN];
    DWORD destinationType;
    void FAR *destinationOptions;
    WORD nFormatOptionsBytes;      // Set by 'PEGetExportOptions', and
                                   // ignored by 'PEExportTo'.
    WORD nDestinationOptionsBytes; // Set by 'PEGetExportOptions', and
                                   // ignored by 'PEExportTo'.
}
    PEExportOptions;
#define PE_SIZEOF_EXPORT_OPTIONS (sizeof (PEExportOptions))

BOOL CRPE_API PEGetExportOptions (short printJob,
                                  PEExportOptions FAR *options);

BOOL CRPE_API PEExportTo (short printJob,
                          PEExportOptions FAR *options);

// Setting page margins
// --------------------

#define PE_SM_DEFAULT  (short)0x8000

BOOL CRPE_API PESetMargins (short printJob,
                            short left,
                            short right,
                            short top,
                            short bottom);

BOOL CRPE_API PEGetMargins (short printJob,
                            short FAR *left,
                            short FAR *right,
                            short FAR *top,
                            short FAR *bottom);


// Setting section height and format
// ---------------------------------

short CRPE_API PEGetNSections (short printJob);

short CRPE_API PEGetSectionCode (short printJob,
                                 short sectionN);

BOOL CRPE_API PESetMinimumSectionHeight (short printJob,
                                         short sectionCode,
                                         short minimumHeight);  // in twips

BOOL CRPE_API PEGetMinimumSectionHeight (short printJob,
                                         short sectionCode,
                                         short FAR *minimumHeight);

typedef struct PESectionOptions
{
    WORD StructSize;            // initialize to PE_SIZEOF_SECTION_OPTIONS

    short visible,              // BOOL values, except use PE_UNCHANGED
          newPageBefore,        // to preserve the existing settings
          newPageAfter,
          keepTogether,
          suppressBlankSection,
          resetPageNAfter,
          printAtBottomOfPage;
    COLORREF backgroundColor;   // Use PE_UNCHANGED_COLOR to preserve the
                                // existing color.
    short underlaySection;      // BOOL values, except use PE_UNCHANGED
    short showArea;             // to preserve the existing settings
    short freeFormPlacement;
}
    PESectionOptions;
#define PE_SIZEOF_SECTION_OPTIONS (sizeof (PESectionOptions))


BOOL CRPE_API PESetSectionFormat (short printJob,
                                  short sectionCode,
                                  PESectionOptions FAR *options);

// format formula name
#define SECTION_VISIBILITY               58
#define NEW_PAGE_BEFORE                  60
#define NEW_PAGE_AFTER                   61
#define KEEP_SECTION_TOGETHER            62
#define SUPPRESS_BLANK_SECTION           63
#define RESET_PAGE_N_AFTER               64
#define PRINT_AT_BOTTOM_OF_PAGE          65
#define UNDERLAY_SECTION                 66
#define SECTION_BACK_COLOUR              67

BOOL CRPE_API PESetSectionFormatFormula(short printJob, 
                                        short sectionCode,
                                        short formulaName,
                                        char FAR * formulaString);

BOOL CRPE_API PEGetSectionFormat (short printJob,
                                  short sectionCode,
                                  PESectionOptions FAR *options);


// Setting font and color info
// ---------------------------

#define PE_FIELDS    0x0001
#define PE_TEXT      0x0002

BOOL CRPE_API PESetFont (short printJob,
                         short sectionCode,
                         short scopeCode,     // PE_FIELDS or PE_TEXT
                         char FAR *faceName,  // 0 for no change
                         short fontFamily,    // FF_DONTCARE for no change
                         short fontPitch,     // DEFAULT_PITCH for no change
                         short charSet,       // DEFAULT_CHARSET for no change
                         short pointSize,     // 0 for no change
                         short isItalic,      // PE_UNCHANGED for no change
                         short isUnderlined,  // ditto
                         short isStruckOut,   // ditto
                         short weight);       // 0 for no change

// Setting Graph/Chart info
// ------------------------
//
// Two parameters are passed to uniquely identify the graph:
//      - section code
//      - graph number in that section
//
// The section code includes whether it is a header or footer, and the
// graph number starts at 0, 1...  The graph number identifies the graph
// by its position in the section
//      - looking top down first,
//      - then left to right if they have the same top.

// Graph Types

#define PE_SIDE_BY_SIDE_BAR_GRAPH             0
#define PE_STACKED_BAR_GRAPH                  2
#define PE_PERCENT_BAR_GRAPH                  3
#define PE_FAKED_3D_SIDE_BY_SIDE_BAR_GRAPH    4
#define PE_FAKED_3D_STACKED_BAR_GRAPH         5
#define PE_FAKED_3D_PERCENT_BAR_GRAPH         6
#define PE_PIE_GRAPH                          40
#define PE_MULTIPLE_PIE_GRAPH                 42
#define PE_PROPORTIONAL_MULTI_PIE_GRAPH       43
#define PE_LINE_GRAPH                         80
#define PE_AREA_GRAPH                         120
#define PE_THREED_BAR_GRAPH                   160
#define PE_USER_DEFINED_GRAPH                 500
#define PE_UNKNOWN_TYPE_GRAPH                 1000

// Graph Directions.
#define PE_GRAPH_ROWS_ONLY                    0
#define PE_GRAPH_COLS_ONLY                    1
#define PE_GRAPH_MIXED_ROW_COL                2
#define PE_GRAPH_MIXED_COL_ROW                3
#define PE_GRAPH_UNKNOWN_DIRECTION            20

// Graph constant for rowGroupN, colGroupN, summarizedFieldN in PEGraphDataInfo
#define PE_GRAPH_DATA_NULL_SELECTION          -1

// Graph text max length
#define PE_GRAPH_TEXT_LEN                     128

typedef struct PEGraphDataInfo
{
    WORD StructSize;            // initialize to PE_SIZEOF_GRAPH_DATA_INFO

    short rowGroupN,            // group number in report.
          colGroupN,            // group number in report.
          summarizedFieldN,     // summarized field number for the group
                                // where the graph stays.
          graphDirection;       // For normal group/total report, the direction,
                                // is always GRAPH_COLS_ONLY.  For CrossTab
                                // report all four options will change the
                                // graph data.
}
    PEGraphDataInfo;
#define PE_SIZEOF_GRAPH_DATA_INFO (sizeof (PEGraphDataInfo))

typedef struct PEGraphTextInfo
{
    WORD StructSize;            // initialize to PE_SIZEOF_GRAPH_TEXT_INFO

    // Strings are null-terminated.
    char graphTitle       [PE_GRAPH_TEXT_LEN];
    char graphSubTitle    [PE_GRAPH_TEXT_LEN];
    char graphFootNote    [PE_GRAPH_TEXT_LEN];
    char graphGroupsTitle [PE_GRAPH_TEXT_LEN];
    char graphSeriesTitle [PE_GRAPH_TEXT_LEN];
    char graphXAxisTitle  [PE_GRAPH_TEXT_LEN];
    char graphYAxisTitle  [PE_GRAPH_TEXT_LEN];
    char graphZAxisTitle  [PE_GRAPH_TEXT_LEN];
}
    PEGraphTextInfo;
#define PE_SIZEOF_GRAPH_TEXT_INFO (sizeof (PEGraphTextInfo))

typedef struct PEGraphOptions
{
    WORD StructSize;            // initialize to PE_SIZEOF_GRAPH_OPTIONS

    double graphMaxValue,
           graphMinValue;

    BOOL   showDataValue,       // Show data values on risers.
           showGridLine,
           verticalBars,
           showLegend;

    char   fontFaceName [PE_GRAPH_TEXT_LEN];
}
    PEGraphOptions;
#define PE_SIZEOF_GRAPH_OPTIONS (sizeof (PEGraphOptions))

BOOL CRPE_API PEGetGraphType (short printJob,
                              short sectionCode,
                              short graphN,
                              short FAR *graphType);

BOOL CRPE_API PEGetGraphData (short printJob,
                              short sectionCode,
                              short graphN,
                              PEGraphDataInfo FAR *graphDataInfo);

BOOL CRPE_API PEGetGraphText (short printJob,
                              short sectionCode,
                              short graphN,
                              PEGraphTextInfo FAR *graphTextInfo);

BOOL CRPE_API PEGetGraphOptions (short printJob,
                                 short sectionCode,
                                 short graphN,
                                 PEGraphOptions FAR *graphOptions);

BOOL CRPE_API PESetGraphType (short printJob,
                              short sectionCode,
                              short graphN,
                              short FAR *graphType);

BOOL CRPE_API PESetGraphData (short printJob,
                              short sectionCode,
                              short graphN,
                              PEGraphDataInfo FAR *graphDataInfo);

BOOL CRPE_API PESetGraphText (short printJob,
                              short sectionCode,
                              short graphN,
                              PEGraphTextInfo FAR *graphTextInfo);

BOOL CRPE_API PESetGraphOptions (short printJob,
                                 short sectionCode,
                                 short graphN,
                                 PEGraphOptions FAR *graphOptions);

// Subreports
// ----------

short CRPE_API PEGetNSubreportsInSection (short printJob,
                                          short sectionCode);

DWORD CRPE_API PEGetNthSubreportInSection (short printJob,
                                           short sectionCode,
                                           short subreportN);

#define PE_SUBREPORT_NAME_LEN 128

typedef struct PESubreportInfo
{
    WORD StructSize;            // Initialize to PE_SIZEOF_SUBREPORT_INFO.  
    
    // Strings are null-terminated.
    char name [PE_SUBREPORT_NAME_LEN];
}
    PESubreportInfo;
#define PE_SIZEOF_SUBREPORT_INFO (sizeof (PESubreportInfo))

BOOL CRPE_API PEGetSubreportInfo (short printJob,
                                  DWORD subreportHandle,
                                  PESubreportInfo FAR *subreportInfo);

short CRPE_API PEOpenSubreport (short parentJob,
                                char FAR *subreportName);
BOOL CRPE_API PECloseSubreport (short printJob);


#if defined (__cplusplus)
}
#endif

// Reset structure alignment
#if defined (__BORLANDC__)
  #pragma option -a.
#elif defined (_MSC_VER)
  #if _MSC_VER >= 900
    #pragma pack (pop)
  #else
    #pragma pack ()
  #endif
#endif

#endif // CRPE_H
