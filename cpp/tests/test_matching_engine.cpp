#include <catch2/catch_test_macros.hpp>
#include "matching_engine.h"

using namespace order_engine;

TEST_CASE("no match when prices don't cross") {
    // submit buy at 99, sell at 101
    // assert trade_history() is empty
}

TEST_CASE("exact match") {
    // submit buy at 100 qty 10, then sell at 100 qty 10
    // assert 1 trade: price=100, qty=10
    // assert both orders status == FILLED
}

TEST_CASE("partial fill") {
    // submit buy at 100 qty 10, then sell at 100 qty 6
    // assert 1 trade qty=6
    // assert buy is PARTIALLY_FILLED, sell is FILLED
}

TEST_CASE("price-time priority") {
    // submit two buys at same price
    // submit sell that only fills one
    // assert the earlier buy is the one filled
}

TEST_CASE("market buy fills at best ask") {
    // submit limit sell at 100 qty 10
    // submit market buy qty 5
    // assert 1 trade: price=100, qty=5
}
