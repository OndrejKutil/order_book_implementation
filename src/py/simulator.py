import market_simulator
from helper.orders import place_orders

sim = market_simulator.Simulator(start_time = 20)

run_time = 100  # Total simulation time
time_step = 1   # Time step for each iteration

agents = [

]

def main():

    while sim.get_current_time() < run_time:

        level1_data = sim.get_current_level1_data()

        for agent in agents:
            
            # Let each agent decide on orders based on current market data

            # Get orders from agent and place them

            # place_orders(sim, orders)

            ...
        
        sim.submit_pending_orders()
        sim.advance_time(time_step)