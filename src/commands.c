// TODO: discuss if we need structs for inventories and stuff
#include "db.h"

typedef struct {
    int itemID;
    char* name;
    int price;
    int quantity;
} item;

typedef struct {
    int orderID;
    int buyOrSell; // 0 for buy, 1 for sell
    int quantity;
    int userID;
} order;


// change to return struct item?? 
item* myInventory(){

}


// return -1 if unsuccessful
int buy(int itemID, int unitPrice, int quantity, int userID){

}

// return -1 if unsuccessful
int sell(int itemID, int unitPrice, int quantity, int userID){

}

// change to return struct orders??
order* myOrders(int* orderCount) {
    static order orders[100]; // Example: static array to hold orders
    *orderCount = 0; // Initialize order count
    // depends on how many orders from database
    return orders;
}

// change to return struct orders??
order* view(int itemID){

}