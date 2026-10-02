
#include <iostream>
#include <queue>
#include <string>
using namespace std;

int main() {
    queue<string> emergencyQueue;
    queue<string> normalQueue;

    int choice;
    string name;

    do {
        cout << "\n--- Patient Queue Management ---\n";
        cout << "1. Add Patient\n";
        cout << "2. Display Queue\n";
        cout << "3. Treat Patient\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                int emergency;
                cout << "Enter patient name: ";
                cin >> name;

                cout << "Emergency patient? (1 = Yes, 0 = No): ";
                cin >> emergency;

                if (emergency == 1) {
                    emergencyQueue.push(name);
                    cout << "Emergency patient added!\n";
                } else {
                    normalQueue.push(name);
                    cout << "Normal patient added!\n";
                }
                break;
            }

            case 2: {
                cout << "\nEmergency Queue:\n";
                queue<string> temp1 = emergencyQueue;

                if (temp1.empty())
                    cout << "No emergency patients.\n";

                while (!temp1.empty()) {
                    cout << temp1.front() << endl;
                    temp1.pop();
                }

                cout << "\nNormal Queue:\n";
                queue<string> temp2 = normalQueue;

                if (temp2.empty())
                    cout << "No normal patients.\n";

                while (!temp2.empty()) {
                    cout << temp2.front() << endl;
                    temp2.pop();
                }
                break;
            }

            case 3:
                if (!emergencyQueue.empty()) {
                    cout << "Treating emergency patient: "
                         << emergencyQueue.front() << endl;
                    emergencyQueue.pop();
                } else if (!normalQueue.empty()) {
                    cout << "Treating normal patient: "
                         << normalQueue.front() << endl;
                    normalQueue.pop();
                } else {
                    cout << "No patients waiting.\n";
                }
                break;

            case 4:
                cout << "Exiting program...\n";
                break;

            default:
                cout << "Invalid choice!\n";
        }

    } while (choice != 4);

    return 0;
}