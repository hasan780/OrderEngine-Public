#include "matching_engine.h"
#include <atomic>
#include <chrono>
#include <stdexcept>

namespace order_engine {

// -----------------------------------------------------------------------------
// Scaffolding below is already implemented for you. Your job this week is the
// ONE method marked "TODO [this week]": match_limit_order.
// -----------------------------------------------------------------------------

static int64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

static std::string next_trade_id() {
    static std::atomic<uint64_t> counter{0};
    return "trade-" + std::to_string(++counter);
}

std::vector<Trade> MatchingEngine::submit_order(std::unique_ptr<Order> order) {
    OrderBook& book = get_or_create_book(order->symbol);
    Order* raw = order.get();
    orders_[raw->order_id] = std::move(order);

    if (raw->order_type == OrderType::LIMIT)
        return match_limit_order(raw, book);
    else if (raw->order_type == OrderType::MARKET)
        return match_market_order(raw, book);
    throw std::invalid_argument("unknown order type");
}

bool MatchingEngine::cancel_order(const std::string& order_id) {
    auto it = orders_.find(order_id);
    if (it == orders_.end())
        return false;
    Order* order = it->second.get();
    if (order->status == OrderStatus::FILLED || order->status == OrderStatus::CANCELLED)
        return false;

    auto book_it = books_.find(order->symbol);
    if (book_it != books_.end() && order->price.has_value())
        book_it->second->cancel_order(order_id, order->side, order->price.value());
    order->status = OrderStatus::CANCELLED;
    return true;
}

OrderBook* MatchingEngine::get_order_book(const std::string& symbol) {
    auto it = books_.find(symbol);
    return it == books_.end() ? nullptr : it->second.get();
}

const std::vector<Trade>& MatchingEngine::trade_history() const {
    return trades_;
}

OrderBook& MatchingEngine::get_or_create_book(const std::string& symbol) {
    auto it = books_.find(symbol);
    if (it == books_.end())
        it = books_.emplace(symbol, std::make_unique<OrderBook>(symbol)).first;
    return *it->second;
}

// Record a fill: update both orders, build + store the Trade.
// Use this from your matching loop — you don't need to change it.
Trade MatchingEngine::create_trade(Order& buy, Order& sell, double price, int64_t qty) {
    buy.filled_quantity  += qty;
    sell.filled_quantity += qty;
    buy.status  = (buy.remaining_quantity()  == 0) ? OrderStatus::FILLED
                                                   : OrderStatus::PARTIALLY_FILLED;
    sell.status = (sell.remaining_quantity() == 0) ? OrderStatus::FILLED
                                                   : OrderStatus::PARTIALLY_FILLED;
    Trade t{
        next_trade_id(),
        buy.symbol,
        buy.order_id,
        sell.order_id,
        price,
        qty,
        now_ns(),
    };
    trades_.push_back(t);
    return t;
}

// -----------------------------------------------------------------------------
// YOUR TASK THIS WEEK
// -----------------------------------------------------------------------------

std::vector<Trade> MatchingEngine::match_limit_order(Order* order, OrderBook& book) {
    // TODO [this week]: implement Price-Time Priority matching.
    //
    // Step by step:
    //   std::vector<Trade> trades;
    //  
    //   while (order->remaining_quantity() > 0) {
    //       1. Get the best opposite level:
    //            BUY  -> PriceLevel* level = book.best_ask();
    //            SELL -> PriceLevel* level = book.best_bid();
    //          If level == nullptr -> break (no liquidity).
    //
    //       2. Check the prices cross:
    //            BUY  matches only if level->price <= order->price.value()
    //            SELL matches only if level->price >= order->price.value()
    //          If they don't cross -> break.
    //
    //       3. Walk level->orders OLDEST FIRST (front of the list = FIFO).
    //          For the front resting order (loop while it has orders and the
    //          aggressor still needs quantity):
    //            - Order* resting = level->orders.front();
    //            - int64_t fill = std::min(order->remaining_quantity(),
    //                                      resting->remaining_quantity());
    //            - decide buyer/seller (BUY aggressor: buy=order, sell=resting)
    //            - trades.push_back(create_trade(*buy, *sell, level->price, fill));
    //              (NOTE: price is the RESTING level's price — the passive side)
    //            - if resting->remaining_quantity() == 0:
    //                  book.cancel_order(resting->order_id, resting->side, level->price);
    //              (this removes it and prunes the empty level for you)
    //
    //   4. Whatever is left over rests on the book:
    //        if (order->remaining_quantity() > 0) book.add_order(order);
    //
    //   return trades;

    // MY CODE
        std::vector<Trade> trades;

        while (order->remaining_quantity() > 0) { //loop if we have orders
            PriceLevel* level = nullptr;

            if (order->side == Side::BUY) { //checks for buy
                level = book.best_ask(); //assigns level to the best ask for the buy unless there isn't one
                if (level == nullptr) { //checks if level has value
                    break;
                }
                if (level->price > order->price.value()) { //checks for if prices don't cross
                    break;
                }
            } else if (order->side == Side::SELL) {
                level = book.best_bid();
                if (level == nullptr) {
                    break;
                }
                if (level->price < order->price.value()) {
                    break;
                }
            } else {
                break;
            }

            while (order->remaining_quantity() > 0 && !level->orders.empty()) { //loops when quantity has value and level is not empty
                Order* resting = level->orders.front(); //resting orders get FIFO'ed
                int64_t fill = std::min(order->remaining_quantity(), resting->remaining_quantity());
            
                Order* buy = nullptr; //declaration of buy and sell
                Order* sell = nullptr;

                if (order->side == Side::BUY) { //if its buy, sell is resting/buy is order
                    sell = resting; //using Order* here again declares a brand new variable inside this scope
                    buy = order;
                } else {
                    buy = resting;
                    sell = order;
                }

                trades.push_back(create_trade(*buy, *sell, level->price, fill)); 

                if (resting->remaining_quantity() == 0) {
                    book.cancel_order(resting->order_id, resting->side, level->price);
                    break; //to exit inner loop after cancelling order
                }
            }
        }

        if (order->remaining_quantity() > 0) book.add_order(order);
        return trades;
    //

    return {};
}

std::vector<Trade> MatchingEngine::match_market_order(Order* order, OrderBook& book) {
    // TODO [later]: (bonus / future) like match_limit_order but with NO price
    //   cross check — keep consuming the best level until filled or book empty.
    return {};
}

}  // namespace order_engine
