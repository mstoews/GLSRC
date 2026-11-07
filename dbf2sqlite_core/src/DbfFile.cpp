//
// DbfFile.cpp - .dbf reading through the vendored shapelib.
//

#include "Source.h"

#include "../vendor/shapelib/shapefil.h"

#include <cstdint>
#include <cstdio>
#include <fstream>

namespace dbf2sqlite::detail {

std::string pathUtf8(const std::filesystem::path& p)
{
    const std::u8string u = p.u8string();
    return std::string(u.begin(), u.end());
}

namespace {

// shapelib always opens "<base>.dbf" / "<base>.DBF" and then looks for a .cpg.
// This hook opens the real file instead (so ACCESS.PSW works) and reports no
// .cpg. pvUserData holds the real path (UTF-8).
struct OpenContext {
    std::string realPath;
    SAHooks     base;
    std::string lastError;
};

thread_local OpenContext* g_ctx = nullptr;

bool endsWithDbf(const char* name)
{
    const std::string s(name ? name : "");
    if (s.size() < 4) return false;
    std::string ext = s.substr(s.size() - 4);
    for (char& c : ext) c = static_cast<char>(c | 0x20);
    return ext == ".dbf";
}

SAFile hookOpen(const char* filename, const char* access, void* user)
{
    auto* ctx = static_cast<OpenContext*>(user);
    if (!endsWithDbf(filename))
        return nullptr;   // .cpg: none
    return ctx->base.FOpen(ctx->realPath.c_str(), access, ctx->base.pvUserData);
}

void hookError(const char* message)
{
    if (g_ctx) g_ctx->lastError = message ? message : "";
}

} // namespace

DbfFile::DbfFile(const std::filesystem::path& path)
{
    // shapelib copies the hooks (and so the pvUserData pointer) into the
    // handle; the context lives as long as the handle does.
    auto* owned = new OpenContext;
    ctx_ = owned;
    OpenContext& ctx = *owned;
    ctx.realPath = pathUtf8(path);
#ifdef _WIN32
    SASetupUtf8Hooks(&ctx.base);       // UTF-8 path -> _wfopen
#else
    SASetupDefaultHooks(&ctx.base);
#endif
    SAHooks hooks = ctx.base;
    hooks.FOpen = hookOpen;
    hooks.Error = hookError;
    hooks.pvUserData = &ctx;
    g_ctx = &ctx;
    DBFHandle h = DBFOpenLL(ctx.realPath.c_str(), "rb", &hooks);
    g_ctx = nullptr;
    if (!h) {
        const std::string msg = "cannot open dBase file " + ctx.realPath +
                                (ctx.lastError.empty() ? "" : ": " + ctx.lastError);
        delete owned;            // the destructor won't run for a throwing ctor
        ctx_ = nullptr;
        throw SourceError(msg);
    }
    h_ = h;
    {
        std::ifstream in(path, std::ios::binary);
        char b = 0;
        if (in.get(b)) version_ = static_cast<unsigned char>(b);
    }

    const int n = DBFGetFieldCount(h);
    int offset = 0;
    for (int i = 0; i < n; ++i) {
        char name[XBASE_FLDNAME_LEN_READ + 1] = {};
        int width = 0, dec = 0;
        DBFGetFieldInfo(h, i, name, &width, &dec);
        DbfField f;
        f.name = name;
        f.type = DBFGetNativeFieldType(h, i);
        f.len = width;
        f.dec = dec;
        f.offset = offset;
        offset += width;
        fields_.push_back(std::move(f));
    }
    recLen_ = offset;
    records_ = DBFGetRecordCount(h);

    // Language driver id: header byte 29 (shapelib exposes it only as "LDID/n").
    if (const char* cp = DBFGetCodePage(h); cp && sscanf_s(cp, "LDID/%d", &ldid_) != 1)
        ldid_ = 0;
}

DbfFile::~DbfFile()
{
    if (h_) DBFClose(static_cast<DBFHandle>(h_));
    delete static_cast<OpenContext*>(ctx_);
}

std::string_view DbfFile::record(int i)
{
    const char* raw = DBFReadTuple(static_cast<DBFHandle>(h_), i);
    if (!raw)
        throw SourceError("cannot read record " + std::to_string(i + 1));
    return std::string_view(raw + 1, static_cast<size_t>(recLen_));
}

bool DbfFile::deleted(int i)
{
    return DBFIsRecordDeleted(static_cast<DBFHandle>(h_), i) != 0;
}

bool looksLikeDbf(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    unsigned char h[32] = {};
    if (!in.read(reinterpret_cast<char*>(h), sizeof h))
        return false;
    switch (h[0]) {
        case 0x02: case 0x03: case 0x04: case 0x05: case 0x30: case 0x43:
        case 0x63: case 0x83: case 0x8B: case 0x8E: case 0xCB: case 0xF5:
            break;
        default:
            return false;
    }
    const auto u = [&](int i) { return static_cast<std::uint32_t>(h[i]); };
    const std::uint32_t records   = u(4) | (u(5) << 8) | (u(6) << 16) | (u(7) << 24);
    const std::uint32_t headerLen = u(8) | (u(9) << 8);
    const std::uint32_t recordLen = u(10) | (u(11) << 8);
    // 32-byte header + 32 bytes per field + 0x0D (+263 bytes in some variants).
    if (headerLen < 33 || headerLen > 32 + 32 * 255 + 1 + 263 || recordLen < 1)
        return false;
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec) return false;
    const std::uint64_t expected = headerLen + static_cast<std::uint64_t>(records) * recordLen;
    return size >= expected && size <= expected + 1;   // optional 0x1A end marker
}

} // namespace dbf2sqlite::detail
