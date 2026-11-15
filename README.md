# Smart Personal Finance Management System

A C++ personal finance manager built for our **PBL (Project-Based Learning)** course by **Team REX**.
It keeps income, expenses, budgets, savings goals, investments and upcoming bills in one place,
and is designed to demonstrate **Object-Oriented Programming** and **Data Structures & Algorithms**
written from scratch in standard C++ (C++14), with no external C++ libraries.

The same C++ core can be used in two ways:

| Version | Program | How you use it |
|---|---|---|
| **Console** | `main.exe` | Text menus in a terminal |
| **Web** (extra, not part of the PBL) | `server.exe` | A web page in your browser at `http://localhost:8080` |

Both use the same `FinanceSystem` class and the same saved data, so every rule and data structure is shared.

---

## Features

- **Transactions:** income and expenses from **Cash** or **Bank**; add, view, delete, **undo**, search (by category, type or ID) and sort (by amount, category or date).
- **Account balances:** an expense is refused if the account does not have enough money.
- **Monthly budgets:** a limit per category per month, with spent / remaining / % used and warnings when exceeded.
- **Savings goals:** progress, amount remaining and months needed.
- **Investments:** Stock, Mutual Fund, Fixed Deposit, Gold and Other, with profit/loss and %.
- **Bills:** schedule bills by due date, release the due ones into a payment line (all at once, or one chosen bill at a time), pay them in order, and move a bill that cannot be paid yet to the back.
- **Saving:** all data is saved to CSV files (they open in Excel) and loaded again at start-up; damaged lines are skipped with a warning.
- **Input checking:** real calendar dates only (e.g. `31-02-2026` is refused, leap years handled), positive amounts, no duplicate IDs.

---

## Data structures and algorithms

All of these are written by the team in the `DSA/` folder.

| Structure / algorithm | Used for | Time complexity |
|---|---|---|
| **Singly linked list** with head and **tail pointer** (`TransactionList`) | Storing all transactions | insert O(1), remove/search O(n) |
| **Hash table** with separate chaining, 1009 buckets (`HashTable`) | Finding a transaction by ID | O(1) average |
| **Stack** (linked) (`UndoStack`) | Undo the last add/delete | O(1) |
| **Circular queue** (array of 500) (`TransactionQueue`) | Bills ready to pay, first in first out | O(1) |
| **Binary min-heap** (array of 500) (`TransactionHeap`) | Scheduled bills, earliest due date on top | insert/remove O(log n), peek O(1) |
| **Heap sort** | Showing and saving scheduled bills in date order | O(n log n) |
| **Linear search** (`Searching`) | Search by category or type | O(n) |
| **Merge sort** (`Sorting`) | Sort by amount, category or date | O(n log n), stable |
| **Arrays** | Budgets (500), goals (200), investments (500) | O(1) access |

**Bill scheduler:** the min-heap decides **when** a bill is due; the FIFO queue decides the **order** in which due bills are paid (like an operating-system scheduler).

## OOP concepts

| Concept | Where |
|---|---|
| Encapsulation | Every class keeps its data private (e.g. balances change only through `deposit()` / `withdraw()`) |
| Abstraction | `Investment` is an abstract class (pure virtual `getType()`) |
| Inheritance | `Stock`, `MutualFund`, `FixedDeposit`, `Gold`, `OtherInvestment` derive from `Investment` |
| Runtime polymorphism | `investments[i]->display()` calls the right `getType()` through a base-class pointer |
| Virtual destructor | Safe `delete` of investments through `Investment*` |
| Composition | `FinanceSystem` *has* an `Account`, a `TransactionList`, a `HashTable`, ... |
| Constructors / destructors, Rule of Three | Classes that own memory free it and disable copying |
| `const`, `static`, overloading, `enum`, `override` | Used throughout |

---

## Project structure

```
PBL AI/
├── main.cpp               Console program (menus)
├── server.cpp             Web server (Windows sockets) that calls FinanceSystem
├── FinanceSystem.h/.cpp   The controller: owns all data and applies every rule
├── Account/               Account (Cash and Bank balances)
├── Transactions/          Transaction (one income/expense, date checks)
├── Budgets/               Budget (monthly limit per category)
├── Goals/                 Goal (savings target)
├── Investment/            Investment + 5 derived classes
├── DSA/                   Linked_list, HashTable, Stack, Queue, Heap, Searching, Sorting
├── Data/                  Saved data (CSV files, created by the programs; not in git)
├── web/                   Built web page served by server.exe
├── web-app/               React source of the web page (only needed to change the design)
├── info/                  Project documents (design, changes, how it works)
└── .vscode/tasks.json     Build tasks for VS Code
```

---

## How to run the project

### Requirements

| You want to... | You need |
|---|---|
| Build the C++ programs | Windows + **MinGW g++** (GCC 6.3 or newer, C++14). The VS Code tasks expect it at `C:\MinGW\bin\g++.exe`. |
| Run the console version | Nothing else |
| Run the web version | Any web browser |
| Change the web page design | **Node.js** (only for this) |

> All commands below are run **from the project folder** (the folder that contains `main.cpp`).

### 1. Console version

**Build (VS Code):** open the project folder and press **Ctrl+Shift+B**
(task: *Build Finance Manager (all files)*).

