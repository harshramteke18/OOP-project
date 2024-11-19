#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <iomanip>

using namespace std;

// Structure to hold user details and orders
struct User {
    string name;
    string address;
    string phone;
    vector<pair<string, double>> orders;  // Each order item with its price
    vector<string> orderDates;
};

class AdminPage {
private:
    string restaurantsFile;
    string menusFile;
    string usersFile;
    map<string, User> users;  // To store user details by name

public:
    AdminPage(const string &restaurantsFile, const string &menusFile, const string &usersFile)
        : restaurantsFile(restaurantsFile), menusFile(menusFile), usersFile(usersFile) {
        loadUsers();
    }

    void displayAdminMenu() {
        int choice;
        while (true) {
            cout << "\nAdmin Page - Select an option:\n";
            cout << "1. Modify Restaurants\n2. Modify Menus\n3. View Users\n4. Exit\n";
            cout << "Choice: ";
            cin >> choice;

            switch (choice) {
                case 1: modifyRestaurantFile(); break;
                case 2: modifyMenuFile(); break;
                case 3: viewUsers(); break;
                case 4: cout << "Exiting Admin Page.\n"; return;
                default: cout << "Invalid option, please try again.\n"; break;
            }
        }
    }

    void modifyRestaurantFile() {
        displayRestaurants();

        int choice;
        cout << "\nModify Restaurants File\n";
        cout << "1. Change Delivery Time\n2. Update Delivery Charge\n3. Exit\n";
        cout << "Choice: ";
        cin >> choice;

        if (choice == 1 || choice == 2) {
            string restaurant;
            cout << "Enter restaurant name: ";
            cin >> restaurant;

            if (choice == 1) changeDeliveryTime(restaurant);
            else if (choice == 2) updateDeliveryCharge(restaurant);
        }
    }

    void modifyMenuFile() {
        displayRestaurants();

        string restaurant;
        cout << "Enter restaurant name: ";
        cin >> restaurant;
        displayMenuForRestaurant(restaurant);

        int choice;
        string item;
        double price;

        cout << "\nModify Menu File\n";
        cout << "1. Change Item Price\n2. Exit\n";
        cout << "Choice: ";
        cin >> choice;

        if (choice == 1) {
            cout << "Enter item name: ";
            cin >> item;
            cout << "Enter new price: ";
            cin >> price;
            updateMenuItemPrice(restaurant, item, price);
        }
    }

    void viewUsers() {
        loadUsers();
        for (const auto &entry : users) {
            const User &user = entry.second;
            cout << "\nUser Details:\n";
            cout << "Name: " << user.name << "\n";
            cout << "Address: " << user.address << "\n";
            cout << "Phone: " << user.phone << "\n";
            cout << "Order Summary:\n";

            double totalBill = 0.0;
            for (size_t i = 0; i < user.orders.size(); ++i) {
                cout << "- " << user.orders[i].first << " - $" << fixed << setprecision(2) << user.orders[i].second << endl;
                totalBill += user.orders[i].second;
            }

            cout << "-----------------------------\n";
            cout << "Total Bill: $" << fixed << setprecision(2) << totalBill << endl;
        }
    }

    void saveUserOrder(const string &userName, const string &userAddress, const string &userPhone,
                       const vector<pair<string, double>> &orderItems) {
        time_t now = time(0);
        string orderDate = ctime(&now);
        orderDate.pop_back();  // Remove the newline character added by ctime

        if (users.find(userName) == users.end()) {
            users[userName] = {userName, userAddress, userPhone, orderItems, {orderDate}};
        } else {
            users[userName].orders.insert(users[userName].orders.end(), orderItems.begin(), orderItems.end());
            users[userName].orderDates.push_back(orderDate);
        }
        saveUsers();
    }

private:
  void loadUsers() {
    ifstream file(usersFile);
    if (!file) {
        cout << "Error opening file: " << usersFile << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;  // Skip empty lines
        
        User user;
        user.name = line.substr(6);  // Extract user name after "Name: "

        // Read address and phone
        if (!getline(file, user.address) || !getline(file, user.phone)) {
            cout << "Skipping user due to missing address or phone\n";
            continue;  // Skip if address or phone is missing
        }

        // Skip "Order Summary:" line
        getline(file, line); // Should be "Order Summary:" -- you can check if needed

        // Read orders until we hit the "===" separator line
        while (getline(file, line) && line != "===") {
            if (line.empty()) continue;  // Skip empty lines

            size_t pos = line.find(" - $");
            if (pos == string::npos) continue;  // Skip lines that don't contain price

            string item = line.substr(0, pos);
            size_t pricePos = line.find(" - $") + 4;
            double price = stod(line.substr(pricePos, line.find(" ", pricePos) - pricePos));

            user.orders.push_back({item, price});
        }
        users[user.name] = user;  // Store the user in the map
    }

    file.close();
}

