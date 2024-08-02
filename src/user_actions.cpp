// === User Actions === //

ACTION drop::closebank(
    name wallet
) {
    require_auth(wallet);

    balances_t balances(get_self(), get_self().value);
    auto bal_itr = balances.find(wallet.value);

    if (bal_itr != balances.end() && bal_itr->balance.amount > 0) {
        transfer_token(wallet, "guda.guda"_n, bal_itr->balance, "🜛 See you later. Token balance refunded");
        balances.erase(bal_itr);
    }
} //END ACTION drop::closebank

ACTION drop::claim(
    name wallet,
    uint64_t drop_id,
    uint32_t qty
) {
    require_auth(wallet);

    drops_t drops(get_self(), get_self().value);
    auto drop_itr = drops.find(drop_id);
    check(drop_itr != drops.end(), "🜛 Drop not found");

    // --- Check drop start and end times --- //
    if (drop_itr->start_time > 0) {
        check(current_time_point().sec_since_epoch() >= drop_itr->start_time, "🜛 Hang on. Drop has not started");
    }
    if (drop_itr->end_time > 0) {
        check(current_time_point().sec_since_epoch() <= drop_itr->end_time, "🜛 Where were you? Drop already ended");
    }
    if (drop_itr->max_claims > 0) {
        check((drop_itr->claims + qty) <= drop_itr->max_claims, "🜛 Too slow. Drop has maxed out");
    }

    claims_t claims(get_self(), wallet.value);
    auto claim_itr = claims.find(drop_id);
    uint32_t claim = qty;
    if (drop_itr->limit > 0) {
        if (qty > drop_itr->limit) {
            check(false, ("🜛 You're over the max of " + std::to_string(drop_itr->limit) + " per claim").c_str());
        }
        if (claim_itr != claims.end()) {
            if ((claim_itr->counter + qty) > drop_itr->limit) {
                check(current_time_point().sec_since_epoch() >= (claim_itr->last_claim + drop_itr->cooldown), "Please wait for cooldown");
            } else {
                claim = claim_itr->counter + qty;
            }
        }
    } else {
        if (claim_itr != claims.end()) {
            claim = claim_itr->counter + qty;
        }
    }

    // --- Check if the user has sufficient balance to pay the price --- //
    asset price = drop_itr->price;
    if (price.amount > 0) {
        balances_t balances(get_self(), get_self().value);
        auto bal_itr = balances.find(wallet.value);
        check(bal_itr != balances.end(), "🜛 You haz no GUDA. Send moor.");
        check(bal_itr->balance.amount >= price.amount * qty, "🜛 You need more GUDA to claim this NFT");

        balances.modify(bal_itr, same_payer, [&](auto &row) {
            row.balance -= asset(price.amount * qty, price.symbol);
        });
    }

    // --- Mint the assets to the user --- //
    for (int32_t i = 0; i < qty; i++) {
        for (int32_t tmplt : drop_itr->target) {
            atomicassets::templates_t templates = atomicassets::get_templates(drop_itr->target_collection);
            auto tmplt_itr = templates.find(tmplt);
            mint_asset(drop_itr->target_collection, tmplt_itr->schema_name, tmplt, wallet);
        }
    }

    drops.modify(drop_itr, get_self(), [&](auto &row) {
        row.claims += qty;
    });

    if (claim_itr != claims.end()) {
        claims.modify(claim_itr, same_payer, [&](auto &row) {
            row.counter = claim;
            row.last_claim = current_time_point().sec_since_epoch();
        });
    } else {
        claims.emplace(wallet, [&](auto &row) {
            row.drop_id = drop_id;
            row.counter = claim;
            row.last_claim = current_time_point().sec_since_epoch();
        });
    }
} //END ACTION drop::claim
