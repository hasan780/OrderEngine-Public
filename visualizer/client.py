"""
Protocol client for the order engine — a thin, engine-agnostic socket client
that speaks the shared wire protocol (see ../PROTOCOL.md).

The dataclasses and the low-level socket plumbing (_readline, _request) are
DONE for you. Your task is the five protocol methods marked "TODO [this week]":
submit, cancel, book, query, symbols. Each sends one request line and parses
the reply — the exact formats are in PROTOCOL.md.

The Streamlit UI (app.py) is written entirely against this interface, so once
these methods work, the visualizer works against the C++ gateway on port 9001.

Standard library only — easy to test against a running gateway.
"""

from __future__ import annotations

import socket
from dataclasses import dataclass, field
from typing import Callable, List, Optional, Tuple


@dataclass
class Trade:
    trade_id: str
    symbol: str
    price: float
    qty: int
    buy_id: str
    sell_id: str


@dataclass
class SubmitResult:
    order_id: str
    status: str            # FILLED | PARTIAL | RESTING | CANCELLED | REJECTED | UNKNOWN
    filled: int
    remaining: int
    trades: List[Trade] = field(default_factory=list)

    def vwap(self) -> Optional[float]:
        vol = sum(t.qty for t in self.trades)
        if vol == 0:
            return None
        return sum(t.price * t.qty for t in self.trades) / vol


Level = Tuple[float, int]   # (price, total_qty)


@dataclass
class BookSnapshot:
    bids: List[Level]      # highest price first
    asks: List[Level]      # lowest price first


class EngineClient:
    """Line-buffered protocol client over one TCP connection."""

    def __init__(self, host: str = "127.0.0.1", port: int = 9001, timeout: float = 3.0):
        self.host, self.port = host, port
        self._sock = socket.create_connection((host, port), timeout=timeout)
        self._buf = ""

    # -- low level (done for you) --------------------------------------

    def _readline(self) -> str:
        while "\n" not in self._buf:
            data = self._sock.recv(4096)
            if not data:
                raise ConnectionError("gateway closed the connection")
            self._buf += data.decode()
        line, self._buf = self._buf.split("\n", 1)
        return line.strip()

    def _request(self, line: str, done: Callable[[str], bool]) -> List[str]:
        """Send one request line; read reply lines until `done(line)` is True."""
        self._sock.sendall((line + "\n").encode())
        out: List[str] = []
        while True:
            resp = self._readline()
            out.append(resp)
            if done(resp):
                return out

    def close(self) -> None:
        try:
            self._sock.close()
        except OSError:
            pass

    # -- protocol commands — YOUR TASK ---------------------------------

    #def submit(self, symbol: str, side: str, otype: str,
     #          qty: int, price: Optional[float] = None) -> SubmitResult:
        # TODO [this week]: send "NEW <symbol> <side> <otype> <qty> [price]"
        #   - price is included only for LIMIT
        #   - read until a line starts with "STATUS" or "ERR"
        #   - parse into a SubmitResult (see _parse_submit hint below)
        raise NotImplementedError

   #def cancel(self, order_id: str) -> SubmitResult:
        # TODO [this week]: send "CANCEL <order_id>", read until STATUS/ERR, parse
     #   raise NotImplementedError

   # def query(self, order_id: str) -> SubmitResult:
        # TODO [this week]: send "QUERY <order_id>", read until STATUS/ERR, parse
    #    raise NotImplementedError

   # def book(self, symbol: str) -> BookSnapshot:
        # TODO [this week]: send "BOOK <symbol>", read until a line == "END"
        #   - collect BID/ASK lines into (price, qty) tuples
        #   - return BookSnapshot(bids=..., asks=...)
     #   raise NotImplementedError

   # def symbols(self) -> List[str]:
        # TODO [this week]: send "SYMBOLS", read until "END"
        #   - return the <name> from each "SYM <name>" line
    #    raise NotImplementedError


    #AI Generated UI
    def submit(self, symbol: str, side: str, otype: str,
            qty: int, price: Optional[float] = None) -> SubmitResult:
        line = f"NEW {symbol} {side} {otype} {qty}"
        if price is not None:
            line += f" {price}"
        lines = self._request(line, done=lambda l: l.startswith("STATUS") or l.startswith("ERR"))
        return self._parse_submit(lines)

    def cancel(self, order_id: str) -> SubmitResult:
        line = f"CANCEL {order_id}"
        lines = self._request(line, done=lambda l: l.startswith("STATUS") or l.startswith("ERR"))
        return self._parse_submit(lines)

    def query(self, order_id: str) -> SubmitResult:
        line = f"QUERY {order_id}"
        lines = self._request(line, done=lambda l: l.startswith("STATUS") or l.startswith("ERR"))
        return self._parse_submit(lines)

    def book(self, symbol: str) -> BookSnapshot:
        line = f"BOOK {symbol}"
        lines = self._request(line, done=lambda l: l == "END")
        bids: List[Level] = []
        asks: List[Level] = []
        for r in lines:
            f = r.split()
            if not f:
                continue
            if f[0] == "BID":
                bids.append((float(f[1]), int(f[2])))
            elif f[0] == "ASK":
                asks.append((float(f[1]), int(f[2])))
        return BookSnapshot(bids=bids, asks=asks)

    def symbols(self) -> List[str]:
        lines = self._request("SYMBOLS", done=lambda l: l == "END")
        out: List[str] = []
        for r in lines:
            f = r.split()
            if f and f[0] == "SYM":
                out.append(f[1])
        return out
    #
    # -- parsing hint ---------------------------------------------------
    # A helper you'll likely want for submit/cancel/query. Turns the reply
    # lines (zero or more TRADE lines + one STATUS line) into a SubmitResult.
    @staticmethod
    def _parse_submit(lines: List[str]) -> SubmitResult:
        trades: List[Trade] = []
        oid, status, filled, remaining = "?", "ERR", 0, 0
        for r in lines:
            f = r.split()
            if not f:
                continue
            if f[0] == "TRADE":                 # TRADE <id> <sym> <price> <qty> <buy> <sell>
                trades.append(Trade(f[1], f[2], float(f[3]), int(f[4]), f[5], f[6]))
            elif f[0] == "STATUS":              # STATUS <id> <status> <filled> <remaining>
                oid, status, filled, remaining = f[1], f[2], int(f[3]), int(f[4])
            elif f[0] == "ERR":
                status = "ERR: " + " ".join(f[1:])
        return SubmitResult(order_id=oid, status=status, filled=filled,
                            remaining=remaining, trades=trades)
