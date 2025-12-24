#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASH_SIZE 10

typedef struct MenuItem {
    char name[50];
    float price;
    struct MenuItem *next;
} MenuItem;

typedef struct Restaurant {
    char id[10];
    char name[50];
    MenuItem *menu;
    struct Restaurant *next;  // for hash collision handling
} Restaurant;

// Hash table for restaurants
Restaurant* hashTable[HASH_SIZE];

// Order data structures
typedef struct OrderItem {
    char name[50];
    int qty;
    float price;
    struct OrderItem *next;
} OrderItem;

typedef struct Order {
    int orderId;
    char customerName[50];
    char address[100];
    char restaurantId[10];
    OrderItem *items;
    float total;
    struct Order *next;
} Order;

Order *queueFront = NULL;
Order *queueRear = NULL;
int nextOrderId = 1;

// Utility functions
int hashFunction(const char *key) {

    unsigned int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = (hash * 31 + key[i]) % HASH_SIZE;
    }
    return hash;
}

Restaurant* createRestaurant(const char* id, const char* name) {
    Restaurant* r = (Restaurant*)malloc(sizeof(Restaurant));
    strncpy(r->id, id, sizeof(r->id));
    r->id[sizeof(r->id)-1] = '\0';
    strncpy(r->name, name, sizeof(r->name));
    r->name[sizeof(r->name)-1] = '\0';
    r->menu = NULL;
    r->next = NULL;
    return r;
}

void addRestaurant(const char* id, const char* name) {
    int idx = hashFunction(id);
    Restaurant* r = createRestaurant(id, name);
    // Insert at head of chain
    r->next = hashTable[idx];
    hashTable[idx] = r;
    printf("Added restaurant\nID = %s Name = %s\n", id, name);
}

