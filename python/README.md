# Order Engine — Python Track

## Setup

```bash
cd python
python -m venv .venv
source .venv/bin/activate      # Windows: .venv\Scripts\activate
pip install -r requirements.txt
```

## Run tests

```bash
pytest tests/ -v
```

## Structure

```
order_engine/
  models.py           # Order, Trade, and status enums — define your fields here
  order_book.py       # PriceLevel and OrderBook — implement add, cancel, best bid/ask
  matching_engine.py  # MatchingEngine — implement submit, cancel, and matching logic
tests/
  test_order_book.py      # Tests for OrderBook
  test_matching_engine.py # Tests for MatchingEngine
```

## Key classes

**`models.py`**
- `Order` — represents a single order (fields to be designed by you)
- `Trade` — represents an executed match (fields to be designed by you)
- `Side`, `OrderType`, `OrderStatus` — enums already provided

**`order_book.py`**
- `PriceLevel` — holds all orders at a single price, FIFO
- `OrderBook` — manages bids and asks; bids sorted highest first, asks lowest first

**`matching_engine.py`**
- `MatchingEngine` — entry point for submitting and cancelling orders
- `_match_limit_order` — walk opposite side and fill where prices cross
- `_match_market_order` — sweep best available prices until filled
- `_create_trade` — build a Trade and update both order statuses
