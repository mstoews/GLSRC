//
// AssetReports.cpp - see AssetReports.h.
//
#include "AssetReports.h"

#include "../Assets.h"
#include "Balances.h"

#include "report/Format.h"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

rpt::Report frame(const ReportContext& ctx, std::string title, std::string footer, bool periodLine)
{
    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = std::move(title);
    if (periodLine) r.subtitles.push_back(CurrentPeriodLine(ctx));
    r.footerNote = std::move(footer);
    return r;
}

// The figures a group footer or the total row adds up.
struct Sums {
    std::int64_t cost = 0, salvage = 0, monthly = 0, accum = 0, book = 0;
    void add(const Asset& a)
    {
        cost += a.cost;
        salvage += a.salvage;
        monthly += a.monthly;
        accum += a.accumulated;
        book += BookValue(a);
    }
    std::vector<rpt::Cell> row(std::string label) const
    {
        return {rpt::Cell::Text(std::move(label)), {}, {}, {}, {}, {}, rpt::Cell::Amount(cost), rpt::Cell::Amount(salvage),
                rpt::Cell::Amount(monthly), rpt::Cell::Amount(accum), rpt::Cell::Amount(book)};
    }
};

} // namespace

rpt::Report FixedAssetListing(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame(ctx, "General Ledger Fixed Asset Summary", "Report File: gl_fa.rpt", true);
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Asset", 0.7, rpt::CellType::Text, "Code"},
                 {"Date", 0.85, rpt::CellType::Date, "Purchased"},
                 {"Date", 0.85, rpt::CellType::Date, "Sold"},
                 {"Description", 1.9, rpt::CellType::Text, "Reference"},
                 {"Months", 0.65, rpt::CellType::Integer, "Used"},
                 {"Life", 0.75, rpt::CellType::Integer, "(Months)"},
                 {"Historical", 1.05, rpt::CellType::Amount, "Cost"},
                 {"Salvage", 0.9, rpt::CellType::Amount, "Value"},
                 {"Monthly", 1.1, rpt::CellType::Amount, "Depreciation"},
                 {"Accumulated", 1.1, rpt::CellType::Amount, "Depreciation"},
                 {"Net Book", 1.05, rpt::CellType::Amount, "Value"}};

    const std::vector<Asset> assets = LoadAssets(code);
    std::map<std::string, std::vector<const Asset*>> byType;   // type order, as the tag would give
    for (const Asset& a : assets) byType[a.type].push_back(&a);

    Sums total;
    int inUse = 0, disposed = 0;
    // The ledger check (D11): register totals by account, of assets in use.
    std::map<std::pair<std::string, std::string>, std::int64_t> costByAccount, accumByAccount;   // (account, dept)
    std::map<std::pair<std::string, std::string>, std::set<std::string>> typesOf;               // for the notes
    for (const auto& [type, list] : byType) {
        const std::optional<AssetType> t = FindAssetType(code, type);
        r.rows.push_back(rpt::Row::GroupHeader(t ? type + "  " + t->description : type + "  (no such type)"));
        Sums sums;
        for (const Asset* a : list) {
            r.rows.push_back(rpt::Row::Detail(
                {rpt::Cell::Text(a->code), rpt::Cell::Date(a->datePurchased),
                 a->dateSold.empty() ? rpt::Cell::Empty() : rpt::Cell::Date(a->dateSold),
                 rpt::Cell::Text(a->disposed() ? a->description + " (disposed)" : a->description),
                 rpt::Cell::Integer(a->monthsUsed), rpt::Cell::Integer(a->life), rpt::Cell::Amount(a->cost),
                 rpt::Cell::Amount(a->salvage), rpt::Cell::Amount(a->monthly), rpt::Cell::Amount(a->accumulated),
                 rpt::Cell::Amount(BookValue(*a))}));
            if (!a->reference.empty()) {
                std::vector<rpt::Cell> ref(r.columns.size());
                ref[3] = rpt::Cell::Text(a->reference);
                r.rows.push_back(rpt::Row::Detail(std::move(ref)));
            }
            if (a->disposed()) {
                ++disposed;
                continue;
            }
            ++inUse;
            sums.add(*a);
            if (t) {
                const auto costKey = std::make_pair(t->asset.account, t->asset.dept);
                const auto accumKey = std::make_pair(t->accum.account, t->accum.dept);
                costByAccount[costKey] += a->cost;
                accumByAccount[accumKey] += a->accumulated;
                typesOf[costKey].insert(type);
                typesOf[accumKey].insert(type);
            }
        }
        r.rows.push_back(rpt::Row::GroupFooter(sums.row("Total " + type)));
        total.cost += sums.cost;
        total.salvage += sums.salvage;
        total.monthly += sums.monthly;
        total.accum += sums.accum;
        total.book += sums.book;
    }
    r.rows.push_back(rpt::Row::Total(total.row(std::to_string(inUse) + (inUse == 1 ? " asset" : " assets")), true));
    if (disposed > 0)
        r.rows.push_back(rpt::Row::Note(std::to_string(disposed) + (disposed == 1 ? " disposed asset is" : " disposed assets are") +
                                        " listed but not totalled."));

    // D11: the register against the ledger, at the current period.
    r.rows.push_back(rpt::Row::Blank());
    r.rows.push_back(rpt::Row::Heading("The register against the ledger, at the end of period " +
                                       std::to_string(ctx.currentPeriod)));
    std::map<std::pair<std::string, std::string>, const AccountBalances*> ledger;
    const std::vector<AccountBalances> balances = LoadBalances(code);
    for (const AccountBalances& b : balances) ledger[{b.account, b.dept}] = &b;
    int differences = 0;
    auto check = [&](const std::pair<std::string, std::string>& key, std::int64_t registerTotal, bool credit,
                     const char* what) {
        const std::string name = key.second.empty() ? key.first : key.first + "-" + key.second;
        const auto it = ledger.find(key);
        if (it == ledger.end()) {
            ++differences;
            std::string types;
            for (const std::string& t : typesOf[key]) types += (types.empty() ? "" : ", ") + t;
            r.rows.push_back(rpt::Row::Note(name + " (" + what + ", type " + types +
                                            "): not in the chart of accounts. Register " +
                                            rpt::FormatAmount(registerTotal) + "."));
            return;
        }
        // Accumulated depreciation is a credit balance: the ledger shows it negative.
        const std::int64_t book = credit ? -it->second->balanceAt(ctx.currentPeriod) : it->second->balanceAt(ctx.currentPeriod);
        std::string line = name + " " + it->second->descript + " (" + what + "): register " +
                           rpt::FormatAmount(registerTotal) + ", ledger " + rpt::FormatAmount(book);
        if (book == registerTotal) {
            line += ". Agrees.";
        } else {
            ++differences;
            line += ". Differs by " + rpt::FormatAmount(registerTotal - book) + ".";
        }
        r.rows.push_back(rpt::Row::Note(line));
    };
    for (const auto& [key, cost] : costByAccount) check(key, cost, false, "cost");
    for (const auto& [key, accum] : accumByAccount) check(key, accum, true, "accumulated depreciation");
    if (costByAccount.empty() && accumByAccount.empty())
        r.rows.push_back(rpt::Row::Note("No asset in use has a type, so there is nothing to check."));
    else if (differences > 0)
        r.rows.push_back(rpt::Row::Note("A difference can be a depreciation voucher not yet posted, or a disposal "
                                        "not yet recorded in Voucher Entry."));
    return r;
}

