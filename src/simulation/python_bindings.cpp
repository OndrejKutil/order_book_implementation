#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// -----------------------------------------------------------------------------
// Python Bindings
// -----------------------------------------------------------------------------
PYBIND11_MODULE(market_simulator, m) {
    m.doc() = "Order Book Simulator Plugin"; // Optional module docstring

    // 1. Expose the OrderSide enum
    // This allows using market_simulator.OrderSide.BUY in Python
    py::enum_<OrderSide>(m, "OrderSide")
        .value("BUY", OrderSide::BUY)
        .value("SELL", OrderSide::SELL)
        .export_values();

    // 2. Expose the Level1Data structure
    // We use def_readonly to make the fields accessible but not modifiable from Python
    py::class_<Level1Data>(m, "Level1Data")
        .def_readonly("bid_price", &Level1Data::bid_price)
        .def_readonly("bid_quantity", &Level1Data::bid_quantity)
        .def_readonly("ask_price", &Level1Data::ask_price)
        .def_readonly("ask_quantity", &Level1Data::ask_quantity)
        .def_readonly("mid_price", &Level1Data::mid_price)
        .def_readonly("spread", &Level1Data::spread)
        .def_readonly("timestamp", &Level1Data::timestamp)
        .def("__repr__", [](const Level1Data &a) {
            return "<Level1Data bid=" + std::to_string(a.bid_price) + 
                   " ask=" + std::to_string(a.ask_price) + ">";
        });

    // 3. Expose the Simulator class
    py::class_<Simulator>(m, "Simulator")
        .def(py::init<>()) // Default constructor
        .def("place_limit_order", &Simulator::place_limit_order, 
             "Place a limit order into the order book",
             py::arg("order_id"), py::arg("trader_id"), py::arg("price"), py::arg("quantity"), py::arg("side"))
        .def("get_current_level1_data", &Simulator::get_current_level1_data, "Get top of book data")
        .def("advance_time", &Simulator::advance_time, "Advance simulation time by dt");
}