#pragma once
#include "models.h"
#include "order_book.h"
#include <memory>
#include <string>
#include <vector>

namespace order_engine {

class MatchingEngine {
public:
    std::vector<Trade> submit_order(std::unique_ptr<Order> order);
    bool               cancel_order(const std::string& order_id);
    OrderBook*         get_order_book(const std::string& symbol);
    const std::vector<Trade>& trade_history() const;

private:
    // TODO [later]: decide what state the engine holds — books, orders, trades.

    std::vector<Trade> match_limit_order (Order* order, OrderBook& book);
    std::vector<Trade> match_market_order(Order* order, OrderBook& book);
    Trade              create_trade      (Order& buy, Order& sell, double price, int64_t qty);
    OrderBook&         get_or_create_book(const std::string& symbol);
};

}  // namespace order_engine
