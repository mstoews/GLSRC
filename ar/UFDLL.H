/*
** File:    UFDll.h
** Author:  Rex Benning, Rick Cameron
** Date:    3 Mar 93
**
** Purpose: Declarations for a user-defined function DLL
*/

#if !defined (UFDLL_H)
#define UFDLL_H

#if defined (__cplusplus)
  #define ExternC extern "C"
#else
  #define ExternC
#endif

#define UFMaxNArguments 50

#define UFMaxStringLength 254
#define UFUnknownStringLength MaxInt16u

typedef signed   char  UFTInt8s;
typedef unsigned char  UFTInt8u;

typedef signed   short UFTInt16s;
typedef unsigned short UFTInt16u;

typedef signed   long  UFTInt32s;
typedef unsigned long  UFTInt32u;

typedef double UFTNumber;
typedef UFTNumber UFTCurrency;

typedef UFTInt32s UFTDate;
typedef UFTInt32s UFTTime;

typedef UFTInt16s UFTBoolean;

#define UFNumberScalingFactor ((UFTNumber) 100.00)

typedef enum
{
    UFUserError = -1,
    UFNoError,
    UFNoMemory,
    UFStringTooBig,
    UFDivideByZero,
    UFArrayIndexTooBig,
    UFStringIndexTooBig,
    UFNotEnoughParameters,
    UFMemCalcFail
}
    UFError;

typedef enum
{
    UFUndefinedReturn,
    UFStringReturn,
    UFNumberReturn,
    UFDateReturn,
    UFBooleanReturn,
    UFCurrencyReturn,
    UFDateRangeReturn,
    UFNumberRangeReturn,
    UFCurrencyRangeReturn,
    UFStringRangeReturn,
    UFStringArrayReturn,
    UFNumberArrayReturn,
    UFDateArrayReturn,
    UFBooleanArrayReturn,
    UFCurrencyArrayReturn,
    UFLastReturnType
}
    UFReturnTypes;

typedef enum
{
    UFUndefinedType,
    UFNumber,
    UFCurrency,
    UFNumberToCurrency,
    UFInteger,
    UFBoolean,
    UFDate,
    UFStringArg,
    UFNumberRange,
    UFCurrencyRange,
    UFNumberToCurrencyRange,
    UFIntegerRange,
    UFBooleanRange, // ... for completeness, not likely used
    UFDateRange,
    UFStringRange,
    UFNumberArray,
    UFCurrencyArray,
    UFNumberToCurrencyArray,
    UFIntegerArray,
    UFBooleanArray,
    UFDateArray,
    UFStringArray,
    UFField
}
    UFTypeDefs;


typedef union
{
    UFTInt32s   ParamInteger;
    UFTNumber   ParamNumber;
    UFTDate     ParamDate;
    UFTBoolean  ParamBoolean;
    UFTCurrency ParamCurrency;
    char        FAR *ParamString;
    UFTNumber   FAR *ParamNumberArray;
    UFTInt32s   FAR *ParamIntegerArray;
    UFTDate     FAR *ParamDateArray;
    UFTBoolean  FAR *ParamBooleanArray;
    UFTCurrency FAR *ParamCurrencyArray;
    char        FAR * FAR *ParamStringArray;
    void        FAR *ParamArrayPointer;

    struct
    {
        UFTInt32s ParamIntStart;
        UFTInt32s ParamIntEnd;
    }
        ParamIntRange;

    struct
    {
        UFTDate    ParamDateStart;
        UFTDate    ParamDateEnd;
    }
        ParamDateRange;

    struct
    {
        UFTNumber  ParamNumberStart;
        UFTNumber  ParamNumberEnd;
    }
        ParamNumberRange;

    struct
    {
        UFTNumber  ParamCurrencyStart;
        UFTNumber  ParamCurrencyEnd;
    }
        ParamCurrencyRange;

    struct
    {
        char FAR *ParamStringStart;
        char FAR *ParamStringEnd;
    }
        ParamStringRange;

    struct _UFMemCalcParam FAR *MemCalcSubParam;
} UFParamUnion;

typedef struct _UFParamListElement
{
    UFTInt16u StructSize;
    struct _UFParamListElement FAR *NextParam;
    UFTInt16s ParamLength;
    UFTypeDefs ParamType;
    UFParamUnion Parameter;
}
    UFParamListElement;

typedef enum
{
    UFLiteral,
    UFArray,
    UFVariable,
    UFExpression
}
    UFMemCalcParmTypes;

