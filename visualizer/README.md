# Order Engine Visualizer

A Streamlit UI that drives the C++ matching engine over the
[wire protocol](../PROTOCOL.md) through its TCP gateway on port 9001.

```
  visualizer  ──TCP (protocol)──►  C++ gateway  ──►  C++ engine
```

## Files
- `client.py`: protocol client (standard library only)
- `app.py`: the Streamlit UI (bid/ask ladder, order form, trade tape, execution stats)

## Run

1. Start the gateway in another terminal:
   ```bash
   cd cpp/build && ./gateway             # port 9001
   ```
2. Launch the visualizer:
   ```bash
   cd visualizer
   pip install -r requirements.txt
   streamlit run app.py
   ```
3. In the sidebar, confirm the host/port (127.0.0.1:9001) and click
   **Connect**. Submit orders and watch the ladder and trade tape update.
