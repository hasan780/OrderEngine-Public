import uuid
import pytest
from order_engine.models import Order, Side, OrderType, OrderStatus
from order_engine.matching_engine import MatchingEngine


def limit(side, price, qty, oid=None, symbol="AAPL"):
    return Order(
        order_id=oid or str(uuid.uuid4()),
        symbol=symbol,
        side=side,
        order_type=OrderType.LIMIT,
        quantity=qty,
        price=price,
    )


def market(side, qty, oid=None, symbol="AAPL"):
    return Order(
        order_id=oid or str(uuid.uuid4()),
        symbol=symbol,
        side=side,
        order_type=OrderType.MARKET,
        quantity=qty,
        price=None,
    )


# ── no cross ────────────────────────────────────────────────────────

def test_no_match_when_prices_dont_cross():
    engine = MatchingEngine()
    engine.submit_order(limit(Side.BUY, 99.0, 10))
    engine.submit_order(limit(Side.SELL, 101.0, 10))
    # bid 99 < ask 101 → nothing trades
    assert engine.get_trade_history() == []


# ── basic fills ─────────────────────────────────────────────────────

def test_exact_match():
    engine = MatchingEngine()
    buy = limit(Side.BUY, 100.0, 10, "b1")
    sell = limit(Side.SELL, 100.0, 10, "s1")
    engine.submit_order(buy)
    trades = engine.submit_order(sell)

    assert len(trades) == 1
    assert trades[0].price == 100.0
    assert trades[0].quantity == 10
    assert trades[0].buy_order_id == "b1"
    assert trades[0].sell_order_id == "s1"
    assert buy.status == OrderStatus.FILLED
    assert sell.status == OrderStatus.FILLED


def test_partial_fill_of_resting_order():
    engine = MatchingEngine()
    buy = limit(Side.BUY, 100.0, 10, "b1")   # rests, 10 available
    sell = limit(Side.SELL, 100.0, 6, "s1")  # only takes 6
    engine.submit_order(buy)
    trades = engine.submit_order(sell)

    assert len(trades) == 1
    assert trades[0].quantity == 6
    assert sell.status == OrderStatus.FILLED
    assert buy.status == OrderStatus.PARTIALLY_FILLED
    assert buy.filled_quantity == 6


def test_aggressor_remainder_rests_on_book():
    engine = MatchingEngine()
    sell = limit(Side.SELL, 100.0, 4, "s1")   # only 4 available
    buy = limit(Side.BUY, 100.0, 10, "b1")    # wants 10
    engine.submit_order(sell)
    trades = engine.submit_order(buy)

    assert len(trades) == 1
    assert trades[0].quantity == 4
    assert buy.status == OrderStatus.PARTIALLY_FILLED
    assert buy.filled_quantity == 4

    # the remaining 6 should now be resting — a new sell of 6 fills it
    trades2 = engine.submit_order(limit(Side.SELL, 100.0, 6, "s2"))
    assert len(trades2) == 1
    assert trades2[0].quantity == 6
    assert buy.status == OrderStatus.FILLED


# ── priority & pricing rules ────────────────────────────────────────

def test_price_time_priority():
    engine = MatchingEngine()
    b1 = limit(Side.BUY, 100.0, 5, "b1")   # earlier
    b2 = limit(Side.BUY, 100.0, 5, "b2")   # later
    engine.submit_order(b1)
    engine.submit_order(b2)
    trades = engine.submit_order(limit(Side.SELL, 100.0, 5, "s1"))

    assert len(trades) == 1
    assert trades[0].buy_order_id == "b1"   # oldest at the level fills first
    assert b1.status == OrderStatus.FILLED
    assert b2.status == OrderStatus.OPEN


def test_trade_executes_at_resting_price():
    engine = MatchingEngine()
    engine.submit_order(limit(Side.SELL, 100.0, 10, "s1"))     # resting ask @ 100
    trades = engine.submit_order(limit(Side.BUY, 105.0, 10, "b1"))  # willing to pay 105

    assert len(trades) == 1
    assert trades[0].price == 100.0   # fills at the passive (resting) price, not 105


# ── sweeping multiple levels ────────────────────────────────────────

def test_sweep_multiple_levels():
    engine = MatchingEngine()
    engine.submit_order(limit(Side.SELL, 10.00, 200, "s1"))
    s2 = limit(Side.SELL, 10.01, 400, "s2")
    engine.submit_order(s2)

    buy = limit(Side.BUY, 10.01, 500, "b1")   # sweeps both levels
    trades = engine.submit_order(buy)

    assert len(trades) == 2
    assert trades[0].quantity == 200 and trades[0].price == 10.00
    assert trades[1].quantity == 300 and trades[1].price == 10.01
    assert buy.status == OrderStatus.FILLED            # all 500 filled
    assert s2.status == OrderStatus.PARTIALLY_FILLED   # 100 left resting
    assert s2.remaining_quantity == 100


# ── market orders (this week) ───────────────────────────────────────

def test_market_buy_fills_at_best_ask():
    engine = MatchingEngine()
    engine.submit_order(limit(Side.SELL, 100.0, 10, "s1"))
    trades = engine.submit_order(market(Side.BUY, 5, "m1"))
    assert len(trades) == 1
    assert trades[0].quantity == 5
    assert trades[0].price == 100.0


def test_market_order_sweeps_three_levels():
    engine = MatchingEngine()
    engine.submit_order(limit(Side.SELL, 10.01, 200, "s1"))
    engine.submit_order(limit(Side.SELL, 10.02, 500, "s2"))
    engine.submit_order(limit(Side.SELL, 10.03, 300, "s3"))

    buy = market(Side.BUY, 1000, "m1")   # sweeps all three levels
    trades = engine.submit_order(buy)

    assert len(trades) == 3
    assert trades[0].quantity == 200 and trades[0].price == 10.01
    assert trades[1].quantity == 500 and trades[1].price == 10.02
    assert trades[2].quantity == 300 and trades[2].price == 10.03
    assert buy.filled_quantity == 1000
    assert buy.status == OrderStatus.FILLED


def test_market_order_partial_when_insufficient_liquidity():
    engine = MatchingEngine()
    engine.submit_order(limit(Side.SELL, 10.01, 200, "s1"))

    buy = market(Side.BUY, 500, "m1")    # only 200 available
    trades = engine.submit_order(buy)

    assert len(trades) == 1
    assert trades[0].quantity == 200
    assert buy.filled_quantity == 200
    assert buy.status == OrderStatus.PARTIALLY_FILLED   # market orders do NOT rest


# ── VWAP (this week) ────────────────────────────────────────────────

def test_vwap_across_sweep():
    from order_engine.matching_engine import vwap
    engine = MatchingEngine()
    engine.submit_order(limit(Side.SELL, 10.01, 200, "s1"))
    engine.submit_order(limit(Side.SELL, 10.02, 500, "s2"))
    engine.submit_order(limit(Side.SELL, 10.03, 300, "s3"))
    trades = engine.submit_order(market(Side.BUY, 1000, "m1"))

    # notional = 200*10.01 + 500*10.02 + 300*10.03 = 10021; volume = 1000
    assert vwap(trades) == pytest.approx(10.021)


def test_vwap_empty_trades_is_zero():
    from order_engine.matching_engine import vwap
    assert vwap([]) == 0.0
