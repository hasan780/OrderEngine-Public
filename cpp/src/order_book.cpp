#include "order_book.h"

namespace order_engine {

OrderBook::OrderBook(std::string symbol) {
    // TODO [this week]: store the symbol
}

int64_t PriceLevel::total_quantity() const {
    // TODO [this week]: sum (order->quantity - order->filled_quantity) for each order
    return 0;
}

void OrderBook::add_order(Order* order) {
    // TODO [this week]:
    //   - get or create the PriceLevel at order->price on the correct side (bids_ or asks_)
    //   - push_back the order pointer into that level's orders
}

Order* OrderBook::cancel_order(const std::string& order_id, Side side, double price) {
    // TODO [this week]:
    //   - find the PriceLevel at (side, price)
    //   - find the order in that level by order_id
    //   - erase it from the deque
    //   - if the level is now empty, erase the price entry from the map
    //   - return the raw pointer, or nullptr if not found
    return nullptr;
}

PriceLevel* OrderBook::best_bid() {
    // TODO [this week]: return pointer to first element of bids_ (highest price), or nullptr if empty
    return nullptr;
}

PriceLevel* OrderBook::best_ask() {
    // TODO [this week]: return pointer to first element of asks_ (lowest price), or nullptr if empty
    return nullptr;
}

std::optional<double> OrderBook::spread() const {
    // TODO [this week]: if either side empty return nullopt; else asks_.begin()->first - bids_.begin()->first
    return std::nullopt;
}

}  // namespace order_engine
