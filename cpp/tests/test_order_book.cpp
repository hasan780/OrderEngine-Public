#include <catch2/catch_test_macros.hpp>
#include "order_book.h"

using namespace order_engine;

TEST_CASE("best bid and ask") {
    // create OrderBook
    // add bids at 100 and 101, ask at 102
    // assert best_bid()->price == 101
    // assert best_ask()->price == 102
}

TEST_CASE("spread") {
    // add bid at 100, ask at 102
    // assert spread() == 2.0
}

TEST_CASE("empty book has no spread") {
    // assert spread() == nullopt on empty book
}

TEST_CASE("cancel order") {
    // add a bid, cancel it, assert best_bid() == nullptr
}

TEST_CASE("FIFO within price level") {
    // add two bids at same price
    // assert orders.front() is the first one added
}
