//
// ReportDesigner.cpp - see ReportDesigner.h.
//
// Translation notes (plan per-port checklist; decisions are spec #7 §8):
//   - D5: the original's GL_ADDREPORT took a description, a name and a .RPT
//     path; Modify appended a duplicate row (spec #7 2.4 #2). The designer
//     edits a definition, and Save replaces the row it opened.
//   - D9: a field the source no longer has stays in the definition, marked
//     "(missing)" in the columns and filters; Preview leaves it out with a
//     note, as the report does when run.
//   - Asked filters ("ask when run") come in S7-M3: a definition that has one
//     keeps it, and the filter list shows it, but the designer doesn't make
//     one yet.
//   - The working copy is the dialog's ReportDefinition: each control writes
//     its part as it changes, so Preview and Save read one place.
//
#include "ReportDesigner.h"

#include "../Accounts.h"   // TrimText
#include "../Globals.h"
#include "../MainDialog.h"
#include "../ReportWriter.h"
#include "RunReport.h"     // CurrentReportContext

#include "report/win32/Preview.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

#include "../PortIds.h"

#include "../LegacyIds.h"   // last: see LegacyIds.h

namespace {

std::wstring wide(const std::string& s) { return fx::Utf8ToWide(s); }

std::string now()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%04u%02u%02u%02u%02u%02u", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return buf;
}

// The tests the filter combo offers, in order, with their words.
const struct {
    rw::Test test;
    const wchar_t* text;
} kTests[] = {
    {rw::Test::Equal, L"is"},           {rw::Test::NotEqual, L"is not"}, {rw::Test::Between, L"from ... to"},
    {rw::Test::In, L"one of"},          {rw::Test::Contains, L"contains"}, {rw::Test::Blank, L"is blank"},
    {rw::Test::NotBlank, L"isn't blank"},
};

const int kSorts[] = {IDC_RW_SORT1, IDC_RW_SORT2, IDC_RW_SORT3};
const int kDescs[] = {IDC_RW_DESC1, IDC_RW_DESC2, IDC_RW_DESC3};
const int kGroups[] = {IDC_RW_GROUP1, IDC_RW_GROUP2};
const int kPages[] = {IDC_RW_PAGE1, IDC_RW_PAGE2};

class DesignerDialog : public MainDialog {
public:
    DesignerDialog(long number, bool copy, bool closeAtOnce) : number_(number), copy_(copy), closeAtOnce_(closeAtOnce) {}

