# 🏦 Bank Account Management System (C++)

A console-based banking application written in **C++17** that demonstrates the core principles of **Object Oriented Programming**. Built as a mini project at **Chandigarh University**.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![Dependencies](https://img.shields.io/badge/dependencies-none-brightgreen)

---

## ✨ Features

- Open **Savings** and **Current** accounts with automatically generated account numbers
- **Deposit**, **withdraw** and **transfer** money (with rollback if a transfer fails)
- Per-account **transaction history** and mini statement
- **Month-end processing**: interest for savings accounts, maintenance fee for current accounts
- List all accounts **sorted by balance**
- Close accounts
- **Persistent storage**: accounts are saved to `accounts.txt` on exit and reloaded on start-up
- Clear error messages through a **custom exception hierarchy**; invalid input never crashes the program

## 📏 Account Rules

| Rule | Savings Account | Current Account |
|---|---|---|
| Minimum balance | Rs. 500 (also the minimum opening deposit) | None |
| Overdraft limit | Not allowed | Rs. 10,000 |
| Maximum per withdrawal | Rs. 50,000 | No limit |
| Month-end action | Interest at 4% p.a. (`balance × 4 / 100 / 12`) | Maintenance fee of Rs. 100 |

## 🧠 OOP Concepts Demonstrated

| Concept | Where it is used |
|---|---|
| Classes & objects | `Account`, `SavingsAccount`, `CurrentAccount`, `Bank`, `Transaction` |
| Encapsulation | `protected` data in `Account`; changed only through `deposit()` / `withdraw()` |
| Abstraction | `Account` is an abstract class with pure virtual `getType()`, `withdraw()`, `monthEnd()` |
| Inheritance | `SavingsAccount` and `CurrentAccount` derive from `Account` |
| Polymorphism | `Bank` calls `withdraw()` / `monthEnd()` through `Account` pointers; each type applies its own rules |
| Constructors / destructors | Account number assignment, live account counter, virtual destructor |
| Static members | `nextAccNo`, `totalAccounts`, `getTotalAccounts()` |
| Friend function | `operator<<` prints an account |
| Operator overloading | `+=` (deposit), `<` (compare balances for sorting), `<<` (output) |
| Templates | `readValue<T>()` validated input for `int`, `double`, `char` |
| Exception handling | `BankException` → `InvalidAmount`, `InsufficientFunds`, `AccountNotFound` |
| STL & smart pointers | `vector<unique_ptr<Account>>`, `sort`, `find_if`, lambdas |
| File handling | `ifstream` / `ofstream` to save and load accounts |

## 🗂️ Class Diagram

![Class diagram](screenshots/class-diagram.png)

## 🚀 Getting Started

### Prerequisites

A C++17 compiler (GCC 9+, Clang, or MSVC). No external libraries are needed.

### Build and run

```bash
git clone https://github.com/<your-username>/<your-repo>.git
cd <your-repo>

g++ -std=c++17 -Wall -Wextra -o bank bank_system.cpp
./bank            # on Windows: bank.exe
```

## 🖥️ Usage

```
=========== BANK ACCOUNT SYSTEM ===========
 1. Open new account
 2. Deposit
 3. Withdraw
 4. Transfer
 5. Account details
 6. Mini statement
 7. List all accounts
 8. Apply month-end (interest / fees)
 9. Close account
 0. Save & exit
===========================================
```

Choose an option by number and follow the prompts. Account numbers start at **1001**.

### Sample output

| | |
|---|---|
| ![Menu](screenshots/01-menu-and-open-account.png) | ![Deposit and rules](screenshots/02-deposit-and-rules.png) |
| ![Transfer](screenshots/03-transfer-month-end-list.png) | ![Statement](screenshots/04-mini-statement.png) |
| ![Errors](screenshots/05-error-handling.png) | ![Persistence](screenshots/06-save-and-reload.png) |

## 💾 Data File Format

Accounts are stored in `accounts.txt` (created next to the executable), one per line:

```
Savings|1001|Aman Singh|5518.33
Current|1002|Riya Sharma|-8600.00
```

Format: `type|account number|holder name|balance`. Transaction history is not saved and starts fresh on each run.

## 🧪 Testing

The program was tested manually with 12 test cases covering valid operations, rule violations, boundary values, invalid input and persistence. Examples:

| Test | Expected result |
|---|---|
| Open savings account with Rs. 100 | Error: minimum opening balance is Rs. 500 |
| Withdraw so a savings balance drops below Rs. 500 | Error: minimum balance must be maintained |
| Current account withdrawal beyond Rs. 10,000 overdraft | Error: overdraft limit exceeded |
| Transfer to the same account / a non-existent account | Error; balances unchanged |
| Type letters where a number is expected | "Invalid input" and re-prompt |
| Exit and run again | Accounts reloaded with the same balances |

## 📁 Project Structure

```
.
├── bank_system.cpp      # complete source code (single file)
├── README.md
└── screenshots/         # sample output images and class diagram
```

## 🔮 Future Improvements

- Add a `Customer` class so one customer can own several accounts
- More account types (Fixed Deposit, Recurring Deposit)
- Save transaction history; move storage to a database (SQLite / MySQL)
- PIN / password login and encrypted storage
- Use a decimal or integer-paise type instead of `double` for money
- Unit tests (e.g. Google Test) and a GUI / web front end

## 👤 Author

**[Your Name]** – UID: [Your UID]
B.E. [Branch], Chandigarh University
Guide: [Faculty Name]

## 📄 License

This project is released under the [MIT License](LICENSE). *(Add a `LICENSE` file to your repository, or change this line to your preferred license.)*
