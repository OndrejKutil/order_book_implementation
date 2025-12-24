from typing import Any, Set, Dict, List, Union
from abc import ABC, abstractmethod
from helper.data_types import Orders, MarketData

# ==============================================================
# Base Agent Class
# ==============================================================

class Agent(ABC):
    def __init__(self, name: str, id: int, id_range: range) -> None:
        self.name : str = name
        self.id : int = id
        self.id_range : range = id_range
        self.used_ids : Set[int] = set()

        self.data : MarketData = MarketData(snapshot=None, level1_data=None, level2_data=None)
        self.submitted_trades : List[Any] = []
        self.trade_history : List[Any] = []


    @abstractmethod
    def update(self, market_data: MarketData) -> None:
        """Update the agent's state based on market data."""
        pass

    @abstractmethod
    def decide_trades(self) -> None:
        """Decide on trades to execute based on the agent's strategy."""
        pass

    def record_trade(self, trade: Any) -> None:
        """Record a submitted trade."""
        self.submitted_trades.append(trade)
        self.trade_history.append(trade)

    def get_new_id(self) -> int:
        """Generate a new unique ID for the agent within the specified range."""
        for new_id in self.id_range:
            if new_id not in self.used_ids:
                self.used_ids.add(new_id)
                return new_id
        raise ValueError("No available IDs in the specified range.")

    @abstractmethod
    def submit_trades(self) -> Orders:
        """Submit the decided trades to the market."""
        pass