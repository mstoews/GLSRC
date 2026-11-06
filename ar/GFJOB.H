/*
** File:    UFJob.h
** Author:  Rick Cameron
** Date:    13 Mar 93
**
** Purpose: Declarations of types and functions to support job management.
*/

#if !defined (UFJOB_H)

#define UFJOB_H

#define CurVersionNumber 0x0100

struct JobInfo
{
    UFTInt32u jobId;
    struct JobInfo FAR *prev;
    struct JobInfo FAR *next;

    // use this pointer to point to your data block
    void FAR *data;
};

extern void InitForJob (UFTInt32u jobId);
extern void TermForJob (UFTInt32u jobId);

extern struct JobInfo FAR *FindJobInfo (UFTInt32u jobId);

// you must implement these
extern void InitJob (struct JobInfo FAR *jobInfo);
extern void TermJob (struct JobInfo FAR *jobInfo);

#endif // !defined (UFJOB_H)
