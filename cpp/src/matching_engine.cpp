#include "matching_engine.h"
#include <atomic>
#include <chrono>
#include <stdexcept>

namespace order_engine {

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

Order* MatchingEngine::find_order(const std::string& order_id) const {
    auto it = orders_.find(order_id);
    return it == orders_.end() ? nullptr : it->second.get();
}

std::vector<std::string> MatchingEngine::symbols() const {
    std::vector<std::string> out;
    for (const auto& kv : books_) out.push_back(kv.first);
    return out;
}

OrderBook& MatchingEngine::get_or_create_book(const std::string& symbol) {
    auto it = books_.find(symbol);
    if (it == books_.end())
        it = books_.emplace(symbol, std::make_unique<OrderBook>(symbol)).first;
    return *it->second;
}

// Record a fill: update both orders, build and store the Trade.
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

// Price-time priority: fill against the best opposite level while prices cross,
// oldest resting order first. Any remainder rests on the book.
std::vector<Trade> MatchingEngine::match_limit_order(Order* order, OrderBook& book) {
    std::vector<Trade> trades;

    while (order->remaining_quantity() > 0) {
        PriceLevel* level = nullptr;

        if (order->side == Side::BUY) {
            level = book.best_ask();
            if (level == nullptr) {
                break;
            }
            if (level->price > order->price.value()) {
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

        while (order->remaining_quantity() > 0 && !level->orders.empty()) {
            Order* resting = level->orders.front();
            int64_t fill = std::min(order->remaining_quantity(), resting->remaining_quantity());

            Order* buy = nullptr;
            Order* sell = nullptr;

            if (order->side == Side::BUY) {
                sell = resting;
                buy = order;
            } else {
                buy = resting;
                sell = order;
            }

            trades.push_back(create_trade(*buy, *sell, level->price, fill));

            // Removing a filled order can erase the level, so re-fetch it on the next pass.
            if (resting->remaining_quantity() == 0) {
                book.cancel_order(resting->order_id, resting->side, level->price);
                break;
            }
        }
    }

    if (order->remaining_quantity() > 0) book.add_order(order);
    return trades;
}

// Market orders take any price and sweep levels until filled or the book is empty.
// They never rest; an unfilled remainder is dropped.
std::vector<Trade> MatchingEngine::match_market_order(Order* order, OrderBook& book) {
    std::vector<Trade> trades;

    while (order->remaining_quantity() > 0) {
        PriceLevel* level = nullptr;

        if (order->side == Side::BUY) {
            level = book.best_ask();
            if (level == nullptr) {
                break;
            }
        } else if (order->side == Side::SELL) {
            level = book.best_bid();
            if (level == nullptr) {
                break;
            }
        } else {
            break;
        }

        while (order->remaining_quantity() > 0 && !level->orders.empty()) {
            Order* resting = level->orders.front();
            int64_t fill = std::min(order->remaining_quantity(), resting->remaining_quantity());

            Order* buy = nullptr;
            Order* sell = nullptr;

            if (order->side == Side::BUY) {
                sell = resting;
                buy = order;
            } else {
                buy = resting;
                sell = order;
            }

            trades.push_back(create_trade(*buy, *sell, level->price, fill));

            if (resting->remaining_quantity() == 0) {
                book.cancel_order(resting->order_id, resting->side, level->price);
                break;
            }
        }
    }

    return trades;
}

// VWAP = sum(price_i * quantity_i) / sum(quantity_i)
double vwap(const std::vector<Trade>& trades) {
    if (trades.empty()) {
        return 0.0;
    }

    double notional = 0.0;
    int64_t volume = 0;

    for (const auto& t : trades) {
        notional += t.price * t.quantity;
        volume += t.quantity;
    }

    return notional / volume;
}

}  // namespace order_engine
