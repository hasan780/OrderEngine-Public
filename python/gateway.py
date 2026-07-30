"""
Python order gateway — exposes your MatchingEngine over TCP using the shared
wire protocol (see PROTOCOL.md at the repo root).

The socket plumbing below is DONE for you. Your task this week is the three
protocol handlers marked "TODO [this week]": handle_new, handle_cancel,
handle_book. Each one maps a protocol line to engine calls and formats the
reply — the exact formats are in PROTOCOL.md.

Run it:
    cd python
    python gateway.py            # listens on 127.0.0.1:9000
Connect from another terminal:
    nc 127.0.0.1 9000
"""

import socket

from order_engine.matching_engine import MatchingEngine
from order_engine.models import Order, Side, OrderType, OrderStatus

HOST, PORT = "127.0.0.1", 9000


def _fmt(price: float) -> str:
    return f"{price:g}"   # 10.01 -> "10.01", 10.0 -> "10"


class Gateway:
    """Translates protocol lines <-> engine calls. One engine, shared state."""

    def __init__(self):
        self.engine = MatchingEngine()
        self._seq = 0

    def _new_id(self) -> str:
        self._seq += 1
        return f"o-{self._seq}"

    def handle(self, line: str) -> list[str]:
        """Dispatch one request line to the right handler. (Done for you.)"""
        parts = line.split()
        if not parts:
            return ["ERR empty"]
        cmd = parts[0].upper()
        if cmd == "NEW":
            return self.handle_new(parts)
        if cmd == "CANCEL":
            return self.handle_cancel(parts)
        if cmd == "BOOK":
            return self.handle_book(parts)
        return [f"ERR unknown command {cmd}"]

    # ------------------------------------------------------------------
    # YOUR TASK THIS WEEK — implement the three handlers below.
    # ------------------------------------------------------------------

    def handle_new(self, parts) -> list[str]:
        # TODO [this week]: NEW <symbol> <BUY|SELL> <LIMIT|MARKET> <qty> [price]
        #   1. Parse the fields (price is present only for LIMIT).
        #   2. Build an Order using self._new_id() as its order_id.
        #   3. trades = self.engine.submit_order(order)
        #      (wrap in try/except ValueError -> STATUS <id> REJECTED 0 <qty>)
        #   4. Return one line per trade, then one STATUS line:
        #        TRADE <trade_id> <symbol> <price> <qty> <buy_id> <sell_id>
        #        STATUS <order_id> <FILLED|PARTIAL|RESTING> <filled> <remaining>
        #      Use _fmt(price) for prices. See PROTOCOL.md for exact formats.
        return ["ERR NEW not implemented yet"]

    def handle_cancel(self, parts) -> list[str]:
        # TODO [this week]: CANCEL <order_id>
        #   - order = self.engine.cancel_order(order_id)
        #   - if None -> STATUS <id> REJECTED 0 0
        #   - else    -> STATUS <id> CANCELLED <filled_quantity> <remaining_quantity>
        return ["ERR CANCEL not implemented yet"]

    def handle_book(self, parts) -> list[str]:
        # TODO [this week]: BOOK <symbol>
        #   - Get the OrderBook (self.engine.get_order_book(symbol)).
        #   - Emit one line per level, highest bids first then lowest asks:
        #        BID <price> <total_qty>
        #        ASK <price> <total_qty>
        #        END
        #   - You'll need a way to list ALL levels with their total quantity.
        #     Add a snapshot()/helper to OrderBook if you don't have one yet.
        return ["END"]


def serve(host: str = HOST, port: int = PORT) -> None:
    """TCP accept/receive loop. Done for you — no need to change this."""
    gw = Gateway()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((host, port))
        srv.listen()
        print(f"python gateway listening on {host}:{port}  (Ctrl-C to stop)", flush=True)
        while True:
            conn, addr = srv.accept()
            print(f"client connected: {addr}", flush=True)
            with conn:
                buffer = ""
                while True:
                    data = conn.recv(1024)
                    if not data:
                        print("client disconnected", flush=True)
                        break
                    buffer += data.decode()
                    while "\n" in buffer:
                        line, buffer = buffer.split("\n", 1)
                        for out in gw.handle(line.strip()):
                            conn.sendall((out + "\n").encode())


if __name__ == "__main__":
    try:
        serve()
    except KeyboardInterrupt:
        print("\nshutting down")
