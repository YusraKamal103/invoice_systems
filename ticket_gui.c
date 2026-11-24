/* ticket_gui.c - simple Windows GUI for the invoice system.
   Uses the same files as ticket.c: products.txt, counter.txt, invoices.txt

   Build:  gcc ticket_gui.c -o ticket_gui.exe -mwindows
   (Dev-C++: Tools > Compiler Options > add  -mwindows  to the linker commands)
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
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

int itemIndex[MAX_ITEMS];
int itemQty[MAX_ITEMS];
int itemCount = 0;

/* ================= data code (same as ticket.c) ================= */

void saveProducts() {
    FILE *fp = fopen("products.txt", "w");
    int i;
    if (fp == NULL) return;
    for (i = 0; i < productCount; i++)
        fprintf(fp, "%s|%s|%.2f\n", products[i].barcode, products[i].name, products[i].price);
    fclose(fp);
}

void addDefault(char *code, char *name, float price) {
    strcpy(products[productCount].barcode, code);
    strcpy(products[productCount].name, name);
    products[productCount].price = price;
    productCount++;
}

void loadProducts() {
    FILE *fp = fopen("products.txt", "r");
    char line[120];

    if (fp == NULL) {
        addDefault("6281001000012", "Milk 1 Litre", 290);
        addDefault("6281001000029", "Bread (Large)", 180);
        addDefault("6281001000036", "Eggs (Dozen)", 420);
        addDefault("6281001000043", "Basmati Rice 5 Kg", 1850);
        addDefault("6281001000050", "Cooking Oil 1 L", 620);
        saveProducts();
        return;
    }
    while (productCount < MAX_PRODUCTS && fgets(line, sizeof(line), fp) != NULL) {
        int got = sscanf(line, "%20[^|]|%40[^|]|%f",
                         products[productCount].barcode,
                         products[productCount].name,
                         &products[productCount].price);
        if (got == 3) productCount++;
    }
    fclose(fp);
}

