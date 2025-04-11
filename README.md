# C Project Repository Template

A template repository for a C project.

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
