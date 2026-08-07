# Order Engine

A limit order book and matching engine written in C++, exposed over a TCP gateway with a live Python/Streamlit visualizer. Built during an 8-week Build Fellowship workshop on order matching engines.

![demo placeholder](docs/demo.gif)
<!-- Replace with a screenshot or short GIF of the visualizer: a resting order,
     a multi-level sweep producing trades, and a cancel. -->

## What it does

- **Price-time priority matching** for LIMIT and MARKET orders, including partial fills and sweeps across multiple price levels.
- **O(1) order cancellation** via a global order-ID index, independent of book depth.
- **Thread-safe order submission**, validated with a concurrent stress test.
- **VWAP execution analytics** in the visualizer — session VWAP, trade count, volume, and a running-VWAP chart that shows how the average execution price moves as an order sweeps through multiple price levels.
- **A TCP gateway** implementing a custom line-based wire protocol (see `PROTOCOL.md`), so the engine can be driven by any client — including the web UI.

## Architecture

```
cpp/
  order_book.h/.cpp        # PriceLevel + OrderBook: price-sorted maps, FIFO per level, global order index
  matching_engine.h/.cpp   # match_limit_order / match_market_order, VWAP
  gateway.cpp               # TCP server: parses protocol lines, calls into the engine, formats replies
visualizer/
  app.py                    # Streamlit visualizer: order entry, live ladder, trade tape, execution stats
  client.py                 # Python client for the wire protocol
PROTOCOL.md                 # Wire protocol spec shared by the gateway and any client
```

Design notes worth reading the source for: `PriceLevel` uses `std::deque` rather than a plain queue or linked list, since it needs FIFO access *and* mid-container erasure for cancellations, with better cache locality than a linked list. `OrderBook` keeps bids and asks in separate `std::map`s (bids sorted descending via `std::greater<double>`) so the best price is always the container's first element — no search needed. The matching loop is written so a cancellation that empties a price level can never leave it holding a dangling pointer: it breaks and re-fetches a fresh level reference rather than reasoning about pointer validity after the fact.

## Build & run

Requires a C++17 compiler and CMake.

```bash
# Build the engine + gateway
cd cpp
mkdir -p build && cd build
cmake ..
make

# Run the gateway
./gateway
# Listens on 127.0.0.1:9001
```

In a second terminal, run the visualizer:

```bash
cd visualizer
python3 -m venv venv && source venv/bin/activate
pip install -r requirements.txt
streamlit run app.py
```

Open the URL Streamlit prints (defaults to `http://localhost:8501`), connect to `127.0.0.1:9001`, and submit orders.

You can also talk to the gateway directly without the UI:

```bash
nc 127.0.0.1 9001
NEW AAPL SELL LIMIT 200 10.01
BOOK AAPL
```

See `PROTOCOL.md` for the full command/reply reference.

## Testing

Unit tests cover the order book (add/cancel/best bid-ask/spread), the matching engine (partial fills, multi-level sweeps, limit vs. market behavior), and a multithreaded stress test that hammers `add_order` concurrently and asserts against `total_quantity()`.

```bash
cd cpp/build
ctest
```

## Known limitations

- `best_bid()` / `best_ask()` / `spread()` reads are not mutex-guarded — only `add_order`/`cancel_order` are, a deliberate scope boundary rather than an oversight.
- Order types are LIMIT and MARKET only; no IOC/FOK/stop/iceberg orders.
- No persistence — the book lives in memory and resets when the gateway restarts.
- No multi-symbol concurrency — the orderbook is not able to process multiple orders from various symbols at once. 
- No high-volume throughput — orders are processed one at a time as they arrive over a single connection, with no batching or concurrent request handling, so it isn't built to handle a large burst of orders at once.

## How this was built

| Week | Focus |
|------|-------|
| 1 | Market foundations, environment setup |
| 2 | Order book design — price-time priority architecture |
| 3 | `add_order` / `cancel_order` — the book's core operations |
| 4 | Matching algorithm — the core loop for trade execution |
| 5 | Market orders — sweeping multiple price levels |
| 6 | Thread safety — concurrent order submission, stress testing |
| 7 | TCP gateway — exposing the engine over a network protocol |
| 8 | Visualizer, execution analytics, final demo |

## Status

Complete: order book data structures → matching engine → thread safety → TCP gateway → visualizer with execution analytics.
