#pragma once
#include "models.h"
#include <optional>
#include <string>
#include <deque> // added by me
#include <map> // added by me
#include <unordered_map> // added by me

namespace order_engine {

// All orders at a single price, FIFO.
struct PriceLevel {
    // TODO [this week]: decide what state a price level holds.
    //   - the price of this level
    //   - the orders at this level, stored FIFO (earliest first)

    // MY CODE
    double price;
    std::deque<Order*> orders;
    //

    int64_t total_quantity() const;
};

// Order book for a single symbol.
// Bids: highest price first.  Asks: lowest price first.
class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    void   add_order   (Order* order);
    Order* cancel_order(const std::string& order_id, Side side, double price);

    PriceLevel*           best_bid();
    PriceLevel*           best_ask();
    std::optional<double> spread() const;

private:
    // TODO [this week]: decide what state the book holds.
    //   - the symbol
    //   - bid levels (keyed by price, highest first)
    //   - ask levels (keyed by price, lowest first)

    // MY CODE
    std::string symbol_;
    std::map<double, PriceLevel, std::greater<double>> bids_;
    std::map<double, PriceLevel> asks_;

    std::unordered_map<std::string, Order*> order_lookup_; //global indexing intialization, unordered for less operations
    //
};

}  // namespace order_engine
