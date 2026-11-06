//
// MemoFile.cpp - .dbt memo files (dBase III and IV). See DbtFile in Source.h.
//

#include "Source.h"

#include <cstdint>
#include <fstream>
#include <sstream>

namespace dbf2sqlite::detail {

namespace {

std::uint32_t le32(const std::string& d, size_t at)
{
    const auto b = [&](size_t i) { return static_cast<std::uint32_t>(static_cast<unsigned char>(d[at + i])); };
    return b(0) | (b(1) << 8) | (b(2) << 16) | (b(3) << 24);
}

} // namespace

DbtFile::DbtFile(const std::filesystem::path& path, int dbfVersion)
    : path_(path), dBase4_(dbfVersion != 0x83)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw SourceError("cannot open memo file " + pathUtf8(path));
    std::stringstream ss;
    ss << in.rdbuf();
    data_ = ss.str();
    if (data_.size() < 512)
        throw SourceError(pathUtf8(path) + ": memo file shorter than its 512-byte header");
    if (dBase4_) {
        const size_t bs = static_cast<unsigned char>(data_[20]) | (static_cast<size_t>(static_cast<unsigned char>(data_[21])) << 8);
        if (bs != 0) {
            if (bs < 64 || bs % 64 != 0)
                throw SourceError(pathUtf8(path) + ": memo block size " + std::to_string(bs) + " is not a dBase IV layout");
            blockSize_ = bs;
        }
    }
}

std::string DbtFile::read(long block) const
{
    const std::string where = pathUtf8(path_) + " block " + std::to_string(block);
    if (block < 1)
        throw SourceError(where + ": bad memo block number");
    const size_t at = static_cast<size_t>(block) * blockSize_;
    if (at >= data_.size())
        throw SourceError(where + ": past the end of the memo file");
    if (dBase4_ && data_.size() - at >= 8 && le32(data_, at) == 0x0008FFFFu) {
        const std::uint32_t len = le32(data_, at + 4);     // includes the 8-byte header
        if (len < 8 || len - 8 > data_.size() - at - 8)
            throw SourceError(where + ": memo length " + std::to_string(len) + " runs past the end of the file");
        return data_.substr(at + 8, len - 8);
    }
    if (dBase4_ && blockSize_ != 512)
        throw SourceError(where + ": no dBase IV memo header");
    // dBase III style (also what dBase IV wrote for files converted from III):
    // text up to the first 0x1A, or 0x1A 0x1A.
    const size_t end = data_.find('\x1a', at);
    return data_.substr(at, (end == std::string::npos ? data_.size() : end) - at);
}

} // namespace dbf2sqlite::detail
