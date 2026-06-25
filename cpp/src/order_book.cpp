#include "order_book.h"

namespace order_engine {

OrderBook::OrderBook(std::string symbol) {
    // TODO [this week]: store the symbol

    // MY CODE
    symbol_ = symbol;
    //
}

int64_t PriceLevel::total_quantity() const {
    // TODO [later]: sum (order->quantity - order->filled_quantity) for each order
    return 0;
}

void OrderBook::add_order(Order* order) {
    // TODO [this week]:
    //   - get or create the PriceLevel at order->price on the correct side (bids_ or asks_)
    //   - push_back the order pointer into that level's orders

    // MY CODE
    if (order->side == Side::BUY) {
        bids_[order->price.value()].price = order->price.value();
        bids_[order->price.value()].orders.push_back(order);

    } else {
        asks_[order->price.value()].price = order->price.value();
        asks_[order->price.value()].orders.push_back(order);
    }
    //
}

Order* OrderBook::cancel_order(const std::string& order_id, Side side, double price) {
    // TODO [this week]:
    //   - find the PriceLevel at (side, price)
    //   - find the order in that level by order_id
    //   - erase it from the deque
    //   - if the level is now empty, erase the price entry from the map
    //   - return the raw pointer, or nullptr if not found

    // MY CODE
    if (side == Side::BUY) {
        auto lvl_iterator = bids_.find(price);
        if (lvl_iterator == bids_.end()) {
            return nullptr;
        }
    for (auto it = lvl_iterator->second.orders.begin(); it != lvl_iterator->second.orders.end(); it++) {
        if ((*it)->order_id == order_id) {
            auto ord_ptr = *it;
            lvl_iterator->second.orders.erase(it);
            if (lvl_iterator->second.orders.empty()) {
                bids_.erase(lvl_iterator);
                }
            return ord_ptr;   
            }
        }
    } else {
        auto lvl_iterator = asks_.find(price);
        if (lvl_iterator == asks_.end()) {
            return nullptr;
        }
    for (auto it = lvl_iterator->second.orders.begin(); it != lvl_iterator->second.orders.end(); it++) {
        if ((*it)->order_id == order_id) {
            auto ord_ptr = *it;
            lvl_iterator->second.orders.erase(it);
            if (lvl_iterator->second.orders.empty()) {
                asks_.erase(lvl_iterator);
                }
            return ord_ptr;   
            }
        }
    }
    //
    return nullptr;
}

PriceLevel* OrderBook::best_bid() {
    // TODO [later]: return pointer to first element of bids_ (highest price), or nullptr if empty
    return nullptr;
}

PriceLevel* OrderBook::best_ask() {
    // TODO [later]: return pointer to first element of asks_ (lowest price), or nullptr if empty
    return nullptr;
}

std::optional<double> OrderBook::spread() const {
    // TODO [later]: if either side empty return nullopt; else asks_.begin()->first - bids_.begin()->first
    return std::nullopt;
}

}  // namespace order_engine
