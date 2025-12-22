#include "simulator.hpp"

Simulator::Simulator() {
    // Initialize the order book
    order_book.advance_time(simulation_time);
}

void Simulator::place_limit_order(OrderID order_id, TraderID trader_id, Price price, Quantity quantity, OrderSide side) {
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

Level1Data Simulator::get_current_level1_data() const {
    return order_book.get_level1_data();
}

void Simulator::advance_time(Timestamp dt) {
    simulation_time += dt;
    order_book.advance_time(simulation_time);
}