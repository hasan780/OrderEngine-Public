# Order Engine (C++)

## Prerequisites

- C++20 compiler (GCC 12+ or Clang 15+)
- CMake 3.20+
- [Catch2 v3](https://github.com/catchorg/Catch2): install via vcpkg or your system package manager

### Install Catch2 (pick one)

**macOS (Homebrew)**
```bash
brew install catch2
```

**Ubuntu/Debian**
```bash
sudo apt install catch2
```

**vcpkg**
```bash
vcpkg install catch2
```

## Build & run tests

```bash
cd cpp
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest --output-on-failure
```

## Structure

```
include/
  models.h            # Order, Trade, and enums
  order_book.h        # PriceLevel and OrderBook declarations
  matching_engine.h   # MatchingEngine declaration
src/
  order_book.cpp      # OrderBook implementation
  matching_engine.cpp # MatchingEngine implementation, VWAP
  gateway.cpp         # TCP gateway (see ../PROTOCOL.md)
tests/
  test_order_book.cpp      # Tests for OrderBook
  test_matching_engine.cpp # Tests for MatchingEngine
```

## Key classes

**`models.h`**
- `Order`: a single order
- `Trade`: an executed match
- `Side`, `OrderType`, `OrderStatus`: enums

**`order_book.h`**
- `PriceLevel`: holds all orders at a single price as a FIFO queue
- `OrderBook`: manages bids and asks; bids sorted highest first, asks lowest first

**`matching_engine.h`**
- `MatchingEngine`: entry point for submitting and cancelling orders
- `match_limit_order`: walk opposite side and fill where prices cross
- `match_market_order`: sweep best available prices until filled
- `create_trade`: build a Trade and update both order statuses