    void saveUsers() {
        ofstream file(usersFile);
        for (const auto &entry : users) {
            const User &user = entry.second;
            file << "Name: " << user.name << endl;
            file << "Address: " << user.address << endl;
            file << "Phone: " << user.phone << endl;
            file << "\nOrder Summary:" << endl;
            for (size_t i = 0; i < user.orders.size(); ++i) {
                file << user.orders[i].first << " - $" << fixed << setprecision(2) << user.orders[i].second;
                file << " on " << user.orderDates[i] << endl;
            }
            file << "===\n";
        }
        file.close();
    }

    void displayRestaurants() {
        ifstream file(restaurantsFile);
        string name;
        double charge;
        int time;
        cout << "\nAvailable Restaurants:\n";
        while (file >> name >> charge >> time) {
            cout << "- " << name << " (Delivery Charge: $" << charge << ", Delivery Time: " << time << " mins)\n";
        }
        file.close();
    }

    void displayMenuForRestaurant(const string &restaurant) {
        ifstream file(menusFile);
        string restName, itemName;
        double price;
        cout << "\nMenu for " << restaurant << ":\n";
        while (file >> restName >> itemName >> price) {
            if (restName == restaurant) {
                cout << "- " << itemName << ": $" << price << endl;
            }
        }
        file.close();
    }

    void changeDeliveryTime(const string &restaurant) {
        int newTime;
        cout << "Enter new delivery time (minutes): ";
        cin >> newTime;

        modifyRestaurantFileData(restaurant, 0, newTime, false, true);
    }

    void updateDeliveryCharge(const string &restaurant) {
        double newCharge;
        cout << "Enter new delivery charge: ";
        cin >> newCharge;

        modifyRestaurantFileData(restaurant, newCharge, 0, true, false);
    }

    void modifyRestaurantFileData(const string &restaurant, double newCharge, int newTime, bool updateCharge, bool updateTime) {
        ifstream file(restaurantsFile);
        vector<string> lines;
        string name;
        double charge;
        int time;

        while (file >> name >> charge >> time) {
            if (name == restaurant) {
                if (updateCharge) charge = newCharge;
                if (updateTime) time = newTime;
            }
            lines.push_back(name + " " + to_string(charge) + " " + to_string(time));
        }
        file.close();

        ofstream outFile(restaurantsFile);
        for (const auto &line : lines) outFile << line << endl;
        outFile.close();
    }

    void updateMenuItemPrice(const string &restaurant, const string &item, double newPrice) {
        ifstream file(menusFile);
        vector<string> lines;
        string restName, itemName;
        double price;

        while (file >> restName >> itemName >> price) {
            if (restName == restaurant && itemName == item) {
                price = newPrice;
            }
            lines.push_back(restName + " " + itemName + " " + to_string(price));
        }
        file.close();

        ofstream outFile(menusFile);
        for (const auto &line : lines) outFile << line << endl;
        outFile.close();
    }
};

int main() {
    string restaurantsFile = "restaurants.txt";
    string menusFile = "menus.txt";
    string usersFile = "users.txt";

    AdminPage adminPage(restaurantsFile, menusFile, usersFile);
    adminPage.displayAdminMenu();

    return 0;
}