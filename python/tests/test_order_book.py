from order_engine.models import Order, Side, OrderType
from order_engine.order_book import OrderBook


def test_best_bid_and_ask():
    # add bids at 100 and 101, ask at 102
    # assert best_bid.price == 101
    # assert best_ask.price == 102
    pass


def test_spread():
    # add bid at 100, ask at 102
    # assert spread == 2.0
    pass


def test_empty_book_spread():
    # assert spread is None on empty book
    pass


def test_cancel_order():
    # add a bid, cancel it, assert best_bid is None
    pass


def test_fifo_within_price_level():
    # add two bids at same price
    # assert first added is first in level
    pass
