#include "drop.hpp"
#include "contract_actions.cpp"
#include "user_actions.cpp"
#include "utils.cpp"

// === Guda Drop === //

void drop::receive_token_transfer(
    name from,
    name to,
    asset quantity,
    std::string memo
) {
    if (to != get_self()) {
        return;
    }

    // --- Check if the received token is the GUDA token from guda.guda contract --- //
    name contract = get_first_receiver();
    check(contract == "guda.guda"_n, "🜛 We only take GUDA tokens here. 🔗 guda.love");
    check(quantity.symbol == symbol("GUDA", 8), "🜛 We only take GUDA tokens here.");

    balances_t balances(get_self(), get_self().value);
    auto bal_itr = balances.find(from.value);

    if (bal_itr == balances.end()) {
        // --- Initialize token balance if not already present --- //
        balances.emplace(to, [&](auto &row) {
            row.wallet = from;
            row.balance = quantity;
        });
    } else {
        balances.modify(bal_itr, same_payer, [&](auto &row) {
            row.balance += quantity;
        });
    }
} //END drop::receive_token_transfer