rpt::Report FixedAssetTypesListing(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    // GL_FATYP.RPT's header has no current period; its title read "General Leger" (D12).
    rpt::Report r = frame(ctx, "General Ledger Fixed Asset Types", "Report Name: gl_fatyp.rpt", false);
    r.columns = {{"Fixed Asset", 0.8, rpt::CellType::Text, "Type"},
                 {"Account For", 1.6},
                 {"Account", 0.8, rpt::CellType::Text, "Code"},
                 {"Dept.", 0.5},
                 {"Description", 3.4}};
    const std::vector<AssetType> types = LoadAssetTypes(code);
    for (const AssetType& t : types) {
        r.rows.push_back(rpt::Row::GroupHeader(t.code + "  " + t.description));
        const std::pair<const char*, const AssetAccount*> rows[] = {
            {"Asset", &t.asset}, {"Expense", &t.expense}, {"Accumulated Depreciation", &t.accum}};
        for (const auto& [label, a] : rows)
            r.rows.push_back(rpt::Row::Detail({{}, rpt::Cell::Text(label), rpt::Cell::Text(a->account),
                                               rpt::Cell::Text(a->dept), rpt::Cell::Text(a->descript)}));
    }
    r.rows.push_back(rpt::Row::Blank());
    r.rows.push_back(rpt::Row::Note(std::to_string(types.size()) + (types.size() == 1 ? " type" : " types")));
    return r;
}
