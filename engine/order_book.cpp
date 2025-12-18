#include "order_book.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

Price OrderBook::get_best_bid() const {
    if (buy_orders.empty()) {
        return 0.0;
    }
    return buy_orders.begin()->first;
}

Price OrderBook::get_best_ask() const {
    if (sell_orders.empty()) {
        return 0.0;
    }
    return sell_orders.begin()->first;
}

bool OrderBook::is_match_possible() const {
    return !buy_orders.empty() && !sell_orders.empty() &&
            get_best_bid() >= get_best_ask();
}

void OrderBook::print_order_book() const {
    std::cout << "Order Book:" << std::endl;
    std::cout << "Buy Orders:" << std::endl;
    for (const auto& [price, orders] : buy_orders) {
        for (const auto& order : orders) {
            std::cout << "ID: " << order.order_id << ", Price: " << price
                        << ", Quantity: " << order.quantity << std::endl;
        }
    }
    std::cout << "Sell Orders:" << std::endl;
    for (const auto& [price, orders] : sell_orders) {
        for (const auto& order : orders) {
            std::cout << "ID: " << order.order_id << ", Price: " << price
                        << ", Quantity: " << order.quantity << std::endl;
        }
    }
}

void OrderBook::print_order_logs() const {
    std::cout << "Order Logs:" << std::endl;
    for (const auto& log : order_logs) {
        std::cout << "Order ID: " << log.order_id
                  << ", Trader ID: " << log.trader_id
                  << ", Price: " << log.price
                  << ", Quantity: " << log.quantity
                  << ", Side: " << ((log.side == OrderSide::BUY) ? "BUY" : "SELL")
                  << ", Type: " << ((log.type == OrderType::LIMIT) ? "LIMIT" : "MARKET")
                  << ", Status: ";
        switch (log.status) {
            case OrderStatus::PLACED:
                std::cout << "PLACED";
                break;
            case OrderStatus::PARTIALLY_FILLED:
                std::cout << "PARTIALLY_FILLED";
                break;
            case OrderStatus::FILLED:
                std::cout << "FILLED";
                break;
            case OrderStatus::UNFILLED:
                std::cout << "UNFILLED";
                break;
            case OrderStatus::CANCELED:
                std::cout << "CANCELED";
                break;
        }
        std::cout << ", Details: " << log.details << std::endl;
    }
}

void OrderBook::place_limit_order(const Order& order) {
    if (order.side == OrderSide::BUY) {
        buy_orders[order.price].push_back(order);

        order_logs.push_back(OrderLog {
            order.order_id,
            order.trader_id,
            order.price,
            order.quantity,
            order.side,
            order.type,
            OrderStatus::PLACED,
            0,
            std::string("Limit buy order placed")
        });

        if (is_match_possible()) {
            match_orders();
        }

    } else {
        sell_orders[order.price].push_back(order);

        order_logs.push_back(OrderLog {
            order.order_id,
            order.trader_id,
            order.price,
            order.quantity,
            order.side,
            order.type,
            OrderStatus::PLACED,
            0,
            std::string("Limit sell order placed")
        });

        if (is_match_possible()) {
            match_orders();
        }
    }
}