typedef struct _UFMemCalcParam // For use when asking the DLL how much memory to allocate
                               // for a returned string or array.
{
    UFTInt16u StructSize;
    struct _UFMemCalcParam FAR *NextMemCalcParam;
    UFTInt16s MemCalcParamLength; // Array or String length
    UFMemCalcParmTypes MemCalcLiteralFlag;
    UFTypeDefs MemCalcParamType;
    UFParamUnion MemCalcParameter;
}
    UFMemCalcParam;

typedef union
{
    UFTNumber   ReturnNumber;
    UFTDate     ReturnDate;
    UFTBoolean  ReturnBoolean;
    UFTCurrency ReturnCurrency;
    char        FAR *ReturnString;

    struct
    {
        UFTInt32s ReturnIntStart;
        UFTInt32s ReturnIntEnd;
    }
        ReturnIntRange;

    struct
    {
        UFTDate ReturnDateStart;
        UFTDate ReturnDateEnd;
    }
        ReturnDateRange;

    struct
    {
        UFTNumber ReturnNumberStart;
        UFTNumber ReturnNumberEnd;
    }
        ReturnNumberRange;

    struct
    {
        char FAR *ReturnStringStart;
        char FAR *ReturnStringEnd;
    }
        ReturnStringRange;

    UFTInt32s UFReturnUserError;

    UFTNumber   FAR *ReturnNumberArray;
    UFTDate     FAR *ReturnDateArray;
    UFTBoolean  FAR *ReturnBooleanArray;
    UFTCurrency FAR *ReturnCurrencyArray;
    char        FAR * FAR *ReturnStringArray;
}
    UFReturnUnion;

typedef enum
{
    UFCalcStringLength,
    UFCalcArraySize
}
    UFMemCalcType;

typedef struct
{
    UFTInt16u StructSize;
    UFReturnUnion ReturnValue;
    UFTInt16s ReturnValueLength;
    UFTInt16s FunctionNumber;
    void (FAR PASCAL *UFErrorHandler) (UFError);
    UFParamListElement FAR *UFParamList;
    UFTInt32u JobId;
}
    UFParamBlock;

typedef struct // For use when asking the DLL how much memory to allocate
               // for a returned string or array.
{
    UFTInt16u StructSize;
    UFMemCalcType MemCalcCommand;
    UFTInt16s UFMemCalcArrayLength;
    UFTInt16s UFMemCalcStringLength;
    UFMemCalcParam FAR *UFMemCalcParamList; // For calculating string sizes
    UFParamListElement FAR *UFParamList;    // For calculating array sizes.
}
    UFMemCalcBlock;


typedef struct
{
    char FAR *FuncDef;                               // The Ascii name of the function
    UFReturnTypes UFReturnKind;                      // Return Type
    UFError (FAR PASCAL *UFEntry) (UFParamBlock FAR *); // Pointer to the entry address in the library
    UFError (FAR PASCAL *UFMemCalcFunc) (UFMemCalcBlock FAR *); // Pointer to a function that will
                                                            // calculate how big a string or array
                                                            // this function will return.
    UFTInt16s ParamCount;                            // The number of parameters
    UFTypeDefs ParamTypeArray [UFMaxNArguments];     // An array of type indicators corresponding
                                                     // to each parameter.
}
    UFFunctionDefinitions;

typedef struct
{
    char FAR *FuncDefString;
    UFError (FAR PASCAL *UFEntry) (UFParamBlock FAR *);
    UFError (FAR PASCAL *UFMemCalcFunc) (UFMemCalcBlock FAR *);
}
    UFFunctionDefStrings;

typedef struct  // An encasing structure for extendability,
                // the first element will indicate, and check, which
                // version of the interface is in use. It is set by
                // the compiler using the sizeof operator. Any changes
                // to the interface must change the size of this structure.
{
    UFTInt16u StructSize;
    UFFunctionDefStrings FAR *UFFunctionDefStrPtr;
}
    UFFunctionDefStringList;

typedef struct
{
    char FAR *FuncTemplate;
}
    UFFunctionTemplates;

typedef struct  // An encasing structure for extendability,
                // the first element will indicate, and check, which
                // version of the interface is in use. It is set by
                // the compiler using the sizeof operator. Any changes
                // to the interface must change the size of this structure.
{
    UFTInt16u StructSize;
    UFFunctionTemplates FAR *UFFuncTemplatePtr;
}
    UFFunctionTemplateList;

typedef struct
{
    char FAR *FuncExample;
}
    UFFunctionExamples;

typedef struct  // An encasing structure for extendability,
                // the first element will indicate, and check, which
                // version of the interface is in use. It is set by
                // the compiler using the sizeof operator. Any changes
                // to the interface must change the size of this structure.
{
    UFTInt16u StructSize;
    UFFunctionExamples FAR *FuncExample;
}
    UFFunctionExampleList;

#endif // UFDLL_H
