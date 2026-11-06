//
// EditRules.cpp - see EditRules.h. Portable C++ (no Win32).
//

#include "EditRules.h"

#include <cstdio>
#include <cstdlib>
#include <cwctype>

namespace win32fx::rules {

namespace {

bool isDigit(wchar_t c) noexcept { return c >= L'0' && c <= L'9'; }

bool matches(wchar_t m, wchar_t c) noexcept
{
    switch (m) {
    case L'9': return isDigit(c);
    case L'A': case L'U': case L'L': return std::iswalpha(static_cast<wint_t>(c)) != 0;
    case L'N': return std::iswalnum(static_cast<wint_t>(c)) != 0;
    default:   return c >= L' ';   // X, ?
    }
}

wchar_t transform(wchar_t m, wchar_t c) noexcept
{
    if (m == L'U') return static_cast<wchar_t>(std::towupper(static_cast<wint_t>(c)));
    if (m == L'L') return static_cast<wchar_t>(std::towlower(static_cast<wint_t>(c)));
    return c;
}

int digitsValue(std::wstring_view s) noexcept
{
    int v = 0;
    for (wchar_t c : s) v = v * 10 + (c - L'0');
    return v;
}

std::wstring padded(int v, int width)
{
    std::wstring s = std::to_wstring(v);
    if (static_cast<int>(s.size()) < width) s.insert(0, static_cast<size_t>(width) - s.size(), L'0');
    return s;
}

bool realDate(int y, int m, int d) noexcept
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (y < 1 || m < 1 || m > 12 || d < 1) return false;
    const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    return d <= days[m - 1] + ((m == 2 && leap) ? 1 : 0);
}

// Positions of the year, month and day fields in a DateMask() string.
struct Fields { size_t y, yLen, m, d; };

Fields fields(const DateFormat& fmt) noexcept
{
    const size_t yLen = fmt.century ? 4 : 2;
    switch (fmt.order) {
    case DateOrder::MDY: return {6, yLen, 0, 3};
    case DateOrder::DMY: return {6, yLen, 3, 0};
    default:             return {0, yLen, yLen + 1, yLen + 4};   // YMD
    }
}

} // namespace

bool IsMaskChar(wchar_t m) noexcept
{
    switch (m) {
    case L'9': case L'A': case L'N': case L'X': case L'?': case L'U': case L'L': return true;
    default: return false;
    }
}

bool ConformPicture(std::wstring_view mask, std::wstring_view text, size_t caret,
                    std::wstring& out, size_t& caretOut)
{
    out.clear();
    if (mask.empty()) {
        out.assign(text);
        caretOut = caret < text.size() ? caret : text.size();
        return true;
    }
    bool caretSet = false;
    auto mark = [&](size_t j) {
        if (!caretSet && j == caret) { caretOut = out.size(); caretSet = true; }
    };
    size_t i = 0;
    for (size_t j = 0; j < text.size();) {
        if (i >= mask.size()) return false;
        const wchar_t m = mask[i++];
        if (!IsMaskChar(m)) {
            if (text[j] == m) { mark(j); ++j; }
            out += m;               // typed or inserted
            continue;
        }
        if (!matches(m, text[j])) return false;
        mark(j);
        out += transform(m, text[j++]);
    }
    if (!caretSet) caretOut = out.size();
    return true;
}

NumberFormat NumberFormatFromMask(std::wstring_view mask, NumberFormat fallback) noexcept
{
    NumberFormat f = fallback;
    int ints = 0, decs = 0;
    bool point = false;
    for (wchar_t c : mask) {
        if (c == L'.') point = true;
        else if (c == L'9') (point ? decs : ints)++;
    }
    if (ints + decs == 0) return f;
    f.intDigits = ints;
    f.decimals = decs;
    return f;
}

bool AcceptNumber(const NumberFormat& fmt, std::wstring_view text) noexcept
{
    size_t p = 0;
    if (p < text.size() && text[p] == L'-') ++p;
    int ints = 0, decs = 0;
    bool point = false;
    for (; p < text.size(); ++p) {
        const wchar_t c = text[p];
        if (c == L',') {
            if (!fmt.separators || point) return false;
        } else if (c == L'.') {
            if (point || fmt.decimals == 0) return false;
            point = true;
        } else if (!isDigit(c)) {
            return false;
        } else if (point ? ++decs > fmt.decimals : ++ints > fmt.intDigits) {
            return false;
        }
    }
    return true;
}

bool ParseNumber(std::wstring_view text, double& value) noexcept
{
    value = 0;
    std::wstring s;
    bool digit = false, point = false;
    for (wchar_t c : text) {
        if (c == L' ' || c == L',') continue;
        if (c == L'-') {
            if (!s.empty()) return false;
        } else if (c == L'.') {
            if (point) return false;
            point = true;
        } else if (isDigit(c)) {
            digit = true;
        } else {
            return false;
        }
        s += c;
    }
    if (!digit) return false;
    value = std::wcstod(s.c_str(), nullptr);
    return true;
}

std::wstring FormatNumber(double value, const NumberFormat& fmt)
{
    char buf[400];
    std::snprintf(buf, sizeof buf, "%.*f", fmt.decimals, value);
    std::string s = buf;
    if (s[0] == '-' && s.find_first_not_of("-0.") == std::string::npos) s.erase(0, 1);   // -0.00
    if (fmt.separators) {
        const size_t start = (s[0] == '-') ? 1 : 0;
        size_t end = s.find('.');
        if (end == std::string::npos) end = s.size();
        for (size_t k = end; k > start + 3; k -= 3) s.insert(k - 3, 1, ',');
    }
    return std::wstring(s.begin(), s.end());
}

std::wstring DateMask(const DateFormat& fmt)
{
    if (fmt.order == DateOrder::YMD) return fmt.century ? L"9999/99/99" : L"99/99/99";
    return fmt.century ? L"99/99/9999" : L"99/99/99";
}

bool DateToYmd(std::wstring_view text, const DateFormat& fmt, std::wstring& ymd)
{
    const std::wstring mask = DateMask(fmt);
    if (text.size() != mask.size()) return false;
    for (size_t i = 0; i < mask.size(); ++i)
        if (mask[i] == L'9' ? !isDigit(text[i]) : text[i] != mask[i]) return false;
    const Fields f = fields(fmt);
    int y = digitsValue(text.substr(f.y, f.yLen));
    if (!fmt.century) y += (y < 50) ? 2000 : 1900;
    const int m = digitsValue(text.substr(f.m, 2));
    const int d = digitsValue(text.substr(f.d, 2));
    if (!realDate(y, m, d)) return false;
    ymd = padded(y, 4) + padded(m, 2) + padded(d, 2);
    return true;
}

bool YmdToDate(std::wstring_view ymd, const DateFormat& fmt, std::wstring& text)
{
    if (ymd.size() != 8) return false;
    for (wchar_t c : ymd)
        if (!isDigit(c)) return false;
    const int y = digitsValue(ymd.substr(0, 4));
    const int m = digitsValue(ymd.substr(4, 2));
    const int d = digitsValue(ymd.substr(6, 2));
    if (!realDate(y, m, d)) return false;
    const std::wstring year = fmt.century ? padded(y, 4) : padded(y % 100, 2);
    switch (fmt.order) {
    case DateOrder::MDY: text = padded(m, 2) + L"/" + padded(d, 2) + L"/" + year; break;
    case DateOrder::DMY: text = padded(d, 2) + L"/" + padded(m, 2) + L"/" + year; break;
    default:             text = year + L"/" + padded(m, 2) + L"/" + padded(d, 2); break;
    }
    return true;
}

} // namespace win32fx::rules
