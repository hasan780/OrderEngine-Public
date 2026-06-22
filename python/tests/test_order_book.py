from order_engine.models import Order, Side, OrderType
from order_engine.order_book import OrderBook


def make_order(order_id, side, price, quantity, symbol="AAPL"):
    return Order(
        order_id=order_id,
        symbol=symbol,
        side=side,
        order_type=OrderType.LIMIT,
        quantity=quantity,
        price=price,
    )


# ── add_order ───────────────────────────────────────────────────────
# We verify an order was added by cancelling it and inspecting the
# returned order (no dependency on best_bid / best_ask).

def test_add_then_cancel_returns_the_same_order():
    book = OrderBook("AAPL")
    book.add_order(make_order("b1", Side.BUY, 100.0, 10))
    removed = book.cancel_order("b1", Side.BUY, 100.0)
    assert removed is not None
    assert removed.order_id == "b1"
    assert removed.quantity == 10


def test_add_order_on_ask_side():
    book = OrderBook("AAPL")
    book.add_order(make_order("a1", Side.SELL, 102.0, 5))
    removed = book.cancel_order("a1", Side.SELL, 102.0)
    assert removed is not None
    assert removed.order_id == "a1"


def test_add_multiple_orders_at_same_price_are_all_cancellable():
    book = OrderBook("AAPL")
    book.add_order(make_order("b1", Side.BUY, 100.0, 10))
    book.add_order(make_order("b2", Side.BUY, 100.0, 15))
    assert book.cancel_order("b1", Side.BUY, 100.0).order_id == "b1"
    assert book.cancel_order("b2", Side.BUY, 100.0).order_id == "b2"


# ── cancel_order ────────────────────────────────────────────────────

def test_cancel_actually_removes_the_order():
    book = OrderBook("AAPL")
    book.add_order(make_order("b1", Side.BUY, 100.0, 10))
    assert book.cancel_order("b1", Side.BUY, 100.0) is not None  # first removes it
    assert book.cancel_order("b1", Side.BUY, 100.0) is None      # gone the second time


def test_cancel_nonexistent_order_returns_none():
    book = OrderBook("AAPL")
    book.add_order(make_order("b1", Side.BUY, 100.0, 10))
    assert book.cancel_order("does-not-exist", Side.BUY, 100.0) is None


def test_cancel_at_wrong_price_returns_none_and_leaves_order():
    book = OrderBook("AAPL")
    book.add_order(make_order("b1", Side.BUY, 100.0, 10))
    assert book.cancel_order("b1", Side.BUY, 99.0) is None    # wrong price
    assert book.cancel_order("b1", Side.BUY, 100.0) is not None  # still there


def test_cancel_one_of_many_leaves_the_other():
    book = OrderBook("AAPL")
    book.add_order(make_order("b1", Side.BUY, 100.0, 10))
    book.add_order(make_order("b2", Side.BUY, 100.0, 15))
    book.cancel_order("b1", Side.BUY, 100.0)
    assert book.cancel_order("b2", Side.BUY, 100.0) is not None  # b2 untouched


# ── best_bid / best_ask / spread ────────────────────────────────────
# NOTE: these are not part of this week's assignment (add_order / cancel_order).
# Uncomment once you implement best_bid / best_ask / spread.

# def test_fifo_within_price_level():
#     book = OrderBook("AAPL")
#     book.add_order(make_order("b1", Side.BUY, 100.0, 5))
#     book.add_order(make_order("b2", Side.BUY, 100.0, 7))
#     assert book.best_bid().orders[0].order_id == "b1"


# def test_best_bid_and_ask():
#     book = OrderBook("AAPL")
#     book.add_order(make_order("b1", Side.BUY, 100.0, 10))
#     book.add_order(make_order("b2", Side.BUY, 101.0, 5))
#     book.add_order(make_order("a1", Side.SELL, 102.0, 8))
#     assert book.best_bid().price == 101.0
#     assert book.best_ask().price == 102.0


# def test_spread():
#     book = OrderBook("AAPL")
#     book.add_order(make_order("b1", Side.BUY, 100.0, 10))
#     book.add_order(make_order("a1", Side.SELL, 102.0, 10))
#     assert book.spread() == 2.0


# def test_empty_book_spread():
#     book = OrderBook("AAPL")
#     assert book.spread() is None
