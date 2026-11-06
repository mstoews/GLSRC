//
// EditRules.h - what the tb* edit controls accept and how they format,
// without any Win32 (the window procedures in PicEdit.cpp etc. call these).
//
// Picture masks (tbPic, and tbDate's "99/99/99"):
//   9  digit            A  letter            N  letter or digit
//   X  any character    ?  any character
//   U  letter, made upper case               L  letter, made lower case
//   anything else is a literal, inserted automatically ("(999) 999 - 9999").
// An empty mask accepts anything.
//
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace win32fx::rules {

// ---- picture masks ----

bool IsMaskChar(wchar_t m) noexcept;

// `text` (the edit's text after a keystroke or paste) fitted to `mask`:
// literals are inserted where the mask has them, U/L change case. False if a
// character doesn't fit its position or the text is longer than the mask.
// `caret` is a position in `text`; `caretOut` the same position in `out`.
bool ConformPicture(std::wstring_view mask, std::wstring_view text, size_t caret,
                    std::wstring& out, size_t& caretOut);

// ---- numbers (tbFloat, tbInteger) ----

struct NumberFormat {
    int intDigits = 13;       // digits before the decimal point
    int decimals = 2;         // digits after it; 0: no point
    bool separators = false;  // display 1,234.50 (FS_SEPARATOR)
};

// From a mask such as "9999999999999.99" (13, 2) or "9999" (4, 0): the 9s
// before and after the '.'. A mask without 9s gives `fallback`.
NumberFormat NumberFormatFromMask(std::wstring_view mask, NumberFormat fallback) noexcept;

// Whether `text` can be (the beginning of) a number in `fmt` while typing:
// "", "-", "12.", "-1,234.5". Separators are ignored wherever they are.
bool AcceptNumber(const NumberFormat& fmt, std::wstring_view text) noexcept;

// The value of `text`, ignoring spaces and separators. False if blank or not a number.
bool ParseNumber(std::wstring_view text, double& value) noexcept;

// `value` with fmt.decimals decimals, and separators if fmt.separators.
std::wstring FormatNumber(double value, const NumberFormat& fmt);

// ---- dates (tbDate) ----

enum class DateOrder { MDY, DMY, YMD };

struct DateFormat {
    DateOrder order = DateOrder::MDY;
    bool century = false;     // four-digit year (DS_CENTURY)
};

// "99/99/99", "99/99/9999" or "9999/99/99".
std::wstring DateMask(const DateFormat& fmt);

// A complete date in `fmt` as YYYYMMDD. False if incomplete or not a real
// date. Two-digit years: 00-49 are 20xx, 50-99 are 19xx.
bool DateToYmd(std::wstring_view text, const DateFormat& fmt, std::wstring& ymd);

// YYYYMMDD (8 digits, a real date) shown in `fmt`. False otherwise.
bool YmdToDate(std::wstring_view ymd, const DateFormat& fmt, std::wstring& text);

} // namespace win32fx::rules
