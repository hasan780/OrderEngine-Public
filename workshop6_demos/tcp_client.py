"""
Workshop 6 — Minimal TCP client (concept demo).

Connects to the gateway, sends a few order lines, prints each reply.
Start tcp_server.py first, then run:
    python tcp_client.py
"""

import socket

HOST, PORT = "127.0.0.1", 9000

ORDERS = [
    "NEW AAPL BUY  LIMIT  100 10.01",
    "NEW AAPL SELL LIMIT   50 10.02",
    "NEW AAPL BUY  MARKET  30",
    "PING",   # anything not starting with NEW is echoed back
]


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))                 # open the connection
        for line in ORDERS:
            s.sendall((line + "\n").encode())   # send one order
            reply = s.recv(1024).decode().strip()
            print(f">> {line}")
            print(f"<< {reply}\n")


if __name__ == "__main__":
    main()
