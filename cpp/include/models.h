#pragma once
#include <cstdint>
#include <optional>
#include <string>

namespace order_engine {

enum class Side       : uint8_t { BUY, SELL };
enum class OrderType  : uint8_t { LIMIT, MARKET };
enum class OrderStatus: uint8_t { OPEN, PARTIALLY_FILLED, FILLED, CANCELLED };

struct Order {
    std::string           order_id;
    std::string           symbol;
    Side                  side;
    OrderType             order_type;
    int64_t               quantity;
    std::optional<double> price;            // nullopt for MARKET orders
    int64_t               timestamp;        // nanoseconds since epoch
    int64_t               filled_quantity = 0;
    OrderStatus           status = OrderStatus::OPEN;

    int64_t remaining_quantity() const { return quantity - filled_quantity; }
};

struct Trade {
    std::string trade_id;
    std::string symbol;
    std::string buy_order_id;
    std::string sell_order_id;
    double      price;
    int64_t     quantity;
    int64_t     timestamp;                  // nanoseconds since epoch
};

}  // namespace order_engine
