//
// MasterData.h - the GL master-data screens of Spec #2 S2-M2, each a modal
// dialog with its TableList beside it:
//
//   RunDepartmentCodes  Accounts > Department Codes, GLSUB (ID_DEPTCODES)  1023  gl/src/gl_deptc.cpp
//   RunMedicalReps      Accounts > Medical Reps (ID_MEDREPS)               1006  gl/src/gl_medrp.cpp
//   RunDoctorCodes      Accounts > Doctor Codes (ID_DOCTOR)                1040  gl/src/gl_drcod.cpp
//   RunProductCodes     Accounts > Product Codes (ID_PRODCODE)             1007  gl/src/gl_prods.cpp
//
// The record operations are in MasterTables.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunDepartmentCodes(HWND frame);
INT_PTR RunMedicalReps(HWND frame);
INT_PTR RunDoctorCodes(HWND frame);
INT_PTR RunProductCodes(HWND frame);

// gl.exe /stress: opens and closes each of the four dialogs `times` times
// and returns the change in the process's GDI + USER object count (0 if
// none), as StressChartOfAccounts does.
int StressMasterData(int times);
