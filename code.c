#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_PRODUCTS 100
#define MAX_ITEMS 50
#define TAX_RATE 17

struct Product {
    char barcode[21];
    char name[41];
    float price;
};

struct Product products[MAX_PRODUCTS];
int productCount = 0;


void saveProducts() {
    FILE *fp = fopen("products.txt", "w");
    int i;

    if (fp == NULL) {
        printf("Could not save products.\n");
        return;
    }
    for (i = 0; i < productCount; i++) {
        fprintf(fp, "%s|%s|%.2f\n", products[i].barcode, products[i].name, products[i].price);
    }
    fclose(fp);
}


void loadProducts() {
    FILE *fp = fopen("products.txt", "r");
    char line[120];

    if (fp == NULL) {
        strcpy(products[0].barcode, "6281001000012");
        strcpy(products[0].name, "Milk 1 Litre");
        products[0].price = 290;

        strcpy(products[1].barcode, "6281001000029");
        strcpy(products[1].name, "Bread (Large)");
        products[1].price = 180;

        strcpy(products[2].barcode, "6281001000036");
        strcpy(products[2].name, "Eggs (Dozen)");
        products[2].price = 420;

        strcpy(products[3].barcode, "6281001000043");
        strcpy(products[3].name, "Basmati Rice 5 Kg");
        products[3].price = 1850;

        strcpy(products[4].barcode, "6281001000050");
        strcpy(products[4].name, "Cooking Oil 1 L");
        products[4].price = 620;

        productCount = 5;
        saveProducts();
        return;
    }
    
    while (productCount < MAX_PRODUCTS && fgets(line, sizeof(line), fp) != NULL) {
        int got = sscanf(line, "%20[^|]|%40[^|]|%f",
                         products[productCount].barcode,
                         products[productCount].name,
                         &products[productCount].price);
        if (got == 3) {
            productCount++;
        }
    }
    fclose(fp);
}


int findProduct(char code[]) {
    int i;
    for (i = 0; i < productCount; i++) {
        if (strcmp(products[i].barcode, code) == 0) {
            return i;
        }
    }
    return -1;
}

void listProducts() {
    int i;
    printf("\n%-15s %-25s %10s\n", "Barcode", "Item", "Price");
    for (i = 0; i < productCount; i++) {
        printf("%-15s %-25s %10.2f\n", products[i].barcode, products[i].name, products[i].price);
    }
}

void addProduct() {
    struct Product p;

    if (productCount >= MAX_PRODUCTS) {
        printf("Database is full.\n");
        return;
    }

    printf("Enter barcode: ");
    scanf("%20s", p.barcode);

    if (findProduct(p.barcode) != -1) {
        printf("This barcode already exists.\n");
        return;
    }

    printf("Enter item name: ");
    scanf(" %40[^\n]", p.name);
    printf("Enter price: ");
    scanf("%f", &p.price);

    if (p.price <= 0) {
        printf("Price must be greater than 0.\n");
        return;
    }

    products[productCount] = p;
    productCount++;
    saveProducts();
    printf("Product added.\n");
}

void updateProduct() {
    char code[21];
    int index;
    float newPrice;

    printf("Enter barcode to update: ");
    scanf("%20s", code);

    index = findProduct(code);
    if (index == -1) {
        printf("Product not found.\n");
        return;
    }

    printf("Current price of %s is %.2f\n", products[index].name, products[index].price);
    printf("Enter new price: ");
    scanf("%f", &newPrice);

    if (newPrice <= 0) {
        printf("Invalid price.\n");
        return;
    }

    products[index].price = newPrice;
    saveProducts();
    printf("Price updated.\n");
}

void deleteProduct() {
    char code[21];
    int index, i;

    printf("Enter barcode to delete: ");
    scanf("%20s", code);

    index = findProduct(code);
    if (index == -1) {
        printf("Product not found.\n");
        return;
    }

  
    for (i = index; i < productCount - 1; i++) {
        products[i] = products[i + 1];
    }
    productCount--;
    saveProducts();
    printf("Product deleted.\n");
}


int getInvoiceNumber() {
    int number = 1000;
    FILE *fp = fopen("counter.txt", "r");

    if (fp != NULL) {
        fscanf(fp, "%d", &number);
        fclose(fp);
    }

    number++;

    fp = fopen("counter.txt", "w");
    if (fp != NULL) {
        fprintf(fp, "%d", number);
        fclose(fp);
    }
    return number;
}


