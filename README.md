# Project Overview

## Description

This is the repository of the OMG, or Olin Marketplace for Goods. It is a terminal-based online market trading platform that allows multiple users to remotely trade any virtual commodities such as Bitcoin, Etheruem, or Dogecoin. Users can post buy/sell orders, view current market demand/supplies of any selected goods, and view trading history.

It features a lightweight terminal user interface to remove any fancy and chaotic visuals in a typical trading website, as well as offering easy access to performing trading operations through short command line instructions. Therefore, Olin Marketplace for Goods platform supports different OS platforms and can handle multiple connections concurrently.

## Dependencies

Olin Marketplace for Goods does not require many packages or complicated dependencies. The only required dependency is the SQLite3 database library for C.

To install SQLite3 databse library for C, follow the following instructions:

Linux users:

```bash
# for Linux
sudo apt update
sudo apt install sqlite3 libsqlite3-dev
```

MacOS users:

```bash
# install via Homebrew
brew install sqlite
```

Windows users:

Option 1: Download Precompiled Binaries

1. Visit the official SQLite download page:  
   https://www.sqlite.org/download.html

2. Download the following files under **“Precompiled Binaries for Windows”**:

   - `sqlite3.c` – amalgamation source file
   - `sqlite3.h` – header file
   - _(Optional)_ `sqlite3ext.h` – only if writing custom SQLite extensions

3. Place all files in your C project folder.

Option 2: Use a Package Manager

```bash
# Using MSYS2
pacman -S mingw-w64-x86_64-sqlite3
```

Verify installation with the following command:

```bash
sqlite3 --version
```

## Instructions

To run Olin Marketplace for Goods:

Navigate to `src/` and build the project. The user should then run the executable `run_server` in the 
`build/src` directory, and the server should start. From there, end users can simply use telnet to connect
to the server that needs to be on local network. The default port is 4242.

## Features

### Coins Available

The platform uses the default OMG coins for trading any online commodities.

- **OMG**: Olin Marketplace for Goods coin, the primary currency.

The platform currently only supports 3 types of online commodities as listed below. More will come in the future:

- **DOGE**: Dogecoin, a popular cryptocurrency.
- **BTC**: Bitcoin, the first and most widely recognized cryptocurrency.
- **ETH**: Ethereum, a decentralized platform and cryptocurrency.

### User Commands

#### 🔍 `myInventory`

Lists the inventory for the current user.

---

#### 💸 `buy <item> <price> <quantity>`

Posts a **buy order**.

- **item**: The name of the commodity to buy.
- **price**: The unit price the client is willing to buy at.
- **quantity**: The number of commodities to buy.

---

#### 💰 `sell <item> <price> <quantity>`

Posts a **sell order**.

- **item**: The name of the commodity to sell.
- **price**: The unit price the client is willing to sell at.
- **quantity**: The number of commodities to sell.

---

#### 📜 `myOrders`

Lists all active buy/sell orders submitted by the current user.

- Returns the IDs of all active orders.

---

#### ❌ `cancelOrder <orderID>`

Cancels a buy/sell order.

- **orderID**: The ID of the order to cancel.

---

#### 📈 `view <item>`

Views the top 5 buy/sell orders for a specific item.

- **item**: The name of the item to check.

## Implementation Details

### Database Structure

`database.db` is the default SQLite database file name that has 3 tables total as follows:

#### Table 1 - `users`

Stores the inventories and info of all users.

| Column   | Type    | Description                       |
| -------- | ------- | --------------------------------- |
| userID   | INTEGER | Primary key, auto-incremented     |
| username | TEXT    | Username of the user              |
| password | TEXT    | Password of the user              |
| name     | TEXT    | Name of the user                  |
| OMG      | INTEGER | Quantity of OMG coin (default 0)  |
| DOGE     | INTEGER | Quantity of DOGE coin (default 0) |
| BTC      | INTEGER | Quantity of BTC coin (default 0)  |
| ETH      | INTEGER | Quantity of ETH coin (default 0)  |

#### Table 2 - `orders`

Stores information about all active orders in the market.

| Column     | Type     | Description                                                      |
| ---------- | -------- | ---------------------------------------------------------------- |
| orderID    | INTEGER  | Primary key, auto-incremented                                    |
| item       | INTEGER  | The item being bought or sold                                    |
| buyOrSell  | INTEGER  | 0 = buy, 1 = sell                                                |
| quantity   | INTEGER  | Quantity of the item                                             |
| unitPrice  | REAL     | Unit price of the item                                           |
| userID     | INTEGER  | ID of the user who placed the order                              |
| created_at | DATETIME | Timestamp when the order was placed (default: CURRENT_TIMESTAMP) |

#### Table 3 - `archives`

Stores information about all archived orders in the archives.

| Column     | Type     | Description                                                      |
| ---------- | -------- | ---------------------------------------------------------------- |
| orderID    | INTEGER  | Primary key, auto-incremented                                    |
| item       | INTEGER  | The item being bought or sold                                    |
| buyOrSell  | INTEGER  | 0 = buy, 1 = sell                                                |
| quantity   | INTEGER  | Quantity of the item                                             |
| unitPrice  | REAL     | Unit price of the item                                           |
| userID     | INTEGER  | ID of the user who placed the order                              |
| created_at | DATETIME | Timestamp when the order was placed (default: CURRENT_TIMESTAMP) |

### File Structure

- run_server.c
- server.c (Oscar)
- db.c (Rewa)
- commands.c (Jack)