void OrderBook::match_orders() {
    while (is_match_possible()) {
        Price best_bid = get_best_bid();
        Price best_ask = get_best_ask();

        auto& buy_queue = buy_orders[best_bid];
        auto& sell_queue = sell_orders[best_ask];

        Order& buy_order = buy_queue.front();
        Order& sell_order = sell_queue.front();

        Quantity trade_quantity = std::min(buy_order.quantity, sell_order.quantity);

        Price price;
        Order incomming_order;

        if (buy_order.timestamp <= sell_order.timestamp) {
            price = buy_order.price;
            incomming_order = buy_order;
        } else {
            price = sell_order.price;
            incomming_order = sell_order;
        }

        buy_order.quantity -= trade_quantity;
        sell_order.quantity -= trade_quantity;

        // Log both orders after the match
        OrderLog buy_log {
            buy_order.order_id,
            buy_order.trader_id,
            price,
            trade_quantity,
            OrderSide::BUY,
            buy_order.type,
            (buy_order.quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
            0,
            std::string("Trade executed")
        };

        OrderLog sell_log {
            sell_order.order_id,
            sell_order.trader_id,
            price,
            trade_quantity,
            OrderSide::SELL,
            sell_order.type,
            (sell_order.quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
            0,
            std::string("Trade executed")
        };

        order_logs.push_back(buy_log);
        order_logs.push_back(sell_log);

        if (buy_order.quantity == 0) {
            buy_queue.erase(buy_queue.begin());
            if (buy_queue.empty()) {
                buy_orders.erase(best_bid);
            }
        }

        if (sell_order.quantity == 0) {
            sell_queue.erase(sell_queue.begin());
            if (sell_queue.empty()) {
                sell_orders.erase(best_ask);
            }
        }
    }
}

Quantity OrderBook::get_total_quantity(OrderSide side) const {
    Quantity total_quantity = 0;

    if (side == OrderSide::BUY) {
        for (const auto& order : buy_orders) {
            for (const auto& o : order.second) {
                total_quantity += o.quantity;
            }
        }
    } else {
        for (const auto& order : sell_orders) {
            for (const auto& o : order.second) {
                total_quantity += o.quantity;
            }
        }
    }

    return total_quantity;
    
}

void OrderBook::place_market_order(const Order& order) {
    if (order.side == OrderSide::BUY) {
        if (sell_orders.empty()) {
            order_logs.push_back(OrderLog {
                order.order_id,
                order.trader_id,
                0.0,
                0,
                order.side,
                order.type,
                OrderStatus::UNFILLED,
                0,
                std::string("No sell orders available")
            });
            return;
        }
    
        Quantity remaining_quantity = order.quantity;
        Price execution_price = 0.0;
        std::vector<std::pair<Price, Quantity>> executions;

        while (remaining_quantity > 0 && !sell_orders.empty()) {
            Order& sell_order = sell_orders.begin()->second.front();
            const Price& sell_price = sell_order.price;

            Quantity trade_quantity = std::min(remaining_quantity, sell_order.quantity);
            remaining_quantity -= trade_quantity;
            sell_order.quantity -= trade_quantity;
            executions.push_back({sell_price, trade_quantity});

            if (sell_order.quantity == 0) {
                sell_orders.begin()->second.erase(sell_orders.begin()->second.begin());
                if (sell_orders.begin()->second.empty()) {
                    sell_orders.erase(sell_orders.begin());
                };
            };
        }

        // compute average price of executions
        if (!executions.empty()) {
            double total_cost = 0.0;
            Quantity total_executed = 0;
            for (const auto& exec : executions) {
                total_cost += exec.first * exec.second;
                total_executed += exec.second;
            }
            execution_price = total_cost / total_executed;
        }

        order_logs.push_back(OrderLog {
            order.order_id,
            order.trader_id,
            execution_price,
            order.quantity - remaining_quantity,
            order.side,
            order.type,
            (remaining_quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
            0,
            std::string("Market buy order executed")
        });

    } else {
        if (buy_orders.empty()) {
            order_logs.push_back(OrderLog {
                order.order_id,
                order.trader_id,
                0.0,
                0,
                order.side,
                order.type,
                OrderStatus::UNFILLED,
                0,
                std::string("No buy orders available")
            });
            return;
        }

        Quantity remaining_quantity = order.quantity;
        Price execution_price = 0.0;
        std::vector<std::pair<Price, Quantity>> executions;

        while (remaining_quantity > 0 && !buy_orders.empty()) {
            Order& buy_order = buy_orders.begin()->second.front();
            const Price& buy_price = buy_order.price;

            Quantity trade_quantity = std::min(remaining_quantity, buy_order.quantity);
            remaining_quantity -= trade_quantity;
            buy_order.quantity -= trade_quantity;
            executions.push_back({buy_price, trade_quantity});

            if (buy_order.quantity == 0) {
                buy_orders.begin()->second.erase(buy_orders.begin()->second.begin());
                if (buy_orders.begin()->second.empty()) {
                    buy_orders.erase(buy_orders.begin());
                };
            };
        }

        // compute average price of executions
        if (!executions.empty()) {
            double total_cost = 0.0;
            Quantity total_executed = 0;
            for (const auto& exec : executions) {
                total_cost += exec.first * exec.second;
                total_executed += exec.second;
            }
            execution_price = total_cost / total_executed;
        }

        order_logs.push_back(OrderLog {
            order.order_id,
            order.trader_id,
            execution_price,
            order.quantity - remaining_quantity,
            order.side,
            order.type,
            (remaining_quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
            0,
            std::string("Market sell order executed")
        });
    }
}

void OrderBook::cancel_order(OrderID order_id) {
    // Search in buy orders
    for (auto it = buy_orders.begin(); it != buy_orders.end(); ++it) {
        auto& orders = it->second;
        auto order_it = std::find_if(orders.begin(), orders.end(),
                                     [order_id](const Order& o) { return o.order_id == order_id; });
        if (order_it != orders.end()) {
            orders.erase(order_it);
            if (orders.empty()) {
                buy_orders.erase(it);
            }
            order_logs.push_back(OrderLog {
                order_id,
                0,
                it->first,
                0,
                OrderSide::BUY,
                OrderType::LIMIT,
                OrderStatus::CANCELED,
                0,
                std::string("Buy order canceled")
            });
            return;
        }
    }

    // Search in sell orders
    for (auto it = sell_orders.begin(); it != sell_orders.end(); ++it) {
        auto& orders = it->second;
        auto order_it = std::find_if(orders.begin(), orders.end(),
                                     [order_id](const Order& o) { return o.order_id == order_id; });
        if (order_it != orders.end()) {
            orders.erase(order_it);
            if (orders.empty()) {
                sell_orders.erase(it);
            }
            order_logs.push_back(OrderLog {
                order_id,
                0,
                it->first,
                0,
                OrderSide::SELL,
                OrderType::LIMIT,
                OrderStatus::CANCELED,
                0,
                std::string("Sell order canceled")
            });
            return;
        }
    }
}

double OrderBook::get_spread() const {
    Price best_bid = get_best_bid();
    Price best_ask = get_best_ask();

    if (best_bid == 0.0 || best_ask == 0.0) {
        return 0.0; // No spread if one side is empty
    }

    return best_ask - best_bid;
}

// test it now
int main() {
    OrderBook ob;

    ob.place_limit_order(Order{1, 1, 10.0, 20, OrderSide::SELL, OrderType::LIMIT, 1000});

    ob.place_limit_order(Order{2, 2, 12.0, 25, OrderSide::BUY, OrderType::LIMIT, 1001});

    ob.place_limit_order(Order{3, 3, 13.25, 15, OrderSide::SELL, OrderType::LIMIT, 1002});

    ob.place_market_order(Order{4, 4, 0.0, 30, OrderSide::BUY, OrderType::MARKET, 1003});

    ob.print_order_book();

    ob.place_market_order(Order{4, 4, 0.0, 30, OrderSide::BUY, OrderType::MARKET, 1003});
    
    ob.print_order_logs();
    ob.print_order_book();

    



    return 0;
}