void printInvoice(FILE *out, int number, char date[], char customer[],
                  int itemIndex[], int itemQty[], int itemCount, float discountPercent) {
    int i;
    float subtotal = 0, total, discount, tax;

    fprintf(out, "\n================================================\n");
    fprintf(out, "                    INVOICE\n");
    fprintf(out, "================================================\n");
    fprintf(out, "Invoice #: %d      Date: %s\n", number, date);
    fprintf(out, "Customer : %s\n", customer);
    fprintf(out, "------------------------------------------------\n");
    fprintf(out, "%-22s %4s %9s %10s\n", "Item", "Qty", "Price", "Total");

    for (i = 0; i < itemCount; i++) {
        int idx = itemIndex[i];
        total = products[idx].price * itemQty[i];
        subtotal = subtotal + total;
        fprintf(out, "%-22.22s %4d %9.2f %10.2f\n",
                products[idx].name, itemQty[i], products[idx].price, total);
    }

    discount = subtotal * discountPercent / 100;
    tax = (subtotal - discount) * TAX_RATE / 100;

    fprintf(out, "------------------------------------------------\n");
    fprintf(out, "%-37s %10.2f\n", "Subtotal", subtotal);
    fprintf(out, "%-37s %10.2f\n", "Discount", -discount);
    fprintf(out, "%-37s %10.2f\n", "Tax (17%)", tax);
    fprintf(out, "%-37s %10.2f\n", "GRAND TOTAL", subtotal - discount + tax);
    fprintf(out, "================================================\n");
}

void createInvoice() {
    int itemIndex[MAX_ITEMS];
    int itemQty[MAX_ITEMS];
    int itemCount = 0;
    char customer[41];
    char code[21];
    int index, qty, i, found;
    float discount;
    int number;
    char date[20];
    time_t now;
    FILE *fp;

    printf("Customer name (type - for walk-in): ");
    scanf(" %40[^\n]", customer);
    if (strcmp(customer, "-") == 0) {
        strcpy(customer, "Walk-in Customer");
    }

    printf("Enter barcodes one by one. Type done to finish.\n");

    while (itemCount < MAX_ITEMS) {
        printf("\nBarcode: ");
        scanf("%20s", code);

        if (strcmp(code, "done") == 0) {
            break;
        }

        index = findProduct(code);
        if (index == -1) {
            printf("Barcode not found.\n");
            continue;
        }

        printf("%s - %.2f\n", products[index].name, products[index].price);
        printf("Quantity: ");
        scanf("%d", &qty);

        if (qty <= 0) {
            printf("Invalid quantity.\n");
            continue;
        }

        
        found = -1;
        for (i = 0; i < itemCount; i++) {
            if (itemIndex[i] == index) {
                found = i;
            }
        }

        if (found != -1) {
            itemQty[found] = itemQty[found] + qty;
        } else {
            itemIndex[itemCount] = index;
            itemQty[itemCount] = qty;
            itemCount++;
        }
    }

    if (itemCount == 0) {
        printf("No items added. Invoice cancelled.\n");
        return;
    }

    printf("Discount %% (0 for none): ");
    scanf("%f", &discount);
    if (discount < 0 || discount > 100) {
        discount = 0;
    }

    number = getInvoiceNumber();
    now = time(NULL);
    strftime(date, sizeof(date), "%d-%m-%Y %H:%M", localtime(&now));

    printInvoice(stdout, number, date, customer, itemIndex, itemQty, itemCount, discount);

    fp = fopen("invoices.txt", "a");
    if (fp != NULL) {
        printInvoice(fp, number, date, customer, itemIndex, itemQty, itemCount, discount);
        fclose(fp);
        printf("\nInvoice saved in invoices.txt\n");
    }
}

void viewInvoices() {
    FILE *fp = fopen("invoices.txt", "r");
    int ch;

    if (fp == NULL) {
        printf("No saved invoices yet.\n");
        return;
    }

    while ((ch = fgetc(fp)) != EOF) {
        printf("%c", ch);
    }
    fclose(fp);
}

int main() {
    int choice;

    loadProducts();

    do {
        printf("\n===== INVOICE SYSTEM =====\n");
        printf("1. Create invoice\n");
        printf("2. View saved invoices\n");
        printf("3. List products\n");
        printf("4. Add product\n");
        printf("5. Update product price\n");
        printf("6. Delete product\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createInvoice(); break;
            case 2: viewInvoices(); break;
            case 3: listProducts(); break;
            case 4: addProduct(); break;
            case 5: updateProduct(); break;
            case 6: deleteProduct(); break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
