#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <iomanip>

using namespace std;

// Template class for MenuItem
template <typename T>
class MenuItem {
public:
    T name;
    double price;

    MenuItem(const T &name, double price) : name(name), price(price) {}

    void display() const {
        cout << "Item: " << name << " - $" << fixed << setprecision(2) << price << endl;
    }
};

// Template class for Restaurant
template <typename T>
class Restaurant {
private:
    T name;
    double deliveryCharge;
    int deliveryTime; // in minutes
    vector<MenuItem<T>> menu;

public:
    Restaurant(const T &name, double deliveryCharge, int deliveryTime)
        : name(name), deliveryCharge(deliveryCharge), deliveryTime(deliveryTime) {}

    void addMenuItem(const MenuItem<T> &item) {
        menu.push_back(item);
    }

    void displayMenu() const {
        cout << "\nMenu for " << name << ":" << endl;
        for (const auto &item : menu) {
            item.display();
        }
    }

    const T& getName() const { return name; }
    double getDeliveryCharge() const { return deliveryCharge; }
    int getDeliveryTime() const { return deliveryTime; }
    const vector<MenuItem<T>>& getMenu() const { return menu; }
};

// Main Class to handle system operations
class MainClass {
private:
    map<string, unique_ptr<Restaurant<string>>> restaurants;
    vector<pair<string, MenuItem<string>>> finalCart; // Pair of restaurant name and MenuItem for final cart
    double totalBill = 0.0;
    int estimatedTime = 0;

    // Function to load restaurants from file
    void loadRestaurants(const string &filename) {
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Could not open restaurants file." << endl;
            return;
        }

        string name;
        double charge;
        int time;
        while (file >> name >> charge >> time) {
            restaurants[name] = make_unique<Restaurant<string>>(name, charge, time);
        }
    }

    // Function to load menus from file
    void loadMenus(const string &filename) {
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Could not open menus file." << endl;
            return;
        }

        string restaurantName, itemName;
        double price;
        while (file >> restaurantName >> itemName >> price) {
            if (restaurants.find(restaurantName) != restaurants.end()) {
                restaurants[restaurantName]->addMenuItem(MenuItem<string>(itemName, price));
            }
        }
    }

