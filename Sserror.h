// sserror.h
//
// Copyright (c) 1992-1993, Visual Tools, Inc. All rights reserved
//

typedef int SSERROR;

#define SSERROR_NONE 0						// No error
#define SSERROR_GENERAL 1					// Error
#define SSERROR_BAD_ARGUMENT 2				// Bad argument provided to function
#define SSERROR_NO_MEMORY 3					// Not enough memory to complete operation
#define SSERROR_BAD_FORMULA 4				// Unable to parse formula
#define SSERROR_BUF_TOO_SHORT 5				// Provided buffer too short for result
#define SSERROR_NOT_FOUND 6					// Name not found (user name or table name)
#define SSERROR_BAD_RC 7					// Bad RC value sent to function
#define SSERROR_BAD_HSS 8					// Bad HSS value passed to function
#define SSERROR_TOO_MANY_HSS 9				// Unable to create any more HSSs
#define SSERROR_NO_TABLE 10					// No table attached to view
#define SSERROR_UNABLE_TO_OPEN_FILE 11		// Unable to open file
#define SSERROR_INVALID_FILE 12				// Tried to read invalid file
#define SSERROR_INSERT_SHIFT_OFF_TABLE 13	// Insert would push cells out of bounds of worksheet
#define SSERROR_ONLY_ONE_RANGE 14			// Cannot do command with more than one range selected
#define SSERROR_NOTHING_TO_PASTE 15			// Called SSEditPaste() but there is nothing to paste
#define SSERROR_BAD_NUMBER_FORMAT 16
#define SSERROR_TOO_MANY_FONTS 17
#define SSERROR_TOO_MANY_SELECTED_RANGES 18
#define SSERROR_UNABLE_TO_WRITE_FILE 19
#define SSERROR_NO_TRANSACTION 20			// SSTransactCommit() or SSTransactRollback() called without first calling SSTransactStart()
#define SSERROR_NOTHING_TO_PRINT 21
#define SSERROR_PRINT_MARGINS_DONT_FIT 22
#define SSERROR_CANCEL 23
#define SSERROR_UNABLE_TO_INITIALIZE_PRINTER 24
#define SSERROR_STRING_TOO_LONG 25
#define SSERROR_FORMULA_TOO_LONG 26
#define SSERROR_UNABLE_TO_OPEN_CLIPBOARD 27	// OpenClipboard() Windows call failed
#define SSERROR_PASTE_WOULD_OVERFLOW_SHEET 28
#define SSERROR_LOCKED_CELLS_CANNOT_BE_MODIFIED 29
#define SSERROR_LOCKED_DOCUMENT_CANNOT_BE_MODIFIED 30
#define SSERROR_INVALID_NAME 31
#define SSERROR_CANT_DELETE_NAME_IN_USE 32
#define SSERROR_UNABLE_TO_FIND_NAME 33
#define SSERROR_NO_WINDOW 34
#define SSERROR_SELECTION 35
#define SSERROR_TOO_MANY_OBJECTS 36
#define SSERROR_INVALID_OBJECT_TYPE 37
#define SSERROR_OBJECT_NOT_FOUND 38
#define SSERROR_INVALID_REQUEST 39

#define SSERROR_LAST SSERROR_INVALID_REQUEST
