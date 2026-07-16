#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "matching_engine.h"
#include <memory>

using namespace order_engine;

static std::unique_ptr<Order> limit_order(const std::string& id, Side side,
                                          double price, int64_t qty,
                                          const std::string& symbol = "AAPL") {
    auto o = std::make_unique<Order>();
    o->order_id   = id;
    o->symbol     = symbol;
    o->side       = side;
    o->order_type = OrderType::LIMIT;
    o->quantity   = qty;
    o->price      = price;
    return o;
}

static std::unique_ptr<Order> market_order(const std::string& id, Side side,
                                           int64_t qty,
                                           const std::string& symbol = "AAPL") {
    auto o = std::make_unique<Order>();
    o->order_id   = id;
    o->symbol     = symbol;
    o->side       = side;
    o->order_type = OrderType::MARKET;
    o->quantity   = qty;
    o->price      = std::nullopt;
    return o;
}

// ── no cross ─────────────────────────────────────────────────────────

TEST_CASE("no match when prices don't cross") {
    MatchingEngine engine;
    engine.submit_order(limit_order("b1", Side::BUY,  99.0, 10));
    engine.submit_order(limit_order("s1", Side::SELL, 101.0, 10));
    REQUIRE(engine.trade_history().empty());
}

// ── basic fills ──────────────────────────────────────────────────────

TEST_CASE("exact match") {
    MatchingEngine engine;
    auto buy_ptr  = limit_order("b1", Side::BUY,  100.0, 10);
    auto sell_ptr = limit_order("s1", Side::SELL, 100.0, 10);
    Order* buy  = buy_ptr.get();
    Order* sell = sell_ptr.get();

    engine.submit_order(std::move(buy_ptr));
    auto trades = engine.submit_order(std::move(sell_ptr));

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].price == 100.0);
    REQUIRE(trades[0].quantity == 10);
    REQUIRE(trades[0].buy_order_id  == "b1");
    REQUIRE(trades[0].sell_order_id == "s1");
    REQUIRE(buy->status  == OrderStatus::FILLED);
    REQUIRE(sell->status == OrderStatus::FILLED);
}

TEST_CASE("partial fill of resting order") {
    MatchingEngine engine;
    auto buy_ptr  = limit_order("b1", Side::BUY,  100.0, 10);
    auto sell_ptr = limit_order("s1", Side::SELL, 100.0, 6);
    Order* buy  = buy_ptr.get();
    Order* sell = sell_ptr.get();

    engine.submit_order(std::move(buy_ptr));
    auto trades = engine.submit_order(std::move(sell_ptr));

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 6);
    REQUIRE(sell->status == OrderStatus::FILLED);
    REQUIRE(buy->status  == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(buy->filled_quantity == 6);
}

TEST_CASE("aggressor remainder rests on the book") {
    MatchingEngine engine;
    auto sell_ptr = limit_order("s1", Side::SELL, 100.0, 4);
    auto buy_ptr  = limit_order("b1", Side::BUY,  100.0, 10);
    Order* buy = buy_ptr.get();

    engine.submit_order(std::move(sell_ptr));
    auto trades = engine.submit_order(std::move(buy_ptr));

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 4);
    REQUIRE(buy->status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(buy->filled_quantity == 4);

    // remaining 6 should be resting — a new sell of 6 fills it
    auto trades2 = engine.submit_order(limit_order("s2", Side::SELL, 100.0, 6));
    REQUIRE(trades2.size() == 1);
    REQUIRE(trades2[0].quantity == 6);
    REQUIRE(buy->status == OrderStatus::FILLED);
}

// ── priority & pricing rules ─────────────────────────────────────────

TEST_CASE("price-time priority") {
    MatchingEngine engine;
    auto b1_ptr = limit_order("b1", Side::BUY, 100.0, 5);   // earlier
    auto b2_ptr = limit_order("b2", Side::BUY, 100.0, 5);   // later
    Order* b1 = b1_ptr.get();
    Order* b2 = b2_ptr.get();

    engine.submit_order(std::move(b1_ptr));
    engine.submit_order(std::move(b2_ptr));
    auto trades = engine.submit_order(limit_order("s1", Side::SELL, 100.0, 5));

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].buy_order_id == "b1");   // oldest fills first
    REQUIRE(b1->status == OrderStatus::FILLED);
    REQUIRE(b2->status == OrderStatus::OPEN);
}

TEST_CASE("trade executes at resting price") {
    MatchingEngine engine;
    engine.submit_order(limit_order("s1", Side::SELL, 100.0, 10));      // resting ask @ 100
    auto trades = engine.submit_order(limit_order("b1", Side::BUY, 105.0, 10));  // pays up to 105

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].price == 100.0);   // passive (resting) price, not 105
}

// ── sweeping multiple levels ─────────────────────────────────────────

