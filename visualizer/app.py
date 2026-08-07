"""
Order engine visualizer — a Streamlit UI that drives EITHER engine over the
shared wire protocol. Point it at the Python gateway (9000) or the C++ gateway
(9001); the code is identical because the protocol is.
 
Run:
    cd visualizer
    pip install -r requirements.txt
    streamlit run app.py
"""
 
import altair as alt
import pandas as pd
import streamlit as st
 
from client import EngineClient
 
st.set_page_config(page_title="Order Engine", page_icon="📈", layout="wide")
 
# ── session state ───────────────────────────────────────────────────
ss = st.session_state
ss.setdefault("client", None)
ss.setdefault("tape", [])        # list of Trade
ss.setdefault("last", None)      # last SubmitResult
ss.setdefault("orders", {})      # order_id -> {"desc", "status"} (your submissions)
ss.setdefault("symbol", "AAPL")
 
 
# ── sidebar: connection ─────────────────────────────────────────────
with st.sidebar:
    st.header("Connection")
    host = st.text_input("Host", "127.0.0.1")
    port = st.number_input("Port", value=9000, step=1,
                           help="Python gateway = 9000, C++ gateway = 9001")
 
    if ss.client is None:
        if st.button("Connect", type="primary", use_container_width=True):
            try:
                ss.client = EngineClient(host, int(port))
                st.success(f"connected to {host}:{port}")
            except OSError as e:
                st.error(f"could not connect: {e}")
    else:
        st.success("connected")
        if st.button("Disconnect", use_container_width=True):
            ss.client.close()
            ss.client = None
            ss.tape = []
            ss.last = None
            ss.orders = {}
 
    # Symbol picker: dropdown of symbols the engine knows, plus a field to add a
    # brand-new one (a fresh engine has no books yet, so you seed the first).
    if ss.client is not None:
        st.divider()
        try:
            known = sorted(ss.client.symbols())
        except OSError:
            known = []
        new_sym = st.text_input("Add new symbol", "").strip().upper()
        if new_sym:
            ss.symbol = new_sym
        else:
            options = known or [ss.symbol]
            if ss.symbol not in options:
                options = [ss.symbol] + options
            ss.symbol = st.selectbox("Symbol", options,
                                     index=options.index(ss.symbol))
        st.caption(f"trading **{ss.symbol}**")
 
 
st.title("📈 Order Engine")
 
if ss.client is None:
    st.info("Start a gateway, then connect from the sidebar.\n\n"
            "Python:  `cd python && python gateway.py`  (port 9000)\n\n"
            "C++:  `./build/gateway`  (port 9001)")
    st.stop()
 
 
# ── order form ──────────────────────────────────────────────────────
left, right = st.columns([1, 2])
 
with left:
    st.subheader("Submit order")
    # Side/Type live OUTSIDE the form so the price field reacts immediately when
    # you switch to MARKET (widgets inside a form don't rerun until you submit).
    side = st.radio("Side", ["BUY", "SELL"], horizontal=True)
    otype = st.radio("Type", ["LIMIT", "MARKET"], horizontal=True)
    with st.form("order", clear_on_submit=False):
        qty = st.number_input("Quantity", min_value=1, value=100, step=1)
        price = st.number_input("Price", min_value=0.0, value=10.01, step=0.01,
                                format="%.2f", disabled=(otype == "MARKET"))
        submitted = st.form_submit_button("Send", type="primary",
                                          use_container_width=True)
    if submitted:
        try:
            res = ss.client.submit(ss.symbol, side, otype, int(qty),
                                   None if otype == "MARKET" else float(price))
            ss.last = res
            ss.tape.extend(res.trades)
            desc = f"{side} {otype} {int(qty)}" + (f" @ {price:g}" if otype == "LIMIT" else "")
            ss.orders[res.order_id] = {"desc": desc, "status": res.status}
        except OSError as e:
            st.error(f"send failed: {e}")
 
    if ss.last is not None:
        r = ss.last
        # Show the order id explicitly — this is what you cancel with.
        st.success(f"**{r.order_id}** — {r.status}  (filled {r.filled}, remaining {r.remaining})")
        if r.trades:
            vw = r.vwap()
            st.metric("Fills / VWAP", f"{len(r.trades)}",
                      f"{vw:.4f}" if vw is not None else "-")
 
    # Refresh the live status of orders we think are still resting, so filled
    # ones drop off the cancel list (the aggregated book has no ids, so we ask
    # the engine directly with QUERY).
    for oid, o in list(ss.orders.items()):
        if o["status"] in ("RESTING", "PARTIAL") and "LIMIT" in o["desc"]:
            try:
                o["status"] = ss.client.query(oid).status
            except OSError:
                pass
 
    # Cancel: pick from your own resting LIMIT orders (market orders never rest).
    open_ids = [oid for oid, o in ss.orders.items()
                if o["status"] in ("RESTING", "PARTIAL") and "LIMIT" in o["desc"]]
    if open_ids:
        choice = st.selectbox(
            "Cancel a resting order",
            open_ids,
            format_func=lambda oid: f"{oid} — {ss.orders[oid]['desc']}",
        )
        if st.button("Cancel", use_container_width=True):
            try:
                res = ss.client.cancel(choice)
                ss.last = res
                if choice in ss.orders:
                    ss.orders[choice]["status"] = res.status
            except OSError as e:
                st.error(f"cancel failed: {e}")
    else:
        st.caption("no resting orders to cancel")
 
 
