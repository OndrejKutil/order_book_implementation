from typing import List, Union
from pydantic import BaseModel
import market_simulator


class Orders(BaseModel):
    orders: List[Union[market_simulator.PendingOrder, market_simulator.PendingMarketOrder]]

def place_orders(sim : market_simulator.Simulator, orders : Orders):
    for order in orders.orders:
        if isinstance(order, market_simulator.PendingOrder):
            sim.place_limit_order(order)
        elif isinstance(order, market_simulator.PendingMarketOrder):
            sim.place_market_order(order)
        else:
            raise ValueError("Unknown order type")