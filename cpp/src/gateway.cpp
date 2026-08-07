// C++ order gateway — exposes your MatchingEngine over TCP using the shared
// wire protocol (see PROTOCOL.md at the repo root).
//
// The socket plumbing (main) and the dispatch (Gateway::handle) are DONE for
// you. Your task this week is the three handlers marked "TODO [this week]":
// handle_new, handle_cancel, handle_book. Each maps a protocol line to engine
// calls and formats the reply — exact formats are in PROTOCOL.md.
//
// Build (CMake target `gateway`):  cmake --build build
// Run:                             ./build/gateway     (listens on 127.0.0.1:9001)
// Connect:                         nc 127.0.0.1 9001

#include "matching_engine.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace order_engine;

namespace {

constexpr const char* HOST = "127.0.0.1";
constexpr int PORT = 9001;

// Format a price the same way on both tracks: 10.01 -> "10.01", 10.0 -> "10".
std::string fmt(double p) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%g", p);
    return buf;
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> out;
    std::istringstream iss(line);
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

class Gateway {
public:
    // Dispatch one request line to the right handler. (Done for you.)
    std::vector<std::string> handle(const std::string& line) {
        auto t = tokenize(line);
        if (t.empty()) return {"ERR empty"};
        std::string cmd = t[0];
        for (char& c : cmd) c = std::toupper(c);
        if (cmd == "NEW")    return handle_new(t);
        if (cmd == "CANCEL") return handle_cancel(t);
        if (cmd == "BOOK")   return handle_book(t);
        if (cmd == "QUERY")  return handle_query(t);
        if (cmd == "SYMBOLS") return handle_symbols(t);
        return {"ERR unknown command " + cmd};
    }

private:
    MatchingEngine engine_;
    uint64_t seq_ = 0;
    std::string new_id() { return "o-" + std::to_string(++seq_); }

    // ------------------------------------------------------------------
    // YOUR TASK THIS WEEK — implement the handlers below.
    // ------------------------------------------------------------------

    //std::vector<std::string> handle_new(const std::vector<std::string>& t) {
        // TODO [this week]: NEW <symbol> <BUY|SELL> <LIMIT|MARKET> <qty> [price]
        //   1. Parse the fields (price is present only for LIMIT).
        //   2. Build a std::make_unique<Order>() using new_id() as order_id.
        //   3. auto trades = engine_.submit_order(std::move(order));
        //      (wrap in try/catch -> STATUS <id> REJECTED 0 <qty>)
        //   4. Return one line per trade, then one STATUS line:
        //        TRADE <trade_id> <symbol> <price> <qty> <buy_id> <sell_id>
        //        STATUS <order_id> <FILLED|PARTIAL|RESTING> <filled> <remaining>
        //      Use fmt(price) for prices. See PROTOCOL.md for exact formats.
        //      (Keep a raw Order* before the move so you can read its status.)
        //(void)t;
    //    return {"ERR NEW not implemented yet"};
    //}