**Build (command line):**

```bash
g++ -std=c++14 -Wall -static-libgcc -static-libstdc++ main.cpp FinanceSystem.cpp Account/Account.cpp Budgets/Budget.cpp Transactions/transactions.cpp Goals/goals.cpp Investment/investment.cpp DSA/Linked_list.cpp DSA/Sorting.cpp DSA/Searching.cpp DSA/Stack.cpp DSA/Queue.cpp DSA/HashTable.cpp DSA/Heap.cpp -o main.exe
```

**Run:**

```powershell
.\main.exe
```

Main menu: `1 Dashboard`, `2 Transactions`, `3 Bills`, `4 Budgets`, `5 Goals`, `6 Investments`,
`7 Set account balances`, `8 Save now`, `0 Save and exit`.
Start with **7** to set your opening Cash and Bank balances. Data is saved when you choose **0** (or **8**).

### 2. Web version

**Build (VS Code):** *Terminal → Run Build Task… → Build web server (server.exe)*.

**Build (command line):**

```bash
g++ -std=c++14 -Wall -static-libgcc -static-libstdc++ server.cpp FinanceSystem.cpp Account/Account.cpp Budgets/Budget.cpp Transactions/transactions.cpp Goals/goals.cpp Investment/investment.cpp DSA/Linked_list.cpp DSA/Sorting.cpp DSA/Searching.cpp DSA/Stack.cpp DSA/Queue.cpp DSA/HashTable.cpp DSA/Heap.cpp -o server.exe -lws2_32
```

**Run:**

```powershell
.\server.exe
```

Your browser opens **http://localhost:8080** automatically (open it yourself if it does not).
Every change is saved immediately. **To stop the server, press Ctrl+C** in its terminal
(closing the browser tab does not stop it).

The web page is already built in `web/`, so **Node.js is not needed** to run it.

### 3. Changing the web page design (optional, needs Node.js)

```bash
cd web-app
npm install        # first time only: downloads React, Recharts, Vite...
npm run build      # rebuilds web/ from web-app/src
```

Or use the VS Code task *Build React frontend (web-app -> web)*. Restart `server.exe` and refresh the browser afterwards.

For live editing: run `server.exe`, then `npm run dev` inside `web-app/` and open **http://localhost:5173**.

### 4. Running on another computer

The build tasks use `-static-libgcc -static-libstdc++`, so the `.exe` files run on any Windows PC **without MinGW installed**. Copy:

| Version | Copy these |
|---|---|
| Console | `main.exe` + the `Data/` folder |
| Web | `server.exe` + the `web/` folder + the `Data/` folder |

### Important notes

- **Run only one program at a time.** `main.exe` and `server.exe` use the same `Data/` files; the one that saves last overwrites the other's changes.
- **The `Data/` folder must exist** next to the program. If it is missing, saving shows an error.
- **Close the program before rebuilding.** A "Permission denied" build error means `main.exe` or `server.exe` is still running.
- **Do not save the CSV files from Excel.** Viewing is fine, but Excel may change the date format, and those rows would then be skipped.
- Dates are always written **DD-MM-YYYY** (e.g. `25-12-2026`).

---

## Saved data

Both programs read and write these files in `Data/` (they are ignored by git, because they hold personal data):

| File | Contents |
|---|---|
| `account.csv` | Cash and Bank balances |
| `transactions.csv` | All transactions |
| `budgets.csv` | Monthly budgets |
| `goals.csv` | Savings goals |
| `investments.csv` | Investments |
| `pending.csv` | Bills ready to pay (queue order) |
| `scheduled.csv` | Scheduled bills (due-date order) |

## Testing

The project was checked with 4 C++ test programs (179 checks: every class and data structure, the controller, the bill scheduler, and saving/loading), a stress test with 5,000 transactions, and automated tests of the web server and page. The test programs are kept locally in `tests/` (not committed); each file starts with its own build command.

## Documentation

Detailed documents are in `info/`:

| Document | About |
|---|---|
| `How_It_Works.docx` | Complete guide to the C++ OOP and DSA design (start here) |
| `Project_Components_Report.docx` | Every component explained, with edge cases and viva questions |
| `changes1.docx`, `changes2.docx`, `changes3.docx` | Every change, why it was made, and its edge cases |
| `frontend_cpp_connection.docx` | How the web page talks to the C++ code, and the libraries used |

## Branches

| Branch | Contains |
|---|---|
| `main` | The console version |
| `frontend` | Console version + web version, tail pointer and the heap-based bill scheduler |

## Known limitations

- Sorting uses merge sort over a temporary array of pointers (O(n) extra memory) and writes the transactions back into the list. It replaced the original bubble sort (O(n²)); measured for 10,000 transactions: merge sort 0.03 s vs bubble sort 18 s. The old bubble sort is still in git history (commit `2cc0459`).
- Fixed maximums: 500 budgets, 200 goals, 500 investments, 500 ready bills, 500 scheduled bills (transactions and undo history have no limit). The main `FinanceSystem` object is `static` so its large arrays (about 180 KB) are not on the stack.
- Undo covers transactions only and is not saved between runs.
- Windows only (the web server uses Windows sockets).
