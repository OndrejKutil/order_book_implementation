from typing import List, Optional, Union

from pydantic import BaseModel, ConfigDict
import market_simulator


class Orders(BaseModel):
    model_config = ConfigDict(arbitrary_types_allowed=True)
    orders: List[Union[market_simulator.PendingOrder, market_simulator.PendingMarketOrder]] 

class MarketData(BaseModel):
    model_config = ConfigDict(arbitrary_types_allowed=True)
    snapshot: Optional[market_simulator.OrderBookSnapshot]
    level1_data: Optional[market_simulator.Level1Data]
    level2_data: Optional[market_simulator.Level2Data]