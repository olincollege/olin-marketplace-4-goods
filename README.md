# C Project Repository Template

A template repository for a C project.

# Database Structure

`info.db` is the SQLite database that has 3 tables total as follows:

## Table 1 - `users`

Stores the inventories and info of all users.

| Column   | Type    | Description                      |
| -------- | ------- | -------------------------------- |
| userID   | INTEGER | Primary key, auto-incremented    |
| name     | TEXT    | Name of the user                 |
| coin_OMG | INTEGER | Quantity of OMG coin (default 0) |
| coin_A   | INTEGER | Quantity of Coin A (default 0)   |
| coin_B   | INTEGER | Quantity of Coin B (default 0)   |
| coin_C   | INTEGER | Quantity of Coin C (default 0)   |

## Table 2 - `orders`

Stores information about all active orders in the market.

| Column    | Type    | Description                         |
| --------- | ------- | ----------------------------------- |
| orderID   | INTEGER | Primary key, auto-incremented       |
| itemID    | INTEGER | ID of the item being bought or sold |
| buyOrSell | INTEGER | 0 = buy, 1 = sell                   |
| quantity  | INTEGER | Quantity of the item                |
| userID    | INTEGER | ID of the user who placed the order |

## Table 3 - `archives`

Stores information about all archived orders in the archives.

| Column    | Type    | Description                         |
| --------- | ------- | ----------------------------------- |
| orderID   | INTEGER | Primary key, auto-incremented       |
| itemID    | INTEGER | ID of the item being bought or sold |
| buyOrSell | INTEGER | 0 = buy, 1 = sell                   |
| quantity  | INTEGER | Quantity of the item                |
| userID    | INTEGER | ID of the user who placed the order |

# File Structure

- run_server.c
- server.c (Oscar)
- db.c (Rewa)
- commands.c (Jack)

# User Commands

## 📦 Commodity Trading CLI Commands

### 🔍 `myInventory`

Lists the inventory for the current user.

---

### 💸 `buy <item> <price> <quantity>`

Posts a **buy order**.

- **item**: The name of the commodity to buy.
- **price**: The unit price the client is willing to buy at.
- **quantity**: The number of commodities to buy.

---

### 💰 `sell <item> <price> <quantity>`

Posts a **sell order**.

- **item**: The name of the commodity to sell.
- **price**: The unit price the client is willing to sell at.
- **quantity**: The number of commodities to sell.

---

### 📜 `myOrders`

Lists all active buy/sell orders submitted by the current user.

- Returns the IDs of all active orders.

---

### ❌ `cancelOrder <orderID>`

Cancels a buy/sell order.

- **orderID**: The ID of the order to cancel.

---

### 📈 `view <item>`

Views the top 5 buy/sell orders for a specific item.

- **item**: The name of the item to check.
