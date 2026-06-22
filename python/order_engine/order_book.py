from typing import Optional
from .models import Order, Side


class PriceLevel:
    # TODO [this week]: decide what state a price level holds.
    #   - the price of this level
    #   - the orders at this level, stored FIFO (earliest first)
    # Define them in __init__.

    def add(self, order: Order) -> None:
        # TODO [this week]: append order to the end of this level (FIFO)
        pass

    def remove(self, order_id: str) -> Optional[Order]:
        # TODO [this week]: find order by id, remove and return it
        #   return None if not found
        pass

    def total_quantity(self) -> int:
        # TODO [later]: sum remaining_quantity across all orders
        pass


class OrderBook:
    # TODO [this week]: decide what state the book holds.
    #   - the symbol
    #   - bid levels (keyed by price, highest first)
    #   - ask levels (keyed by price, lowest first)
    # Define them in __init__.

    def add_order(self, order: Order) -> None:
        # TODO [this week]:
        #   - get or create the PriceLevel at order.price on the correct side
        #   - append the order to that level
        pass

    def cancel_order(self, order_id: str, side: Side, price: float) -> Optional[Order]:
        # TODO [this week]:
        #   - find the PriceLevel at (side, price)
        #   - remove the order from that level
        #   - if the level is now empty, delete it
        #   - return the removed order, or None if not found
        pass

    def best_bid(self) -> Optional[PriceLevel]:
        # TODO [later]: return the PriceLevel with the highest bid price (None if no bids)
        pass

    def best_ask(self) -> Optional[PriceLevel]:
        # TODO [later]: return the PriceLevel with the lowest ask price (None if no asks)
        pass

    def spread(self) -> Optional[float]:
        # TODO [later]: return best_ask.price - best_bid.price (None if either side empty)
        pass
