"""Type stubs for market_simulator C++ extension module."""

from enum import Enum
from typing import List

class OrderSide(Enum):
    """Enumeration for order side (buy or sell)"""
    BUY = 0
    SELL = 1

class OrderType(Enum):
    """Enumeration for order type"""
    LIMIT = 0
    MARKET = 1

class OrderStatus(Enum):
    """Enumeration for order status"""
    PLACED = 0
    PARTIALLY_FILLED = 1
    FILLED = 2
    UNFILLED = 3
    CANCELED = 4

class PriceLevel:
    """Price level in the order book"""
    price: float
    """Price at this level"""
    total_quantity: int
    """Total quantity at this price"""
    order_count: int
    """Number of orders at this price"""

class Level1Data:
    """Top-of-book market data snapshot"""
    bid_price: float
    """Best bid price"""
    bid_quantity: int
    """Total quantity at best bid"""
    ask_price: float
    """Best ask price"""
    ask_quantity: int
    """Total quantity at best ask"""
    mid_price: float
    """Mid price between best bid and ask"""
    spread: float
    """Bid-ask spread"""
    timestamp: int
    """Timestamp of the data snapshot"""

class Level2Data:
    """Level 2 market data snapshot"""
    timestamp: int
    """Timestamp of the data snapshot"""
    bids: List[PriceLevel]
    """List of bid levels (sorted descending)"""
    asks: List[PriceLevel]
    """List of ask levels (sorted ascending)"""

class OrderBookSnapshot:
    """Full order book snapshot"""
    timestamp: int
    """Timestamp of the snapshot"""
    bids: List[PriceLevel]
    """List of bid levels (sorted descending)"""
    asks: List[PriceLevel]
    """List of ask levels (sorted ascending)"""
    best_bid: float
    """Best bid price"""
    best_ask: float
    """Best ask price"""
    mid_price: float
    """Mid price"""
    spread: float
    """Bid-ask spread"""
    total_bid_volume: int
    """Total bid volume"""
    total_ask_volume: int
    """Total ask volume"""

class Order:
    """Order structure"""
    order_id: int
    """Unique identifier for the order"""
    trader_id: int
    """Identifier of the trader"""
    price: float
    """Order price"""
    quantity: int
    """Order quantity"""
    side: OrderSide
    """Order side (BUY or SELL)"""
    type: OrderType
    """Order type (LIMIT or MARKET)"""
    timestamp: int
    """Timestamp when order was created"""

class OrderLog:
    """Order log entry"""
    order_id: int
    """Unique identifier for the order"""
    trader_id: int
    """Identifier of the trader"""
    price: float
    """Order price"""
    quantity: int
    """Order quantity"""
    side: OrderSide
    """Order side (BUY or SELL)"""
    type: OrderType
    """Order type (LIMIT or MARKET)"""
    status: OrderStatus
    """Current status of the order"""
    timestamp: int
    """Timestamp of the log entry"""
    details: str
    """Additional details about the order event"""

class Trade:
    """Trade structure"""
    trade_id: int
    """Unique identifier for the trade"""
    buy_order_id: int
    """Buy order ID"""
    sell_order_id: int
    """Sell order ID"""
    aggressor_side: OrderSide
    """Which side was the aggressor"""
    buyer_id: int
    """Buyer trader ID"""
    seller_id: int
    """Seller trader ID"""
    price: float
    """Execution price"""
    quantity: int
    """Execution quantity"""
    timestamp: int
    """Timestamp of the trade"""

class PendingOrder:
    """Structure representing a pending order"""
    order_id: int
    """Unique identifier for the order"""
    trader_id: int
    """Identifier of the trader placing the order"""
    price: float
    """Limit price for the order"""
    quantity: int
    """Number of shares/contracts"""
    side: OrderSide
    """Order side (BUY or SELL)"""
    
    def __init__(
        self,
        order_id: int,
        trader_id: int,
        price: float,
        quantity: int,
        side: OrderSide
    ) -> None:
        """
        Create a pending limit order
        
        Args:
            order_id: Unique identifier for the order
            trader_id: Identifier of the trader placing the order
            price: Limit price for the order
            quantity: Number of shares/contracts
            side: BUY or SELL
        """
        ...

class PendingMarketOrder:
    """Structure representing a pending market order"""
    order_id: int
    """Unique identifier for the order"""
    trader_id: int
    """Identifier of the trader placing the order"""
    quantity: int
    """Number of shares/contracts"""
    side: OrderSide
    """Order side (BUY or SELL)"""
    
    def __init__(
        self,
        order_id: int,
        trader_id: int,
        quantity: int,
        side: OrderSide
    ) -> None:
        """
        Create a pending market order
        
        Args:
            order_id: Unique identifier for the order
            trader_id: Identifier of the trader placing the order
            quantity: Number of shares/contracts
            side: BUY or SELL
        """
        ...

class Simulator:
    """Order book market simulator"""
    
    def __init__(self, start_time: int = 0) -> None:
        """
        Initialize the simulator with an optional start time
        
        Args:
            start_time: Simulation start timestamp (default is 0)
        """
        ...
    
    def place_limit_order(self, pending_order: PendingOrder) -> None:
        """
        Place a limit order into the order book
        
        Args:
            pending_order: The pending limit order to place
        """
        ...
    
    def place_market_order(self, pending_market_order: PendingMarketOrder) -> None:
        """
        Place a market order into the order book
        
        Args:
            pending_market_order: The pending market order to place
        """
        ...
    
    def get_all_trader_orders(self, trader_id: int) -> List[Order]:
        """
        Get all orders for a specific trader
        
        Args:
            trader_id: Identifier of the trader
            
        Returns:
            List of all orders placed by the trader
        """
        ...
    
    def cancel_order(self, order_id: int) -> None:
        """
        Cancel an existing order
        
        Args:
            order_id: Unique identifier of the order to cancel
        """
        ...
    
    def modify_order(self, order_id: int, new_price: float, new_quantity: int) -> None:
        """
        Modify an existing order's price and/or quantity
        
        Args:
            order_id: Unique identifier of the order to modify
            new_price: New price for the order
            new_quantity: New quantity for the order
        """
        ...
    
    def submit_pending_orders(self) -> None:
        """
        Submit all pending orders to the order book
        
        Processes queued orders and matches them against the book
        """
        ...
    
    def get_current_level1_data(self) -> Level1Data:
        """
        Get top of book data
        
        Returns:
            Current best bid, ask, mid price, and spread
        """
        ...
    
    def get_current_level2_data(self) -> Level2Data:
        """
        Get Level 2 market data
        
        Returns:
            Current order book depth data
        """
        ...
    
    def get_current_snapshot(self) -> OrderBookSnapshot:
        """
        Get full order book snapshot
        
        Returns:
            Current full order book state
        """
        ...
    
    def advance_time(self, dt: int) -> None:
        """
        Advance simulation time by dt
        
        Args:
            dt: Time increment to advance
        """
        ...
    
    def get_current_time(self) -> int:
        """
        Get the current simulation time
        
        Returns:
            Current simulation timestamp
        """
        ...
    
    def get_order_logs(self) -> List[OrderLog]:
        """
        Get the order logs
        
        Returns:
            List of all order log entries
        """
        ...
    
    def get_trade_logs(self) -> List[Trade]:
        """
        Get the trade logs
        
        Returns:
            List of all trade log entries
        """
        ...
