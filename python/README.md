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

Tests in `test_order_book.py` should pass out of the box.  
Tests in `test_matching_engine.py` will fail until you implement the matching logic — that's your task!

## Structure

```
order_engine/
  models.py          # Order, Trade, enums — read this first
  order_book.py      # OrderBook with price levels — already implemented
  matching_engine.py # MatchingEngine — implement the TODO methods here
tests/
  test_order_book.py      # Should pass from the start
  test_matching_engine.py # Your implementation will make these pass
```
