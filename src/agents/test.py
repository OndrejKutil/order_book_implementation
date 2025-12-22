import sys
import os

# Ensure we can import the module if running from project root
sys.path.append(os.path.join(os.getcwd(), 'src'))

import market_simulator

sim = market_simulator.Simulator()

sim.place_limit_order(order_id = 1 , trader_id = 1, price = 100.0, quantity = 10, side = market_simulator.OrderSide.BUY)

data = sim.get_current_level1_data()
print(data)