    long saved = 0;

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        for (const rw::Source& s : rw::Sources()) Combo(IDC_RW_SOURCE, CB_ADDSTRING, 0, wide(s.name).c_str());
        for (const auto& t : kTests) Combo(IDC_RW_FTEST, CB_ADDSTRING, 0, t.text);
        Combo(IDC_RW_FTEST, CB_SETCURSEL, 0, nullptr);
        Load();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        SetFocus(GetDlgItem(hwnd(), IDC_RW_NAME));
        return false;   // the focus is set
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (filling_) return true;
        switch (id) {
        case IDC_RW_NAME:
        case IDC_RW_TITLE:
        case IDC_RW_DESCRIPTION:
        case IDC_RW_SUBTITLE:
            if (code == EN_CHANGE) ReadHeader();
            return true;
        case IDC_RW_SOURCE:
            if (code == CBN_SELCHANGE) SourceChanged();
            return true;
        case IDC_RW_FIELDS:
            if (code == LBN_DBLCLK) AddColumn();
            return true;
        case IDC_RW_COLUMNS:
            if (code == LBN_SELCHANGE) ShowColumn();
            return true;
        case IDC_RW_COLLABEL:
        case IDC_RW_COLWIDTH:
            if (code == EN_CHANGE) ReadColumn();
            return true;
        case IDC_RW_FFIELD:
        case IDC_RW_FTEST:
            if (code == CBN_SELCHANGE) ShowFilterInputs();
            return true;
        case IDC_RW_FILTERS:
            return true;
        default: break;
        }
        for (int k = 0; k < 3; ++k)
            if (id == kSorts[k] && code == CBN_SELCHANGE) {
                ReadSortGroups();
                return true;
            }
        for (int k = 0; k < 2; ++k)
            if (id == kGroups[k] && code == CBN_SELCHANGE) {
                ReadSortGroups();
                return true;
            }
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDC_RW_LANDSCAPE:
        case IDC_RW_GRANDTOTAL:
        case IDC_RW_COUNT: ReadHeader(); return true;
        case IDC_RW_ADDCOL: AddColumn(); return true;
        case IDC_RW_REMOVECOL: RemoveColumn(); return true;
        case IDC_RW_UP: MoveColumn(-1); return true;
        case IDC_RW_DOWN: MoveColumn(1); return true;
        case IDC_RW_COLTOTAL:
        case IDC_RW_COLBLANK: ReadColumn(); return true;
        case IDC_RW_DESC1:
        case IDC_RW_DESC2:
        case IDC_RW_DESC3:
        case IDC_RW_PAGE1:
        case IDC_RW_PAGE2: ReadSortGroups(); return true;
        case IDC_RW_FADD: AddFilter(); return true;
        case IDC_RW_FREMOVE: RemoveFilter(); return true;
        case IDC_RW_PREVIEW: Preview(); return true;
        case IDC_RW_SAVE: Save(); return true;
        case IDCANCEL:
            if (AskToSave()) End(saved ? IDOK : IDCANCEL);   // not MainDialog::Cancel: the list's tables stay open
            return true;
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    // ---- the controls ----------------------------------------------------------------

    LRESULT Combo(int id, UINT msg, WPARAM wp, const wchar_t* text) const
    {
        return SendDlgItemMessageW(hwnd(), id, msg, wp, reinterpret_cast<LPARAM>(text));
    }
    LRESULT List(int id, UINT msg, WPARAM wp = 0, LPARAM lp = 0) const { return SendDlgItemMessageW(hwnd(), id, msg, wp, lp); }
    std::string Text(int id) const { return fx::WideToUtf8(GetFieldW(id)); }

    std::wstring GetFieldW(int id) const
    {
        const int n = GetWindowTextLengthW(GetDlgItem(hwnd(), id));
        std::wstring s(static_cast<size_t>(n) + 1, L'\0');
        GetDlgItemTextW(hwnd(), id, s.data(), n + 1);
        s.resize(static_cast<size_t>(n));
        return s;
    }

    const rw::Source* Src() const { return rw::FindSource(def_.source); }

    std::wstring FieldLabel(const std::string& key) const
    {
        const rw::Source* s = Src();
        const int i = s ? s->fieldIndex(key) : -1;
        return i < 0 ? wide(key) + L" (missing)" : wide(s->fields[static_cast<size_t>(i)].label);
    }

    // ---- loading -----------------------------------------------------------------------

    void Load()
    {
        if (number_) {
            try {
                if (const auto stored = rw::FindDefinition(cb, number_)) {
                    const rw::ParseResult p = rw::ParseDefinition(stored->text);
                    if (p.definition) {
                        def_ = *p.definition;
                    } else {
                        Message(fx::Format(L"The report can't be read (line %d):\n\n%s\n\nIt opens empty: saving replaces it.",
                                           p.line, wide(p.error).c_str()),
                                MB_ICONEXCLAMATION);
                        def_.name = stored->name;
                    }
                }
            } catch (const std::exception& e) {
                Message(L"The report could not be read:\n\n" + wide(e.what()), MB_ICONSTOP);
            }
        }
        if (def_.source.empty() && !rw::Sources().empty()) {
            def_.source = rw::Sources().front().key;
            def_.sourceVersion = rw::Sources().front().version;
        }
        if (copy_) {
            def_.name = rw::CopyName(cb, def_.name);
            number_ = 0;   // saved as a new report
        }
        ShowAll();
        dirty_ = copy_;
        Caption();
        const std::vector<std::string> missing = rw::MissingFields(def_);
        if (!missing.empty()) {
            std::wstring list;
            for (const std::string& m : missing) list += L"\n  " + wide(m);
            Message(L"These fields aren't in the source any more; they are marked \"(missing)\" and left out when the "
                    L"report runs (remove them and save to tidy the report):" + list,
                    MB_ICONINFORMATION);
        }
    }

