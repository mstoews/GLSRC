//
// Layer-1 tests: Field4::SetField / AssignField against a real EDIT control.
// Windows only (the portable test build skips this file's contents).
//

#ifdef _WIN32

#include "TestDb.h"

#include <Windows.h>

using namespace test;

namespace {

constexpr UINT kEditId = 100;

// A hidden top-level window with one EDIT child, standing in for a dialog.
struct HiddenDialog {
    HWND parent = nullptr;
    HiddenDialog()
    {
        parent = CreateWindowExW(0, L"STATIC", L"", WS_OVERLAPPED, 0, 0, 200, 100,
                                 nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        REQUIRE(parent != nullptr);
        HWND edit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD, 0, 0, 180, 20, parent,
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditId)),
                                    GetModuleHandleW(nullptr), nullptr);
        REQUIRE(edit != nullptr);
    }
    ~HiddenDialog() { DestroyWindow(parent); }

    std::wstring text() const
    {
        wchar_t buf[256] = {};
        GetDlgItemTextW(parent, static_cast<int>(kEditId), buf, 256);
        return buf;
    }
    void setText(const wchar_t* t) const { SetDlgItemTextW(parent, static_cast<int>(kEditId), t); }
};

} // namespace

TEST_CASE("SetField copies the buffered text into the control", "[field4][win32]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    createAccounts(code);
    Data4 d(code, "accts");
    addAccount(d, "100", "01", "Caf\xC3\xA9", 12.5);       // UTF-8 "Café"
    HiddenDialog dlg;

    REQUIRE(Field4(d, "descript").SetField(dlg.parent, kEditId) == r4success);
    CHECK(dlg.text() == L"Caf\u00e9               ");     // 20 bytes: "Café" is 5 in UTF-8
    REQUIRE(Field4(d, "curr_bal").SetField(dlg.parent, kEditId) == r4success);
    CHECK(dlg.text() == L"       12.50");
}

TEST_CASE("AssignField reads the control back into the field", "[field4][win32]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    createAccounts(code);
    Data4 d(code, "accts");
    addAccount(d, "100", "01", "Cash", 0);
    HiddenDialog dlg;

    dlg.setText(L"Petty \u00e9");
    Field4 descript(d, "descript");
    REQUIRE(descript.AssignField(dlg.parent, kEditId) == r4success);
    CHECK(std::string(descript.str()) == "Petty \xC3\xA9            ");

    dlg.setText(L"1,234.50");                               // numeric: parsed, separators ignored
    Field4 bal(d, "curr_bal");
    REQUIRE(bal.AssignField(dlg.parent, kEditId) == r4success);
    CHECK(std::string(bal.str()) == "     1234.50");

    REQUIRE(d.flush() == r4success);
    CHECK(raw(code, "SELECT curr_bal FROM accts") == "1234.5");
    CHECK(Field4(d, "dept").AssignField(dlg.parent, 999) == e4parm);   // no such control
}

#endif // _WIN32
