"""
Workshop 6 — Race condition & mutex demo.

Two threads both increment a shared "volume" — exactly what OrderBook.add_order
does when it updates the cached total quantity (read -> modify -> write).

Run it a few times:
    python race_condition_demo.py

You'll see the UNSAFE version land below the expected total (lost updates),
and the SAFE version hit it exactly every time.
"""

import threading
import time

ITERS = 20_000      # increments per thread
THREADS = 2
EXPECTED = ITERS * THREADS


class SharedBook:
    """A stand-in for the cached volume inside a PriceLevel / OrderBook."""

    def __init__(self):
        self.volume = 0

    def add_unsafe(self, qty: int) -> None:
        # This mirrors: v = self.volume; v += qty; self.volume = v
        # Those three steps are NOT atomic. The time.sleep(0) forces the thread
        # to yield right in the middle — so the other thread reads the SAME old
        # value, and one of the two updates gets overwritten (lost).
        # (Python's GIL usually hides this; the yield makes it reproducible.)
        tmp = self.volume      # read
        time.sleep(0)          # yield → widen the race window
        self.volume = tmp + qty  # write (based on a possibly-stale read)

    def add_safe(self, qty: int, lock: threading.Lock) -> None:
        # The lock makes the whole read-modify-write one indivisible step.
        with lock:
            tmp = self.volume
            time.sleep(0)
            self.volume = tmp + qty


def run(safe: bool) -> int:
    book = SharedBook()
    lock = threading.Lock()

    def worker():
        for _ in range(ITERS):
            if safe:
                book.add_safe(1, lock)
            else:
                book.add_unsafe(1)

    threads = [threading.Thread(target=worker) for _ in range(THREADS)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    return book.volume


if __name__ == "__main__":
    print(f"Expected total: {EXPECTED}\n")

    print("UNSAFE (no lock) — run it a few times, the number keeps changing:")
    for i in range(3):
        result = run(safe=False)
        flag = "OK" if result == EXPECTED else f"LOST {EXPECTED - result} updates"
        print(f"  run {i + 1}: {result:>7}   {flag}")

    print("\nSAFE (with mutex) — correct every single time:")
    for i in range(3):
        result = run(safe=True)
        flag = "OK" if result == EXPECTED else "WRONG"
        print(f"  run {i + 1}: {result:>7}   {flag}")
