// ============================================================
// TechFix - Role-Based IT Support Ticket Tracker
// Final Project: Build Your Own Application
//
// A menu-driven C++ application that manages IT support tickets.
// Demonstrates:
//   - Classes and objects with inheritance (User -> Technician/Manager/Admin)
//   - Virtual permission-check methods that behave differently per role
//     (this is the role-based access control - the "unique twist")
//   - Full CRUD (Add / View / Update / Delete) on ticket records
//   - Long-term data storage: tickets are saved to tickets.txt and
//     automatically reloaded the next time the program runs
//   - Binary search to find a ticket quickly by ID
//   - Input validation on every prompt
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
using namespace std;

// ------------------------------------------------------------
// Ticket: the record being managed (added / viewed / updated / deleted)
// ------------------------------------------------------------
class Ticket {
private:
    int id;
    string customer;
    string issue;
    string priority; // Low, Medium, High
    string status;   // Open, InProgress, Closed

public:
    Ticket() : id(0) {}

    Ticket(int i, string c, string iss, string p, string s)
        : id(i), customer(c), issue(iss), priority(p), status(s) {}

    // ----- Getters -----
    int getId() const { return id; }
    string getCustomer() const { return customer; }
    string getIssue() const { return issue; }
    string getPriority() const { return priority; }
    string getStatus() const { return status; }

    // ----- Setters (used by Update) -----
    void setStatus(const string& s) { status = s; }
    void setPriority(const string& p) { priority = p; }

    void display() const {
        cout << "Ticket #" << id << endl;
        cout << "  Customer: " << customer << endl;
        cout << "  Issue:    " << issue << endl;
        cout << "  Priority: " << priority << endl;
        cout << "  Status:   " << status << endl;
        cout << "-----------------------------" << endl;
    }
};

vector<Ticket> tickets; // all tickets currently loaded

// ------------------------------------------------------------
// Base class: User
// Declares virtual permission checks. Each role (derived class)
// overrides these to control what that role is allowed to do -
// this is the inheritance + polymorphism requirement, applied as
// role-based access control for the app's "unique twist".
// ------------------------------------------------------------
class User {
protected:
    string username;

public:
    User(string name) : username(name) {}
    virtual ~User() {}

    virtual string getRole() const { return "User"; }
    virtual bool canAdd()    const { return false; }
    virtual bool canUpdate() const { return false; }
    virtual bool canDelete() const { return false; }

    string getUsername() const { return username; }
};

// Technician: can log and update tickets, but not delete them
class Technician : public User {
public:
    Technician(string name) : User(name) {}
    string getRole() const override { return "Technician"; }
    bool canAdd()    const override { return true; }
    bool canUpdate() const override { return true; }
    bool canDelete() const override { return false; }
};

// Manager: same as Technician for now, but modeled separately so the
// role can be given extra manager-only powers later (e.g. reporting)
class Manager : public User {
public:
    Manager(string name) : User(name) {}
    string getRole() const override { return "Manager"; }
    bool canAdd()    const override { return true; }
    bool canUpdate() const override { return true; }
    bool canDelete() const override { return false; }
};

// Admin: full access, including deleting tickets
class Admin : public User {
public:
    Admin(string name) : User(name) {}
    string getRole() const override { return "Admin"; }
    bool canAdd()    const override { return true; }
    bool canUpdate() const override { return true; }
    bool canDelete() const override { return true; }
};

// ------------------------------------------------------------
// Input validation helpers
// ------------------------------------------------------------
int readIntInRange(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            return value;
        }
        cout << "Invalid input. Please enter a number between "
             << minVal << " and " << maxVal << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readLine(const string& prompt) {
    string value;
    cout << prompt;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear leftover newline
    getline(cin, value);
    while (value.empty()) {
        cout << "Input cannot be empty. " << prompt;
        getline(cin, value);
    }
    return value;
}

string readWord(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!value.empty()) return value;
        cout << "Input cannot be empty.\n";
    }
}

