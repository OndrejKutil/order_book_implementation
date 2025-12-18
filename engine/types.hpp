#pragma once
#include <string>
#include <cstdint>

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

struct Order {
    OrderID order_id;
    TraderID trader_id;
    Price price;
    Quantity quantity;
    OrderSide side;
    OrderType type;
    uint64_t timestamp; // Unix timestamp in milliseconds
};

struct Snapshot {
    Price best_bid;
    Price best_ask;
    Quantity total_bid_quantity;
    Quantity total_ask_quantity;
    uint64_t timestamp; // Unix timestamp in milliseconds
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
    uint64_t timestamp; // Unix timestamp in milliseconds
    std::string details; // Additional details about the order event
};