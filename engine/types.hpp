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