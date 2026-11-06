//
// stub_impl.cpp
//
// M0 stub bodies for cb4sqlite.  Every method throws cb4sqlite::Error so that
// any accidental call surfaces immediately; M1 replaces this file with the
// real implementations.
//
// The cb global is defined here so that App.cpp can `extern Code4 cb;` against
// it from M5 onward.  Until M5 wires up an open(), `cb.handle()` returns null.
//

#include "cb4sqlite.h"

namespace cb4sqlite {

// --- global cb instance -----------------------------------------------------
Code4 cb;

// --- Code4 ------------------------------------------------------------------
[[noreturn]] static void notImpl(const char* what) {
    throw Error(std::string("cb4sqlite M0 stub: not implemented: ") + what);
}

int      Code4::open(const char*) { notImpl("Code4::open"); }
void     Code4::closeAll()        { notImpl("Code4::closeAll"); }
void     Code4::initUndo()        { notImpl("Code4::initUndo"); }
sqlite3* Code4::handle() noexcept { return nullptr; }

// --- Data4 ------------------------------------------------------------------
Data4::Data4(Code4*, const char*) {}
Data4::~Data4() = default;

int  Data4::top()                { notImpl("Data4::top"); }
int  Data4::bottom()             { notImpl("Data4::bottom"); }
int  Data4::skip(long)           { notImpl("Data4::skip"); }
int  Data4::seek(const char*)    { notImpl("Data4::seek(str)"); }
int  Data4::seek(long)           { notImpl("Data4::seek(long)"); }
int  Data4::append()             { notImpl("Data4::append"); }
int  Data4::flush()              { return 0; /* spec §4.6: no-op */ }
long Data4::recCount() noexcept  { return 0; }
long Data4::recNo()    noexcept  { return 0; }
bool Data4::eof()      noexcept  { return true; }
int  Data4::numFields() const noexcept { return 0; }
int  Data4::SetIndex(const char*) { notImpl("Data4::SetIndex"); }
int  Data4::go(long)             { notImpl("Data4::go"); }
bool Data4::isValid() const noexcept { return false; }
void Data4::close()              {}

// --- Field4 -----------------------------------------------------------------
Field4::Field4(Data4&, int) {}
Field4::Field4(Data4&, const char*) {}
Field4::~Field4() = default;

const char* Field4::str()  const           { return ""; }
double      Field4::num()  const           { return 0.0; }
long        Field4::len()  const noexcept  { return 0; }
const char* Field4::name() const noexcept  { return ""; }
int         Field4::assign(const char*)    { notImpl("Field4::assign(str)"); }
int         Field4::assign(double)         { notImpl("Field4::assign(double)"); }
int         Field4::assignDouble(double)   { notImpl("Field4::assignDouble"); }
int         Field4::SetField(HWND, UINT)   { notImpl("Field4::SetField"); }
int         Field4::AssignField(HWND, UINT){ notImpl("Field4::AssignField"); }
bool        Field4::isValid() const noexcept { return false; }

// --- Date4 ------------------------------------------------------------------
void  Date4::today()           { /* M1: fill year_/month_/day_ from local time */ }
std::string Date4::str() const { return std::string(8, '0'); }

// --- Relate4set -------------------------------------------------------------
Relate4set::Relate4set(Data4*) {}
Relate4set::~Relate4set() = default;

int  Relate4set::querySet(const char*) { notImpl("Relate4set::querySet"); }
int  Relate4set::top()                 { notImpl("Relate4set::top"); }
int  Relate4set::skip(long)            { notImpl("Relate4set::skip"); }
bool Relate4set::eof() const noexcept  { return true; }
void Relate4set::unlock()              {}
void Relate4set::free()                {}

} // namespace cb4sqlite
