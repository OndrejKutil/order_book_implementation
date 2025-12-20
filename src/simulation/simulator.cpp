#include "../book/order_book.hpp"

class Simulator {
    private:
        OrderBook order_book;
        Timestamp simulation_time = 0;

    public:
        Simulator() {
            // Initialize the order book
            order_book.advance_time(simulation_time);
        }

        // Helper method to place a limit order from Python
        // We decompose the Order struct into individual arguments for easier Python usage
        void place_limit_order(OrderID order_id, TraderID trader_id, Price price, Quantity quantity, OrderSide side) {
            Order order;
            order.order_id = order_id;
            order.trader_id = trader_id;
            order.price = price;
            order.quantity = quantity;
            order.side = side;
            order.type = OrderType::LIMIT;
            order.timestamp = simulation_time;
            
            order_book.place_limit_order(order);
        }

        // Expose Level 1 market data (Best Bid/Ask)
        Level1Data get_current_level1_data() const {
            return order_book.get_level1_data();
        }
        
        // Advance the simulation time
        void advance_time(Timestamp dt) {
            simulation_time += dt;
            order_book.advance_time(simulation_time);
        }
};