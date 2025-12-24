#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <stdexcept>
#include "simulator.hpp"
#include "../order_book/types.hpp"

namespace py = pybind11;

// ============================================================================
// Python Bindings
// =============================================================================

PYBIND11_MODULE(market_simulator, m) {
     m.doc() = "Order Book Simulator Plugin"; // Optional module docstring

     // =============================================
     // Enums
     // =============================================

    // Expose the OrderSide enum
    // This allows using market_simulator.OrderSide.BUY in Python
     py::enum_<OrderSide>(m, "OrderSide", "Enumeration for order side (buy or sell)")
         .value("BUY", OrderSide::BUY, "Buy order")
         .value("SELL", OrderSide::SELL, "Sell order")
         .export_values();

     // Expose the OrderType enum
     // This allows using market_simulator.OrderType.LIMIT in Python
     py::enum_<OrderType>(m, "OrderType", "Enumeration for order type (limit or market)")
         .value("LIMIT", OrderType::LIMIT, "Limit order")
         .value("MARKET", OrderType::MARKET, "Market order")
         .export_values();

     // Expose the OrderStatus enum
     // This allows using market_simulator.OrderStatus.PLACED in Python
     py::enum_<OrderStatus>(m, "OrderStatus", "Enumeration for order status")
          .value("PLACED", OrderStatus::PLACED, "Order has been placed")
          .value("PARTIALLY_FILLED", OrderStatus::PARTIALLY_FILLED, "Order has been partially filled")
          .value("FILLED", OrderStatus::FILLED, "Order has been completely filled")
          .value("UNFILLED", OrderStatus::UNFILLED, "Order remains unfilled")
          .value("CANCELED", OrderStatus::CANCELED, "Order has been canceled")
          .export_values();

     // =============================================
     // Structures
     // =============================================

    // Expose the Level1Data structure
    // We use def_readonly to make the fields accessible but not modifiable from Python
    // This allows users to read the values but not change them directly
     py::class_<Level1Data>(m, "Level1Data", "Top-of-book market data snapshot")
          .def_readonly("bid_price", &Level1Data::bid_price, "Best bid price")
          .def_readonly("bid_quantity", &Level1Data::bid_quantity, "Total quantity at best bid")
          .def_readonly("ask_price", &Level1Data::ask_price, "Best ask price")
          .def_readonly("ask_quantity", &Level1Data::ask_quantity, "Total quantity at best ask")
          .def_readonly("mid_price", &Level1Data::mid_price, "Mid price between best bid and ask")
          .def_readonly("spread", &Level1Data::spread, "Bid-ask spread")
          .def_readonly("timestamp", &Level1Data::timestamp, "Timestamp of the data snapshot")
          
          // String representation for easier debugging
          .def("__repr__", [](const Level1Data &x) {
               return "<Level1Data timestamp=" + std::to_string(x.timestamp) + ">";
          })

          // Convert to dictionary for easy access in Python
          .def("to_dict", [](const Level1Data &x) {
               py::dict d;
               d["bid_price"] = x.bid_price;
               d["bid_quantity"] = x.bid_quantity;
               d["ask_price"] = x.ask_price;
               d["ask_quantity"] = x.ask_quantity;
               d["mid_price"] = x.mid_price;
               d["spread"] = x.spread;
               d["timestamp"] = x.timestamp;
               return d;
          })

          // Pickle support for serialization
          // basically representing a __getstate__ and __setstate__ pairs
          .def(py::pickle(
               [](const Level1Data &x) {
                    return py::make_tuple(
                         x.bid_price,
                         x.bid_quantity,
                         x.ask_price,
                         x.ask_quantity,
                         x.mid_price,
                         x.spread,
                         x.timestamp
                    );
               },
               [](py::tuple t) {
                    if (t.size() != 7) {
                         throw std::runtime_error("Invalid state for Level1Data");
                    }
                    Level1Data x;
                    x.bid_price = t[0].cast<Price>();
                    x.bid_quantity = t[1].cast<Quantity>();
                    x.ask_price = t[2].cast<Price>();
                    x.ask_quantity = t[3].cast<Quantity>();
                    x.mid_price = t[4].cast<Price>();
                    x.spread = t[5].cast<Price>();
                    x.timestamp = t[6].cast<Timestamp>();
                    return x;
               }
          ))
          ;

     // Expose the Level2Data structure
     py::class_<Level2Data>(m, "Level2Data", "Level 2 market data snapshot")
          .def_readonly("bids", &Level2Data::bids, "List of bid levels (price, quantity)")
          .def_readonly("asks", &Level2Data::asks, "List of ask levels (price, quantity)")
          .def_readonly("timestamp", &Level2Data::timestamp, "Timestamp of the data snapshot")
          .def("__repr__", [](const Level2Data &x) {
               return "<Level2Data timestamp=" + std::to_string(x.timestamp) + ">";
          })
          .def("to_dict", [](const Level2Data &x) {
               auto levels_to_list = [](const std::vector<PriceLevel> &levels) {
                    py::list out;
                    for (const auto &lvl : levels) {
                         py::dict d;
                         d["price"] = lvl.price;
                         d["total_quantity"] = lvl.total_quantity;
                         d["order_count"] = lvl.order_count;
                         out.append(std::move(d));
                    }
                    return out;
               };

               py::dict d;
               d["bids"] = levels_to_list(x.bids);
               d["asks"] = levels_to_list(x.asks);
               d["timestamp"] = x.timestamp;
               return d;
          })
          .def(py::pickle(
               [](const Level2Data &x) {
                    auto levels_to_tuples = [](const std::vector<PriceLevel> &levels) {
                         py::list out;
                         for (const auto &lvl : levels) {
                              out.append(py::make_tuple(lvl.price, lvl.total_quantity, lvl.order_count));
                         }
                         return out;
                    };

                    return py::make_tuple(
                         levels_to_tuples(x.bids),
                         levels_to_tuples(x.asks),
                         x.timestamp
                    );
               },
               [](py::tuple t) {
                    if (t.size() != 3) {
                         throw std::runtime_error("Invalid state for Level2Data");
                    }

                    auto tuples_to_levels = [](py::handle seq) {
                         std::vector<PriceLevel> levels;
                         py::iterable iterable = py::reinterpret_borrow<py::iterable>(seq);
                         for (py::handle item : iterable) {
                              py::tuple tup = py::reinterpret_borrow<py::tuple>(item);
                              if (tup.size() != 3) {
                                   throw std::runtime_error("Invalid PriceLevel state");
                              }
                              PriceLevel lvl;
                              lvl.price = tup[0].cast<Price>();
                              lvl.total_quantity = tup[1].cast<Quantity>();
                              lvl.order_count = tup[2].cast<std::uint32_t>();
                              levels.push_back(lvl);
                         }
                         return levels;
                    };

                    Level2Data x;
                    x.bids = tuples_to_levels(t[0]);
                    x.asks = tuples_to_levels(t[1]);
                    x.timestamp = t[2].cast<Timestamp>();
                    return x;
               }
          ))
          ;

     // Expose the OrderBookSnapshot structure for full book data
     py::class_<OrderBookSnapshot>(m, "OrderBookSnapshot", "Full order book snapshot")
          .def_readonly("timestamp", &OrderBookSnapshot::timestamp, "Timestamp of the snapshot")     
          .def_readonly("bids", &OrderBookSnapshot::bids, "List of bid orders")
          .def_readonly("asks", &OrderBookSnapshot::asks, "List of ask orders")
          .def_readonly("best_bid", &OrderBookSnapshot::best_bid, "Best bid price")
          .def_readonly("best_ask", &OrderBookSnapshot::best_ask, "Best ask price")
          .def_readonly("mid_price", &OrderBookSnapshot::mid_price, "Mid price between best bid and ask")
          .def_readonly("spread", &OrderBookSnapshot::spread, "Bid-ask spread")
          .def_readonly("total_bid_volume", &OrderBookSnapshot::total_bid_volume, "Total volume on the bid side")
          .def_readonly("total_ask_volume", &OrderBookSnapshot::total_ask_volume, "Total volume on the ask side")
          
          .def("__repr__", [](const OrderBookSnapshot &x) {
               return "<OrderBookSnapshot timestamp=" + std::to_string(x.timestamp) + ">";
          })
          .def("to_dict", [](const OrderBookSnapshot &x) {
               auto levels_to_list = [](const std::vector<PriceLevel> &levels) {
                    py::list out;
                    for (const auto &lvl : levels) {
                         py::dict d;
                         d["price"] = lvl.price;
                         d["total_quantity"] = lvl.total_quantity;
                         d["order_count"] = lvl.order_count;
                         out.append(std::move(d));
                    }
                    return out;
               };

               py::dict d;
               d["timestamp"] = x.timestamp;
               d["bids"] = levels_to_list(x.bids);
               d["asks"] = levels_to_list(x.asks);
               d["best_bid"] = x.best_bid;
               d["best_ask"] = x.best_ask;
               d["mid_price"] = x.mid_price;
               d["spread"] = x.spread;
               d["total_bid_volume"] = x.total_bid_volume;
               d["total_ask_volume"] = x.total_ask_volume;
               return d;
          })
          .def(py::pickle(
               [](const OrderBookSnapshot &x) {
                    auto levels_to_tuples = [](const std::vector<PriceLevel> &levels) {
                         py::list out;
                         for (const auto &lvl : levels) {
                              out.append(py::make_tuple(lvl.price, lvl.total_quantity, lvl.order_count));
                         }
                         return out;
                    };

                    return py::make_tuple(
                         x.timestamp,
                         levels_to_tuples(x.bids),
                         levels_to_tuples(x.asks),
                         x.best_bid,
                         x.best_ask,
                         x.mid_price,
                         x.spread,
                         x.total_bid_volume,
                         x.total_ask_volume
                    );
               },
               [](py::tuple t) {
                    if (t.size() != 9) {
                         throw std::runtime_error("Invalid state for OrderBookSnapshot");
                    }

                    auto tuples_to_levels = [](py::handle seq) {
                         std::vector<PriceLevel> levels;
                         py::iterable iterable = py::reinterpret_borrow<py::iterable>(seq);
                         for (py::handle item : iterable) {
                              py::tuple tup = py::reinterpret_borrow<py::tuple>(item);
                              if (tup.size() != 3) {
                                   throw std::runtime_error("Invalid PriceLevel state");
                              }
                              PriceLevel lvl;
                              lvl.price = tup[0].cast<Price>();
                              lvl.total_quantity = tup[1].cast<Quantity>();
                              lvl.order_count = tup[2].cast<std::uint32_t>();
                              levels.push_back(lvl);
                         }
                         return levels;
                    };

                    OrderBookSnapshot x;
                    x.timestamp = t[0].cast<Timestamp>();
                    x.bids = tuples_to_levels(t[1]);
                    x.asks = tuples_to_levels(t[2]);
                    x.best_bid = t[3].cast<Price>();
                    x.best_ask = t[4].cast<Price>();
                    x.mid_price = t[5].cast<Price>();
                    x.spread = t[6].cast<Price>();
                    x.total_bid_volume = t[7].cast<Quantity>();
                    x.total_ask_volume = t[8].cast<Quantity>();
                    return x;
               }
          ))
          ;

     // Expose the PendingOrder structure
     py::class_<PendingOrder>(m, "PendingOrder", "Structure representing a pending order")
          .def(py::init<OrderID, TraderID, Price, Quantity, OrderSide>(),
               py::arg("order_id"), py::arg("trader_id"), py::arg("price"), py::arg("quantity"), py::arg("side"))
          .def_readonly("order_id", &PendingOrder::order_id, "Unique identifier for the order")
          .def_readonly("trader_id", &PendingOrder::trader_id, "Identifier of the trader placing the order")
          .def_readonly("price", &PendingOrder::price, "Limit price for the order")
          .def_readonly("quantity", &PendingOrder::quantity, "Number of shares/contracts")
          .def_readonly("side", &PendingOrder::side, "Order side (BUY or SELL)")
          .def("__repr__", [](const PendingOrder &x) {
              return "<PendingOrder order_id=" + std::to_string(x.order_id) + ">";
          })
          .def("to_dict", [](const PendingOrder &x) {
               py::dict d;
               d["order_id"] = x.order_id;
               d["trader_id"] = x.trader_id;
               d["price"] = x.price;
               d["quantity"] = x.quantity;
               d["side"] = x.side;
               return d;
          })
          .def(py::pickle(
               [](const PendingOrder &x) {
                    return py::make_tuple(x.order_id, x.trader_id, x.price, x.quantity, x.side);
               },
               [](py::tuple t) {
                    if (t.size() != 5) {
                         throw std::runtime_error("Invalid state for PendingOrder");
                    }
                    PendingOrder x;
                    x.order_id = t[0].cast<OrderID>();
                    x.trader_id = t[1].cast<TraderID>();
                    x.price = t[2].cast<Price>();
                    x.quantity = t[3].cast<Quantity>();
                    x.side = t[4].cast<OrderSide>();
                    return x;
               }
          ))
          ;

     // Expose the PendingMarketOrder structure
     py::class_<PendingMarketOrder>(m, "PendingMarketOrder", "Structure representing a pending market order")
          .def(py::init<OrderID, TraderID, Quantity, OrderSide>(),
               py::arg("order_id"), py::arg("trader_id"), py::arg("quantity"), py::arg("side"))
          .def_readonly("order_id", &PendingMarketOrder::order_id, "Unique identifier for the order")
          .def_readonly("trader_id", &PendingMarketOrder::trader_id, "Identifier of the trader placing the order")
          .def_readonly("quantity", &PendingMarketOrder::quantity, "Number of shares/contracts")
          .def_readonly("side", &PendingMarketOrder::side, "Order side (BUY or SELL)")
          .def("__repr__", [](const PendingMarketOrder &x) {
              return "<PendingMarketOrder order_id=" + std::to_string(x.order_id) + ">";
          })
          .def("to_dict", [](const PendingMarketOrder &x) {
               py::dict d;
               d["order_id"] = x.order_id;
               d["trader_id"] = x.trader_id;
               d["quantity"] = x.quantity;
               d["side"] = x.side;
               return d;
          })
          .def(py::pickle(
               [](const PendingMarketOrder &x) {
                    return py::make_tuple(x.order_id, x.trader_id, x.quantity, x.side);
               },
               [](py::tuple t) {
                    if (t.size() != 4) {
                         throw std::runtime_error("Invalid state for PendingMarketOrder");
                    }
                    PendingMarketOrder x;
                    x.order_id = t[0].cast<OrderID>();
                    x.trader_id = t[1].cast<TraderID>();
                    x.quantity = t[2].cast<Quantity>();
                    x.side = t[3].cast<OrderSide>();
                    return x;
               }
          ))
          ;

     // Expose the OrderLog structure for working with order logs
     py::class_<OrderLog>(m, "OrderLog", "Log entry for an order event")
          .def_readonly("order_id", &OrderLog::order_id, "Unique identifier for the order")
          .def_readonly("trader_id", &OrderLog::trader_id, "Identifier of the trader")
          .def_readonly("price", &OrderLog::price, "Price of the order")
          .def_readonly("quantity", &OrderLog::quantity, "Quantity of the order")
          .def_readonly("side", &OrderLog::side, "Order side (BUY or SELL)")
          .def_readonly("type", &OrderLog::type, "Order type (LIMIT or MARKET)")
          .def_readonly("status", &OrderLog::status, "Current status of the order")
          .def_readonly("timestamp", &OrderLog::timestamp, "Timestamp of the order event")
          .def_readonly("details", &OrderLog::details, "Additional details about the order event")
          .def("__repr__", [](const OrderLog &x) {
              return "<OrderLog order_id=" + std::to_string(x.order_id) + ">";
          })
          .def("to_dict", [](const OrderLog &x) {
               py::dict d;
               d["order_id"] = x.order_id;
               d["trader_id"] = x.trader_id;
               d["price"] = x.price;
               d["quantity"] = x.quantity;
               d["side"] = x.side;
               d["type"] = x.type;
               d["status"] = x.status;
               d["timestamp"] = x.timestamp;
               d["details"] = x.details;
               return d;
          })
          .def(py::pickle(
               [](const OrderLog &x) {
                    return py::make_tuple(
                         x.order_id,
                         x.trader_id,
                         x.price,
                         x.quantity,
                         x.side,
                         x.type,
                         x.status,
                         x.timestamp,
                         x.details
                    );
               },
               [](py::tuple t) {
                    if (t.size() != 9) {
                         throw std::runtime_error("Invalid state for OrderLog");
                    }
                    OrderLog x;
                    x.order_id = t[0].cast<OrderID>();
                    x.trader_id = t[1].cast<TraderID>();
                    x.price = t[2].cast<Price>();
                    x.quantity = t[3].cast<Quantity>();
                    x.side = t[4].cast<OrderSide>();
                    x.type = t[5].cast<OrderType>();
                    x.status = t[6].cast<OrderStatus>();
                    x.timestamp = t[7].cast<Timestamp>();
                    x.details = t[8].cast<std::string>();
                    return x;
               }
          ))
          ;

     // Expose the Trade structure for working with trade logs
     py::class_<Trade>(m, "TradeLog", "Structure representing a trade execution")
          .def_readonly("trade_id", &Trade::trade_id, "Unique identifier for the trade")
          .def_readonly("buy_order_id", &Trade::buy_order_id, "Order ID of the buy order")
          .def_readonly("sell_order_id", &Trade::sell_order_id, "Order ID of the sell order")
          .def_readonly("aggressor_side", &Trade::aggressor_side, "Side of the aggressor order")
          .def_readonly("buyer_id", &Trade::buyer_id, "Identifier of the buyer trader")
          .def_readonly("seller_id", &Trade::seller_id, "Identifier of the seller trader")
          .def_readonly("price", &Trade::price, "Execution price of the trade")
          .def_readonly("quantity", &Trade::quantity, "Quantity traded")
          .def_readonly("timestamp", &Trade::timestamp, "Timestamp of the trade execution")
          .def("__repr__", [](const Trade &x) {
              return "<Trade trade_id=" + std::to_string(x.trade_id) + ">";
          })
          .def("to_dict", [](const Trade &x) {
               py::dict d;
               d["trade_id"] = x.trade_id;
               d["buy_order_id"] = x.buy_order_id;
               d["sell_order_id"] = x.sell_order_id;
               d["aggressor_side"] = x.aggressor_side;
               d["buyer_id"] = x.buyer_id;
               d["seller_id"] = x.seller_id;
               d["price"] = x.price;
               d["quantity"] = x.quantity;
               d["timestamp"] = x.timestamp;
               return d;
          })
          .def(py::pickle(
               [](const Trade &x) {
                    return py::make_tuple(
                         x.trade_id,
                         x.buy_order_id,
                         x.sell_order_id,
                         x.aggressor_side,
                         x.buyer_id,
                         x.seller_id,
                         x.price,
                         x.quantity,
                         x.timestamp
                    );
               },
               [](py::tuple t) {
                    if (t.size() != 9) {
                         throw std::runtime_error("Invalid state for TradeLog");
                    }
                    Trade x;
                    x.trade_id = t[0].cast<TradeID>();
                    x.buy_order_id = t[1].cast<OrderID>();
                    x.sell_order_id = t[2].cast<OrderID>();
                    x.aggressor_side = t[3].cast<OrderSide>();
                    x.buyer_id = t[4].cast<TraderID>();
                    x.seller_id = t[5].cast<TraderID>();
                    x.price = t[6].cast<Price>();
                    x.quantity = t[7].cast<Quantity>();
                    x.timestamp = t[8].cast<Timestamp>();
                    return x;
               }
          ))
          ;

     // Expose the PriceLevel structure
     py::class_<PriceLevel>(m, "PriceLevel", "Structure representing a price level in the order book")
          .def_readonly("price", &PriceLevel::price, "Price at this level")
          .def_readonly("total_quantity", &PriceLevel::total_quantity, "Total quantity available at this price level")
          .def_readonly("order_count", &PriceLevel::order_count, "Number of orders at this price level")
          .def("__repr__", [](const PriceLevel &x) {
                 return "<PriceLevel price=" + std::to_string(x.price) + " quantity=" + std::to_string(x.total_quantity) + ">";
          })
          .def("to_dict", [](const PriceLevel &x) {
               py::dict d;
               d["price"] = x.price;
               d["total_quantity"] = x.total_quantity;
               d["order_count"] = x.order_count;
               return d;
          })
          .def(py::pickle(
               [](const PriceLevel &x) {
                    return py::make_tuple(x.price, x.total_quantity, x.order_count);
               },
               [](py::tuple t) {
                    if (t.size() != 3) {
                         throw std::runtime_error("Invalid state for PriceLevel");
                    }
                    PriceLevel x;
                    x.price = t[0].cast<Price>();
                    x.total_quantity = t[1].cast<Quantity>();
                    x.order_count = t[2].cast<std::uint32_t>();
                    return x;
               }
          ))
          ;

     // Expose the Order structure
     py::class_<Order>(m, "Order", "Structure representing an order in the order book")
          .def_readonly("order_id", &Order::order_id, "Unique identifier for the order")
          .def_readonly("trader_id", &Order::trader_id, "Identifier of the trader")
          .def_readonly("price", &Order::price, "Order price")
          .def_readonly("quantity", &Order::quantity, "Order quantity")
          .def_readonly("side", &Order::side, "Order side (BUY or SELL)")
          .def_readonly("type", &Order::type, "Order type (LIMIT or MARKET)")
          .def_readonly("timestamp", &Order::timestamp, "Timestamp when order was created")
          .def("__repr__", [](const Order &x) {
               return "<Order order_id=" + std::to_string(x.order_id) + ">";
          })
          .def("to_dict", [](const Order &x) {
               py::dict d;
               d["order_id"] = x.order_id;
               d["trader_id"] = x.trader_id;
               d["price"] = x.price;
               d["quantity"] = x.quantity;
               d["side"] = x.side;
               d["type"] = x.type;
               d["timestamp"] = x.timestamp;
               return d;
          })
          .def(py::pickle(
               [](const Order &x) {
                    return py::make_tuple(
                         x.order_id,
                         x.trader_id,
                         x.price,
                         x.quantity,
                         x.side,
                         x.type,
                         x.timestamp
                    );
               },
               [](py::tuple t) {
                    if (t.size() != 7) {
                         throw std::runtime_error("Invalid state for Order");
                    }
                    Order x;
                    x.order_id = t[0].cast<OrderID>();
                    x.trader_id = t[1].cast<TraderID>();
                    x.price = t[2].cast<Price>();
                    x.quantity = t[3].cast<Quantity>();
                    x.side = t[4].cast<OrderSide>();
                    x.type = t[5].cast<OrderType>();
                    x.timestamp = t[6].cast<Timestamp>();
                    return x;
               }
          ))
          ;


     // =============================================
     // Simulator Class
     // =============================================


     // Expose the Simulator class
     py::class_<Simulator>(m, "Simulator", "Order book market simulator")
         .def(py::init<Timestamp>(), 
              py::arg("start_time") = 0,
              "Initialize the simulator with an optional start time\n\n"
              "Args:\n"
              "    start_time (float, optional): Simulation start timestamp (default is 0)")

         // Limit and market orders
         .def("place_limit_order", &Simulator::place_limit_order, 
              "Place a limit order into the order book\n\n"
              "Args:\n"
              "    pending_order (PendingOrder): The pending limit order to place",
              py::arg("pending_order"))

         .def("place_market_order", &Simulator::place_market_order,
              "Place a market order into the order book\n\n"
              "Args:\n"
              "    pending_market_order (PendingMarketOrder): The pending market order to place",
              py::arg("pending_market_order"))

          .def("get_all_trader_orders", &Simulator::get_all_trader_orders,
               "Get all orders for a specific trader\n\n"
               "Args:\n"
               "    trader_id (int): Identifier of the trader\n\n"
               "Returns:\n"
               "    List[Order]: List of all orders placed by the trader",
               py::arg("trader_id"))

          .def("cancel_order", &Simulator::cancel_order,
               "Cancel an existing order\n\n"
               "Args:\n"
               "    order_id (int): Unique identifier of the order to cancel",
               py::arg("order_id"))

          .def("modify_order", &Simulator::modify_order,
               "Modify an existing order's price and/or quantity\n\n"
               "Args:\n"
               "    order_id (int): Unique identifier of the order to modify\n"
               "    new_price (float): New price for the order\n"
               "    new_quantity (int): New quantity for the order",
               py::arg("order_id"), py::arg("new_price"), py::arg("new_quantity"))

          // Submit pending orders
          .def("submit_pending_orders", &Simulator::submit_pending_orders, 
               "Submit all pending orders to the order book\n\n"
               "Processes queued orders and matches them against the book")

          // Book data
          .def("get_current_level1_data", &Simulator::get_current_level1_data, 
               "Get top of book data\n\n"
               "Returns:\n"
               "    Level1Data: Current best bid, ask, mid price, and spread")

          .def("get_current_level2_data", &Simulator::get_current_level2_data, 
              "Get Level 2 market data\n\n"
              "Returns:\n"
              "    Level2Data: Current order book depth data")

          .def("get_current_snapshot", &Simulator::get_current_snapshot, 
              "Get full order book snapshot\n\n"
              "Returns:\n"
              "    OrderBookSnapshot: Current full order book state")

          // Time management
          .def("advance_time", &Simulator::advance_time, 
               "Advance simulation time by dt\n\n"
               "Args:\n"
               "    dt (float): Time increment to advance",
               py::arg("dt"))

          .def("get_current_time", &Simulator::get_current_time, 
               "Get the current simulation time\n\n"
               "Returns:\n"
               "    float: Current simulation timestamp")

          // logs
          .def("get_order_logs", &Simulator::get_order_logs,
               "Get the order logs\n\n"
               "Returns:\n"
               "    List[OrderLog]: List of all order log entries")

          .def("get_trade_logs", &Simulator::get_trade_logs,
               "Get the trade logs\n\n"
               "Returns:\n"
               "    List[TradeLog]: List of all trade log entries");

}