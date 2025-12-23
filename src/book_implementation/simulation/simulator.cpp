#include "simulator.hpp"

Simulator::Simulator(Timestamp start_time = 0) {
    simulation_time = start_time;
    
    // Initialize the order book
    order_book.advance_time(simulation_time);
}

void Simulator::place_limit_order(PendingOrder pending_order) {
    Order order;
    order.order_id = pending_order.order_id;
    order.trader_id = pending_order.trader_id;
    order.price = pending_order.price;
    order.quantity = pending_order.quantity;
    order.side = pending_order.side;
    order.type = OrderType::LIMIT;
    order.timestamp = simulation_time;
    
    pending_orders[pending_order.trader_id] = order;
}

void Simulator::place_market_order(PendingMarketOrder pending_market_order) {
    Order order;
    order.order_id = pending_market_order.order_id;
    order.trader_id = pending_market_order.trader_id;
    order.price = 0.0; // Price is irrelevant for market orders
    order.quantity = pending_market_order.quantity;
    order.side = pending_market_order.side;
    order.type = OrderType::MARKET;
    order.timestamp = simulation_time;
    
    pending_orders[pending_market_order.trader_id] = order;
}

void Simulator::submit_pending_orders() {
    for (const auto& [trader_id, order] : pending_orders) {
        if (order.type == OrderType::LIMIT) {
            order_book.place_limit_order(order);
        } else if (order.type == OrderType::MARKET) {
            order_book.place_market_order(order);
        }
    }
    pending_orders.clear();
}

Level1Data Simulator::get_current_level1_data() const {
    return order_book.get_level1_data();
}

Level2Data Simulator::get_current_level2_data() const {
    return order_book.get_level2_data();
}

OrderBookSnapshot Simulator::get_current_snapshot() const {
    return order_book.get_snapshot(simulation_time);
}

void Simulator::advance_time(Timestamp dt) {
    simulation_time += dt;
    order_book.advance_time(simulation_time);
}

Timestamp Simulator::get_current_time() const {
    return simulation_time;
}

std::vector<Order> Simulator::get_all_trader_orders(TraderID trader_id) const {
    return order_book.get_all_trader_orders(trader_id);
}

void Simulator::cancel_order(OrderID order_id) {
    order_book.cancel_order(order_id);
}

void Simulator::modify_order(OrderID order_id, Price new_price, Quantity new_quantity) {
    order_book.modify_order(order_id, new_price, new_quantity);
}
