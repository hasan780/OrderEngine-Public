# 2026 June Order Engine Workshop

Build Fellowship 2026 — 8-week project: build a high-performance order matching engine from scratch.

## Tracks

| Track | Folder | Language | Prerequisites |
|-------|--------|----------|---------------|
| Python | [`python/`](python/) | Python 3.11+ | pip |
| C++ | [`cpp/`](cpp/) | C++20 | CMake, Catch2 |

Each track has its own `README.md` with setup instructions, build commands, and week-by-week tasks.

## How to get started

1. **Fork this repo** to your own GitHub account.
2. `cd` into the track you chose (`python/` or `cpp/`).
3. Follow the setup steps in that folder's `README.md`.
4. Run the tests — order book tests should pass, matching engine tests will fail until you implement them.
5. Implement the `TODO` methods week by week.

## Weekly plan (high-level)

| WS | Topic | Focus |
|----|-------|-------|
| 1 | Introduction & Setup | Market foundations and GitHub environment |
| 2 | LOB Design | Whiteboarding Price-Time priority architecture |
| 3 | Building the Book | Implementing Add/Cancel logic in Python/C++ |
| 4 | Matching Algorithm | Coding the core loop for trade executions |
| 5 | Market Orders | Extending engine to sweep multiple price levels |
| 6 | Network Programming | Intro to TCP/IP and socket servers |
| 7 | Trading API | Connecting engine via text-based protocols |
| 8 | Final Demo | Presenting full GitHub repo and live engine |
