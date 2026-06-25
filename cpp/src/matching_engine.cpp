#include "matching_engine.h"

namespace order_engine {

std::vector<Trade> MatchingEngine::submit_order(std::unique_ptr<Order> order) {
    // TODO [later]:
    //   - get_or_create_book for order->symbol
    //   - store order in the orders_ map
    //   - if LIMIT  -> return match_limit_order(raw_ptr, book)
    //   - if MARKET -> return match_market_order(raw_ptr, book)
    return {};
}

bool MatchingEngine::cancel_order(const std::string& order_id) {
    // TODO [later]:
    //   - look up order in orders_
    //   - if not found or not OPEN/PARTIALLY_FILLED, return false
    //   - call book.cancel_order(order_id, side, price) using the order's side/price
    //   - set order->status = CANCELLED
    //   - return true
    return false;
}

OrderBook* MatchingEngine::get_order_book(const std::string& symbol) {
    // TODO [later]: find symbol in books_, return pointer or nullptr
    return nullptr;
}

const std::vector<Trade>& MatchingEngine::trade_history() const {
    // commenting out line below so code compiles 6/25/2026 HASAN
    //return trades_;
    return {}; //placeholder for above
}

std::vector<Trade> MatchingEngine::match_limit_order(Order* order, OrderBook& book) {
    // TODO [later]:
    //   - if BUY:  while best_ask() exists and ask.price <= order->price
    //                  fill against the front of that level (FIFO), call create_trade
    //                  remove empty levels from the book
    //   - if SELL: while best_bid() exists and bid.price >= order->price
    //                  fill against the front of that level (FIFO), call create_trade
    //                  remove empty levels from the book
    //   - if order->remaining_quantity() > 0, book.add_order(order)
    //   - return collected trades
    return {};
}

std::vector<Trade> MatchingEngine::match_market_order(Order* order, OrderBook& book) {
    // TODO [later]: same as limit but no price constraint —
    //   keep consuming the best level until filled or the book is empty
    return {};
}

Trade MatchingEngine::create_trade(Order& buy, Order& sell, double price, int64_t qty) {
    // TODO [later]:
    //   - buy.filled_quantity  += qty
    //   - sell.filled_quantity += qty
    //   - update status on both: FILLED if fully consumed, else PARTIALLY_FILLED
    //   - build Trade with new id, symbol, order ids, price, qty, timestamp
    //   - append to trades_ and return it
    return {};
}

OrderBook& MatchingEngine::get_or_create_book(const std::string& symbol) {
    // TODO [later]: if symbol not in books_, create a new OrderBook and insert it;
    //   return a reference to the book
    //commenting out line below so the code compiles for now 6/25/2026 HASAN
    //return *books_[symbol];
    return *static_cast<OrderBook*>(nullptr); //placeholder for above
}

}  // namespace order_engine