    void Caption()
    {
        std::wstring c = number_ ? L"Report Designer - " + wide(def_.name) : L"Report Designer - new report";
        if (dirty_) c += L" *";
        SetWindowTextW(hwnd(), c.c_str());
    }

    void ShowAll()
    {
        filling_ = true;
        SetDlgItemTextW(hwnd(), IDC_RW_NAME, wide(def_.name).c_str());
        SetDlgItemTextW(hwnd(), IDC_RW_TITLE, wide(def_.title == def_.name ? std::string() : def_.title).c_str());
        SetDlgItemTextW(hwnd(), IDC_RW_DESCRIPTION,
                        wide(def_.description == def_.name ? std::string() : def_.description).c_str());
        SetDlgItemTextW(hwnd(), IDC_RW_SUBTITLE, wide(def_.subtitle).c_str());
        SetCheck(IDC_RW_LANDSCAPE, def_.landscape);
        SetCheck(IDC_RW_GRANDTOTAL, def_.grandTotal);
        SetCheck(IDC_RW_COUNT, def_.count);
        int sel = -1;
        for (size_t i = 0; i < rw::Sources().size(); ++i)
            if (rw::Sources()[i].key == def_.source) sel = static_cast<int>(i);
        Combo(IDC_RW_SOURCE, CB_SETCURSEL, static_cast<WPARAM>(sel), nullptr);
        filling_ = false;
        ShowSource();
    }

    // Everything that depends on the source: its description, its fields
    // (the field list, sort, group and filter combos), the columns and filters.
    void ShowSource()
    {
        filling_ = true;
        const rw::Source* s = Src();
        SetDlgItemTextW(hwnd(), IDC_RW_SOURCEINFO, s ? wide(s->description).c_str() : L"");
        List(IDC_RW_FIELDS, LB_RESETCONTENT);
        Combo(IDC_RW_FFIELD, CB_RESETCONTENT, 0, nullptr);
        for (int id : {IDC_RW_SORT1, IDC_RW_SORT2, IDC_RW_SORT3, IDC_RW_GROUP1, IDC_RW_GROUP2}) {
            Combo(id, CB_RESETCONTENT, 0, nullptr);
            Combo(id, CB_ADDSTRING, 0, L"(none)");
        }
        if (s)
            for (const rw::Field& f : s->fields) {
                const std::wstring label = wide(f.label);
                List(IDC_RW_FIELDS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
                Combo(IDC_RW_FFIELD, CB_ADDSTRING, 0, label.c_str());
                for (int id : {IDC_RW_SORT1, IDC_RW_SORT2, IDC_RW_SORT3, IDC_RW_GROUP1, IDC_RW_GROUP2})
                    Combo(id, CB_ADDSTRING, 0, label.c_str());
            }
        Combo(IDC_RW_FFIELD, CB_SETCURSEL, 0, nullptr);
        for (int k = 0; k < 3; ++k) {
            const int i = k < static_cast<int>(def_.sort.size()) && s ? s->fieldIndex(def_.sort[static_cast<size_t>(k)].field) : -1;
            Combo(kSorts[k], CB_SETCURSEL, static_cast<WPARAM>(i + 1), nullptr);
            SetCheck(kDescs[k], k < static_cast<int>(def_.sort.size()) && def_.sort[static_cast<size_t>(k)].descending);
        }
        for (int k = 0; k < 2; ++k) {
            const int i = k < static_cast<int>(def_.groups.size()) && s ? s->fieldIndex(def_.groups[static_cast<size_t>(k)].field) : -1;
            Combo(kGroups[k], CB_SETCURSEL, static_cast<WPARAM>(i + 1), nullptr);
            SetCheck(kPages[k], k < static_cast<int>(def_.groups.size()) && def_.groups[static_cast<size_t>(k)].newPage);
        }
        filling_ = false;
        ShowColumns(0);
        ShowFilters();
        ShowFilterInputs();
    }

    void ShowColumns(int select)
    {
        filling_ = true;
        List(IDC_RW_COLUMNS, LB_RESETCONTENT);
        for (const rw::ColumnDef& c : def_.columns) {
            std::wstring t = c.label.empty() ? FieldLabel(c.field) : wide(c.label);
            if (c.total) t += L"  [total]";
            List(IDC_RW_COLUMNS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t.c_str()));
        }
        if (!def_.columns.empty())
            List(IDC_RW_COLUMNS, LB_SETCURSEL, static_cast<WPARAM>(std::clamp(select, 0, static_cast<int>(def_.columns.size()) - 1)));
        filling_ = false;
        ShowColumn();
    }

