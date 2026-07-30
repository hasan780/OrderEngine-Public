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

    std::vector<std::string> handle_new(const std::vector<std::string>& t) {
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
        (void)t;
        return {"ERR NEW not implemented yet"};
    }

    std::vector<std::string> handle_cancel(const std::vector<std::string>& t) {
        // TODO [this week]: CANCEL <order_id>
        //   - bool ok = engine_.cancel_order(order_id);
        //   - if (!ok) -> STATUS <id> REJECTED 0 0
        //   - else     -> STATUS <id> CANCELLED <filled> <remaining>
        //     (you'll need a way to look the order up to read filled/remaining)
        (void)t;
        return {"ERR CANCEL not implemented yet"};
    }

    std::vector<std::string> handle_book(const std::vector<std::string>& t) {
        // TODO [this week]: BOOK <symbol>
        //   - OrderBook* book = engine_.get_order_book(symbol);
        //   - Emit one line per level, highest bids first then lowest asks:
        //        BID <price> <total_qty>
        //        ASK <price> <total_qty>
        //        END
        //   - You'll need a way to list ALL levels with their total quantity.
        //     Add a snapshot helper to OrderBook if you don't have one yet.
        (void)t;
        return {"END"};
    }

    std::vector<std::string> handle_query(const std::vector<std::string>& t) {
        // TODO [this week]: QUERY <order_id>
        //   - Look the order up (add a find_order(id) helper to the engine).
        //   - If unknown -> STATUS <id> UNKNOWN 0 0
        //   - Else map its live status to a protocol word and reply:
        //        STATUS <id> <FILLED|PARTIAL|RESTING|CANCELLED> <filled> <remaining>
        (void)t;
        return {"ERR QUERY not implemented yet"};
    }

    std::vector<std::string> handle_symbols(const std::vector<std::string>& t) {
        // TODO [this week]: SYMBOLS
        //   - List the symbols the engine has a book for (add symbols()).
        //   - Emit one "SYM <name>" line each, then "END".
        (void)t;
        return {"END"};
    }
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
