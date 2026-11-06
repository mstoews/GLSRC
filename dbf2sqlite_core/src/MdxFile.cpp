//
// MdxFile.cpp - tag definitions from a dBase IV production index (.mdx).
// Only the headers are read; the B-tree pages are not needed because the
// SQLite index is rebuilt from the key expression.
//

#include "Source.h"

#include <cstdint>
#include <fstream>
#include <iterator>

namespace dbf2sqlite::detail {

namespace {

constexpr size_t kChunk       = 512;
constexpr size_t kSlotsOffset = 544;   // 512 + 32
constexpr size_t kSlotSize    = 32;
constexpr size_t kExprOffset  = 24;
constexpr size_t kFilterOffset = 0x2FA;
constexpr size_t kExprMax     = 220;

std::uint16_t u16(const std::string& b, size_t at)
{
    return static_cast<std::uint16_t>(static_cast<unsigned char>(b[at]) |
                                      (static_cast<unsigned char>(b[at + 1]) << 8));
}

std::uint32_t u32(const std::string& b, size_t at)
{
    return static_cast<std::uint32_t>(u16(b, at)) | (static_cast<std::uint32_t>(u16(b, at + 2)) << 16);
}

// NUL-terminated string of at most `max` bytes at `at`; must be printable.
std::string cstr(const std::string& b, size_t at, size_t max, const std::string& what)
{
    std::string out;
    for (size_t i = 0; i < max && at + i < b.size(); ++i) {
        const auto c = static_cast<unsigned char>(b[at + i]);
        if (c == 0) return out;
        if (c < 0x20 || c > 0x7E)
            throw SourceError(what + ": unexpected byte 0x" + "0123456789ABCDEF"[c >> 4] +
                              "0123456789ABCDEF"[c & 15] + " - not a dBase IV .mdx layout");
        out += static_cast<char>(c);
    }
    return out;
}

std::string trimRight(std::string s)
{
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

} // namespace

std::vector<MdxTag> readMdxTags(const std::filesystem::path& path)
{
    const std::string where = pathUtf8(path);
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw SourceError("cannot open " + where);
    const std::string b((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    if (b.size() < kSlotsOffset || b[0] != 2)
        throw SourceError(where + ": not a dBase IV .mdx file (bad version byte or too short)");
    const std::uint16_t slotSize = u16(b, 26);
    const std::uint16_t numTags  = u16(b, 28);
    if (slotSize != kSlotSize || numTags > 48)
        throw SourceError(where + ": unexpected tag table (slot size " + std::to_string(slotSize) +
                          ", " + std::to_string(numTags) + " tags)");

    std::vector<MdxTag> tags;
    for (size_t i = 0; i < numTags; ++i) {
        const size_t slot = kSlotsOffset + i * kSlotSize;
        if (slot + kSlotSize > b.size())
            throw SourceError(where + ": tag table truncated");
        const std::string ctx = where + " tag #" + std::to_string(i + 1);

        MdxTag t;
        t.name = trimRight(cstr(b, slot + 4, 10, ctx + " name"));
        const size_t hdr = static_cast<size_t>(u32(b, slot)) * kChunk;
        if (t.name.empty() || hdr < kSlotsOffset || hdr + kFilterOffset + 1 > b.size())
            throw SourceError(ctx + ": bad tag slot (name '" + t.name + "', header at " +
                              std::to_string(hdr) + ")");

        const auto typeCode = static_cast<unsigned char>(b[hdr + 8]);
        t.keyType    = b[hdr + 9];
        t.descending = (typeCode & 0x08) != 0;
        t.unique     = (typeCode & 0x40) != 0 || u16(b, hdr + 22) == 0x4000;
        t.expression = trimRight(cstr(b, hdr + kExprOffset, kExprMax, ctx + " expression"));
        t.filter     = trimRight(cstr(b, hdr + kFilterOffset, kExprMax, ctx + " filter"));
        if (t.keyType != 'C' && t.keyType != 'N' && t.keyType != 'D')
            throw SourceError(ctx + " (" + t.name + "): unknown key type");
        if (t.expression.empty())
            throw SourceError(ctx + " (" + t.name + "): empty key expression");
        tags.push_back(std::move(t));
    }
    return tags;
}

} // namespace dbf2sqlite::detail
