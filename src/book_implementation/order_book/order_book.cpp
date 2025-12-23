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

void OrderBook::place_limit_order(const Order& order) {
    Order working_order = order;
    
    if (order.side == OrderSide::BUY) {
        // Try to match against existing sell orders (resting orders)
        while (working_order.quantity > 0 && !sell_orders.empty() && 
               working_order.price >= sell_orders.begin()->first) {
            
            Price resting_price = sell_orders.begin()->first;
            auto& sell_queue = sell_orders[resting_price];
            Order& resting_order = sell_queue.front();
            
            Quantity trade_quantity = std::min(working_order.quantity, resting_order.quantity);
            
            // Execution price is the resting order's price
            Price execution_price = resting_price;
            
            working_order.quantity -= trade_quantity;
            resting_order.quantity -= trade_quantity;
            
            // Log the trade
            trade_logs.push_back(Trade {
                next_trade_id++,
                working_order.order_id,
                resting_order.order_id,
                OrderSide::BUY,
                working_order.trader_id,
                resting_order.trader_id,
                execution_price,
                trade_quantity,
                0
            });
            
            // Log the trade for both orders
            order_logs.push_back(OrderLog {
                working_order.order_id,
                working_order.trader_id,
                execution_price,
                trade_quantity,
                OrderSide::BUY,
                working_order.type,
                (working_order.quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
                0,
                std::string("Trade executed")
            });
            
            order_logs.push_back(OrderLog {
                resting_order.order_id,
                resting_order.trader_id,
                execution_price,
                trade_quantity,
                OrderSide::SELL,
                resting_order.type,
                (resting_order.quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
                0,
                std::string("Trade executed")
            });
            
            // Remove filled resting order
            if (resting_order.quantity == 0) {
                order_index.erase(resting_order.order_id);
                sell_queue.erase(sell_queue.begin());
                if (sell_queue.empty()) {
                    sell_orders.erase(resting_price);
                }
            }
        }
        
        // Add remaining quantity to the book
        if (working_order.quantity > 0) {
            buy_orders[working_order.price].push_back(working_order);
            order_index[working_order.order_id] = {working_order.price, OrderSide::BUY};
            order_logs.push_back(OrderLog {
                working_order.order_id,
                working_order.trader_id,
                working_order.price,
                working_order.quantity,
                working_order.side,
                working_order.type,
                OrderStatus::PLACED,
                0,
                std::string("Limit buy order placed")
            });
        }

        invariant_check();

    } else {
        // Try to match against existing buy orders (resting orders)
        while (working_order.quantity > 0 && !buy_orders.empty() && 
               working_order.price <= buy_orders.begin()->first) {
            
            Price resting_price = buy_orders.begin()->first;
            auto& buy_queue = buy_orders[resting_price];
            Order& resting_order = buy_queue.front();
            
            Quantity trade_quantity = std::min(working_order.quantity, resting_order.quantity);
            
            // Execution price is the resting order's price
            Price execution_price = resting_price;
            
            working_order.quantity -= trade_quantity;
            resting_order.quantity -= trade_quantity;
            
            // Log the trade
            trade_logs.push_back(Trade {
                next_trade_id++,
                resting_order.order_id,
                working_order.order_id,
                OrderSide::SELL,
                resting_order.trader_id,
                working_order.trader_id,
                execution_price,
                trade_quantity,
                0
            });
            
            // Log the trade for both orders
            order_logs.push_back(OrderLog {
                working_order.order_id,
                working_order.trader_id,
                execution_price,
                trade_quantity,
                OrderSide::SELL,
                working_order.type,
                (working_order.quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
                0,
                std::string("Trade executed")
            });
            
            order_logs.push_back(OrderLog {
                resting_order.order_id,
                resting_order.trader_id,
                execution_price,
                trade_quantity,
                OrderSide::BUY,
                resting_order.type,
                (resting_order.quantity == 0) ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED,
                0,
                std::string("Trade executed")
            });
            
            // Remove filled resting order
            if (resting_order.quantity == 0) {
                order_index.erase(resting_order.order_id);
                buy_queue.erase(buy_queue.begin());
                if (buy_queue.empty()) {
                    buy_orders.erase(resting_price);
                }
            }
        }
        
        // Add remaining quantity to the book
        if (working_order.quantity > 0) {
            sell_orders[working_order.price].push_back(working_order);
            order_index[working_order.order_id] = {working_order.price, OrderSide::SELL};
            order_logs.push_back(OrderLog {
                working_order.order_id,
                working_order.trader_id,
                working_order.price,
                working_order.quantity,
                working_order.side,
                working_order.type,
                OrderStatus::PLACED,
                0,
                std::string("Limit sell order placed")
            });
        }

        invariant_check();
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
            
            // Log each trade
            trade_logs.push_back(Trade {
                next_trade_id++,
                order.order_id,
                sell_order.order_id,
                OrderSide::BUY,
                order.trader_id,
                sell_order.trader_id,
                sell_price,
                trade_quantity,
                0
            });

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
            
            // Log each trade
            trade_logs.push_back(Trade {
                next_trade_id++,
                buy_order.order_id,
                order.order_id,
                OrderSide::SELL,
                buy_order.trader_id,
                order.trader_id,
                buy_price,
                trade_quantity,
                0
            });

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
    invariant_check();
}

void OrderBook::cancel_order(OrderID order_id) {
    // Use order index for O(1) lookup
    auto index_it = order_index.find(order_id);
    if (index_it == order_index.end()) {
        return; // Order not found
    }
    
    Price price = index_it->second.first;
    OrderSide side = index_it->second.second;
    
    if (side == OrderSide::BUY) {
        auto& orders = buy_orders[price];
        auto order_it = std::find_if(orders.begin(), orders.end(),
                                     [order_id](const Order& o) { return o.order_id == order_id; });
        if (order_it != orders.end()) {
            orders.erase(order_it);
            if (orders.empty()) {
                buy_orders.erase(price);
            }
            order_index.erase(order_id);
            order_logs.push_back(OrderLog {
                order_id,
                0,
                price,
                0,
                OrderSide::BUY,
                OrderType::LIMIT,
                OrderStatus::CANCELED,
                0,
                std::string("Buy order canceled")
            });
        }
    } else {
        auto& orders = sell_orders[price];
        auto order_it = std::find_if(orders.begin(), orders.end(),
                                     [order_id](const Order& o) { return o.order_id == order_id; });
        if (order_it != orders.end()) {
            orders.erase(order_it);
            if (orders.empty()) {
                sell_orders.erase(price);
            }
            order_index.erase(order_id);
            order_logs.push_back(OrderLog {
                order_id,
                0,
                price,
                0,
                OrderSide::SELL,
                OrderType::LIMIT,
                OrderStatus::CANCELED,
                0,
                std::string("Sell order canceled")
            });
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

Price OrderBook::get_mid_price() const {
    Price best_bid = get_best_bid();
    Price best_ask = get_best_ask();

    if (best_bid == 0.0 || best_ask == 0.0) {
        return 0.0;
    }

    return (best_bid + best_ask) / 2.0;
}

OrderBookSnapshot OrderBook::get_snapshot(Timestamp timestamp) const {
    OrderBookSnapshot snapshot;
    snapshot.timestamp = timestamp;
    snapshot.best_bid = get_best_bid();
    snapshot.best_ask = get_best_ask();
    snapshot.mid_price = get_mid_price();
    snapshot.spread = get_spread();
    
    // Build bid levels
    snapshot.total_bid_volume = 0;
    for (const auto& [price, orders] : buy_orders) {
        Quantity level_quantity = 0;
        for (const auto& order : orders) {
            level_quantity += order.quantity;
        }
        snapshot.bids.push_back(PriceLevel{price, level_quantity, static_cast<std::uint32_t>(orders.size())});
        snapshot.total_bid_volume += level_quantity;
    }
    
    // Build ask levels
    snapshot.total_ask_volume = 0;
    for (const auto& [price, orders] : sell_orders) {
        Quantity level_quantity = 0;
        for (const auto& order : orders) {
            level_quantity += order.quantity;
        }
        snapshot.asks.push_back(PriceLevel{price, level_quantity, static_cast<std::uint32_t>(orders.size())});
        snapshot.total_ask_volume += level_quantity;
    }
    
    return snapshot;
}

Level1Data OrderBook::get_level1_data() const {
    Level1Data data;
    data.timestamp = current_time;
    data.bid_price = get_best_bid();
    data.ask_price = get_best_ask();
    data.mid_price = get_mid_price();
    data.spread = get_spread();
    
    // Get quantities at best bid/ask
    data.bid_quantity = 0;
    if (!buy_orders.empty()) {
        for (const auto& order : buy_orders.begin()->second) {
            data.bid_quantity += order.quantity;
        }
    }
    
    data.ask_quantity = 0;
    if (!sell_orders.empty()) {
        for (const auto& order : sell_orders.begin()->second) {
            data.ask_quantity += order.quantity;
        }
    }
    
    return data;
}

Level2Data OrderBook::get_level2_data() const {
    Level2Data data;
    data.timestamp = current_time;
    
    // Bids
    for (const auto& [price, orders] : buy_orders) {
        Quantity level_quantity = 0;
        for (const auto& order : orders) {
            level_quantity += order.quantity;
        }
        data.bids.push_back(PriceLevel{price, level_quantity, static_cast<std::uint32_t>(orders.size())});
    }
    
    // Asks
    for (const auto& [price, orders] : sell_orders) {
        Quantity level_quantity = 0;
        for (const auto& order : orders) {
            level_quantity += order.quantity;
        }
        data.asks.push_back(PriceLevel{price, level_quantity, static_cast<std::uint32_t>(orders.size())});
    }
    
    return data;
}

Quantity OrderBook::get_depth_at_price(Price price, OrderSide side) const {
    Quantity total = 0;
    
    if (side == OrderSide::BUY) {
        auto it = buy_orders.find(price);
        if (it != buy_orders.end()) {
            for (const auto& order : it->second) {
                total += order.quantity;
            }
        }
    } else {
        auto it = sell_orders.find(price);
        if (it != sell_orders.end()) {
            for (const auto& order : it->second) {
                total += order.quantity;
            }
        }
    }
    
    return total;
}

std::vector<PriceLevel> OrderBook::get_bid_levels(size_t depth) const {
    std::vector<PriceLevel> levels;
    size_t count = 0;
    
    for (const auto& [price, orders] : buy_orders) {
        if (count >= depth) break;
        
        Quantity level_quantity = 0;
        for (const auto& order : orders) {
            level_quantity += order.quantity;
        }
        
        levels.push_back(PriceLevel{price, level_quantity, static_cast<std::uint32_t>(orders.size())});
        count++;
    }
    
    return levels;
}

std::vector<PriceLevel> OrderBook::get_ask_levels(size_t depth) const {
    std::vector<PriceLevel> levels;
    size_t count = 0;
    
    for (const auto& [price, orders] : sell_orders) {
        if (count >= depth) break;
        
        Quantity level_quantity = 0;
        for (const auto& order : orders) {
            level_quantity += order.quantity;
        }
        
        levels.push_back(PriceLevel{price, level_quantity, static_cast<std::uint32_t>(orders.size())});
        count++;
    }
    
    return levels;
}

void OrderBook::modify_order(OrderID order_id, Price new_price, Quantity new_quantity) {
    // Find and remove the old order
    Order old_order;
    bool found = false;
    
    // Search in buy orders
    for (auto it = buy_orders.begin(); it != buy_orders.end(); ++it) {
        auto& orders = it->second;
        auto order_it = std::find_if(orders.begin(), orders.end(),
                                     [order_id](const Order& o) { return o.order_id == order_id; });
        if (order_it != orders.end()) {
            old_order = *order_it;
            found = true;
            orders.erase(order_it);
            if (orders.empty()) {
                buy_orders.erase(it);
            }
            break;
        }
    }
    
    // Search in sell orders if not found
    if (!found) {
        for (auto it = sell_orders.begin(); it != sell_orders.end(); ++it) {
            auto& orders = it->second;
            auto order_it = std::find_if(orders.begin(), orders.end(),
                                         [order_id](const Order& o) { return o.order_id == order_id; });
            if (order_it != orders.end()) {
                old_order = *order_it;
                found = true;
                orders.erase(order_it);
                if (orders.empty()) {
                    sell_orders.erase(it);
                }
                break;
            }
        }
    }
    
    if (!found) {
        return; // Order not found
    }
    
    // Create modified order with new price and quantity
    Order modified_order = old_order;
    modified_order.price = new_price;
    modified_order.quantity = new_quantity;
    modified_order.timestamp = current_time; // Reset timestamp (loses time priority)
    
    // Place the modified order
    place_limit_order(modified_order);
    
    order_logs.push_back(OrderLog {
        order_id,
        old_order.trader_id,
        new_price,
        new_quantity,
        old_order.side,
        old_order.type,
        OrderStatus::PLACED,
        current_time,
        std::string("Order modified")
    });
}

std::vector<Order> OrderBook::get_all_trader_orders(TraderID trader_id) const {
    std::vector<Order> trader_orders;
    
    // Check buy orders
    for (const auto& [price, orders] : buy_orders) {
        for (const auto& order : orders) {
            if (order.trader_id == trader_id) {
                trader_orders.push_back(order);
            }
        }
    }
    
    // Check sell orders
    for (const auto& [price, orders] : sell_orders) {
        for (const auto& order : orders) {
            if (order.trader_id == trader_id) {
                trader_orders.push_back(order);
            }
        }
    }
    
    return trader_orders;
}