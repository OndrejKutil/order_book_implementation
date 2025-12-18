#include "order_book.hpp"
#include <iostream>

// Example demonstrating simulation features
int main() {
    OrderBook ob;
    
    std::cout << "=== Order Book Simulation Demo ===" << std::endl << std::endl;
    
    // Time 0: Initial orders
    ob.advance_time(0);
    ob.place_limit_order(Order{1, 101, 100.0, 50, OrderSide::BUY, OrderType::LIMIT, 0});
    ob.place_limit_order(Order{2, 102, 99.0, 30, OrderSide::BUY, OrderType::LIMIT, 0});
    ob.place_limit_order(Order{3, 201, 101.0, 40, OrderSide::SELL, OrderType::LIMIT, 0});
    ob.place_limit_order(Order{4, 202, 102.0, 25, OrderSide::SELL, OrderType::LIMIT, 0});
    
    std::cout << "Time 0: Initial book setup" << std::endl;
    ob.print_order_book();
    
    // Get Level 1 data
    Level1Data level1 = ob.get_level1_data();
    std::cout << "\nLevel 1 Market Data:" << std::endl;
    std::cout << "  Best Bid: " << level1.bid_price << " x " << level1.bid_quantity << std::endl;
    std::cout << "  Best Ask: " << level1.ask_price << " x " << level1.ask_quantity << std::endl;
    std::cout << "  Mid Price: " << level1.mid_price << std::endl;
    std::cout << "  Spread: " << level1.spread << std::endl;
    
    // Get snapshot
    OrderBookSnapshot snapshot = ob.get_snapshot(0);
    std::cout << "\nOrder Book Snapshot:" << std::endl;
    std::cout << "  Bid Levels: " << snapshot.bids.size() << std::endl;
    for (const auto& level : snapshot.bids) {
        std::cout << "    " << level.price << " x " << level.total_quantity 
                  << " (" << level.order_count << " orders)" << std::endl;
    }
    std::cout << "  Ask Levels: " << snapshot.asks.size() << std::endl;
    for (const auto& level : snapshot.asks) {
        std::cout << "    " << level.price << " x " << level.total_quantity 
                  << " (" << level.order_count << " orders)" << std::endl;
    }
    
    // Time 1000: Aggressive buy order that crosses the spread
    std::cout << "\n=== Time 1000: Aggressive buy order crossing spread ===" << std::endl;
    ob.advance_time(1000);
    ob.place_limit_order(Order{5, 103, 101.5, 60, OrderSide::BUY, OrderType::LIMIT, 1000});
    
    std::cout << "\nTrades executed:" << std::endl;
    ob.print_trade_logs();
    
    std::cout << "\nOrder book after trade:" << std::endl;
    ob.print_order_book();
    
    // Time 2000: Market order
    std::cout << "\n=== Time 2000: Market order ===" << std::endl;
    ob.advance_time(2000);
    ob.place_market_order(Order{6, 104, 0.0, 30, OrderSide::SELL, OrderType::MARKET, 2000});
    
    std::cout << "\nAll trades:" << std::endl;
    ob.print_trade_logs();
    
    // Time 3000: Modify an order
    std::cout << "\n=== Time 3000: Modify order ===" << std::endl;
    ob.advance_time(3000);
    ob.modify_order(2, 99.5, 40);  // Modify order 2
    
    std::cout << "\nFinal order book:" << std::endl;
    ob.print_order_book();
    
    std::cout << "\nFinal Level 1 Data:" << std::endl;
    level1 = ob.get_level1_data();
    std::cout << "  Best Bid: " << level1.bid_price << " x " << level1.bid_quantity << std::endl;
    std::cout << "  Best Ask: " << level1.ask_price << " x " << level1.ask_quantity << std::endl;
    std::cout << "  Mid Price: " << level1.mid_price << std::endl;

    return 0;
}
