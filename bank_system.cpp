/*
 * ============================================================
 *  BANK ACCOUNT MANAGEMENT SYSTEM  (Object Oriented Programming in C++)
 *  Mini Project - Chandigarh University
 * ------------------------------------------------------------
 *  OOP concepts demonstrated:
 *    1. Classes & Objects          6. Static members
 *    2. Encapsulation              7. Friend function
 *    3. Abstraction (abstract cls) 8. Operator overloading
 *    4. Inheritance                9. Exception handling (custom)
 *    5. Polymorphism (virtual)    10. Templates, STL, smart pointers,
 *                                     File handling
 * ============================================================
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <limits>

using namespace std;

// ---------------------------------------------------------------
//  Utility
// ---------------------------------------------------------------
string currentTime() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M", localtime(&now));
    return string(buf);
}

// ---------------------------------------------------------------
//  Custom exception hierarchy
// ---------------------------------------------------------------
class BankException : public runtime_error {
public:
    explicit BankException(const string& msg) : runtime_error(msg) {}
};

class InvalidAmountException : public BankException {
public:
    InvalidAmountException() : BankException("Amount must be greater than zero.") {}
};

class InsufficientFundsException : public BankException {
public:
    explicit InsufficientFundsException(const string& msg) : BankException(msg) {}
};

class AccountNotFoundException : public BankException {
public:
    explicit AccountNotFoundException(int no)
        : BankException("Account number " + to_string(no) + " was not found.") {}
};

// ---------------------------------------------------------------
//  Transaction record
// ---------------------------------------------------------------
struct Transaction {
    string type;
    double amount;
    double balanceAfter;
    string timestamp;
};

// Tag used to pick the "restore from file" constructors
struct RestoreTag {};

// ---------------------------------------------------------------
//  Abstract base class : Account
// ---------------------------------------------------------------
class Account {
protected:
    int accNo;
    string holder;
    double balance;
    vector<Transaction> history;

    static int nextAccNo;       // shared by all accounts
    static int totalAccounts;   // live account counter

    void record(const string& type, double amt) {
        history.push_back({type, amt, balance, currentTime()});
    }

public:
    // Constructor for a brand-new account
    Account(const string& name, double initial)
        : accNo(nextAccNo++), holder(name), balance(initial) {
        ++totalAccounts;
        if (initial > 0) record("Opening Deposit", initial);
    }

    // Constructor used while loading from file
    Account(int no, const string& name, double bal, RestoreTag)
        : accNo(no), holder(name), balance(bal) {
        ++totalAccounts;
        if (no >= nextAccNo) nextAccNo = no + 1;
        record("Balance Restored", bal);
    }

    virtual ~Account() { --totalAccounts; }

    // ---- pure virtual functions make Account abstract ----
    virtual string getType() const = 0;
    virtual void withdraw(double amt) = 0;
    virtual void monthEnd() = 0;

    // ---- common behaviour ----
    void deposit(double amt) {
        if (amt <= 0) throw InvalidAmountException();
        balance += amt;
        record("Deposit", amt);
    }

    // Getters (encapsulation: data is read-only from outside)
    int getAccNo() const { return accNo; }
    const string& getHolder() const { return holder; }
    double getBalance() const { return balance; }
    const vector<Transaction>& getHistory() const { return history; }
    static int getTotalAccounts() { return totalAccounts; }

    // Operator overloading
    Account& operator+=(double amt) { deposit(amt); return *this; }
    bool operator<(const Account& other) const { return balance < other.balance; }

    friend ostream& operator<<(ostream& os, const Account& a);

    string serialize() const {
        string safe = holder;
        replace(safe.begin(), safe.end(), '|', ' ');
        ostringstream ss;
        ss << getType() << '|' << accNo << '|' << safe << '|' << fixed << setprecision(2) << balance;
        return ss.str();
    }

    void printStatement() const {
        cout << "\n----- Statement : A/C " << accNo << " (" << holder << ") -----\n";
        cout << left << setw(18) << "Date/Time" << setw(18) << "Type"
             << right << setw(12) << "Amount" << setw(14) << "Balance" << "\n";
        cout << string(62, '-') << "\n";
        for (const auto& t : history) {
            cout << left << setw(18) << t.timestamp << setw(18) << t.type
                 << right << setw(12) << fixed << setprecision(2) << t.amount
                 << setw(14) << t.balanceAfter << "\n";
        }
        cout << string(62, '-') << "\n";
    }
};

int Account::nextAccNo = 1001;
int Account::totalAccounts = 0;

// Friend function: accesses protected data directly
ostream& operator<<(ostream& os, const Account& a) {
    os << left << setw(8) << a.accNo << setw(20) << a.holder << setw(10) << a.getType()
       << right << setw(14) << fixed << setprecision(2) << a.balance;
    return os;
}

// ---------------------------------------------------------------
//  Derived class 1 : SavingsAccount
// ---------------------------------------------------------------
class SavingsAccount : public Account {
    static constexpr double MIN_BALANCE = 500.0;
    static constexpr double INTEREST_RATE = 4.0;       // % per annum
    static constexpr double PER_TXN_LIMIT = 50000.0;

    static double validate(double initial) {
        if (initial < MIN_BALANCE)
            throw InsufficientFundsException("Savings account needs a minimum opening balance of Rs. 500.");
        return initial;
    }

public:
    SavingsAccount(const string& name, double initial) : Account(name, validate(initial)) {}
    SavingsAccount(int no, const string& name, double bal, RestoreTag t) : Account(no, name, bal, t) {}

    string getType() const override { return "Savings"; }

    void withdraw(double amt) override {
        if (amt <= 0) throw InvalidAmountException();
        if (amt > PER_TXN_LIMIT)
            throw BankException("Savings accounts allow at most Rs. 50,000 per withdrawal.");
        if (balance - amt < MIN_BALANCE)
            throw InsufficientFundsException("Withdrawal denied: minimum balance of Rs. 500 must be maintained.");
        balance -= amt;
        record("Withdrawal", amt);
    }

    void monthEnd() override {
        double interest = balance * INTEREST_RATE / 100.0 / 12.0;
        balance += interest;
        record("Interest Credit", interest);
    }
};

// ---------------------------------------------------------------
//  Derived class 2 : CurrentAccount
// ---------------------------------------------------------------
class CurrentAccount : public Account {
    static constexpr double OVERDRAFT_LIMIT = 10000.0;
    static constexpr double MAINTENANCE_FEE = 100.0;

public:
    CurrentAccount(const string& name, double initial) : Account(name, initial) {}
    CurrentAccount(int no, const string& name, double bal, RestoreTag t) : Account(no, name, bal, t) {}

    string getType() const override { return "Current"; }

    void withdraw(double amt) override {
        if (amt <= 0) throw InvalidAmountException();
        if (balance - amt < -OVERDRAFT_LIMIT)
            throw InsufficientFundsException("Withdrawal denied: overdraft limit of Rs. 10,000 exceeded.");
        balance -= amt;
        record("Withdrawal", amt);
    }

    void monthEnd() override {
        balance -= MAINTENANCE_FEE;
        record("Maintenance Fee", MAINTENANCE_FEE);
    }
};

// ---------------------------------------------------------------
//  Bank : manages a collection of accounts (composition)
// ---------------------------------------------------------------
class Bank {
    vector<unique_ptr<Account>> accounts;

    Account& find(int no) {
        for (auto& a : accounts)
            if (a->getAccNo() == no) return *a;
        throw AccountNotFoundException(no);
    }

public:
    int openAccount(char type, const string& name, double initial) {
        unique_ptr<Account> acc;
        if (type == 'S' || type == 's')      acc = make_unique<SavingsAccount>(name, initial);
        else if (type == 'C' || type == 'c') acc = make_unique<CurrentAccount>(name, initial);
        else throw BankException("Unknown account type. Use S (Savings) or C (Current).");
        int no = acc->getAccNo();
        accounts.push_back(move(acc));
        return no;
    }

    void deposit(int no, double amt)  { find(no) += amt; }
    void withdraw(int no, double amt) { find(no).withdraw(amt); }   // runtime polymorphism

    void transfer(int from, int to, double amt) {
        if (from == to) throw BankException("Cannot transfer to the same account.");
        Account& src = find(from);
        Account& dst = find(to);
        src.withdraw(amt);
        try { dst.deposit(amt); }
        catch (...) { src.deposit(amt); throw; }    // roll back
    }

    void show(int no)      { cout << "\n" << find(no) << "\n"; }
    void statement(int no) { find(no).printStatement(); }

    void closeAccount(int no) {
        auto it = find_if(accounts.begin(), accounts.end(),
                          [no](const unique_ptr<Account>& a) { return a->getAccNo() == no; });
        if (it == accounts.end()) throw AccountNotFoundException(no);
        accounts.erase(it);
    }

    void applyMonthEnd() {
        for (auto& a : accounts) a->monthEnd();
    }

    void listAll() const {
        if (accounts.empty()) { cout << "\nNo accounts available.\n"; return; }
        vector<const Account*> sorted;
        for (const auto& a : accounts) sorted.push_back(a.get());
        sort(sorted.begin(), sorted.end(),
             [](const Account* x, const Account* y) { return *y < *x; });  // richest first

        cout << "\n" << left << setw(8) << "A/C No" << setw(20) << "Holder" << setw(10) << "Type"
             << right << setw(14) << "Balance" << "\n" << string(52, '=') << "\n";
        for (auto* a : sorted) cout << *a << "\n";
        cout << string(52, '=') << "\n";
        cout << "Total live accounts: " << Account::getTotalAccounts() << "\n";
    }

    void save(const string& file) const {
        ofstream out(file);
        if (!out) throw BankException("Could not open file for writing.");
        for (const auto& a : accounts) out << a->serialize() << "\n";
    }

    int load(const string& file) {
        ifstream in(file);
        if (!in) return 0;
        string line;
        int count = 0;
        while (getline(in, line)) {
            istringstream ss(line);
            string type, no, name, bal;
            if (!getline(ss, type, '|') || !getline(ss, no, '|') ||
                !getline(ss, name, '|') || !getline(ss, bal, '|')) continue;
            try {
                int n = stoi(no);
                double b = stod(bal);
                if (type == "Savings")
                    accounts.push_back(make_unique<SavingsAccount>(n, name, b, RestoreTag{}));
                else if (type == "Current")
                    accounts.push_back(make_unique<CurrentAccount>(n, name, b, RestoreTag{}));
                else continue;
                ++count;
            } catch (const exception&) { /* skip corrupt line */ }
        }
        return count;
    }
};

