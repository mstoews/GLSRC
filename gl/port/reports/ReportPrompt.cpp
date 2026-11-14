//
// ReportPrompt.cpp - see ReportPrompt.h.
//
// Translation notes (plan per-port checklist; decisions are spec #7 §8):
//   - D6: a report's filter can ask its values when it runs. The template
//     has four rows of every kind of input; each row shows the kind its field
//     takes and hides the rest, and the dialog shrinks to the rows used.
//   - The first time, a period starts at the current period and a date range
//     at the current period's dates; after that, at the last answers (kept
//     until the program exits by RunUserReport).
//
#include "ReportPrompt.h"

#include "../Accounts.h"   // TrimText
#include "../Ar.h"         // LoadCustomers
#include "../Globals.h"
#include "../MainDialog.h"
#include "RunReport.h"     // CurrentReportContext

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

#include "../PortIds.h"

#include "../LegacyIds.h"   // last: see LegacyIds.h

namespace {

std::wstring wide(const std::string& s) { return fx::Utf8ToWide(s); }

enum class Input { Period, Date, Customer, Text };

struct Question {
    size_t filter = 0;          // its index in the definition's filters
    const rw::Field* field = nullptr;
    rw::Test test = rw::Test::Equal;
    Input input = Input::Text;
    bool pair = false;          // from ... to
    std::string label;
};

class PromptDialog : public MainDialog {
public:
    PromptDialog(const rw::ReportDefinition& def, rw::Answers& answers, bool closeAtOnce)
        : def_(def), answers_(answers), closeAtOnce_(closeAtOnce)
    {
    }

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetWindowTextW(hwnd(), wide(def_.name).c_str());
        const rw::Source* src = rw::FindSource(def_.source);
        const ReportContext ctx = CurrentReportContext();
        for (size_t k = 0; k < def_.filters.size() && questions_.size() < rw::kMaxQuestions; ++k) {
            const rw::Filter& f = def_.filters[k];
            const int i = src && f.ask ? src->fieldIndex(f.field) : -1;
            if (i < 0) continue;
            Question q;
            q.filter = k;
            q.field = &src->fields[static_cast<size_t>(i)];
            q.test = f.test;
            q.pair = f.test == rw::Test::Between;
            q.input = q.field->key == "period"                ? Input::Period
                      : q.field->type == rw::FieldType::Date ? Input::Date
                      : q.field->key == "customer"           ? Input::Customer
                                                             : Input::Text;
            q.label = (f.question.empty() ? q.field->label : f.question) + ":";
            questions_.push_back(q);
        }
        try {
            for (const Customer& c : LoadCustomers(cb)) customers_.push_back({TrimText(c.number), TrimText(c.name)});
        } catch (const std::exception&) {
            // No AR tables: the customer combo is empty.
        }
        for (int row = 0; row < static_cast<int>(rw::kMaxQuestions); ++row) ShowRow(row, ctx);
        Shrink();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code == BN_CLICKED && id == IDOK) {
            Ok();
            return true;
        }
        if (code == BN_CLICKED && id == IDCANCEL) {   // not MainDialog::Cancel: the caller's tables stay open
            End(IDCANCEL);
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

private:
    void Show(int id, bool on) { ShowWindow(GetDlgItem(hwnd(), id), on ? SW_SHOW : SW_HIDE); }

    void ShowRow(int row, const ReportContext& ctx)
    {
        const bool used = row < static_cast<int>(questions_.size());
        const Question* q = used ? &questions_[static_cast<size_t>(row)] : nullptr;
        Show(IDC_RWP_LABEL + row, used);
        if (used) SetDlgItemTextW(hwnd(), IDC_RWP_LABEL + row, wide(q->label).c_str());
        for (int side = 0; side < 2; ++side) {
            const bool on = used && (side == 0 || q->pair);
            Show((side ? IDC_RWP_COMBO2 : IDC_RWP_COMBO1) + row, on && (q->input == Input::Period || q->input == Input::Customer));
            Show((side ? IDC_RWP_EDIT2 : IDC_RWP_EDIT1) + row, on && q->input == Input::Text);
            Show((side ? IDC_RWP_DATE2 : IDC_RWP_DATE1) + row, on && q->input == Input::Date);
        }
        Show(IDC_RWP_TO + row, used && q->pair);
        if (!used) return;

        // The starting values: the last answers, else the current period's.
        std::vector<std::string> start;
        if (auto a = answers_.find(q->filter); a != answers_.end()) start = a->second;
        const FiscalPeriod* current = ctx.period(ctx.currentPeriod);
        for (int side = 0; side < (q->pair ? 2 : 1); ++side) {
            const std::string value = static_cast<size_t>(side) < start.size() ? start[static_cast<size_t>(side)] : std::string();
            const int combo = (side ? IDC_RWP_COMBO2 : IDC_RWP_COMBO1) + row;
            switch (q->input) {
            case Input::Period: {
                SendDlgItemMessageW(hwnd(), combo, CB_RESETCONTENT, 0, 0);
                int pick = -1;
                for (const FiscalPeriod& p : ctx.periods) {
                    const std::wstring t = std::to_wstring(p.number) + L"  " + wide(p.description);
                    const LRESULT at = SendDlgItemMessageW(hwnd(), combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t.c_str()));
                    SendDlgItemMessageW(hwnd(), combo, CB_SETITEMDATA, static_cast<WPARAM>(at), p.number);
                    if ((value.empty() ? ctx.currentPeriod : std::atoi(value.c_str())) == p.number) pick = static_cast<int>(at);
                }
                SendDlgItemMessageW(hwnd(), combo, CB_SETCURSEL, static_cast<WPARAM>(pick), 0);
                break;
            }
            case Input::Customer: {
                SendDlgItemMessageW(hwnd(), combo, CB_RESETCONTENT, 0, 0);
                int pick = -1;
                for (size_t i = 0; i < customers_.size(); ++i) {
                    const std::wstring t = wide(customers_[i].first + "  " + customers_[i].second);
                    const LRESULT at = SendDlgItemMessageW(hwnd(), combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t.c_str()));
                    SendDlgItemMessageW(hwnd(), combo, CB_SETITEMDATA, static_cast<WPARAM>(at), static_cast<LPARAM>(i));
                    if (customers_[i].first == TrimText(value)) pick = static_cast<int>(at);
                }
                SendDlgItemMessageW(hwnd(), combo, CB_SETCURSEL, static_cast<WPARAM>(pick < 0 ? 0 : pick), 0);
                break;
            }
            case Input::Date: {
                std::string d = value;
                if (d.empty() && current && current->endDate.size() == 8)
                    d = side == 0 && q->pair ? current->endDate.substr(0, 6) + "01" : current->endDate;
                SetField((side ? IDC_RWP_DATE2 : IDC_RWP_DATE1) + row, d);
                break;
            }
            case Input::Text: {
                std::string t = value;
                if (q->test == rw::Test::In) {   // all the values, comma-separated
                    t.clear();
                    for (size_t i = 0; i < start.size(); ++i) t += (i ? ", " : "") + start[i];
                }
                SetDlgItemTextW(hwnd(), (side ? IDC_RWP_EDIT2 : IDC_RWP_EDIT1) + row, wide(t).c_str());
                break;
            }
            }
        }
    }

    // The dialog ends below its last row.
    void Shrink()
    {
        const int rows = std::max(1, static_cast<int>(questions_.size()));
        RECT unit{0, 22 + 20 * rows + 4, 0, 0};
        MapDialogRect(hwnd(), &unit);
        RECT ok{};
        GetWindowRect(GetDlgItem(hwnd(), IDOK), &ok);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&ok), 2);
        const int dy = unit.top - ok.top;
        for (int id : {IDOK, IDCANCEL}) {
            RECT r{};
            GetWindowRect(GetDlgItem(hwnd(), id), &r);
            MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&r), 2);
            SetWindowPos(GetDlgItem(hwnd(), id), nullptr, r.left, r.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        RECT w{};
        GetWindowRect(hwnd(), &w);
        SetWindowPos(hwnd(), nullptr, 0, 0, w.right - w.left, w.bottom - w.top + dy, SWP_NOMOVE | SWP_NOZORDER);
    }

    std::string Text(int id) const
    {
        wchar_t buf[256] = {};
        GetDlgItemTextW(hwnd(), id, buf, 256);
        return TrimText(fx::WideToUtf8(buf));
    }

    std::string ComboValue(int id, Input input) const
    {
        const LRESULT sel = SendDlgItemMessageW(hwnd(), id, CB_GETCURSEL, 0, 0);
        if (sel == CB_ERR) return {};
        const LRESULT data = SendDlgItemMessageW(hwnd(), id, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
        if (input == Input::Period) return std::to_string(data);
        return data >= 0 && static_cast<size_t>(data) < customers_.size() ? customers_[static_cast<size_t>(data)].first
                                                                          : std::string();
    }

    void Ok()
    {
        rw::Answers next = answers_;
        const rw::Source* src = rw::FindSource(def_.source);
        for (size_t row = 0; row < questions_.size(); ++row) {
            const Question& q = questions_[row];
            const int r = static_cast<int>(row);
            std::vector<std::string> values;
            for (int side = 0; side < (q.pair ? 2 : 1); ++side) {
                switch (q.input) {
                case Input::Period:
                case Input::Customer: values.push_back(ComboValue((side ? IDC_RWP_COMBO2 : IDC_RWP_COMBO1) + r, q.input)); break;
                case Input::Date: values.push_back(GetDate((side ? IDC_RWP_DATE2 : IDC_RWP_DATE1) + r)); break;
                case Input::Text: values.push_back(Text((side ? IDC_RWP_EDIT2 : IDC_RWP_EDIT1) + r)); break;
                }
            }
            if (q.test == rw::Test::In && q.input == Input::Text) {   // comma-separated
                std::vector<std::string> list;
                std::string cur;
                for (char ch : values[0] + ",") {
                    if (ch == ',') {
                        if (!TrimText(cur).empty()) list.push_back(TrimText(cur));
                        cur.clear();
                    } else {
                        cur += ch;
                    }
                }
                values = list;
            }
            bool empty = values.empty();
            for (const std::string& v : values) empty = empty || (v.empty() && q.test != rw::Test::Equal && q.test != rw::Test::NotEqual);
            rw::Filter check = def_.filters[q.filter];
            check.ask = false;
            check.values = values;
            std::string why = empty ? "Answer \"" + q.label.substr(0, q.label.size() - 1) + "\"." : std::string();
            if (why.empty() && src) why = rw::FilterProblem(*src, check);
            if (!why.empty()) {
                Message(wide(why), MB_ICONINFORMATION);
                SetFocus(GetDlgItem(hwnd(), (q.input == Input::Date ? IDC_RWP_DATE1 : q.input == Input::Text ? IDC_RWP_EDIT1 : IDC_RWP_COMBO1) + r));
                return;
            }
            next[q.filter] = values;
        }
        answers_ = next;
        End(IDOK);
    }

    const rw::ReportDefinition& def_;
    rw::Answers& answers_;
    bool closeAtOnce_;
    std::vector<Question> questions_;
    std::vector<std::pair<std::string, std::string>> customers_;   // number, name
};

} // namespace

bool AskReportQuestions(HWND owner, const rw::ReportDefinition& def, rw::Answers& answers, bool closeAtOnce)
{
    bool any = false;
    for (const rw::Filter& f : def.filters) any = any || f.ask;
    if (!any) return true;
    PromptDialog dlg(def, answers, closeAtOnce);
    return dlg.DoModal(IDD_RW_PROMPT, owner) == IDOK;
}
