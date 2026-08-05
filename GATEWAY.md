# Building the Gateway (this week)

The **gateway** exposes your engine over the network with a small TCP server
that speaks the shared [wire protocol](PROTOCOL.md). This week you implement the
three protocol handlers; the socket plumbing is already written for you.

Once both tracks implement the *same* protocol, a single visualizer (or `nc`)
can drive either engine.

| Track | File to edit | Port |
|-------|--------------|------|
| Python | `python/gateway.py` | `9000` |
| C++ | `cpp/src/gateway.cpp` | `9001` |

## Your task

Fill in the three handlers marked `TODO [this week]`:

- **`handle_new`** — parse `NEW ...`, submit to the engine, reply with `TRADE`
  lines + a `STATUS` line
- **`handle_cancel`** — cancel by id, reply with a `STATUS` line
- **`handle_book`** — reply with the `BID`/`ASK`/`END` ladder snapshot

All exact formats are in [PROTOCOL.md](PROTOCOL.md). Tip: for `handle_book`
you'll want a way to list every price level with its total quantity — add a
small `snapshot` helper to your `OrderBook` if you don't have one.

## Run it

**Python** (no build):
```bash
cd python
python gateway.py
```
> If `python` on your machine is older than 3.11 or your venv is mismatched,
> run with an explicit interpreter, e.g.
> `PYTHONPATH=".:.venv/lib/python3.12/site-packages" python3.12 gateway.py`.

**C++** (compile first):
```bash
cd cpp
cmake -B build -DCMAKE_BUILD_TYPE=Debug   # first time only
cmake --build build                       # produces build/gateway
./build/gateway
```

## Connect and test

```bash
nc 127.0.0.1 9000        # Python (use 9001 for C++)
```
```
NEW AAPL SELL LIMIT 200 10.01
NEW AAPL SELL LIMIT 400 10.02
BOOK AAPL
NEW AAPL BUY MARKET 500
BOOK AAPL
```

Out of the box the handlers reply `ERR ... not implemented yet` — making the
real replies appear is your job. `Ctrl-C` stops the server. If a port is stuck,
free it with `lsof -ti :9000 | xargs kill`.
