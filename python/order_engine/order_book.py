from typing import Optional
from .models import Order, Side


class PriceLevel:
    def add(self, order: Order) -> None:
        # append order to end of list
        pass

    def remove(self, order_id: str) -> Optional[Order]:
        # find order by id, remove and return it
        # return None if not found
        pass

    def total_quantity(self) -> int:
        # sum remaining_quantity across all orders
        pass


class OrderBook:
    def add_order(self, order: Order) -> None:
        # get or create PriceLevel at order.price on the correct side
        # append order to that level
        pass

    def cancel_order(self, order_id: str, side: Side, price: float) -> Optional[Order]:
        # find the PriceLevel at (side, price)
        # remove order from level
        # if level is now empty, delete it
        # return removed order or None
        pass

    def best_bid(self) -> Optional[PriceLevel]:
        # return the PriceLevel with the highest bid price
        # return None if no bids
        pass

    def best_ask(self) -> Optional[PriceLevel]:
        # return the PriceLevel with the lowest ask price
        # return None if no asks
        pass

    def spread(self) -> Optional[float]:
        # return best_ask.price - best_bid.price
        # return None if either side is empty
        pass
