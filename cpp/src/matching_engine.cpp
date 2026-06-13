#include "matching_engine.h"

namespace order_engine {

std::vector<Trade> MatchingEngine::submit_order(std::unique_ptr<Order> order) {
    // get_or_create_book for order->symbol
    // store order in orders_ map
    // if LIMIT  -> return match_limit_order(raw_ptr, book)
    // if MARKET -> return match_market_order(raw_ptr, book)
    return {};
}

bool MatchingEngine::cancel_order(const std::string& order_id) {
    // look up order in orders_
    // if not found or not OPEN/PARTIALLY_FILLED, return false
    // call book.cancel_order(order_id, side, price)
    // set order->status = CANCELLED
    // return true
    return false;
}

OrderBook* MatchingEngine::get_order_book(const std::string& symbol) {
    // find symbol in books_, return pointer or nullptr
    return nullptr;
}

const std::vector<Trade>& MatchingEngine::trade_history() const {
    return trades_;
}

std::vector<Trade> MatchingEngine::match_limit_order(Order* order, OrderBook& book) {
    // if BUY:  while best_ask() exists and ask.price <= order->price
    //              fill against front of that level (FIFO), call create_trade
    //              remove empty levels from book
    // if SELL: while best_bid() exists and bid.price >= order->price
    //              fill against front of that level (FIFO), call create_trade
    //              remove empty levels from book
    // if order->remaining_quantity() > 0, book.add_order(order)
    // return collected trades
    return {};
}

std::vector<Trade> MatchingEngine::match_market_order(Order* order, OrderBook& book) {
    // same as limit but no price constraint — keep consuming best level until filled or book empty
    // return collected trades
    return {};
}

Trade MatchingEngine::create_trade(Order& buy, Order& sell, double price, int64_t qty) {
    // buy.filled_quantity  += qty
    // sell.filled_quantity += qty
    // update status on both: FILLED if fully consumed, else PARTIALLY_FILLED
    // build Trade with new id, symbol, order ids, price, qty, timestamp
    // append to trades_
    // return trade
    return {};
}

OrderBook& MatchingEngine::get_or_create_book(const std::string& symbol) {
    // if symbol not in books_, create new OrderBook and insert
    // return reference to the book
    return *books_[symbol];
}

}  // namespace order_engine
