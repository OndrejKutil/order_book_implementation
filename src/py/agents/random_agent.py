from .agent import Agent
import random
from helper.data_types import Orders, MarketData
from typing import Any, Union, List
from market_simulator import PendingMarketOrder, PendingOrder
from market_simulator import OrderSide, OrderType


# ==============================================================
# Random Agent Implementation
# ==============================================================

class RandomAgent(Agent):
    def __init__(self, name: str, id: int, id_range: range) -> None:
        super().__init__(name, id, id_range)

        self.min_price : float = 0.01
        self.mid_price : float = self.min_price
        self.trading_probability : float = 0.35  # 35% chance to trade each update cycle

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
                order_type : OrderType = random.choice([OrderType.MARKET, OrderType.LIMIT])
                quantity : int = random.randint(1, 10)
                price : float = max(self.min_price, self.mid_price * (1 + random.uniform(-0.05, 0.05)))

                if order_type == OrderType.MARKET:
                    order : Union[PendingMarketOrder, PendingOrder] = PendingMarketOrder(
                        order_id=self.get_new_id(),
                        trader_id=self.id,
                        quantity=quantity,
                        side=order_side
                    )
                else:
                    order : Union[PendingMarketOrder, PendingOrder] = PendingOrder(
                        order_id=self.get_new_id(),
                        trader_id=self.id,
                        quantity=quantity,
                        side=order_side,
                        price=price
                    )
                self.record_trade(order)

    # Submit the trades to the market
    def submit_trades(self) -> Orders:
        return Orders(orders=self.submitted_trades.copy())