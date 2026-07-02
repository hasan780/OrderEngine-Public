#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "order_book.h"

using namespace order_engine;

static Order make_order(const std::string& id, Side side, double price,
                        int64_t qty, const std::string& symbol = "AAPL") {
    return Order{
        .order_id   = id,
        .symbol     = symbol,
        .side       = side,
        .order_type = OrderType::LIMIT,
        .quantity   = qty,
        .price      = price,
        .timestamp  = 0,
    };
}

// ── add_order ───────────────────────────────────────────────────────
// We verify an order was added by cancelling it and inspecting the
// returned order (no dependency on best_bid / best_ask).

TEST_CASE("add then cancel returns the same order") {
    OrderBook book("AAPL");
    auto b1 = make_order("b1", Side::BUY, 100.0, 10);
    book.add_order(&b1);
    Order* removed = book.cancel_order("b1", Side::BUY, 100.0);
    REQUIRE(removed != nullptr);
    REQUIRE(removed->order_id == "b1");
    REQUIRE(removed->quantity == 10);
}

TEST_CASE("add order on ask side") {
    OrderBook book("AAPL");
    auto a1 = make_order("a1", Side::SELL, 102.0, 5);
    book.add_order(&a1);
    Order* removed = book.cancel_order("a1", Side::SELL, 102.0);
    REQUIRE(removed != nullptr);
    REQUIRE(removed->order_id == "a1");
}

TEST_CASE("multiple orders at same price are all cancellable") {
    OrderBook book("AAPL");
    auto b1 = make_order("b1", Side::BUY, 100.0, 10);
    auto b2 = make_order("b2", Side::BUY, 100.0, 15);
    book.add_order(&b1);
    book.add_order(&b2);
    REQUIRE(book.cancel_order("b1", Side::BUY, 100.0)->order_id == "b1");
    REQUIRE(book.cancel_order("b2", Side::BUY, 100.0)->order_id == "b2");
}

// ── cancel_order ────────────────────────────────────────────────────

TEST_CASE("cancel actually removes the order") {
    OrderBook book("AAPL");
    auto b1 = make_order("b1", Side::BUY, 100.0, 10);
    book.add_order(&b1);
    REQUIRE(book.cancel_order("b1", Side::BUY, 100.0) != nullptr);  // first removes it
    REQUIRE(book.cancel_order("b1", Side::BUY, 100.0) == nullptr);  // gone the second time
}

TEST_CASE("cancel nonexistent order returns nullptr") {
    OrderBook book("AAPL");
    auto b1 = make_order("b1", Side::BUY, 100.0, 10);
    book.add_order(&b1);
    REQUIRE(book.cancel_order("does-not-exist", Side::BUY, 100.0) == nullptr);
}

TEST_CASE("cancel at wrong price returns nullptr and leaves order") {
    OrderBook book("AAPL");
    auto b1 = make_order("b1", Side::BUY, 100.0, 10);
    book.add_order(&b1);
    REQUIRE(book.cancel_order("b1", Side::BUY, 99.0) == nullptr);    // wrong price
    REQUIRE(book.cancel_order("b1", Side::BUY, 100.0) != nullptr);   // still there
}

TEST_CASE("cancel one of many leaves the other") {
    OrderBook book("AAPL");
    auto b1 = make_order("b1", Side::BUY, 100.0, 10);
    auto b2 = make_order("b2", Side::BUY, 100.0, 15);
    book.add_order(&b1);
    book.add_order(&b2);
    book.cancel_order("b1", Side::BUY, 100.0);
    REQUIRE(book.cancel_order("b2", Side::BUY, 100.0) != nullptr);   // b2 untouched
}

// ── best_bid / best_ask / spread ────────────────────────────────────
// NOTE: these are not part of this week's assignment (add_order / cancel_order).
// Uncomment once you implement best_bid / best_ask / spread.

 TEST_CASE("FIFO within price level") {
     OrderBook book("AAPL");
     auto b1 = make_order("b1", Side::BUY, 100.0, 5);
     auto b2 = make_order("b2", Side::BUY, 100.0, 7);
     book.add_order(&b1);
     book.add_order(&b2);
     REQUIRE(book.best_bid()->orders.front()->order_id == "b1");
 }

 TEST_CASE("best bid and ask") {
     OrderBook book("AAPL");
     auto b1 = make_order("b1", Side::BUY, 100.0, 10);
     auto b2 = make_order("b2", Side::BUY, 101.0, 5);
     auto a1 = make_order("a1", Side::SELL, 102.0, 8);
     book.add_order(&b1);
     book.add_order(&b2);
     book.add_order(&a1);
     REQUIRE(book.best_bid()->price == 101.0);
    REQUIRE(book.best_ask()->price == 102.0);
 }

 TEST_CASE("spread") {
     OrderBook book("AAPL");
     auto b1 = make_order("b1", Side::BUY, 100.0, 10);
     auto a1 = make_order("a1", Side::SELL, 102.0, 10);
     book.add_order(&b1);
     book.add_order(&a1);
     REQUIRE_THAT(*book.spread(), Catch::Matchers::WithinAbs(2.0, 1e-9));
 }

 TEST_CASE("empty book has no spread") {
     OrderBook book("AAPL");
     REQUIRE_FALSE(book.spread().has_value());
 }