    int SelectedColumn() const
    {
        const LRESULT i = List(IDC_RW_COLUMNS, LB_GETCURSEL);
        return i == LB_ERR || i >= static_cast<LRESULT>(def_.columns.size()) ? -1 : static_cast<int>(i);
    }

    void ShowColumn()
    {
        filling_ = true;
        const int i = SelectedColumn();
        const bool on = i >= 0;
        for (int id : {IDC_RW_COLLABEL, IDC_RW_COLWIDTH, IDC_RW_COLTOTAL, IDC_RW_COLBLANK}) EnableWindow(GetDlgItem(hwnd(), id), on);
        if (on) {
            const rw::ColumnDef& c = def_.columns[static_cast<size_t>(i)];
            SetDlgItemTextW(hwnd(), IDC_RW_COLLABEL, wide(c.label).c_str());
            SetDlgItemTextW(hwnd(), IDC_RW_COLWIDTH, c.width > 0 ? fx::Format(L"%g", c.width).c_str() : L"");
            SetCheck(IDC_RW_COLTOTAL, c.total);
            SetCheck(IDC_RW_COLBLANK, c.blankZero);
            // A total only means something for amounts and numbers.
            const rw::Source* s = Src();
            const int f = s ? s->fieldIndex(c.field) : -1;
            const bool numeric = f >= 0 && (s->fields[static_cast<size_t>(f)].type == rw::FieldType::Amount ||
                                            s->fields[static_cast<size_t>(f)].type == rw::FieldType::Integer);
            EnableWindow(GetDlgItem(hwnd(), IDC_RW_COLTOTAL), numeric);
            EnableWindow(GetDlgItem(hwnd(), IDC_RW_COLBLANK), numeric);
        } else {
            for (int id : {IDC_RW_COLLABEL, IDC_RW_COLWIDTH}) SetDlgItemTextW(hwnd(), id, L"");
            SetCheck(IDC_RW_COLTOTAL, false);
            SetCheck(IDC_RW_COLBLANK, false);
        }
        filling_ = false;
    }

