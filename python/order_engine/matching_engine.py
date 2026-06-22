from typing import Dict, List, Optional
from .models import Order, Trade
from .order_book import OrderBook


class MatchingEngine:
    # TODO [later]: decide what state the engine holds.
    #   - books:  symbol -> OrderBook
    #   - orders: order_id -> Order
    #   - trades: list of all executed trades
    # Define them in __init__.

    def submit_order(self, order: Order) -> List[Trade]:
        # TODO [later]:
        #   - get or create the OrderBook for order.symbol
        #   - store order in self.orders
        #   - if LIMIT  -> call _match_limit_order
        #   - if MARKET -> call _match_market_order
        #   - return the list of trades generated
        pass

    def cancel_order(self, order_id: str) -> Optional[Order]:
        # TODO [later]:
        #   - look up order by order_id
        #   - if not found or already filled/cancelled, return None
        #   - remove from the order book (read side/price off the order)
        #   - set order.status = CANCELLED
        #   - return the order
        pass

    def get_order_book(self, symbol: str) -> Optional[OrderBook]:
        # TODO [later]: return the OrderBook for symbol, or None
        pass

    def get_trade_history(self) -> List[Trade]:
        # TODO [later]: return a copy of all trades
        pass

    def _match_limit_order(self, order: Order, book: OrderBook) -> List[Trade]:
        # TODO [later]:
        #   - if BUY:  while best_ask exists and best_ask.price <= order.price -> fill
        #   - if SELL: while best_bid exists and best_bid.price >= order.price -> fill
        #   - for each match: create trade, update filled_quantity and status on both orders
        #   - if order still has remaining quantity, add it to the book
        #   - return trades
        pass

    def _match_market_order(self, order: Order, book: OrderBook) -> List[Trade]:
        # TODO [later]:
        #   - if BUY:  consume asks from lowest price up until filled or book empty
        #   - if SELL: consume bids from highest price down until filled or book empty
        #   - create trades as quantity is consumed
        #   - return trades
        pass

    def _create_trade(self, buy: Order, sell: Order, price: float, quantity: int) -> Trade:
        # TODO [later]:
        #   - buy.filled_quantity  += quantity
        #   - sell.filled_quantity += quantity
        #   - set status = FILLED if fully filled, else PARTIALLY_FILLED
        #   - build and return the Trade object
        pass
