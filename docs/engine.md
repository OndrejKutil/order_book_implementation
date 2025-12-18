# Order Book Engine Documentation

Hey there! 👋 This document breaks down how the Order Book Engine works. I've written this to help you get up to speed quickly, whether you're debugging a trade mismatch or adding a new feature.

## Overview

At its core, this engine is a **Limit Order Book (LOB)**. It matches buyers and sellers based on a **Price-Time Priority** algorithm. This is the standard way most financial exchanges work.

Think of it like a queue at a store, but with a twist:
1.  **Price Priority:** People willing to pay more (buyers) or sell for less (sellers) get to cut the line.
2.  **Time Priority:** If two people offer the exact same price, the one who got there first gets served first.

## Core Architecture

The engine is built around the `OrderBook` class found in `engine/order_book.hpp`. It's designed to be a self-contained simulation unit—you feed it orders, and it spits out trades and market data.

### Data Structures (The "Secret Sauce")

We use a specific combination of C++ STL containers to balance speed and simplicity.

1.  **The Book (`buy_orders` & `sell_orders`)**:
    *   **Structure:** `std::map<Price, std::vector<Order>>`
    *   **Why?** `std::map` keeps our prices sorted automatically.
        *   For **Bids (Buys)**, we use `std::greater<Price>` so the *highest* price is at the top (`begin()`).
        *   For **Asks (Sells)**, we use `std::less<Price>` (default) so the *lowest* price is at the top.
    *   **Inside the Map:** The value is a `std::vector<Order>`. This represents the queue of orders at that specific price level. We treat it like a FIFO (First-In-First-Out) queue to enforce time priority.

2.  **The Index (`order_index`)**:
    *   **Structure:** `std::map<OrderID, std::pair<Price, OrderSide>>`
    *   **Why?** If a user wants to cancel `Order #123`, we don't want to search the entire book for it. This index gives us O(1) (or technically O(log N)) lookup to find exactly which price level and side the order is sitting on.

## Key Features

### 1. Order Matching
When a new **Limit Order** comes in, the engine checks if it can be matched immediately (this is called "crossing the spread").
*   **Aggressive Orders:** If a buy order comes in at $100, and there's a sell order sitting there for $99, we match them immediately at $99 (the resting price).
*   **Passive Orders:** If the buy order is for $98, and the cheapest seller is $99, the order just sits in the book waiting for a seller.

**Market Orders** are aggressive by definition—they take whatever price is available until they are filled or the book is empty.

### 2. Market Data Snapshots
We can pull the state of the market at any timestamp.
*   **Level 1 Data:** Just the best bid and best ask (the "Top of Book"). Useful for simple tickers.
*   **Level 2 Data (Snapshots):** The full depth of the book. Shows all price levels and volumes. Great for visualizing the market depth chart.

### 3. Logging & Audit
We don't just delete orders when they match. We keep a paper trail:
*   **`order_logs`**: Tracks every lifecycle event (PLACED, FILLED, CANCELED).
*   **`trade_logs`**: Records every successful trade, including who the buyer/seller was and who was the "aggressor" (the one who initiated the trade).

## How to Use It

Here is a quick snippet of how you might drive the engine in a test or simulation:

```cpp
#include "engine/order_book.hpp"

// 1. Create the book
OrderBook book;

// 2. Create an order
Order my_buy_order = {
    .order_id = 1,
    .trader_id = 101,
    .price = 150.0,
    .quantity = 10,
    .side = OrderSide::BUY,
    .type = OrderType::LIMIT,
    .timestamp = 1000
};

// 3. Place it
book.place_limit_order(my_buy_order);

// 4. Check the market
Level1Data top = book.get_level1_data();
// top.bid_price should now be 150.0
```
