#pragma once
#include "../book/order_book.hpp"

class Simulator {
    private:
        OrderBook order_book;
        Timestamp simulation_time = 0;

    public:
        Simulator();

        // Helper method to place a limit order from Python
        void place_limit_order(OrderID order_id, TraderID trader_id, Price price, Quantity quantity, OrderSide side);

        // Expose Level 1 market data (Best Bid/Ask)
        Level1Data get_current_level1_data() const;
        
        // Advance the simulation time
        void advance_time(Timestamp dt);
};
