//
// Company.h - the company-wide lookups the frame and the screens share.
//
// Legacy homes: GetCompany / szGetPeriod in gl/src/ap_repts.cpp,
// Data4::UpdateUserTime in gl/src/gl.cpp.
//
#pragma once

#include "cb4sqlite.h"

#include <string>

// The company name: the first gl_prof record, trailing blanks removed. Empty
// if there is no gl_prof table or no record (the original returned an
// uninitialised buffer).
std::wstring GetCompany();

// The status-bar period: pd_desc of the lowest-numbered gl_pds period whose
// status is "0" (open), walking the "period" tag from the bottom as
// szGetPeriod did. Empty if there is none.
std::wstring GetCurrentPeriod();

// Data4::UpdateUserTime (a member the app added to CodeBase's Data4): stamps
// the current record's "update", "user" and "time" fields. False, and
// nothing assigned, if the table lacks any of them, as the original. The
// real gl_prof has none of the three, so Company Profile's Save never stamps.
bool UpdateUserTime(Data4& data, const char* user);
