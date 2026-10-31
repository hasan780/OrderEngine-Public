#include "order_book.h"

namespace order_engine {

OrderBook::OrderBook(std::string symbol) {
    symbol_ = symbol;
}

int64_t PriceLevel::total_quantity() const {
    int64_t tot_quant = 0;
    for (auto order : orders) {
        tot_quant += order->remaining_quantity();
    }
    return tot_quant;
}

void OrderBook::add_order(Order* order) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (order->quantity <= 0) {
        return;
    }
    if (order->price.has_value() && order->price.value() <= 0) {
        return;
    }

    if (order->side == Side::BUY) {
        bids_[order->price.value()].price = order->price.value();
        bids_[order->price.value()].orders.push_back(order);
    } else {
        asks_[order->price.value()].price = order->price.value();
        asks_[order->price.value()].orders.push_back(order);
    }
    order_lookup_[order->order_id] = order;
}

Order* OrderBook::cancel_order(const std::string& order_id, Side side, double price) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it_lookup = order_lookup_.find(order_id);
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
    return nullptr;
}

PriceLevel* OrderBook::best_bid() {
    if (bids_.empty()) {
        return nullptr;
    }
    return &bids_.begin()->second;
}

PriceLevel* OrderBook::best_ask() {
    if (asks_.empty()) {
        return nullptr;
    }
    return &asks_.begin()->second;
}

std::optional<double> OrderBook::spread() const {
    if (asks_.empty() || bids_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first - bids_.begin()->first;
}

}  // namespace order_engine
