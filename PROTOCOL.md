# Order Engine Wire Protocol (v1)

A tiny, line-based text protocol that both the **Python** and **C++** gateways
implement identically. Because the format is the same on both sides, the
**visualizer is written once** and works against either engine — just point it
at a different port.

```
   client (visualizer / nc)  ──TCP──►  gateway  ──►  MatchingEngine
                             ◄──TCP──            ◄──
```

## Ground rules

- **Transport:** TCP. One **message per line**, terminated by `\n`.
- **Encoding:** UTF-8 ASCII. Fields are separated by **one or more spaces**.
- **Case:** keywords (`NEW`, `BUY`, `LIMIT`, …) are **UPPERCASE**.
- **No JSON** — keep it token-based so C++ needs no extra libraries.
- The gateway reads a request line, calls the engine, and writes back zero or
  more response lines. Every response block ends so the client knows it's done
  (see each command).

---

## Requests  (client → engine)

### NEW — submit an order
```
NEW <symbol> <BUY|SELL> <LIMIT|MARKET> <qty> [price]
```
- `price` is **required for LIMIT**, **omitted for MARKET**.
- The gateway assigns the `order_id` and returns it in the `STATUS` line.

Examples:
```
NEW AAPL BUY  LIMIT  100 10.01
NEW AAPL SELL MARKET  50
```

### CANCEL — cancel a resting order
```
CANCEL <order_id>
```

### BOOK — request a snapshot of the book (used to draw the ladder)
```
BOOK <symbol>
```

### QUERY — ask for one order's live status
```
QUERY <order_id>
```
Because the book snapshot is aggregated (no ids), this is how a client checks
whether a specific order is still resting or has since been filled/cancelled.

### SYMBOLS — list the symbols the engine has a book for
```
SYMBOLS
```
Lets a client populate a symbol dropdown. Reply is `SYM <name>` lines then `END`.

---

## Responses  (engine → client)

### For `NEW`
Zero or more `TRADE` lines (one per fill), then exactly one `STATUS` line:
```
TRADE <trade_id> <symbol> <price> <qty> <buy_order_id> <sell_order_id>
STATUS <order_id> <FILLED|PARTIAL|RESTING|REJECTED> <filled_qty> <remaining_qty>
```
- `TRADE` lines come in fill order (the order they were matched).
- `STATUS` status meanings:
  - `FILLED`   — fully filled
  - `PARTIAL`  — partially filled; the rest is now resting (limit only)
  - `RESTING`  — no fill; the whole order is resting on the book (limit only)
  - `REJECTED` — invalid order (bad price/qty, or a market order with no liquidity left unfilled is still reported via `PARTIAL`/`FILLED`, not `REJECTED`)

Example — a buy that sweeps two levels then rests the remainder:
```
TRADE t-1 AAPL 10.01 200 o-7 o-3
TRADE t-2 AAPL 10.02 100 o-7 o-5
STATUS o-7 PARTIAL 300 200
```

### For `CANCEL`
```
STATUS <order_id> <CANCELLED|REJECTED> <filled_qty> <remaining_qty>
```
- `REJECTED` if the id is unknown or already filled/cancelled.

### For `BOOK`
Bid levels (highest price first), then ask levels (lowest price first), then `END`:
```
BID <price> <total_qty>
BID <price> <total_qty>
ASK <price> <total_qty>
ASK <price> <total_qty>
END
```
`total_qty` is the resting quantity at that price level. An empty side simply
contributes no lines. A completely empty book is just:
```
END
```

### Errors
Any request the gateway can't parse:
```
ERR <message>
```

---

## A full example session

```
>> NEW AAPL SELL LIMIT 200 10.01
<< STATUS o-1 RESTING 0 200

>> NEW AAPL SELL LIMIT 400 10.02
<< STATUS o-2 RESTING 0 400

>> BOOK AAPL
<< ASK 10.01 200
<< ASK 10.02 400
<< END

>> NEW AAPL BUY MARKET 500
<< TRADE t-1 AAPL 10.01 200 o-3 o-1
<< TRADE t-2 AAPL 10.02 300 o-3 o-2
<< STATUS o-3 FILLED 500 0

>> BOOK AAPL
<< ASK 10.02 100
<< END
```

---

## Notes for implementers

- **Gateways are thin.** Parse the line → build an `Order` → call
  `submit_order` / `cancel_order` → format the returned trades. Draw `BOOK`
  from `best_bid`/`best_ask` (walk the levels) or a snapshot helper.
- **VWAP** is a client-side concern: the visualizer can compute it from the
  `TRADE` lines it receives (or the engine's `vwap()` helper can be exposed
  later as an optional `VWAP` response line — not part of v1).
- **Ports (convention):** Python gateway `9000`, C++ gateway `9001`. The
  visualizer takes host/port as arguments so it can target either.
- **Keep v1 frozen.** Any additions (new commands, extra fields) go in a v2 so
  the shared visualizer never breaks.
