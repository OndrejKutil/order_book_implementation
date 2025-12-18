#pragma once
#include "types.hpp"
#include <map>
#include <vector>
#include <functional>

class OrderBook {
    private:
        std::map<Price, std::vector<Order>, std::greater<Price>> buy_orders;
        std::map<Price, std::vector<Order>> sell_orders;

        Price get_best_bid() const;
        Price get_best_ask() const;
        bool is_match_possible() const;
        Quantity get_total_quantity(OrderSide side) const;
    
    public:
        std::vector<OrderLog> order_logs;

        OrderBook() = default;
        virtual ~OrderBook() = default;

        void place_limit_order(const Order& order);
        void place_market_order(const Order& order);
        void cancel_order(OrderID order_id);

        void match_orders();
        void print_order_book() const;
        void print_order_logs() const;

};