Restaurant* findRestaurant(const char* id) {
    int idx = hashFunction(id);
    Restaurant* curr = hashTable[idx];
    while (curr) {
        if (strcmp(curr->id, id) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

MenuItem* createMenuItem(const char* name, float price) {
    MenuItem* m = (MenuItem*)malloc(sizeof(MenuItem));
    strncpy(m->name, name, sizeof(m->name));
    m->name[sizeof(m->name)-1] = '\0';
    m->price = price;
    m->next = NULL;
    return m;
}

void addMenuItem(const char* restId, const char* name, float price) {
    Restaurant* r = findRestaurant(restId);
    if (!r) {
        printf("Restaurant id %s not found\n", restId);
        return;
    }
    MenuItem* m = createMenuItem(name, price);
    // insert at end of menu list
    if (!r->menu) {
        r->menu = m;
    } else {
        MenuItem* cur = r->menu;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = m;
    }
    printf("Added menu item %s (%.2f) to restaurant id %s\n", name, price, restId);
}

void displayRestaurants() {
    printf("Restaurants and IDs:\n");
    for (int i = 0; i < HASH_SIZE; i++) {
        Restaurant* curr = hashTable[i];
        while (curr) {
            printf("ID: %s, Name: %s\n", curr->id, curr->name);
            curr = curr->next;
        }
    }
}

void displayMenu(const char *restId) {
    Restaurant* r = findRestaurant(restId);
    if (!r) {
        printf("Restaurant id %s not found\n", restId);
        return;
    }
    printf("Menu for %s (ID %s):\n", r->name, r->id);
    MenuItem* cur = r->menu;
    int i = 1;
    while (cur) {
        printf(" %d. %s - %.2f\n", i, cur->name, cur->price);
        cur = cur->next;
        i++;
    }
}

Order* createOrder(const char* customerName, const char* address, const char* restaurantId) {
    Order* o = (Order*)malloc(sizeof(Order));
    o->orderId = nextOrderId++;
    strncpy(o->customerName, customerName, sizeof(o->customerName));
    o->customerName[sizeof(o->customerName)-1] = '\0';
    strncpy(o->address, address, sizeof(o->address));
    o->address[sizeof(o->address)-1] = '\0';
    strncpy(o->restaurantId, restaurantId, sizeof(o->restaurantId));
    o->restaurantId[sizeof(o->restaurantId)-1] = '\0';
    o->items = NULL;
    o->total = 0;
    o->next = NULL;
    return o;
}

OrderItem* createOrderItem(const char* name, int qty, float price) {
    OrderItem* oi = (OrderItem*)malloc(sizeof(OrderItem));
    strncpy(oi->name, name, sizeof(oi->name));
    oi->name[sizeof(oi->name)-1] = '\0';
    oi->qty = qty;
    oi->price = price;
    oi->next = NULL;
    return oi;
}

void enqueueOrder(Order* o) {
    if (!queueFront) {
        queueFront = queueRear = o;
    } else {
        queueRear->next = o;
        queueRear = o;
    }
    printf("Order %d added to queue\n", o->orderId);
}

Order* dequeueOrder() {
    if (!queueFront) {
        return NULL;
    }
    Order* o = queueFront;
    queueFront = queueFront->next;
    if (!queueFront) {
        queueRear = NULL;
    }
    o->next = NULL;
    return o;
}

void processNextOrder() {
    if (!queueFront) {
        printf("No pending orders to process.\n");
        return;
    }
    Order* o = dequeueOrder();
    printf("Processing Order %d for %s (Restaurant ID %s)\n", o->orderId, o->customerName, o->restaurantId);
    printf("Items:\n");
    OrderItem* cur = o->items;
    while (cur) {
        printf(" - %s x%d = %.2f\n", cur->name, cur->qty, cur->price * cur->qty);
        cur = cur->next;
    }
    printf("Total: %.2f\n", o->total);
    // Save to file
    FILE* file = fopen("orders.txt", "a");
    if (file) {
        fprintf(file, "OrderID: %d, Customer: %s, RestaurantID: %s, Total: %.2f, Address: %s\n", 
                o->orderId, o->customerName, o->restaurantId, o->total, o->address);
        OrderItem* oi = o->items;
        fprintf(file, "Items: ");
        while (oi) {
            fprintf(file, "%s x%d (%.2f); ", oi->name, oi->qty, oi->price * oi->qty);
            oi = oi->next;
        }
        fprintf(file, "\n\n");
        fclose(file);
    }
    // free order memory
    OrderItem* oi = o->items;
    while (oi) {
        OrderItem* temp = oi;
        oi = oi->next;
        free(temp);
    }
    free(o);
}

void displayPendingOrders() {
    printf("Pending Orders:\n");
    Order* cur = queueFront;
    while (cur) {
        printf(" OrderID %d for %s\n", cur->orderId, cur->customerName);
        cur = cur->next;
    }
}

// File I/O for restaurants
void loadRestaurants() {
    FILE* file = fopen("restaurants.txt", "r");
    if (!file) return;
    int count;
    if (fscanf(file, "%d", &count) != 1) { fclose(file); return; }
    for (int i = 0; i < count; i++) {
        char id[10];
        char name[50];
        if (fscanf(file, "%9s %49[^\n]", id, name) != 2) break;
        while (getchar()!='\n'); // cleanse any remaining input
        addRestaurant(id, name);
        int nitems;
        fscanf(file, "%d", &nitems);
        for (int j = 0; j < nitems; j++) {
            char itemName[50];
            float price;
            fscanf(file, "%49[^\n] %f", itemName, &price);
            while (getchar()!='\n');
            addMenuItem(id, itemName, price);
        }
    }
    fclose(file);
}

void saveRestaurants() {
    FILE* file = fopen("restaurants.txt", "w");
    if (!file) return;
    // count restaurants
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Restaurant* r = hashTable[i];
        while (r) { count++; r = r->next; }
    }
    fprintf(file, "%d\n", count);
    // write each restaurant and its menu
    for (int i = 0; i < HASH_SIZE; i++) {
        Restaurant* r = hashTable[i];
        while (r) {
            fprintf(file, "%s %s\n", r->id, r->name);
            // count menu items
            int mcount = 0;
            MenuItem* m = r->menu;
            while (m) { mcount++; m = m->next; }
            fprintf(file, "%d\n", mcount);
            m = r->menu;
            while (m) {
                fprintf(file, "%s %.2f\n", m->name, m->price);
                m = m->next;
            }
            r = r->next;
        }
    }
    fclose(file);
}

int main() {
    loadRestaurants();
    int choice;
    while (1) {
        printf("\nMenu:\n");
        printf("1. Display Restaurants\n");
        printf("2. Add Restaurant\n");
        printf("3. Add Menu Item to Restaurant\n");
        printf("4. Display Menu for Restaurant\n");
        printf("5. Place Order\n");
        printf("6. Process Next Order\n");
        printf("7. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            displayRestaurants();
        } else if (choice == 2) {
            char id[10], name[50];
            printf("Enter restaurant ID: ");
            scanf("%9s", id);
            while (getchar()!='\n');
            printf("Enter restaurant name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';
            addRestaurant(id, name);
        } else if (choice == 3) {
            char id[10], itemName[50];
            float price;
            printf("Enter restaurant ID: ");
            scanf("%9s", id);
            while (getchar()!='\n');
            printf("Enter item name: ");
            fgets(itemName, sizeof(itemName), stdin);
            itemName[strcspn(itemName, "\n")] = '\0';
            printf("Enter item price: ");
            scanf("%f", &price);
            while (getchar()!='\n');
            addMenuItem(id, itemName, price);
        } else if (choice == 4) {
            char id[10];
            printf("Enter restaurant ID: ");
            scanf("%9s", id);
            while (getchar()!='\n');
            displayMenu(id);
        } else if (choice == 5) {
            char customer[50], address[100], restId[10];
            int numItems;
            printf("Enter customer name: ");
            while (getchar()!='\n');
            fgets(customer, sizeof(customer), stdin);
            customer[strcspn(customer, "\n")] = '\0';

            printf("Enter delivery address: ");
            fgets(address, sizeof(address), stdin);
            address[strcspn(address, "\n")] = '\0';

            printf("Enter restaurant ID: ");
            scanf("%9s", restId);
            while (getchar()!='\n');

            Restaurant* r = findRestaurant(restId);
            if (!r) {
                printf("Restaurant not found.\n");
                continue;
            }
            displayMenu(restId);

            printf("Number of items to order: ");
            scanf("%d", &numItems);
            while (getchar()!='\n');

            Order* o = createOrder(customer, address, restId);
            for (int i = 0; i < numItems; i++) {
                int itemIndex, qty;
                printf("Enter item number and quantity (e.g. 1 2): ");
                scanf("%d %d", &itemIndex, &qty);
                while (getchar()!='\n');
                MenuItem* mi = r->menu;
                int idx = 1;
                while (mi && idx < itemIndex) {
                    mi = mi->next;
                    idx++;
                }
                if (!mi) {
                    printf("Invalid item number.\n");
                    continue;
                }
                OrderItem* oi = createOrderItem(mi->name, qty, mi->price);
                if (!o->items) {
                    o->items = oi;
                } else {
                    OrderItem* tmp = o->items;
                    while (tmp->next) tmp = tmp->next;
                    tmp->next = oi;
                }
                o->total += mi->price * qty;
            }
            enqueueOrder(o);
        } else if (choice == 6) {
            processNextOrder();
        } else if (choice == 7) {
            saveRestaurants();
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
    return 0;
}
