#include "order_book.h"

namespace order_engine {

OrderBook::OrderBook(std::string symbol) {
    // TODO [this week]: store the symbol

    // MY CODE
    symbol_ = symbol;
    //
}

int64_t PriceLevel::total_quantity() const {
    // TODO [this week]: sum (order->quantity - order->filled_quantity) for each order

    //MY CODE
    int64_t tot_quant = 0;
    for (auto order : orders) {
        tot_quant += order->remaining_quantity();
    }
    return tot_quant;
    //
    return 0;
}

void OrderBook::add_order(Order* order) {
    // TODO [this week]:
    //   - get or create the PriceLevel at order->price on the correct side (bids_ or asks_)
    //   - push_back the order pointer into that level's orders

    // MY CODE
    // validation of values before (HASAN NOTES)
    if (order->quantity <= 0) {
        return;
    } 
    if (order->price.has_value() && order->price.value() <= 0) { //AI improvement: check if it also has a value initially 
        return;
    }

    if (order->side == Side::BUY) {
        bids_[order->price.value()].price = order->price.value();
        bids_[order->price.value()].orders.push_back(order);

    } else {
        asks_[order->price.value()].price = order->price.value();
        asks_[order->price.value()].orders.push_back(order);
    }
    // IMPROVEMENT: global indexing goes here (HASAN NOTES)
    order_lookup_[order->order_id] = order;
}

Order* OrderBook::cancel_order(const std::string& order_id, Side side, double price) {
    // TODO [this week]:
    //   - find the PriceLevel at (side, price)
    //   - find the order in that level by order_id
    //   - erase it from the deque
    //   - if the level is now empty, erase the price entry from the map
    //   - return the raw pointer, or nullptr if not found

    // MY CODE
    // IMPROVEMENT: instead of looking up by price or size, add global indexing + updating
    // and cleanup to proper places. also above add the validation, add 1 UNIT test case, this is HW.
    // priority of hw: 3 new function, global indexing, validation, unit test
    // also check email and validate compilation
    auto it_lookup = order_lookup_.find(order_id); // global indexing added, not implemented in rest of code yet
    if (it_lookup == order_lookup_.end()) {
        return nullptr;
    }

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
    // TODO [this week]: return pointer to first element of bids_ (highest price), or nullptr if empty
    
    //MY CODE
    if (bids_.empty()) {
        return nullptr;
    } else {
        return &bids_.begin()->second;
    }
    //
    
    return nullptr;
}

PriceLevel* OrderBook::best_ask() {
    // TODO [this week]: return pointer to first element of asks_ (lowest price), or nullptr if empty
    
    //MY CODE
    if (asks_.empty()) {
        return nullptr;
    } else {
        return &asks_.begin()->second;
    }
    //

    return nullptr;
}

std::optional<double> OrderBook::spread() const {
    // TODO [this week]: if either side empty return nullopt; else asks_.begin()->first - bids_.begin()->first
    
    //MY CODE
    if (asks_.empty() || bids_.empty()) {
        return std::nullopt;
    } else {
        return asks_.begin()->first - bids_.begin()->first;
    }
    //

    return std::nullopt;
}

}  // namespace order_engine