TEST_CASE("sweep multiple levels") {
    MatchingEngine engine;
    engine.submit_order(limit_order("s1", Side::SELL, 10.00, 200));
    auto s2_ptr = limit_order("s2", Side::SELL, 10.01, 400);
    Order* s2 = s2_ptr.get();
    engine.submit_order(std::move(s2_ptr));

    auto buy_ptr = limit_order("b1", Side::BUY, 10.01, 500);
    Order* buy = buy_ptr.get();
    auto trades = engine.submit_order(std::move(buy_ptr));

    REQUIRE(trades.size() == 2);
    REQUIRE(trades[0].quantity == 200);
    REQUIRE(trades[0].price == 10.00);
    REQUIRE(trades[1].quantity == 300);
    REQUIRE(trades[1].price == 10.01);
    REQUIRE(buy->status == OrderStatus::FILLED);             // all 500 filled
    REQUIRE(s2->status == OrderStatus::PARTIALLY_FILLED);    // 100 left resting
    REQUIRE(s2->remaining_quantity() == 100);
}

// ── market orders (this week) ────────────────────────────────────────

TEST_CASE("market buy fills at best ask") {
    MatchingEngine engine;
    engine.submit_order(limit_order("s1", Side::SELL, 100.0, 10));
    auto trades = engine.submit_order(market_order("m1", Side::BUY, 5));
    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 5);
    REQUIRE(trades[0].price == 100.0);
}

TEST_CASE("market order sweeps three levels") {
    MatchingEngine engine;
    engine.submit_order(limit_order("s1", Side::SELL, 10.01, 200));
    engine.submit_order(limit_order("s2", Side::SELL, 10.02, 500));
    engine.submit_order(limit_order("s3", Side::SELL, 10.03, 300));

    auto buy_ptr = market_order("m1", Side::BUY, 1000);
    Order* buy = buy_ptr.get();
    auto trades = engine.submit_order(std::move(buy_ptr));

    REQUIRE(trades.size() == 3);
    REQUIRE(trades[0].quantity == 200); REQUIRE(trades[0].price == 10.01);
    REQUIRE(trades[1].quantity == 500); REQUIRE(trades[1].price == 10.02);
    REQUIRE(trades[2].quantity == 300); REQUIRE(trades[2].price == 10.03);
    REQUIRE(buy->filled_quantity == 1000);
    REQUIRE(buy->status == OrderStatus::FILLED);
}

TEST_CASE("market order partial when insufficient liquidity") {
    MatchingEngine engine;
    engine.submit_order(limit_order("s1", Side::SELL, 10.01, 200));

    auto buy_ptr = market_order("m1", Side::BUY, 500);   // only 200 available
    Order* buy = buy_ptr.get();
    auto trades = engine.submit_order(std::move(buy_ptr));

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 200);
    REQUIRE(buy->filled_quantity == 200);
    REQUIRE(buy->status == OrderStatus::PARTIALLY_FILLED);   // market orders don't rest
}

TEST_CASE("market sell sweeps bids") {
    // A SELL market order consumes the BID side, highest price first.
    MatchingEngine engine;
    engine.submit_order(limit_order("b1", Side::BUY, 10.00, 200));
    engine.submit_order(limit_order("b2", Side::BUY,  9.99, 500));
    engine.submit_order(limit_order("b3", Side::BUY,  9.98, 300));

    auto sell_ptr = market_order("m1", Side::SELL, 1000);
    Order* sell = sell_ptr.get();
    auto trades = engine.submit_order(std::move(sell_ptr));

    REQUIRE(trades.size() == 3);
    REQUIRE(trades[0].quantity == 200); REQUIRE(trades[0].price == 10.00);
    REQUIRE(trades[1].quantity == 500); REQUIRE(trades[1].price == 9.99);
    REQUIRE(trades[2].quantity == 300); REQUIRE(trades[2].price == 9.98);
    REQUIRE(sell->filled_quantity == 1000);
    REQUIRE(sell->status == OrderStatus::FILLED);
}

// ── VWAP (this week) ─────────────────────────────────────────────────

TEST_CASE("vwap across a sweep") {
    MatchingEngine engine;
    engine.submit_order(limit_order("s1", Side::SELL, 10.01, 200));
    engine.submit_order(limit_order("s2", Side::SELL, 10.02, 500));
    engine.submit_order(limit_order("s3", Side::SELL, 10.03, 300));
    auto trades = engine.submit_order(market_order("m1", Side::BUY, 1000));

    // notional = 200*10.01 + 500*10.02 + 300*10.03 = 10021; volume = 1000
    REQUIRE_THAT(vwap(trades), Catch::Matchers::WithinAbs(10.021, 1e-9));
}

TEST_CASE("vwap of no trades is zero") {
    REQUIRE(vwap({}) == 0.0);
}