    //AI generated handle_new
    std::vector<std::string> handle_new(const std::vector<std::string>& t) {
    // NEW <symbol> <BUY|SELL> <LIMIT|MARKET> <qty> [price]
    // Minimum shape: NEW symbol side type qty  →  5 tokens including "NEW" itself.
    if (t.size() < 5) return {"ERR NEW requires symbol side type qty [price]"};

    std::string symbol = t[1];

    std::string side_str = t[2];
    for (char& c : side_str) c = std::toupper(c);
    Side side;
    if (side_str == "BUY") side = Side::BUY;
    else if (side_str == "SELL") side = Side::SELL;
    else return {"ERR bad side " + t[2]};

    std::string type_str = t[3];
    for (char& c : type_str) c = std::toupper(c);
    OrderType type;
    if (type_str == "LIMIT") type = OrderType::LIMIT;
    else if (type_str == "MARKET") type = OrderType::MARKET;
    else return {"ERR bad order type " + t[3]};

    int64_t qty;
    try {
        qty = std::stoll(t[4]);
    } catch (...) {
        return {"ERR bad qty " + t[4]};
    }

    // Price is required for LIMIT, must be absent for MARKET.
    std::optional<double> price = std::nullopt;
    if (type == OrderType::LIMIT) {
        if (t.size() < 6) return {"ERR LIMIT requires a price"};
        try {
            price = std::stod(t[5]);
        } catch (...) {
            return {"ERR bad price " + t[5]};
        }
    }

    auto order = std::make_unique<Order>();
    order->order_id   = new_id();
    order->symbol     = symbol;
    order->side       = side;
    order->order_type = type;
    order->quantity   = qty;
    order->price      = price;

    // Grab the raw pointer + id BEFORE moving — same pattern you used in
    // create_trade calls: once submit_order takes ownership via std::move,
    // `order` itself is empty, but `raw` still points at the live object.
    Order* raw = order.get();
    std::string id = raw->order_id;

    std::vector<Trade> trades;
    try {
        trades = engine_.submit_order(std::move(order));
    } catch (...) {
        return {"STATUS " + id + " REJECTED 0 " + std::to_string(qty)};
    }

    // Build one TRADE line per fill, in the order they were matched.
    std::vector<std::string> out;
    for (const Trade& tr : trades) {
        out.push_back("TRADE " + tr.trade_id + " " + tr.symbol + " " +
                       fmt(tr.price) + " " + std::to_string(tr.quantity) + " " +
                       tr.buy_order_id + " " + tr.sell_order_id);
    }

    // Map your internal OrderStatus enum to the protocol's status words.
    std::string status_word;
    switch (raw->status) {
        case OrderStatus::FILLED:           status_word = "FILLED";  break;
        case OrderStatus::PARTIALLY_FILLED: status_word = "PARTIAL"; break;
        default:                            status_word = "RESTING"; break;  // OPEN
    }
    out.push_back("STATUS " + id + " " + status_word + " " +
                   std::to_string(raw->filled_quantity) + " " +
                   std::to_string(raw->remaining_quantity()));
    return out;
    }
    //

    //std::vector<std::string> handle_cancel(const std::vector<std::string>& t) {
        // TODO [this week]: CANCEL <order_id>
        //   - bool ok = engine_.cancel_order(order_id);
        //   - if (!ok) -> STATUS <id> REJECTED 0 0
        //   - else     -> STATUS <id> CANCELLED <filled> <remaining>
        //     (you'll need a way to look the order up to read filled/remaining)
        //(void)t;
        //return {"ERR CANCEL not implemented yet"};
    //}

    //AI Generated handle_cancel
    std::vector<std::string> handle_cancel(const std::vector<std::string>& t) {
    // CANCEL <order_id>
    if (t.size() < 2) return {"ERR CANCEL requires order_id"};
    std::string id = t[1];

    // Look up the order BEFORE canceling, so we can still read its
    // filled/remaining quantities after cancel_order flips its status.
    Order* order = engine_.find_order(id);
    if (order == nullptr) {
        return {"STATUS " + id + " REJECTED 0 0"};
    }

    bool ok = engine_.cancel_order(id);
    if (!ok) {
        // Found the order, but it's already FILLED/CANCELLED — can't cancel again.
        return {"STATUS " + id + " REJECTED " +
                std::to_string(order->filled_quantity) + " " +
                std::to_string(order->remaining_quantity())};
    }

    return {"STATUS " + id + " CANCELLED " +
            std::to_string(order->filled_quantity) + " " +
            std::to_string(order->remaining_quantity())};
    }
    //

    //std::vector<std::string> handle_book(const std::vector<std::string>& t) {
        // TODO [this week]: BOOK <symbol>
        //   - OrderBook* book = engine_.get_order_book(symbol);
        //   - Emit one line per level, highest bids first then lowest asks:
        //        BID <price> <total_qty>
        //        ASK <price> <total_qty>
        //        END
        //   - You'll need a way to list ALL levels with their total quantity.
        //     Add a snapshot helper to OrderBook if you don't have one yet.
        //(void)t;
        //return {"END"};
    //}

