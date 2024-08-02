#include <eosio/eosio.hpp>
#include "atomicassets-interface.hpp"

using namespace eosio;

// === Drop Contract === //

CONTRACT drop : public contract {
    public:
        using contract::contract;

        // === Actions === //

        // --- Create a new drop --- //
        ACTION create(
            name target_collection,
            std::vector<int32_t> target,
            asset price,
            uint32_t max_claims,
            uint32_t limit,
            uint32_t start_time,
            uint32_t end_time,
            uint32_t cooldown
        );

        // --- Update claims for a drop --- //
        ACTION updateclaims(
            uint64_t drop_id,
            uint32_t max_claims,
            uint32_t limit
        );

        // --- Update timing for a drop --- //
        ACTION updatetime(
            uint64_t drop_id,
            uint32_t start_time,
            uint32_t end_time,
            uint32_t cooldown
        );

        // --- Update price for a drop --- //
        ACTION updateprice(
            uint64_t drop_id,
            asset price
        );

        // --- Remove a drop --- //
        ACTION remove(
            uint64_t drop_id
        );

        // --- Close the balance of a wallet --- //
        ACTION closebank(
            name wallet
        );

        // --- Claim tokens for a drop --- //
        ACTION claim(
            name wallet,
            uint64_t drop_id,
            uint32_t qty
        );

        // --- Handle incoming token transfer --- //
        [[eosio::on_notify("*::transfer")]] void receive_token_transfer(
            name from,
            name to,
            asset quantity,
            std::string memo
        );

    private:
        // === Tables === //

        // --- Store drop records --- //
        TABLE drops_s {
            uint64_t drop_id;
            name target_collection;
            std::vector<int32_t> target;
            asset price;
            uint32_t max_claims;
            uint32_t limit;
            uint32_t claims;
            uint32_t start_time;
            uint32_t end_time;
            uint32_t cooldown;

            auto primary_key() const { return drop_id; }
        }; //END TABLE drops_s
        typedef multi_index<name("drops"), drops_s> drops_t;

        // --- Store balance records --- //
        TABLE balances_s {
            name wallet;
            asset balance;

            auto primary_key() const { return wallet.value; }
        }; //END TABLE balances_s
        typedef multi_index<name("balances"), balances_s> balances_t;

        // --- Store claim records --- //
        TABLE claims_s {
            uint64_t drop_id;
            uint32_t counter;
            uint32_t last_claim;

            auto primary_key() const { return drop_id; }
        }; //END TABLE claims_s
        typedef multi_index<name("claims"), claims_s> claims_t;

        // === Helper Functions === //

        // --- Mint asset to a receiver --- //
        void mint_asset(
            name collection,
            name schema,
            int32_t template_id,
            name receiver
        );

        // --- Transfer token to a receiver --- //
        void transfer_token(
            name receiver,
            name contract,
            asset amount,
            std::string memo
        );
}; //END CONTRACT drop