// ------------------------------------------------------------
// Keeps tickets sorted by ID (required for binary search)
// ------------------------------------------------------------
void sortTicketsById() {
    for (size_t i = 1; i < tickets.size(); i++) {
        Ticket key = tickets[i];
        int j = static_cast<int>(i) - 1;
        while (j >= 0 && tickets[j].getId() > key.getId()) {
            tickets[j + 1] = tickets[j];
            j--;
        }
        tickets[j + 1] = key;
    }
}

// Binary search for a ticket by ID. Returns the index, or -1.
int binarySearchById(int searchId) {
    int left = 0;
    int right = static_cast<int>(tickets.size()) - 1;

    while (left <= right) {
        int mid = (left + right) / 2;
        int midId = tickets[mid].getId();

        if (midId == searchId) return mid;
        if (midId < searchId) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// ------------------------------------------------------------
// File I/O: load and save tickets (long-term data storage)
// File format per line: id customer issue priority status
// (customer/issue are single tokens - no embedded spaces - kept
//  simple to match the plain space-delimited format used elsewhere
//  in this course; use underscores for multi-word entries, e.g. John_Smith)
// ------------------------------------------------------------
const string DATA_FILE = "tickets.txt";

void loadTickets() {
    ifstream file(DATA_FILE);
    if (!file.is_open()) return; // no saved tickets yet - that's fine

    int id;
    string customer, issue, priority, status;

    while (file >> id >> customer >> issue >> priority >> status) {
        tickets.push_back(Ticket(id, customer, issue, priority, status));
    }

    file.close();
    sortTicketsById();
}

void saveTickets() {
    ofstream file(DATA_FILE);
    if (!file.is_open()) {
        cout << "Error: could not open " << DATA_FILE << " for writing.\n";
        return;
    }

    for (const Ticket& t : tickets) {
        file << t.getId() << " "
             << t.getCustomer() << " "
             << t.getIssue() << " "
             << t.getPriority() << " "
             << t.getStatus() << "\n";
    }

    file.close();
    cout << "Tickets saved to " << DATA_FILE << ".\n";
}

// ------------------------------------------------------------
// CRUD operations
// ------------------------------------------------------------
void addTicket() {
    int id = readIntInRange("Enter Ticket ID: ", 1, 999999);

    if (binarySearchById(id) != -1) {
        cout << "A ticket with that ID already exists.\n";
        return;
    }

    string customer = readWord("Enter customer name (single word, e.g. JohnSmith): ");
    string issue = readWord("Enter issue (single word/phrase, e.g. WifiDown): ");

    cout << "Select priority:\n  1. Low\n  2. Medium\n  3. High\n";
    int pChoice = readIntInRange("Enter choice: ", 1, 3);
    string priority = (pChoice == 1) ? "Low" : (pChoice == 2) ? "Medium" : "High";

    tickets.push_back(Ticket(id, customer, issue, priority, "Open"));
    sortTicketsById();

    cout << "Ticket added successfully.\n\n";
}

void viewTickets() {
    if (tickets.empty()) {
        cout << "No tickets found.\n\n";
        return;
    }

    cout << "\n===== ALL TICKETS =====\n\n";
    for (const Ticket& t : tickets) {
        t.display();
    }
    cout << endl;
}

void searchTicket() {
    int id = readIntInRange("Enter Ticket ID to search: ", 1, 999999);
    int index = binarySearchById(id);

    if (index == -1) {
        cout << "Ticket not found.\n\n";
        return;
    }

    cout << "\nTicket Found:\n";
    tickets[index].display();
    cout << endl;
}

void updateTicket() {
    int id = readIntInRange("Enter Ticket ID to update: ", 1, 999999);
    int index = binarySearchById(id);

    if (index == -1) {
        cout << "Ticket not found.\n\n";
        return;
    }

    cout << "Select new status:\n  1. Open\n  2. InProgress\n  3. Closed\n";
    int sChoice = readIntInRange("Enter choice: ", 1, 3);
    string status = (sChoice == 1) ? "Open" : (sChoice == 2) ? "InProgress" : "Closed";

    cout << "Select new priority:\n  1. Low\n  2. Medium\n  3. High\n";
    int pChoice = readIntInRange("Enter choice: ", 1, 3);
    string priority = (pChoice == 1) ? "Low" : (pChoice == 2) ? "Medium" : "High";

    tickets[index].setStatus(status);
    tickets[index].setPriority(priority);

    cout << "Ticket updated successfully.\n\n";
}

void deleteTicket() {
    int id = readIntInRange("Enter Ticket ID to delete: ", 1, 999999);
    int index = binarySearchById(id);

    if (index == -1) {
        cout << "Ticket not found.\n\n";
        return;
    }

    tickets.erase(tickets.begin() + index);
    cout << "Ticket deleted successfully.\n\n";
}

// ------------------------------------------------------------
// Login: choose a role, which determines menu permissions
// (creates the right derived User object - factory pattern)
// ------------------------------------------------------------
User* login() {
    cout << "===== TechFix Login =====\n";
    cout << "1. Technician\n";
    cout << "2. Manager\n";
    cout << "3. Admin\n";
    int roleChoice = readIntInRange("Select your role: ", 1, 3);
    string name = readWord("Enter your username: ");

    if (roleChoice == 1) return new Technician(name);
    if (roleChoice == 2) return new Manager(name);
    return new Admin(name);
}

// ------------------------------------------------------------
// Menu - options shown depend on the logged-in user's permissions
// ------------------------------------------------------------
void showMenu(User* user) {
    cout << "\n===== TechFix Menu (" << user->getRole()
         << ": " << user->getUsername() << ") =====\n";
    int optionNum = 1;

    if (user->canAdd())    cout << optionNum++ << ". Add Ticket\n";
    cout << optionNum++ << ". View All Tickets\n";
    cout << optionNum++ << ". Search Ticket\n";
    if (user->canUpdate()) cout << optionNum++ << ". Update Ticket\n";
    if (user->canDelete()) cout << optionNum++ << ". Delete Ticket\n";
    cout << optionNum++ << ". Save Tickets\n";
    cout << optionNum << ". Exit\n";
}

int main() {
    loadTickets(); // long-term storage: bring back last session's tickets

    User* currentUser = login();
    cout << "\nLogged in as " << currentUser->getRole()
         << " " << currentUser->getUsername() << ".\n";

    bool exitProgram = false;
    while (!exitProgram) {
        // Build the menu mapping fresh each loop, since it depends on role
        vector<pair<int, string>> menuActions; // (option number, action name)
        int optionNum = 1;

        if (currentUser->canAdd())    menuActions.push_back({optionNum++, "add"});
        menuActions.push_back({optionNum++, "view"});
        menuActions.push_back({optionNum++, "search"});
        if (currentUser->canUpdate()) menuActions.push_back({optionNum++, "update"});
        if (currentUser->canDelete()) menuActions.push_back({optionNum++, "delete"});
        menuActions.push_back({optionNum++, "save"});
        menuActions.push_back({optionNum, "exit"});

        showMenu(currentUser);
        int choice = readIntInRange("Enter choice: ", 1, optionNum);
        cout << endl;

        // Find which action the chosen number maps to
        string action;
        for (auto& pairItem : menuActions) {
            if (pairItem.first == choice) {
                action = pairItem.second;
                break;
            }
        }

        if (action == "add") addTicket();
        else if (action == "view") viewTickets();
        else if (action == "search") searchTicket();
        else if (action == "update") updateTicket();
        else if (action == "delete") deleteTicket();
        else if (action == "save") saveTickets();
        else if (action == "exit") {
            saveTickets(); // auto-save on exit so nothing is lost
            cout << "Thank you for using TechFix. Goodbye!\n";
            exitProgram = true;
        }
    }

    delete currentUser;
    return 0;
}