    //AI Generated handle_book
    std::vector<std::string> handle_book(const std::vector<std::string>& t) {
    // BOOK <symbol>
    if (t.size() < 2) return {"ERR BOOK requires symbol"};
    std::string symbol = t[1];

    std::vector<std::string> out;
    OrderBook* book = engine_.get_order_book(symbol);
    if (book != nullptr) {
        // bids_ is already sorted highest-first (std::greater<double> comparator),
        // so a plain forward iteration is already in the right order.
        for (const auto& [price, level] : book->bids()) {
            out.push_back("BID " + fmt(price) + " " + std::to_string(level.total_quantity()));
        }
        // asks_ is sorted lowest-first by default (std::map's natural ordering),
        // also already correct with a plain forward iteration.
        for (const auto& [price, level] : book->asks()) {
            out.push_back("ASK " + fmt(price) + " " + std::to_string(level.total_quantity()));
        }
    }
    // If get_order_book returns nullptr (symbol never traded), we just fall
    // through with an empty `out` — matching PROTOCOL.md's "a completely
    // empty book is just: END".
    out.push_back("END");
    return out;
    }
    //

    //std::vector<std::string> handle_query(const std::vector<std::string>& t) {
        // TODO [this week]: QUERY <order_id>
        //   - Look the order up (add a find_order(id) helper to the engine).
        //   - If unknown -> STATUS <id> UNKNOWN 0 0
        //   - Else map its live status to a protocol word and reply:
        //        STATUS <id> <FILLED|PARTIAL|RESTING|CANCELLED> <filled> <remaining>
        //(void)t;
        //return {"ERR QUERY not implemented yet"};
    //}

    //AI Generated handle_query
    std::vector<std::string> handle_query(const std::vector<std::string>& t) {
    // QUERY <order_id>
    if (t.size() < 2) return {"ERR QUERY requires order_id"};
    std::string id = t[1];

    Order* order = engine_.find_order(id);
    if (order == nullptr) {
        return {"STATUS " + id + " UNKNOWN 0 0"};
    }

    std::string status_word;
    switch (order->status) {
        case OrderStatus::FILLED:            status_word = "FILLED";    break;
        case OrderStatus::PARTIALLY_FILLED:  status_word = "PARTIAL";   break;
        case OrderStatus::CANCELLED:         status_word = "CANCELLED"; break;
        default:                             status_word = "RESTING";   break;  // OPEN
    }

    return {"STATUS " + id + " " + status_word + " " +
            std::to_string(order->filled_quantity) + " " +
            std::to_string(order->remaining_quantity())};
    }
    //

    //std::vector<std::string> handle_symbols(const std::vector<std::string>& t) {
        // TODO [this week]: SYMBOLS
        //   - List the symbols the engine has a book for (add symbols()).
        //   - Emit one "SYM <name>" line each, then "END".
        //(void)t;
        //return {"END"};
    //}

    //AI Generated handle_symbols
    std::vector<std::string> handle_symbols(const std::vector<std::string>& t) {
    // SYMBOLS
    (void)t;  // no arguments to this command
    std::vector<std::string> out;
    for (const std::string& sym : engine_.symbols()) {
        out.push_back("SYM " + sym);
    }
    out.push_back("END");
    return out;
    }
    //
};

}  // namespace

int main() {
    Gateway gw;

    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    ::inet_pton(AF_INET, HOST, &addr.sin_addr);

    if (::bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::perror("bind");
        return 1;
    }
    ::listen(srv, 1);
    std::cout << "c++ gateway listening on " << HOST << ":" << PORT
              << "  (Ctrl-C to stop)" << std::endl;

    while (true) {
        int conn = ::accept(srv, nullptr, nullptr);
        if (conn < 0) continue;
        std::cout << "client connected" << std::endl;

        std::string buffer;
        char chunk[1024];
        while (true) {
            ssize_t n = ::recv(conn, chunk, sizeof(chunk), 0);
            if (n <= 0) { std::cout << "client disconnected" << std::endl; break; }
            buffer.append(chunk, n);
            //std::cerr << "[gateway] buffer now: " << buffer << std::endl;
            std::size_t pos;
            while ((pos = buffer.find('\n')) != std::string::npos) {
                std::string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 1);
                for (const std::string& out : gw.handle(line)) {
                    std::string msg = out + "\n";
                    ::send(conn, msg.data(), msg.size(), 0);
                }
            }
        }
        ::close(conn);
    }
    return 0;
}
