from dataclasses import dataclass, field
from enum import Enum
from typing import Optional
import time


class Side(Enum):
    BUY = "BUY"
    SELL = "SELL"


class OrderType(Enum):
    LIMIT = "LIMIT"
    MARKET = "MARKET"


class OrderStatus(Enum):
    OPEN = "OPEN"
    PARTIALLY_FILLED = "PARTIALLY_FILLED"
    FILLED = "FILLED"
    CANCELLED = "CANCELLED"


@dataclass
class Order:
    order_id: str
    symbol: str
    side: Side
    order_type: OrderType
    quantity: int
    price: Optional[float]                      # None for MARKET orders
    timestamp: int = field(default_factory=time.time_ns)
    filled_quantity: int = 0
    status: OrderStatus = OrderStatus.OPEN

    @property
    def remaining_quantity(self) -> int:
        return self.quantity - self.filled_quantity


@dataclass
class Trade:
    trade_id: str
    symbol: str
    buy_order_id: str
    sell_order_id: str
    price: float
    quantity: int
    timestamp: int = field(default_factory=time.time_ns)
