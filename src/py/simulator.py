import market_simulator
from helper.data_types import Orders, MarketData
from helper.place_orders import place_orders
from typing import List
from agents import (random_agent)
from agents.agent import Agent

# ==============================================================
# Simulation Setup
# ==============================================================

# Initialize the market simulator
sim : market_simulator.Simulator = market_simulator.Simulator(start_time = 20)

# Simulation parameters
run_time : int = 100  # Total simulation time
time_step : int = 1   # Time step for each iteration

# Initialize agents
agents : List[Agent] = [
    random_agent.RandomAgent(name="RandomAgent1", id=1, id_range=range(1000, 8000))
]

# ==============================================================
# Main Simulation Loop
# ==============================================================

def main():

    # Run the simulation loop
    while sim.get_current_time() < run_time:

        # Gather market data for each tick
        market_data : MarketData = MarketData(
            snapshot=sim.get_current_snapshot(),
            level1_data=sim.get_current_level1_data(),
            level2_data=sim.get_current_level2_data()
        )

        # Let each agent update their state and decide on trades
        for agent in agents:

            agent.update(market_data)
            agent.decide_trades()

            orders : Orders = agent.submit_trades()

            # Place orders into the simulator queue
            place_orders(sim, orders)
        
        # Let simulator process orders (randomly chooses which trader to process first)
        sim.submit_pending_orders()
        sim.advance_time(time_step)

    order_logs : List[market_simulator.OrderLog] = sim.get_order_logs()
    trade_logs : List[market_simulator.TradeLog] = sim.get_trade_logs()

    print("Simulation completed.")
    print(f"Total Orders Processed: {len(order_logs)}")
    print(f"Total Trades Executed: {len(trade_logs)}")

    print("Order Logs Sample:", order_logs[:5])
    print("Sample full first trade log:")
    print(trade_logs[0].to_dict() if trade_logs else "No trades executed.")

if __name__ == "__main__":
    main()