#pragma once
#include <string>
#include <cstdint>
#include <vector>

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET
};

using OrderID = std::uint64_t;
using TraderID = std::uint64_t;
using Price = double;
using Quantity = std::uint32_t;
using Timestamp = std::uint64_t; // Unix timestamp in milliseconds

struct Order {
    OrderID order_id;
    TraderID trader_id;
    Price price;
    Quantity quantity;
    OrderSide side;
    OrderType type;
    Timestamp timestamp; // Unix timestamp in milliseconds
};

enum class OrderStatus {
    PLACED,
    PARTIALLY_FILLED,
    FILLED,
    UNFILLED,
    CANCELED
};


struct OrderLog {
    OrderID order_id;
    TraderID trader_id;
    Price price;
    Quantity quantity;
    OrderSide side;
    OrderType type;
    OrderStatus status;
    Timestamp timestamp; // Unix timestamp in milliseconds
    std::string details; // Additional details about the order event
};

using TradeID = std::uint64_t;

struct Trade {
    TradeID trade_id;
    OrderID buy_order_id;
    OrderID sell_order_id;
    OrderSide aggressor_side;
    TraderID buyer_id;
    TraderID seller_id;
    Price price;
    Quantity quantity;
    Timestamp timestamp; // Unix timestamp in milliseconds
};

// Market data structures for agents/analysis
struct PriceLevel {
    Price price;
    Quantity total_quantity;
    std::uint32_t order_count;
};

struct OrderBookSnapshot {
    Timestamp timestamp;
    std::vector<PriceLevel> bids;  // Sorted descending (best bid first)
    std::vector<PriceLevel> asks;  // Sorted ascending (best ask first)
    Price best_bid;
    Price best_ask;
    Price mid_price;  // (best_bid + best_ask) / 2
    Price spread;
    Quantity total_bid_volume;
    Quantity total_ask_volume;
};

// Level 1 market data (top of book)
struct Level1Data {
    Timestamp timestamp;
    Price bid_price;
    Quantity bid_quantity;
    Price ask_price;
    Quantity ask_quantity;
    Price mid_price;
    Price spread;
};