# Order Engine Visualizer

A Streamlit UI that drives **either** engine over the shared
[wire protocol](../PROTOCOL.md). Point it at the Python gateway (port 9000) or
the C++ gateway (port 9001) — the code is identical because the protocol is.

```
  visualizer  ──TCP (protocol)──►  Python gateway  ──►  Python engine
                              └──►  C++ gateway     ──►  C++ engine
```

## Files
- `client.py` — engine-agnostic protocol client (standard library only)
- `app.py` — the Streamlit UI (bid/ask ladder, order form, trade tape)

## Run

1. Start a gateway in another terminal:
   ```bash
   cd python && python gateway.py        # port 9000
   # or
   cd cpp && ./build/gateway             # port 9001
   ```
2. Launch the visualizer:
   ```bash
   cd visualizer
   pip install -r requirements.txt
   streamlit run app.py
   ```
3. In the sidebar, set the host/port (9000 for Python, 9001 for C++) and click
   **Connect**. Submit orders and watch the ladder and trade tape update.

## Demo tip
Run both gateways at once and just change the **Port** in the sidebar to flip
the same UI between the Python and C++ engines — same protocol, two backends.
