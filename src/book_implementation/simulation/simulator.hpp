#pragma once
#include "../order_book/order_book.hpp"

// =============================================
// Simulator Class Definition
// =============================================


struct PendingOrder {
    OrderID order_id;
    TraderID trader_id;
    Price price;      // For limit orders
    Quantity quantity;
    OrderSide side;
};

struct PendingMarketOrder {
    OrderID order_id;
    TraderID trader_id;
    Quantity quantity;
    OrderSide side;
};

class Simulator {
    private:
        OrderBook order_book;
        Timestamp simulation_time = 0;
        std::map<TraderID, Order> pending_orders;

    public:
        Simulator(Timestamp start_time);

        // Place orders
        void place_limit_order(PendingOrder pending_order);
        void place_market_order(PendingMarketOrder pending_market_order);
        std::vector<Order> get_all_trader_orders(TraderID trader_id) const;

        void cancel_order(OrderID order_id);
        void modify_order(OrderID order_id, Price new_price, Quantity new_quantity);

        // Submit orders into the orderbook, for now random by traders to simulate activity
        void submit_pending_orders();

        // Expose Market data
        Level1Data get_current_level1_data() const;
        Level2Data get_current_level2_data() const;
        OrderBookSnapshot get_current_snapshot() const;

        // Order and Trade logs
        const std::vector<OrderLog>& get_order_logs() const { return order_book.order_logs; }
        const std::vector<Trade>& get_trade_logs() const { return order_book.trade_logs; }
        
        // Advance the simulation time
        void advance_time(Timestamp dt);
        Timestamp get_current_time() const;
};