#include <iostream>
using namespace std;

int main()
{
    string itemName;
    int quantity;
    float cost;

    cout << "Enter Item Name: ";
    cin >> itemName;
    cout << "Enter quantity of items: ";
    cin >> quantity;
    cout << "Enter cost for each item: ";
    cin >> cost;

    cout << "Total cost for " << itemName << " is $" << (quantity * cost) << endl;

    return 0;
}
