from .agent import Agent
import random
from helper.data_types import Orders, MarketData
from market_simulator import PendingOrder
from market_simulator import OrderSide
import pprint


# ==============================================================
# Random Agent Implementation
# ==============================================================

# A simple agent that places random buy and sell orders
# Only uses limit orders to avoid complications with market orders

class RandomAgent(Agent):
    def __init__(self, name: str, id: int, id_range: range) -> None:
        super().__init__(name, id, id_range)

        self.min_price : float = 0.01
        self.mid_price : float = self.min_price
        self.trading_probability : float = 0.35  # 35% chance to trade each update cycle

        # Track own orders to prevent self-trading
        self.best_bid : float = 0.0  # Highest price we're willing to buy at
        self.best_ask : float = float('inf')  # Lowest price we're willing to sell at

    # Update internal market data
    def update(self, market_data: MarketData) -> None:
        # save market data
        self.data = market_data
        incoming_mid: float = self.data.snapshot.mid_price if self.data.snapshot else 0.0
        if incoming_mid > 0:
            self.mid_price = incoming_mid
        else:
            self.mid_price = max(self.mid_price, self.min_price)

    # Decide on random trades
    def decide_trades(self) -> None:

        # Check if we should trade this cycle
        if random.randrange(0, 100, 1) < self.trading_probability * 100:
            trade_count : int = random.randint(1, 5)

            for _ in range(trade_count):
                order_side : OrderSide = random.choice([OrderSide.BUY, OrderSide.SELL])
                quantity : int = random.randint(1, 10)
                price : float = max(self.min_price, self.mid_price * (1 + random.uniform(-0.05, 0.05)))
                
                # Prevent self-trading: sell above our best bid, buy below our best ask
                if order_side == OrderSide.SELL and self.best_bid > 0:
                    price = max(price, self.best_bid + 0.01)  # Sell at least 0.01 above our best bid
                elif order_side == OrderSide.BUY and self.best_ask < float('inf'):
                    price = min(price, self.best_ask - 0.01)  # Buy at most 0.01 below our best ask
                
                # Ensure price is always valid (at least min_price)
                price = max(price, self.min_price)

                order : PendingOrder = PendingOrder(
                    order_id=self.get_new_id(),
                    trader_id=self.id,
                    quantity=quantity,
                    side=order_side,
                    price=price
                )
                self.record_trade(order)
                
                # Update our best bid/ask based on the order we just placed
                if order_side == OrderSide.BUY:
                    self.best_bid = max(self.best_bid, price)  # Track highest buy price
                else:
                    self.best_ask = min(self.best_ask, price)  # Track lowest sell price

    # Submit the trades to the market
    def submit_trades(self) -> Orders:
        
        print()
        print("="*50)
        pprint.pprint([trade.to_dict() for trade in self.submitted_trades])
        print("="*50)
        print()

        return Orders(orders=self.submitted_trades.copy())
    