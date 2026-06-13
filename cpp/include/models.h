#pragma once
#include <cstdint>
#include <optional>
#include <string>

namespace order_engine {

enum class Side       : uint8_t { BUY, SELL };
enum class OrderType  : uint8_t { LIMIT, MARKET };
enum class OrderStatus: uint8_t { OPEN, PARTIALLY_FILLED, FILLED, CANCELLED };

struct Order {
    // think: what fields does an order need?
};

struct Trade {
    // think: what fields does a completed trade need?
};

}  // namespace order_engine
