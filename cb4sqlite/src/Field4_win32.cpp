//
// Field4_win32.cpp - dialog-control helpers from the original Field class
// (gl/src/gl_field.cpp), on the W APIs. Windows only.
//

#ifdef _WIN32

#include "Field4.h"
#include "Internal.h"

#include <string>

namespace cb4sqlite {

int Field4::SetField(HWND hParent, UINT controlId) const
{
    if (!data_) return e4fieldName;
    // As the original: the buffered text as-is, trailing padding included.
    const std::string_view text = str();
    const int n = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), n);
    return SetDlgItemTextW(hParent, static_cast<int>(controlId), wide.c_str()) ? r4success : e4parm;
}

int Field4::AssignField(HWND hParent, UINT controlId)
{
    if (!data_) return e4fieldName;
    HWND ctl = GetDlgItem(hParent, static_cast<int>(controlId));
    if (!ctl) return e4parm;
    std::wstring wide(static_cast<size_t>(GetWindowTextLengthW(ctl)) + 1, L'\0');
    wide.resize(static_cast<size_t>(GetWindowTextW(ctl, wide.data(), static_cast<int>(wide.size()))));
    const int n = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                                      nullptr, 0, nullptr, nullptr);
    std::string text(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), text.data(), n,
                        nullptr, nullptr);
    if (detail::isNumericType(type())) {
        // The original read tbFloat controls with FloatGetValue; here the
        // control text is parsed, ignoring thousands separators.
        std::string digits;
        for (char c : text)
            if (c != ',') digits += c;
        return assign(digits.c_str());
    }
    return assign(text.c_str());
}

} // namespace cb4sqlite

#endif // _WIN32
