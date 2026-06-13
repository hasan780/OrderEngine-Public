#include "order_book.h"

namespace order_engine {

OrderBook::OrderBook(std::string symbol) { /* store symbol */ }

int64_t PriceLevel::total_quantity() const {
    // sum (order->quantity - order->filled_quantity) for each order
    return 0;
}

void OrderBook::add_order(Order* order) {
    // get or create PriceLevel at order->price on the correct side (bids_ or asks_)
    // push_back order pointer into level.orders
}

Order* OrderBook::cancel_order(const std::string& order_id, Side side, double price) {
    // find the PriceLevel at (side, price)
    // find order in level.orders by order_id
    // erase it from the deque
    // if level is now empty, erase the price entry from the map
    // return the raw pointer, or nullptr if not found
    return nullptr;
}

PriceLevel* OrderBook::best_bid() {
    // return pointer to first element of bids_ (highest price), or nullptr if empty
    return nullptr;
}

PriceLevel* OrderBook::best_ask() {
    // return pointer to first element of asks_ (lowest price), or nullptr if empty
    return nullptr;
}

std::optional<double> OrderBook::spread() const {
    // if either side empty return nullopt
    // return asks_.begin()->first - bids_.begin()->first
    return std::nullopt;
}

}  // namespace order_engine
