#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "simulator.hpp"
#include "../order_book/types.hpp"

namespace py = pybind11;

// -----------------------------------------------------------------------------
// Python Bindings
// -----------------------------------------------------------------------------
PYBIND11_MODULE(market_simulator, m) {
     m.doc() = "Order Book Simulator Plugin"; // Optional module docstring

    // 1. Expose the OrderSide enum
    // This allows using market_simulator.OrderSide.BUY in Python
     py::enum_<OrderSide>(m, "OrderSide", "Enumeration for order side (buy or sell)")
         .value("BUY", OrderSide::BUY, "Buy order")
         .value("SELL", OrderSide::SELL, "Sell order")
         .export_values();

    // 2. Expose the Level1Data structure
    // We use def_readonly to make the fields accessible but not modifiable from Python
     py::class_<Level1Data>(m, "Level1Data", "Top-of-book market data snapshot")
          .def_readonly("bid_price", &Level1Data::bid_price, "Best bid price")
          .def_readonly("bid_quantity", &Level1Data::bid_quantity, "Total quantity at best bid")
          .def_readonly("ask_price", &Level1Data::ask_price, "Best ask price")
          .def_readonly("ask_quantity", &Level1Data::ask_quantity, "Total quantity at best ask")
          .def_readonly("mid_price", &Level1Data::mid_price, "Mid price between best bid and ask")
          .def_readonly("spread", &Level1Data::spread, "Bid-ask spread")
          .def_readonly("timestamp", &Level1Data::timestamp, "Timestamp of the data snapshot")
          .def("__repr__", [](const Level1Data &x) {
               return "<Level1Data timestamp=" + std::to_string(x.timestamp) + ">";
          });

     py::class_<Level2Data>(m, "Level2Data", "Level 2 market data snapshot")
          .def_readonly("bids", &Level2Data::bids, "List of bid levels (price, quantity)")
          .def_readonly("asks", &Level2Data::asks, "List of ask levels (price, quantity)")
          .def_readonly("timestamp", &Level2Data::timestamp, "Timestamp of the data snapshot")
          .def("__repr__", [](const Level2Data &x) {
               return "<Level2Data timestamp=" + std::to_string(x.timestamp) + ">";
          });

     py::class_<OrderBookSnapshot>(m, "OrderBookSnapshot", "Full order book snapshot")
          .def_readonly("bids", &OrderBookSnapshot::bids, "List of bid orders")
          .def_readonly("asks", &OrderBookSnapshot::asks, "List of ask orders")
          .def_readonly("timestamp", &OrderBookSnapshot::timestamp, "Timestamp of the snapshot")
          .def("__repr__", [](const OrderBookSnapshot &x) {
               return "<OrderBookSnapshot timestamp=" + std::to_string(x.timestamp) + ">";
          });

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
          });

     py::class_<PendingMarketOrder>(m, "PendingMarketOrder", "Structure representing a pending market order")
          .def(py::init<OrderID, TraderID, Quantity, OrderSide>(),
               py::arg("order_id"), py::arg("trader_id"), py::arg("quantity"), py::arg("side"))
          .def_readonly("order_id", &PendingMarketOrder::order_id, "Unique identifier for the order")
          .def_readonly("trader_id", &PendingMarketOrder::trader_id, "Identifier of the trader placing the order")
          .def_readonly("quantity", &PendingMarketOrder::quantity, "Number of shares/contracts")
          .def_readonly("side", &PendingMarketOrder::side, "Order side (BUY or SELL)")
          .def("__repr__", [](const PendingMarketOrder &x) {
              return "<PendingMarketOrder order_id=" + std::to_string(x.order_id) + ">";
          });

     // 3. Expose the Simulator class
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