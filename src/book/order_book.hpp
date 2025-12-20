#pragma once
#include "types.hpp"
#include <map>
#include <vector>
#include <functional>
#include <stdexcept>

/**
 * Limit Order Book with Price-Time Priority Matching
 * 
 * This order book implements a continuous double auction with the following characteristics:
 * 
 * MATCHING ALGORITHM:
 * - Price-Time Priority: Orders are matched first by best price, then by time (FIFO within price level)
 * - Execution Price: Always uses the resting order's price (maker price), not the incoming order's price
 * - Immediate Matching: Incoming orders that cross the spread are matched immediately before being added to the book
 * 
 * DATA STRUCTURES:
 * - Buy Orders: std::map with std::greater<Price> for descending price order (best bid first)
 * - Sell Orders: std::map with std::less<Price> for ascending price order (best ask first)
 * - Within each price level: std::vector maintains FIFO order (front = earliest order)
 * - Order Index: std::map for O(1) order lookup by order_id (for cancellations/modifications)
 * 
 * SIMULATION FEATURES:
 * - Timestamping: current_time tracks simulation clock
 * - Snapshots: Capture full order book state at any time
 * - Market Data: Level 1 (top of book) and Level 2 (depth) data available
 * - Trade Logging: Every trade is logged with both order IDs and execution details
 * - Order Logging: All order events (placed, filled, canceled, modified) are logged
 */
class OrderBook {
    private:
        // FIFO queues at each price level (Price-Time Priority)
        // Buy orders: higher prices first (std::greater), then FIFO within price level
        std::map<Price, std::vector<Order>, std::greater<Price>> buy_orders;
        // Sell orders: lower prices first (std::less by default), then FIFO within price level
        std::map<Price, std::vector<Order>> sell_orders;
        
        // Fast order lookup for cancellations/modifications - O(1) access
        std::map<OrderID, std::pair<Price, OrderSide>> order_index;

        Price get_best_bid() const;
        Price get_best_ask() const;
        bool is_match_possible() const;
        Quantity get_total_quantity(OrderSide side) const;
    
    public:
        std::vector<OrderLog> order_logs;
        std::vector<Trade> trade_logs;
        TradeID next_trade_id = 1;
        Timestamp current_time = 0;  // Simulation clock

        OrderBook() = default;
        virtual ~OrderBook() = default;

        // Order management
        void place_limit_order(const Order& order);
        void place_market_order(const Order& order);
        void cancel_order(OrderID order_id);
        void modify_order(OrderID order_id, Price new_price, Quantity new_quantity);

        // Market data queries
        double get_spread() const;
        Price get_mid_price() const;
        OrderBookSnapshot get_snapshot(Timestamp timestamp) const;
        Level1Data get_level1_data() const;
        
        // Order book depth at specific price levels
        Quantity get_depth_at_price(Price price, OrderSide side) const;
        std::vector<PriceLevel> get_bid_levels(size_t depth = 10) const;
        std::vector<PriceLevel> get_ask_levels(size_t depth = 10) const;
        
        // Time management for simulations
        void advance_time(Timestamp new_time) { current_time = new_time; }
        Timestamp get_current_time() const { return current_time; }

        void print_order_book() const;
        void print_order_logs() const;
        void print_trade_logs() const;

        void clear() {
            buy_orders.clear();
            sell_orders.clear();
            order_index.clear();
            order_logs.clear();
            trade_logs.clear();
            next_trade_id = 1;
        }

        void invariant_check() const {
            // check every order for negative quantity or price
            for (const auto& [price, orders] : buy_orders) {
                for (const auto& order : orders) {
                    if (order.quantity == 0 || order.price <= 0.0) {
                        throw std::runtime_error("Invariant violation: Invalid buy order");
                    }
                }
            }
            for (const auto& [price, orders] : sell_orders) {
                for (const auto& order : orders) {
                    if (order.quantity == 0 || order.price <= 0.0) {
                        throw std::runtime_error("Invariant violation: Invalid sell order");
                    }
                }
            }

            // check that best bid is less than best ask (no crossing)
            // If best_bid >= best_ask, orders should have matched
            if (!buy_orders.empty() && !sell_orders.empty()) {
                if (get_best_bid() >= get_best_ask()) {
                    throw std::runtime_error("Invariant violation: Best bid >= best ask (orders should have matched)");
                }
            }
        }

};