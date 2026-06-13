# Order Engine — C++ Track

## Prerequisites

- C++20 compiler (GCC 12+ or Clang 15+)
- CMake 3.20+
- [Catch2 v3](https://github.com/catchorg/Catch2) — install via vcpkg or your system package manager

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

Tests in `test_order_book.cpp` should pass out of the box.  
Tests in `test_matching_engine.cpp` will fail until you implement the matching logic.

## Structure

```
include/
  models.h           # Order, Trade, enums — read this first
  order_book.h       # OrderBook declaration
  matching_engine.h  # MatchingEngine declaration
src/
  order_book.cpp     # Already implemented
  matching_engine.cpp # Implement the TODO methods here
tests/
  test_order_book.cpp      # Should pass from the start
  test_matching_engine.cpp # Your implementation will make these pass
```
