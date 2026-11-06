//
// Encoding.cpp - source code page -> UTF-8.
//
// CodeBase stored text exactly as the Win16 app passed it: bytes in the
// machine's ANSI code page (1252 for Western installs, 932 = Shift-JIS for the
// Japanese one in eiwa/). The dBase language-driver byte, when set, says which.
//

#include "Source.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdint>

namespace dbf2sqlite::detail {

int codePageFromLdid(int ldid) noexcept
{
    switch (ldid) {
        case 0x01: return 437;
        case 0x02: return 850;
        case 0x03: case 0x57: case 0x58: case 0x59: return 1252;
        case 0x13: case 0x7B: return 932;
        case 0x64: return 852;
        case 0x65: return 866;
        case 0x78: return 950;
        case 0x79: return 949;
        case 0x7A: return 936;
        case 0x7C: return 874;
        case 0x7D: return 1255;
        case 0x7E: return 1256;
        case 0xC8: return 1250;
        case 0xC9: return 1251;
        case 0xCA: return 1254;
        case 0xCB: return 1253;
        default:   return 0;
    }
}

namespace {

void appendUtf8(std::string& out, std::uint32_t cp)
{
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

// 0x80..0x9F of Windows-1252 (0 = undefined there; mapped to U+FFFD).
constexpr std::uint16_t kCp1252High[32] = {
    0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
    0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178,
};

// 0x80..0xFF of code page 437.
constexpr std::uint16_t kCp437High[128] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
};

bool isAscii(std::string_view s) noexcept
{
    for (char c : s)
        if (static_cast<unsigned char>(c) >= 0x80) return false;
    return true;
}

} // namespace

std::string toUtf8(std::string_view bytes, int codePage)
{
    if (isAscii(bytes))
        return std::string(bytes);

    std::string out;
    out.reserve(bytes.size() * 2);
    switch (codePage) {
        case 1252:
        case 28591:
            for (char ch : bytes) {
                const auto c = static_cast<unsigned char>(ch);
                std::uint32_t cp = c;
                if (codePage == 1252 && c >= 0x80 && c < 0xA0)
                    cp = kCp1252High[c - 0x80] ? kCp1252High[c - 0x80] : 0xFFFD;
                appendUtf8(out, cp);
            }
            return out;
        case 437:
            for (char ch : bytes) {
                const auto c = static_cast<unsigned char>(ch);
                appendUtf8(out, c < 0x80 ? c : kCp437High[c - 0x80]);
            }
            return out;
        default:
            break;
    }
#ifdef _WIN32
    const int n = static_cast<int>(bytes.size());
    const UINT cp = static_cast<UINT>(codePage);
    const int wlen = MultiByteToWideChar(cp, 0, bytes.data(), n, nullptr, 0);
    if (wlen > 0) {
        std::wstring w(static_cast<size_t>(wlen), L'\0');
        MultiByteToWideChar(cp, 0, bytes.data(), n, w.data(), wlen);
        const int ulen = WideCharToMultiByte(CP_UTF8, 0, w.data(), wlen, nullptr, 0, nullptr, nullptr);
        out.assign(static_cast<size_t>(ulen), '\0');
        WideCharToMultiByte(CP_UTF8, 0, w.data(), wlen, out.data(), ulen, nullptr, nullptr);
        return out;
    }
#endif
    throw SourceError("code page " + std::to_string(codePage) +
                      " is not available on this platform (built in: 1252, 28591, 437)");
}

} // namespace dbf2sqlite::detail
