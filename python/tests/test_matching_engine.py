from order_engine.models import Order, Side, OrderType, OrderStatus
from order_engine.matching_engine import MatchingEngine


def test_no_match_when_prices_dont_cross():
    # submit buy at 99, sell at 101
    # assert no trades generated
    pass


def test_exact_match():
    # submit buy at 100 qty 10, then sell at 100 qty 10
    # assert 1 trade with price=100, qty=10
    # assert both orders status == FILLED
    pass


def test_partial_fill():
    # submit buy at 100 qty 10, then sell at 100 qty 6
    # assert 1 trade qty=6
    # assert buy is PARTIALLY_FILLED, sell is FILLED
    pass


def test_price_time_priority():
    # submit two buys at same price
    # submit sell that only fills one
    # assert the earlier buy is the one filled
    pass


def test_market_buy_fills_at_best_ask():
    # submit limit sell at 100 qty 10
    # submit market buy qty 5
    # assert 1 trade at price=100, qty=5
    pass