// ---------------------------------------------------------------
//  Input helpers (function template)
// ---------------------------------------------------------------
template <typename T>
T readValue(const string& prompt) {
    T v;
    while (true) {
        cout << prompt;
        if (cin >> v) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return v; }
        if (cin.eof()) throw BankException("Input ended.");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input, please try again.\n";
    }
}

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) throw BankException("Input ended.");
    return s;
}

void printMenu() {
    cout << "\n=========== BANK ACCOUNT SYSTEM ===========\n"
         << " 1. Open new account\n"
         << " 2. Deposit\n"
         << " 3. Withdraw\n"
         << " 4. Transfer\n"
         << " 5. Account details\n"
         << " 6. Mini statement\n"
         << " 7. List all accounts\n"
         << " 8. Apply month-end (interest / fees)\n"
         << " 9. Close account\n"
         << " 0. Save & exit\n"
         << "===========================================\n";
}

// ---------------------------------------------------------------
//  main
// ---------------------------------------------------------------
int main() {
    const string FILE_NAME = "accounts.txt";
    Bank bank;
    int loaded = bank.load(FILE_NAME);
    if (loaded) cout << loaded << " account(s) loaded from " << FILE_NAME << ".\n";

    while (true) {
        try {
            printMenu();
            int choice = readValue<int>("Enter choice: ");

            switch (choice) {
            case 1: {
                char t = readValue<char>("Type (S = Savings, C = Current): ");
                string name = readLine("Holder name: ");
                double amt = readValue<double>("Opening deposit (Rs.): ");
                int no = bank.openAccount(t, name, amt);
                cout << "Account created. Your account number is " << no << ".\n";
                break;
            }
            case 2: {
                int no = readValue<int>("Account number: ");
                bank.deposit(no, readValue<double>("Amount: "));
                cout << "Deposit successful.\n";
                break;
            }
            case 3: {
                int no = readValue<int>("Account number: ");
                bank.withdraw(no, readValue<double>("Amount: "));
                cout << "Withdrawal successful.\n";
                break;
            }
            case 4: {
                int from = readValue<int>("From account: ");
                int to = readValue<int>("To account: ");
                bank.transfer(from, to, readValue<double>("Amount: "));
                cout << "Transfer successful.\n";
                break;
            }
            case 5: bank.show(readValue<int>("Account number: ")); break;
            case 6: bank.statement(readValue<int>("Account number: ")); break;
            case 7: bank.listAll(); break;
            case 8: bank.applyMonthEnd(); cout << "Month-end processing complete.\n"; break;
            case 9: {
                bank.closeAccount(readValue<int>("Account number: "));
                cout << "Account closed.\n";
                break;
            }
            case 0:
                bank.save(FILE_NAME);
                cout << "Data saved. Thank you for banking with us!\n";
                return 0;
            default:
                cout << "Invalid choice.\n";
            }
        } catch (const BankException& e) {
            if (string(e.what()) == "Input ended.") { bank.save(FILE_NAME); return 0; }
            cout << "Error: " << e.what() << "\n";
        }
    }
}
