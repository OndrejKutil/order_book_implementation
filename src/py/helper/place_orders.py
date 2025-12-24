import market_simulator
from helper.data_types import Orders

def place_orders(sim : market_simulator.Simulator, orders : Orders):
    for order in orders.orders:
        if isinstance(order, market_simulator.PendingOrder):
            sim.place_limit_order(order)
        elif isinstance(order, market_simulator.PendingMarketOrder):
            sim.place_market_order(order)
        else:
            raise ValueError("Unknown order type")
       