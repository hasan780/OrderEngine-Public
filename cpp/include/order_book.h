#pragma once
#include "models.h"
#include <optional>
#include <string>

namespace order_engine {

// All orders at a single price, FIFO.
struct PriceLevel {
    // think: what do you need to store here?

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
    // think: what data structures hold the bid and ask levels?
};

}  // namespace order_engine