    void ShowFilters()
    {
        List(IDC_RW_FILTERS, LB_RESETCONTENT);
        const rw::Source* s = Src();
        for (const rw::Filter& f : def_.filters) {
            const std::wstring t = s ? wide(rw::DescribeFilter(*s, f)) : wide(f.field);
            List(IDC_RW_FILTERS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t.c_str()));
        }
    }

    rw::Test SelectedTest() const
    {
        const LRESULT i = Combo(IDC_RW_FTEST, CB_GETCURSEL, 0, nullptr);
        return i >= 0 && i < static_cast<LRESULT>(std::size(kTests)) ? kTests[i].test : rw::Test::Equal;
    }

    // The value boxes the chosen test takes: none, one, or two (from ... to).
    void ShowFilterInputs()
    {
        const rw::Test t = SelectedTest();
        const bool none = t == rw::Test::Blank || t == rw::Test::NotBlank;
        ShowWindow(GetDlgItem(hwnd(), IDC_RW_FVALUE1), none ? SW_HIDE : SW_SHOW);
        ShowWindow(GetDlgItem(hwnd(), IDC_RW_FVALUE2), t == rw::Test::Between ? SW_SHOW : SW_HIDE);
    }

    // ---- reading the controls into the definition ----------------------------------

    void Changed()
    {
        if (!dirty_) {
            dirty_ = true;
            Caption();
        }
    }

    void ReadHeader()
    {
        def_.name = TrimText(Text(IDC_RW_NAME));
        const std::string title = TrimText(Text(IDC_RW_TITLE)), desc = TrimText(Text(IDC_RW_DESCRIPTION));
        def_.title = title.empty() ? def_.name : title;
        def_.description = desc.empty() ? def_.name : desc;
        def_.subtitle = TrimText(Text(IDC_RW_SUBTITLE));
        def_.landscape = IsChecked(IDC_RW_LANDSCAPE);
        def_.grandTotal = IsChecked(IDC_RW_GRANDTOTAL);
        def_.count = IsChecked(IDC_RW_COUNT);
        Changed();
    }

    void SourceChanged()
    {
        const LRESULT i = Combo(IDC_RW_SOURCE, CB_GETCURSEL, 0, nullptr);
        if (i < 0 || i >= static_cast<LRESULT>(rw::Sources().size())) return;
        const rw::Source& next = rw::Sources()[static_cast<size_t>(i)];
        if (next.key == def_.source) return;
        const bool any = !def_.columns.empty() || !def_.sort.empty() || !def_.groups.empty() || !def_.filters.empty();
        if (any && Message(L"Change the source? The columns, sort, groups and filters are cleared: they name the old "
                           L"source's fields.",
                           MB_YESNO | MB_ICONQUESTION) != IDYES) {
            ShowAll();   // puts the old source back in the combo
            return;
        }
        def_.source = next.key;
        def_.sourceVersion = next.version;
        def_.columns.clear();
        def_.sort.clear();
        def_.groups.clear();
        def_.filters.clear();
        Changed();
        ShowSource();
    }

    void AddColumn()
    {
        const rw::Source* s = Src();
        const LRESULT f = List(IDC_RW_FIELDS, LB_GETCURSEL);
        if (!s || f == LB_ERR || f >= static_cast<LRESULT>(s->fields.size())) {
            MessageBeep(MB_OK);
            return;
        }
        const rw::Field& field = s->fields[static_cast<size_t>(f)];
        rw::ColumnDef c;
        c.field = field.key;
        c.total = field.type == rw::FieldType::Amount;   // amounts are usually totalled
        const int at = SelectedColumn() < 0 ? static_cast<int>(def_.columns.size()) : SelectedColumn() + 1;
        def_.columns.insert(def_.columns.begin() + at, c);
        Changed();
        ShowColumns(at);
    }

    void RemoveColumn()
    {
        const int i = SelectedColumn();
        if (i < 0) return;
        def_.columns.erase(def_.columns.begin() + i);
        Changed();
        ShowColumns(i);
    }

    void MoveColumn(int by)
    {
        const int i = SelectedColumn(), j = i + by;
        if (i < 0 || j < 0 || j >= static_cast<int>(def_.columns.size())) return;
        std::swap(def_.columns[static_cast<size_t>(i)], def_.columns[static_cast<size_t>(j)]);
        Changed();
        ShowColumns(j);
    }

    void ReadColumn()
    {
        const int i = SelectedColumn();
        if (i < 0) return;
        rw::ColumnDef& c = def_.columns[static_cast<size_t>(i)];
        c.label = TrimText(Text(IDC_RW_COLLABEL));
        const double w = std::atof(Text(IDC_RW_COLWIDTH).c_str());
        c.width = w > 0 ? w : 0;
        c.total = IsChecked(IDC_RW_COLTOTAL) && IsWindowEnabled(GetDlgItem(hwnd(), IDC_RW_COLTOTAL));
        c.blankZero = IsChecked(IDC_RW_COLBLANK) && IsWindowEnabled(GetDlgItem(hwnd(), IDC_RW_COLBLANK));
        Changed();
        // Refresh the list's text without losing the edit box's caret.
        std::wstring t = c.label.empty() ? FieldLabel(c.field) : wide(c.label);
        if (c.total) t += L"  [total]";
        filling_ = true;
        List(IDC_RW_COLUMNS, LB_DELETESTRING, static_cast<WPARAM>(i));
        List(IDC_RW_COLUMNS, LB_INSERTSTRING, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(t.c_str()));
        List(IDC_RW_COLUMNS, LB_SETCURSEL, static_cast<WPARAM>(i));
        filling_ = false;
    }

    void ReadSortGroups()
    {
        const rw::Source* s = Src();
        if (!s) return;
        def_.sort.clear();
        for (int k = 0; k < 3; ++k) {
            const LRESULT i = Combo(kSorts[k], CB_GETCURSEL, 0, nullptr);
            if (i >= 1 && i <= static_cast<LRESULT>(s->fields.size()))
                def_.sort.push_back({s->fields[static_cast<size_t>(i - 1)].key, IsChecked(kDescs[k])});
        }
        def_.groups.clear();
        for (int k = 0; k < 2; ++k) {
            const LRESULT i = Combo(kGroups[k], CB_GETCURSEL, 0, nullptr);
            if (i >= 1 && i <= static_cast<LRESULT>(s->fields.size()))
                def_.groups.push_back({s->fields[static_cast<size_t>(i - 1)].key, IsChecked(kPages[k])});
        }
        Changed();
    }

    void AddFilter()
    {
        const rw::Source* s = Src();
        const LRESULT f = Combo(IDC_RW_FFIELD, CB_GETCURSEL, 0, nullptr);
        if (!s || f < 0 || f >= static_cast<LRESULT>(s->fields.size())) {
            MessageBeep(MB_OK);
            return;
        }
        rw::Filter flt;
        flt.field = s->fields[static_cast<size_t>(f)].key;
        flt.test = SelectedTest();
        const std::string a = TrimText(Text(IDC_RW_FVALUE1)), b = TrimText(Text(IDC_RW_FVALUE2));
        switch (flt.test) {
        case rw::Test::Blank:
        case rw::Test::NotBlank: break;
        case rw::Test::Between: flt.values = {a, b}; break;
        case rw::Test::In: {
            std::string cur;
            for (char ch : a + ",") {
                if (ch == ',') {
                    if (!TrimText(cur).empty()) flt.values.push_back(TrimText(cur));
                    cur.clear();
                } else {
                    cur += ch;
                }
            }
            break;
        }
        default: flt.values = {a}; break;
        }
        const bool needsValues = flt.test != rw::Test::Blank && flt.test != rw::Test::NotBlank;
        bool empty = needsValues && flt.values.empty();
        for (const std::string& v : flt.values) empty = empty || (v.empty() && flt.test != rw::Test::Equal && flt.test != rw::Test::NotEqual);
        if (empty) {
            Message(L"Type the filter's value (both, for from ... to).", MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), IDC_RW_FVALUE1));
            return;
        }
        if (const std::string why = rw::FilterProblem(*s, flt); !why.empty()) {
            Message(wide(why), MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), IDC_RW_FVALUE1));
            return;
        }
        def_.filters.push_back(flt);
        Changed();
        ShowFilters();
        for (int id : {IDC_RW_FVALUE1, IDC_RW_FVALUE2}) SetDlgItemTextW(hwnd(), id, L"");
    }

    void RemoveFilter()
    {
        const LRESULT i = List(IDC_RW_FILTERS, LB_GETCURSEL);
        if (i == LB_ERR || i >= static_cast<LRESULT>(def_.filters.size())) {
            MessageBeep(MB_OK);
            return;
        }
        def_.filters.erase(def_.filters.begin() + i);
        Changed();
        ShowFilters();
    }

    // ---- Preview, Save, Close --------------------------------------------------------

    void Preview()
    {
        if (def_.columns.empty()) {
            Message(L"Add at least one column first: pick a field and Add.", MB_ICONINFORMATION);
            return;
        }
        rw::ReportDefinition d = def_;
        if (TrimText(d.name).empty()) d.name = "(new report)";
        if (TrimText(d.title).empty()) d.title = d.name;
        try {
            const ReportContext ctx = CurrentReportContext();
            rw::BuildResult b = rw::BuildReport(cb, ctx, d);
            if (!b.report) {
                Message(wide(b.refusal), MB_ICONINFORMATION);
                return;
            }
            rpt::Report r = std::move(*b.report);
            r.company = ctx.company;
            r.printedOn = ctx.printedOn;
            rpt::win32::ShowPreviewWindow(hwnd(), std::move(r));
        } catch (const std::exception& e) {
            Message(L"The report could not be prepared:\n\n" + wide(e.what()), MB_ICONSTOP);
        }
    }

    bool Save()
    {
        ReadHeader();
        try {
            if (const auto p = rw::CheckDefinition(cb, def_, number_)) {
                Message(wide(p->text), MB_ICONEXCLAMATION);
                const int focus = p->setting == rw::DefinitionSetting::Name      ? IDC_RW_NAME
                                  : p->setting == rw::DefinitionSetting::Source  ? IDC_RW_SOURCE
                                  : p->setting == rw::DefinitionSetting::Filters ? IDC_RW_FILTERS
                                                                                 : IDC_RW_FIELDS;
                SetFocus(GetDlgItem(hwnd(), focus));
                return false;
            }
            // Without logins (GL.INI [ACCESS] PSW=F) the user is NOLOGIN: kept blank, so the list says "(you)".
            const std::string user = TrimText(szUser) == "NOLOGIN" ? std::string() : TrimText(szUser);
            number_ = rw::SaveDefinition(cb, number_, def_, user, now());
        } catch (const std::exception& e) {
            Message(L"The report could not be saved:\n\n" + wide(e.what()), MB_ICONSTOP);
            return false;
        }
        saved = number_;
        dirty_ = false;
        Caption();
        return true;
    }

    // True when the dialog can close: nothing changed, saved, or changes dropped.
    bool AskToSave()
    {
        if (!dirty_) return true;
        switch (Message(L"Save the changes to the report?", MB_YESNOCANCEL | MB_ICONQUESTION)) {
        case IDYES: return Save();
        case IDNO: return true;
        default: return false;
        }
    }

    long number_;
    bool copy_;
    bool closeAtOnce_;
    bool dirty_ = false;
    bool filling_ = false;   // the controls are being filled: their notifications aren't edits
    rw::ReportDefinition def_;
};

} // namespace

long RunReportDesigner(HWND owner, long number, bool copy, bool closeAtOnce)
{
    rw::EnsureReportWriterTables(cb);
    DesignerDialog dlg(number, copy, closeAtOnce);
    if (dlg.DoModal(IDD_RW_DESIGN, owner) == -1) return -1;
    return dlg.saved;
}

int StressReportDesigner(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    long existing = 0;
    for (const rw::StoredDefinition& d : rw::LoadDefinitions(cb)) existing = d.number;
    auto openBoth = [&] {
        if (RunReportDesigner(nullptr, 0, false, true) == -1) return false;
        return !existing || RunReportDesigner(nullptr, existing, false, true) != -1;
    };
    if (!openBoth()) return -1;   // first open: class registration
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (!openBoth()) return -1;
    return objects() - before;
}