int findProduct(char code[]) {
    int i;
    for (i = 0; i < productCount; i++)
        if (strcmp(products[i].barcode, code) == 0) return i;
    return -1;
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

void printInvoice(FILE *out, int number, char date[], char customer[], float discountPercent) {
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
    fprintf(out, "%-37s %10.2f\n", "Tax (17%%)", tax);
    fprintf(out, "%-37s %10.2f\n", "GRAND TOTAL", subtotal - discount + tax);
    fprintf(out, "================================================\n");
}

/* ================= GUI code ================= */

/* an ID for every control, so we can find it later */
enum {
    ID_CUST = 100, ID_CODE, ID_QTY, ID_ADD, ID_ILIST, ID_REMOVE, ID_DISC,
    ID_TOTALS, ID_GEN, ID_CLEAR, ID_VIEW,
    ID_PLIST, ID_PB, ID_PN, ID_PP, ID_PADD, ID_PUPD, ID_PDEL
};

HWND hMain;
HFONT hMono;
int ready = 0;

/* get a control by its ID */
HWND ctl(int id) { return GetDlgItem(hMain, id); }

/* create any control: class name, text, style, position, size, ID */
HWND make(char *cls, char *text, DWORD style, int x, int y, int w, int h, int id) {
    HWND c = CreateWindow(cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h,
                          hMain, (HMENU)id, GetModuleHandle(NULL), NULL);
    SendMessage(c, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return c;
}

void label(char *text, int x, int y, int w) { make("STATIC", text, 0, x, y + 3, w, 18, -1); }

/* a label in the fixed-width font (used as a column header above a list) */
void monoLabel(char *text, int x, int y, int w) {
    HWND c = make("STATIC", text, 0, x, y + 3, w, 18, -1);
    SendMessage(c, WM_SETFONT, (WPARAM)hMono, TRUE);
}
void edit(char *text, int x, int y, int w, int id) { make("EDIT", text, WS_BORDER | ES_AUTOHSCROLL, x, y, w, 22, id); }
void button(char *text, int x, int y, int w, int h, int id) { make("BUTTON", text, BS_PUSHBUTTON, x, y, w, h, id); }

/* a list box that uses the fixed-width font so columns line up */
void listbox(int x, int y, int w, int h, int id) {
    HWND lb = make("LISTBOX", "", WS_BORDER | WS_VSCROLL | LBS_NOTIFY, x, y, w, h, id);
    SendMessage(lb, WM_SETFONT, (WPARAM)hMono, TRUE);
}

void message(char *text) { MessageBox(hMain, text, "Invoice System", MB_OK | MB_ICONWARNING); }

float getNumber(int id) {
    char buf[32];
    GetWindowText(ctl(id), buf, sizeof buf);
    return (float)atof(buf);
}

/* redraw the invoice item list and the totals */
void refreshItems() {
    char line[100], text[200];
    float sub = 0, disc, tax, d = getNumber(ID_DISC);
    int i;

    if (d < 0 || d > 100) d = 0;
    SendMessage(ctl(ID_ILIST), LB_RESETCONTENT, 0, 0);
    for (i = 0; i < itemCount; i++) {
        struct Product *p = &products[itemIndex[i]];
        float total = p->price * itemQty[i];
        sub += total;
        sprintf(line, "%-20.20s %4d %9.2f %10.2f", p->name, itemQty[i], p->price, total);
        SendMessage(ctl(ID_ILIST), LB_ADDSTRING, 0, (LPARAM)line);
    }
    disc = sub * d / 100;
    tax = (sub - disc) * TAX_RATE / 100;
    sprintf(text, "Subtotal:  %.2f\r\nDiscount:  -%.2f\r\nTax (%d%%):  %.2f\r\nGRAND TOTAL:  %.2f",
            sub, disc, TAX_RATE, tax, sub - disc + tax);
    SetWindowText(ctl(ID_TOTALS), text);
}

/* redraw the product list */
void refreshProducts() {
    char line[100];
    int i;
    SendMessage(ctl(ID_PLIST), LB_RESETCONTENT, 0, 0);
    for (i = 0; i < productCount; i++) {
        sprintf(line, "%-14s %-22.22s %9.2f", products[i].barcode, products[i].name, products[i].price);
        SendMessage(ctl(ID_PLIST), LB_ADDSTRING, 0, (LPARAM)line);
    }
}

/* ---------- invoice buttons ---------- */

void onAddItem() {
    char code[32];
    int index, qty, i, found = -1;

    GetWindowText(ctl(ID_CODE), code, sizeof code);
    if (code[0] == 0) return;

    index = findProduct(code);
    if (index == -1) { message("Barcode not found."); return; }

    qty = (int)getNumber(ID_QTY);
    if (qty <= 0) { message("Invalid quantity."); return; }

    for (i = 0; i < itemCount; i++)
        if (itemIndex[i] == index) found = i;

    if (found != -1) {
        itemQty[found] += qty;
    } else if (itemCount < MAX_ITEMS) {
        itemIndex[itemCount] = index;
        itemQty[itemCount] = qty;
        itemCount++;
    } else {
        message("Invoice is full.");
        return;
    }
    SetWindowText(ctl(ID_CODE), "");
    SetWindowText(ctl(ID_QTY), "1");
    SetFocus(ctl(ID_CODE));
    refreshItems();
}

void onRemoveItem() {
    int s = (int)SendMessage(ctl(ID_ILIST), LB_GETCURSEL, 0, 0), i;
    if (s < 0) return;
    for (i = s; i < itemCount - 1; i++) {
        itemIndex[i] = itemIndex[i + 1];
        itemQty[i] = itemQty[i + 1];
    }
    itemCount--;
    refreshItems();
}

void onGenerate() {
    char customer[41], date[20];
    float discount = getNumber(ID_DISC);
    int number;
    time_t now;
    FILE *fp;

    if (itemCount == 0) { message("No items added. Add at least one item."); return; }
    if (discount < 0 || discount > 100) discount = 0;

    GetWindowText(ctl(ID_CUST), customer, sizeof customer);
    if (customer[0] == 0 || strcmp(customer, "-") == 0)
        strcpy(customer, "Walk-in Customer");

    number = getInvoiceNumber();
    now = time(NULL);
    strftime(date, sizeof(date), "%d-%m-%Y %H:%M", localtime(&now));

    fp = fopen("invoices.txt", "a");
    if (fp == NULL) { message("Could not write invoices.txt"); return; }
    printInvoice(fp, number, date, customer, discount);
    fclose(fp);

    itemCount = 0;
    SetWindowText(ctl(ID_CUST), "");
    SetWindowText(ctl(ID_DISC), "0");
    refreshItems();

    WinExec("notepad.exe invoices.txt", SW_SHOW);   /* show the saved invoices */
}

void onViewInvoices() {
    FILE *fp = fopen("invoices.txt", "r");
    if (fp == NULL) { message("No saved invoices yet."); return; }
    fclose(fp);
    WinExec("notepad.exe invoices.txt", SW_SHOW);
}

/* ---------- product buttons ---------- */

void onProductAdd() {
    struct Product p;
    char price[32];

    if (productCount >= MAX_PRODUCTS) { message("Database is full."); return; }

    GetWindowText(ctl(ID_PB), p.barcode, sizeof p.barcode);
    GetWindowText(ctl(ID_PN), p.name, sizeof p.name);
    GetWindowText(ctl(ID_PP), price, sizeof price);
    p.price = (float)atof(price);

    if (p.barcode[0] == 0 || p.name[0] == 0) { message("Enter barcode and name."); return; }
    if (findProduct(p.barcode) != -1) { message("This barcode already exists."); return; }
    if (p.price <= 0) { message("Price must be greater than 0."); return; }

    products[productCount] = p;
    productCount++;
    saveProducts();
    SetWindowText(ctl(ID_PB), "");
    SetWindowText(ctl(ID_PN), "");
    SetWindowText(ctl(ID_PP), "");
    refreshProducts();
}

void onProductUpdate() {
    int s = (int)SendMessage(ctl(ID_PLIST), LB_GETCURSEL, 0, 0);
    float newPrice = getNumber(ID_PP);

    if (s < 0) { message("Select a product in the list first."); return; }
    if (newPrice <= 0) { message("Invalid price."); return; }

    products[s].price = newPrice;
    saveProducts();
    refreshProducts();
    refreshItems();
}

void onProductDelete() {
    int s = (int)SendMessage(ctl(ID_PLIST), LB_GETCURSEL, 0, 0), i;

    if (s < 0) { message("Select a product in the list first."); return; }
    if (MessageBox(hMain, "Delete this product?", "Invoice System", MB_YESNO) != IDYES) return;

    for (i = s; i < productCount - 1; i++)
        products[i] = products[i + 1];
    productCount--;
    saveProducts();
    itemCount = 0;          /* product numbers changed, so start the invoice again */
    refreshProducts();
    refreshItems();
}

/* clicking a product copies its price into the price box */
void onProductSelected() {
    int s = (int)SendMessage(ctl(ID_PLIST), LB_GETCURSEL, 0, 0);
    char price[32];
    if (s < 0) return;
    sprintf(price, "%.2f", products[s].price);
    SetWindowText(ctl(ID_PP), price);
}

/* ---------- window ---------- */

void buildWindow() {
    /* left side: invoice */
    label("Customer (empty = walk-in):", 10, 10, 160);
    edit("", 175, 10, 255, ID_CUST);
    label("Barcode:", 10, 42, 55);
    edit("", 70, 42, 140, ID_CODE);
    label("Qty:", 220, 42, 30);
    edit("1", 252, 42, 40, ID_QTY);
    button("Add item", 305, 41, 125, 24, ID_ADD);

    monoLabel("Item                  Qty      Price      Total", 10, 72, 420);
    listbox(10, 94, 420, 210, ID_ILIST);

    button("Remove selected", 10, 312, 130, 26, ID_REMOVE);
    label("Discount %:", 170, 314, 70);
    edit("0", 245, 312, 50, ID_DISC);

    make("STATIC", "", WS_BORDER, 10, 350, 250, 76, ID_TOTALS);
    button("Generate invoice", 290, 350, 140, 36, ID_GEN);
    button("Clear", 290, 392, 140, 26, ID_CLEAR);
    button("View saved invoices", 10, 440, 180, 28, ID_VIEW);

    /* right side: products */
    monoLabel("Barcode        Item                        Price", 460, 10, 420);
    listbox(460, 32, 420, 270, ID_PLIST);

    label("Barcode:", 460, 314, 60);
    edit("", 525, 314, 150, ID_PB);
    label("Name:", 460, 344, 60);
    edit("", 525, 344, 220, ID_PN);
    label("Price:", 460, 374, 60);
    edit("", 525, 374, 90, ID_PP);

    button("Add product", 460, 410, 125, 28, ID_PADD);
    button("Update price", 595, 410, 125, 28, ID_PUPD);
    button("Delete", 730, 410, 100, 28, ID_PDEL);

    refreshItems();
    refreshProducts();
}

LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
    case WM_CREATE:
        hMain = h;
        buildWindow();
        ready = 1;
        return 0;

    case WM_COMMAND:
        switch (LOWORD(w)) {
        case ID_ADD:    onAddItem(); break;
        case ID_REMOVE: onRemoveItem(); break;
        case ID_CLEAR:  itemCount = 0; refreshItems(); break;
        case ID_GEN:    onGenerate(); break;
        case ID_VIEW:   onViewInvoices(); break;
        case ID_PADD:   onProductAdd(); break;
        case ID_PUPD:   onProductUpdate(); break;
        case ID_PDEL:   onProductDelete(); break;
        case ID_PLIST:
            if (HIWORD(w) == LBN_SELCHANGE) onProductSelected();
            break;
        case ID_DISC:
            if (HIWORD(w) == EN_CHANGE && ready) refreshItems();
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(h, msg, w, l);
}

int WINAPI WinMain(HINSTANCE hi, HINSTANCE hprev, LPSTR cmd, int show) {
    WNDCLASS wc;
    MSG msg;
    RECT r = { 0, 0, 900, 480 };
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    loadProducts();
    hMono = (HFONT)GetStockObject(ANSI_FIXED_FONT);

    memset(&wc, 0, sizeof wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hi;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "InvoiceGui";
    RegisterClass(&wc);

    AdjustWindowRect(&r, style, FALSE);
    CreateWindow("InvoiceGui", "Invoice System", style, 100, 80,
                 r.right - r.left, r.bottom - r.top, NULL, NULL, hi, NULL);
    ShowWindow(hMain, show);

    while (GetMessage(&msg, NULL, 0, 0)) {
        /* pressing Enter in the barcode box adds the item (for barcode scanners) */
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN && msg.hwnd == ctl(ID_CODE)) {
            onAddItem();
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}