public:
    MainClass(const string &restaurantsFile, const string &menusFile) {
        loadRestaurants(restaurantsFile);
        loadMenus(menusFile);
    }

    void displayRestaurants() const {
        cout << "\nAvailable Restaurants:" << endl;
        int i = 1;
        for (const auto &entry : restaurants) {
            cout << i << ". " << entry.first << " (Delivery Charge: $" << entry.second->getDeliveryCharge()
                 << ", Delivery Time: " << entry.second->getDeliveryTime() << " mins)" << endl;
            ++i;
        }
    }

    void selectRestaurant() {
        int choice;
        displayRestaurants();

        while (true) {
            cout << "\nSelect a restaurant by number (or 0 to finish): ";
            cin >> choice;

            if (choice == 0) break; // User finished selecting restaurants

            int restaurantIndex = 1;
            string selectedRestaurant;
            for (const auto &entry : restaurants) {
                if (restaurantIndex == choice) {
                    selectedRestaurant = entry.first;
                    break;
                }
                restaurantIndex++;
            }

            if (!selectedRestaurant.empty() && restaurants.find(selectedRestaurant) != restaurants.end()) {
                cout << "\nYou have selected: " << selectedRestaurant << endl;
                restaurants[selectedRestaurant]->displayMenu();
                placeOrder(restaurants[selectedRestaurant]->getMenu(), selectedRestaurant);
            } else {
                cout << "Invalid choice. Please try again." << endl;
            }
        }

        reviewCart();  // Review the final cart before checkout
    }

    void placeOrder(const vector<MenuItem<string>> &menu, const string &restaurantName) {
        string itemName;
        int quantity;

        cout << "\nSelect items from the menu. Type 'done' to finish for this restaurant." << endl;
        while (true) {
            cout << "Enter item name: ";
            cin >> itemName;
            if (itemName == "done") break;

            auto it = find_if(menu.begin(), menu.end(), [&](const MenuItem<string> &item) {
                return item.name == itemName;
            });

            if (it != menu.end()) {
                cout << "Enter quantity: ";
                cin >> quantity;
                double itemTotal = it->price * quantity;

                // Add item(s) to final cart
                for (int i = 0; i < quantity; ++i) {
                    finalCart.emplace_back(restaurantName, *it);
                }
                
                totalBill += itemTotal;
                cout << "Added " << quantity << " x " << itemName << " from " << restaurantName 
                     << " for $" << itemTotal << endl;
            } else {
                cout << "Item not found. Try again." << endl;
            }
        }
    }

    void reviewCart() {
        cout << "\nYour Final Cart:" << endl;
        map<string, int> itemCount;
        for (const auto &entry : finalCart) {
            itemCount[entry.second.name]++;
            cout << entry.second.name << " from " << entry.first << " - $" << entry.second.price << endl;
        }

        cout << "\nTotal Bill: $" << totalBill << endl;

        string removeItem;
        while (true) {
            cout << "\nWould you like to remove any items? (yes/no): ";
            cin >> removeItem;
            if (removeItem == "no") break;

            cout << "Enter the item name to remove: ";
            cin >> removeItem;
            
            auto it = find_if(finalCart.begin(), finalCart.end(), [&](const pair<string, MenuItem<string>> &entry) {
                return entry.second.name == removeItem;
            });

            if (it != finalCart.end()) {
                totalBill -= it->second.price;
                finalCart.erase(it);
                cout << removeItem << " removed from cart. Updated Total: $" << totalBill << endl;
            } else {
                cout << "Item not found in the cart. Try again." << endl;
            }
        }

        checkout();
    }

    void checkout() {
        double maxDeliveryCharge = 0.0;
        
        // Calculate the maximum delivery charge from selected restaurants
        for (const auto &entry : finalCart) {
            maxDeliveryCharge = max(maxDeliveryCharge, restaurants[entry.first]->getDeliveryCharge());
        }

        cout << "\nFinal Order Summary:" << endl;
        for (const auto &entry : finalCart) {
            cout << entry.second.name << " from " << entry.first << " - $" << entry.second.price << endl;
        }

        totalBill += maxDeliveryCharge;
        cout << "Total with Delivery Charges: $" << totalBill << endl;

        // Get user details
        string name, address, phone;
        cout << "\nEnter your name: ";
        cin >> name;
        cout << "Enter your address: ";
        cin.ignore();  // To clear any leftover input buffer
        getline(cin, address);
        cout << "Enter your phone number: ";
        cin >> phone;

        // Save user details to file
        saveUserDetails(name, address, phone);
    }

    // Function to save user details and order summary to users.txt
    void saveUserDetails(const string &name, const string &address, const string &phone) {
        ofstream outFile("users.txt", ios::app);
        if (!outFile) {
            cerr << "Error: Could not open users.txt file for writing." << endl;
            return;
        }

        outFile << "Name: " << name << endl;
        outFile << "Address: " << address << endl;
        outFile << "Phone: " << phone << endl;

        outFile << "\nOrder Summary:" << endl;
        for (const auto &entry : finalCart) {
            outFile << entry.second.name << " from " << entry.first << " - $" << entry.second.price << endl;
        }

        outFile << "\nTotal Bill: $" << totalBill << endl;
        outFile << "===\n" << endl;
    }
};

int main() {
    // Files containing restaurants and menu items
    string restaurantsFile = "restaurants.txt";
    string menusFile = "menus.txt";

    // Load system with restaurants and menus
    MainClass system(restaurantsFile, menusFile);

    // User selects restaurants and places orders
    system.selectRestaurant();

    return 0;
}