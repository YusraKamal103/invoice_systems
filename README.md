# Invoice Generation System (C)

A console-based invoice generation system written in C. Items are looked up by **barcode**, so the item name and price are filled in automatically. Products and invoices are saved in text files, so data stays saved between runs.

**Students:**

| Name | Roll No |
|------|---------|
| Yusra Kamal | 26K-2518 |
| Samaan | 26K-2516 |

**Section:** BDS-1A
**University:** FAST University (FAST NUCES)
**Program:** BS Data Science

---

## Features

### Product Database
- List all products with barcode, name and price
- Add a new product (duplicate barcodes are rejected)
- Update the price of an existing product
- Delete a product
- Stored in `products.txt` (up to 100 products)
- On the first run, **5 demo items** are created automatically so you can test right away:

| Barcode | Item | Price |
|---------|------|-------|
| 6281001000012 | Milk 1 Litre | 290 |
| 6281001000029 | Bread (Large) | 180 |
| 6281001000036 | Eggs (Dozen) | 420 |
| 6281001000043 | Basmati Rice 5 Kg | 1850 |
| 6281001000050 | Cooking Oil 1 L | 620 |

### Invoice Creation
- Enter the customer name, or type `-` for a walk-in customer
- Enter barcodes one by one and the item name and price are shown automatically
- Enter the quantity for each item
- Scanning the same barcode again adds to the existing quantity instead of creating a duplicate line
- Type `done` to finish adding items (up to 50 different items per invoice)
- Apply a **discount percentage** (0 for none)
- **17% GST** is calculated on the amount after discount
- Date and time are added automatically
- Invoice numbers are generated automatically, starting from **1001** (tracked in `counter.txt`)

### Saved Invoices
- Every invoice is printed on screen and also appended to `invoices.txt`
- Saved invoices can be viewed from the main menu

---

## Invoice Calculation

```
Subtotal    = sum of (price x quantity)
Discount    = Subtotal x Discount% / 100
Tax (17%)   = (Subtotal - Discount) x 17 / 100
Grand Total = Subtotal - Discount + Tax
```

---

## Files

| File | Purpose |
|------|---------|
| `invoice.c` | Main source code |
| `products.txt` | Product database, one product per line as `barcode\|name\|price` |
| `invoices.txt` | All saved invoices |
| `counter.txt` | Keeps track of the last invoice number used |

`products.txt`, `invoices.txt` and `counter.txt` are created automatically when the program runs.

---

## How to Compile and Run

Make sure a C compiler such as GCC is installed.

```bash
gcc invoice.c -o invoice
./invoice
```

On Windows:

```bash
gcc invoice.c -o invoice.exe
invoice.exe
```

---

## Main Menu

```
===== INVOICE SYSTEM =====
1. Create invoice
2. View saved invoices
3. List products
4. Add product
5. Update product price
6. Delete product
0. Exit
```

## How to Use

1. Run the program and choose an option from the menu.
2. To make an invoice, choose **1**, enter the customer name, then enter a barcode for each item (for example `6281001000012`) and its quantity.
3. Type `done` when finished, then enter a discount percentage.
4. The invoice is shown on screen and saved in `invoices.txt`.
5. Choose **2** at any time to view all saved invoices.
6. Use options **3 to 6** to manage the product database.

---

## Concepts Used

- Structures (`struct Product`)
- Arrays (product list and invoice items)
- File handling (reading, writing and appending text files)
- Functions for modular code
- Loops and `switch` menu (`do...while`)
- String handling (`string.h`)
- Date and time (`time.h`)
- Basic input validation (invalid price, quantity, discount and unknown barcodes)

---

## Tools and Technologies

- **Language:** C
- **Compiler:** GCC
- **Storage:** Plain text files
- **Interface:** Console / command line

---

## Limitations

- Maximum of 100 products and 50 different items per invoice
- Saved invoices can be viewed but not searched or deleted
- Barcodes are typed in manually, not scanned with a hardware scanner
- Editing a product's price does not change invoices that were already saved
