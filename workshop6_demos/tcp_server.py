"""
Workshop 6 — Minimal TCP order gateway (concept demo).

This is the "front door" an exchange puts in front of the matching engine.
It does the classic five socket steps: socket -> bind -> listen -> accept -> recv/send.

Run it in one terminal:
    python tcp_server.py

Then connect from another terminal, either with our client:
    python tcp_client.py
or with netcat:
    nc 127.0.0.1 9000

Protocol (one order per line):
    NEW <SYMBOL> <BUY|SELL> <LIMIT|MARKET> <QTY> [PRICE]
e.g.
    NEW AAPL BUY LIMIT 100 10.01
    NEW AAPL SELL MARKET 50
"""

import socket

HOST, PORT = "127.0.0.1", 9000


def handle(line: str) -> str:
    """Parse one order line and return a reply. (No real engine — just a demo.)

    In a full system this is where you would build an Order and call
    engine.submit_order(order), then format the resulting trades.
    """
    parts = line.split()
    if not parts:
        return "ERR empty"
    if parts[0].upper() != "NEW":
        return f"ECHO {line}"
    try:
        _, symbol, side, otype, qty, *rest = parts
        qty = int(qty)
        price = float(rest[0]) if rest else None
    except (ValueError, IndexError):
        return "ERR bad format — NEW <SYM> <BUY|SELL> <LIMIT|MARKET> <QTY> [PRICE]"

    price_str = f"@ {price}" if price is not None else "(market)"
    return f"OK accepted {side} {otype} {qty} {symbol} {price_str}"


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((HOST, PORT))       # claim the address + port
        srv.listen()                 # start accepting connections
        print(f"gateway listening on {HOST}:{PORT}  (Ctrl-C to stop)", flush=True)

        while True:
            conn, addr = srv.accept()    # block until a client connects
            print(f"client connected: {addr}", flush=True)
            with conn:
                while True:
                    data = conn.recv(1024)      # read bytes from the client
                    if not data:                # empty = client hung up
                        print("client disconnected", flush=True)
                        break
                    for line in data.decode().splitlines():
                        line = line.strip()
                        reply = handle(line)
                        print(f"  recv: {line!r}  ->  {reply}", flush=True)  # show it live
                        conn.sendall((reply + "\n").encode())   # write back


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nshutting down")
