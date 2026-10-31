#pragma once
#include "models.h"
#include <deque>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace order_engine {

// All orders at a single price, FIFO.
struct PriceLevel {
    double price;
    std::deque<Order*> orders;

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

    const std::map<double, PriceLevel, std::greater<double>>& bids() const { return bids_; }
    const std::map<double, PriceLevel>& asks() const { return asks_; }

private:
    std::mutex mutex_;  // guards bids_, asks_, and order_lookup_

    std::string symbol_;
    std::map<double, PriceLevel, std::greater<double>> bids_;
    std::map<double, PriceLevel> asks_;

    std::unordered_map<std::string, Order*> order_lookup_;  // order_id -> order
};

}  // namespace order_engine
