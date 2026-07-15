import uuid
from typing import Dict, List, Optional
from .models import Order, Trade, OrderType, OrderStatus, Side
from .order_book import OrderBook


class MatchingEngine:
    # ------------------------------------------------------------------
    # Scaffolding below is already implemented for you. Your job this week
    # is the ONE method marked "TODO [this week]": _match_limit_order.
    # ------------------------------------------------------------------

    def __init__(self):
        self._books: Dict[str, OrderBook] = {}    # symbol -> OrderBook
        self._orders: Dict[str, Order] = {}       # order_id -> Order
        self._trades: List[Trade] = []            # every executed trade

    def submit_order(self, order: Order) -> List[Trade]:
        """Entry point: store the order, then route it to the matcher."""
        book = self._get_or_create_book(order.symbol)
        self._orders[order.order_id] = order

        if order.order_type == OrderType.LIMIT:
            return self._match_limit_order(order, book)
        elif order.order_type == OrderType.MARKET:
            return self._match_market_order(order, book)
        raise ValueError(f"unknown order type: {order.order_type}")

    def cancel_order(self, order_id: str) -> Optional[Order]:
        order = self._orders.get(order_id)
        if order is None or order.status in (OrderStatus.FILLED, OrderStatus.CANCELLED):
            return None
        book = self._books.get(order.symbol)
        if book is not None and order.price is not None:
            book.cancel_order(order_id, order.side, order.price)
        order.status = OrderStatus.CANCELLED
        return order

    def get_order_book(self, symbol: str) -> Optional[OrderBook]:
        return self._books.get(symbol)

    def get_trade_history(self) -> List[Trade]:
        return list(self._trades)

    def _get_or_create_book(self, symbol: str) -> OrderBook:
        if symbol not in self._books:
            self._books[symbol] = OrderBook(symbol)
        return self._books[symbol]

    def _create_trade(self, buy: Order, sell: Order, price: float, quantity: int) -> Trade:
        """Record a fill: update both orders, build + store the Trade.
        Use this from your matching loop — you don't need to change it.
        """
        buy.filled_quantity += quantity
        sell.filled_quantity += quantity
        for o in (buy, sell):
            o.status = (OrderStatus.FILLED if o.remaining_quantity == 0
                        else OrderStatus.PARTIALLY_FILLED)
        trade = Trade(
            trade_id=str(uuid.uuid4()),
            symbol=buy.symbol,
            buy_order_id=buy.order_id,
            sell_order_id=sell.order_id,
            price=price,
            quantity=quantity,
        )
        self._trades.append(trade)
        return trade

    # ------------------------------------------------------------------
    # YOUR TASK THIS WEEK
    # ------------------------------------------------------------------

    def _match_limit_order(self, order: Order, book: OrderBook) -> List[Trade]:
        # TODO [this week]: implement Price-Time Priority matching.
        #
        # Step by step:
        #   trades = []
        #
        #   while order.remaining_quantity > 0:
        #       1. Get the best opposite level:
        #            BUY  -> book.best_ask()
        #            SELL -> book.best_bid()
        #          If it is None -> break (no liquidity).
        #
        #       2. Check the prices cross:
        #            BUY  matches only if level.price <= order.price
        #            SELL matches only if level.price >= order.price
        #          If they don't cross -> break.
        #
        #       3. Walk that level's resting orders, OLDEST FIRST (FIFO).
        #          Take a snapshot: for resting in list(level.orders.values())
        #          For each resting order:
        #            - if order.remaining_quantity == 0: break
        #            - fill_qty = min(order.remaining_quantity, resting.remaining_quantity)
        #            - decide buyer/seller:
        #                BUY  aggressor -> buy = order,  sell = resting
        #                SELL aggressor -> buy = resting, sell = order
        #            - trade = self._create_trade(buy, sell, level.price, fill_qty)
        #              (NOTE: price is the RESTING level's price, the passive side)
        #            - trades.append(trade)
        #            - if resting.remaining_quantity == 0:
        #                  book.cancel_order(resting.order_id, resting.side, resting.price)
        #
        #   4. Whatever is left over rests on the book:
        #        if order.remaining_quantity > 0:
        #            book.add_order(order)
        #
        #   return trades
        pass

    def _match_market_order(self, order: Order, book: OrderBook) -> List[Trade]:
        # TODO [this week]: sweep the book to fill a MARKET order.
        #
        # A market order has NO limit price. It takes whatever liquidity exists,
        # walking deeper into the book (level by level) until it is filled or
        # the book runs out. Unlike a limit order, a market order NEVER rests.
        #
        # Step by step:
        #   trades = []
        #
        #   while order.remaining_quantity > 0:
        #       1. Get the best opposite level:
        #            BUY  -> book.best_ask()
        #            SELL -> book.best_bid()
        #          If it is None -> break (no more liquidity; stop here).
        #
        #       2. NO price-cross check — a market order accepts any price.
        #
        #       3. Walk that level's resting orders, OLDEST FIRST (FIFO).
        #          Take a snapshot: for resting in list(level.orders.values())
        #          For each resting order:
        #            - if order.remaining_quantity == 0: break
        #            - fill_qty = min(order.remaining_quantity, resting.remaining_quantity)
        #            - decide buyer/seller:
        #                BUY  aggressor -> buy = order,  sell = resting
        #                SELL aggressor -> buy = resting, sell = order
        #            - trade = self._create_trade(buy, sell, level.price, fill_qty)
        #            - trades.append(trade)
        #            - if resting.remaining_quantity == 0:
        #                  book.cancel_order(resting.order_id, resting.side, resting.price)
        #
        #   4. Do NOT rest the remainder. If the book ran dry, the order is only
        #      partially filled and that's fine — a market order just stops.
        #
        #   return trades
        pass


def vwap(trades: List[Trade]) -> float:
    """Volume-Weighted Average Price across a list of executed trades.

        VWAP = sum(price_i * quantity_i) / sum(quantity_i)

    When a market order sweeps several price levels, it fills at many different
    prices. VWAP collapses those fills into ONE effective price — the number a
    trader is told they "really" paid for the whole order. Each Trade carries a
    ``.price`` and a ``.quantity``.

    TODO [this week]: implement it.
      1. If ``trades`` is empty, return 0.0 (nothing executed — avoid /0).
      2. notional = sum(t.price * t.quantity for t in trades)   # total money
      3. volume   = sum(t.quantity for t in trades)             # total shares
      4. return notional / volume

    Worked example (a 1,000-share buy sweeping three asks):
        fills    = 200 @ 10.01, 500 @ 10.02, 300 @ 10.03
        notional = 200*10.01 + 500*10.02 + 300*10.03 = 10021.0
        volume   = 200 + 500 + 300 = 1000
        vwap     = 10021.0 / 1000 = 10.021
    """
    # TODO [this week]: implement using the formula in the docstring above
    pass