# ── the ladder (live) ───────────────────────────────────────────────
# A fragment with run_every re-fetches the book on a timer, so the ladder
# updates even when orders arrive from another client (nc, the C++ gateway).
@st.fragment(run_every=1.0)
def render_book():
    st.subheader(f"Order book — {ss.symbol}")
    try:
        book = ss.client.book(ss.symbol)
    except OSError as e:
        st.error(f"book request failed: {e}")
        return
 
    # asks shown highest-price at top, then bids highest-first below,
    # so the spread sits in the middle like a real ladder.
    rows = ([{"side": "ASK", "price": p, "qty": q} for p, q in reversed(book.asks)] +
            [{"side": "BID", "price": p, "qty": q} for p, q in book.bids])
 
    if rows:
        df = pd.DataFrame(rows)
 
        def _style(row):
            # light tint + explicit dark text so it's readable in light OR dark theme
            if row["side"] == "ASK":
                css = "background-color: #fdecea; color: #b0281a; font-weight: 600"
            else:
                css = "background-color: #e7f6ec; color: #1b7a3d; font-weight: 600"
            return [css] * len(row)
 
        st.dataframe(
            df.style.apply(_style, axis=1).format({"price": "{:g}", "qty": "{:d}"}),
            hide_index=True, use_container_width=True,
        )
 
        best_bid = book.bids[0][0] if book.bids else None
        best_ask = book.asks[0][0] if book.asks else None
        c1, c2, c3 = st.columns(3)
        c1.metric("Best bid", f"{best_bid:g}" if best_bid is not None else "—")
        c2.metric("Best ask", f"{best_ask:g}" if best_ask is not None else "—")
        spread = (best_ask - best_bid) if (best_bid is not None and best_ask is not None) else None
        c3.metric("Spread", f"{spread:g}" if spread is not None else "—")
    else:
        st.caption("book is empty — submit some orders")
 
 
with right:
    render_book()
 
 
# ── trade tape ──────────────────────────────────────────────────────
st.subheader("Trade tape")
if ss.tape:
    tape_df = pd.DataFrame(
        [{"trade": t.trade_id, "price": t.price, "qty": t.qty,
          "buy": t.buy_id, "sell": t.sell_id} for t in reversed(ss.tape)]
    )
    st.dataframe(tape_df, hide_index=True, use_container_width=True, height=240)
else:
    st.caption("no trades yet")
 
 
# ── execution stats (VWAP) ──────────────────────────────────────────
# Session-wide VWAP across every trade in the tape (not just the last order's
# own fills — that's what the "Fills / VWAP" metric above already shows).
# Computed client-side from the same TRADE lines already populating the tape,
# so this needs no changes to the gateway or engine.
st.subheader("Execution stats")
if ss.tape:
    total_qty = sum(t.qty for t in ss.tape)
    total_notional = sum(t.price * t.qty for t in ss.tape)
    session_vwap = total_notional / total_qty if total_qty else None
 
    c1, c2, c3 = st.columns(3)
    c1.metric("Session VWAP", f"{session_vwap:.4f}" if session_vwap is not None else "—")
    c2.metric("Trades", f"{len(ss.tape)}")
    c3.metric("Volume", f"{total_qty:,}")
 
    # Running VWAP after each trade, in the order it occurred — shows how the
    # average execution price moves as an order sweeps through multiple price
    # levels (a big multi-level sweep should visibly bend the line).
    running, cum_qty, cum_notional = [], 0, 0.0
    for t in ss.tape:
        cum_qty += t.qty
        cum_notional += t.price * t.qty
        running.append(cum_notional / cum_qty)
    vwap_df = pd.DataFrame({"trade #": range(1, len(running) + 1),
                            "running VWAP": running})
 
    # st.line_chart auto-scales its y-axis to start near 0, which flattens the
    # line when VWAP only moves within a narrow band (e.g. 10.01-10.05). Using
    # altair directly lets us zoom the y-axis to the actual min/max of the
    # series (with a little padding) so real movement is visible on screen.
    y_min, y_max = min(running), max(running)
    pad = max((y_max - y_min) * 0.15, 0.01)
    chart = (
        alt.Chart(vwap_df)
        .mark_line(point=True)
        .encode(
            x=alt.X("trade #:Q", title="trade #"),
            y=alt.Y("running VWAP:Q", scale=alt.Scale(domain=[y_min - pad, y_max + pad])),
        )
    )
    st.altair_chart(chart, use_container_width=True)
else:
    st.caption("no trades yet — stats appear once orders start filling")
 

