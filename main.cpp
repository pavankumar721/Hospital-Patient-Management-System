
#include <iostream>
#include <string>
#include <cctype>
#include "sqlite3.h"

using namespace std;

// Read an integer safely
int readInt(const string& message, int minValue, int maxValue) {
    int value;

    while (true) {
        cout << message;

        if (cin >> value && value >= minValue && value <= maxValue) {
            cin.ignore(10000, '\n');
            return value;
        }

        cout << "Invalid input. Enter a value between "
             << minValue << " and " << maxValue << ".\n";
        cin.clear();
        cin.ignore(10000, '\n');
    }
}

// Validate a person's name

bool validName(const string& name) {
    if (name.empty()) return false;

    bool hasLetter = false;

    for (char ch : name) {
        if (isalpha(static_cast<unsigned char>(ch))) {
            hasLetter = true;
        } else if (ch != ' ') {
            return false;
        }
    }

    return hasLetter;
}

// Read a valid name
string readName(const string& message) {
    string name;

    while (true) {
        cout << message;
        getline(cin >> ws, name);

        if (validName(name)) return name;

        cout << "Invalid name. Use only letters and spaces.\n";
    }
}

// Check whether patient exists
bool patientExists(sqlite3* db, int id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT 1 FROM patients WHERE id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);
    bool exists = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);

    return exists;
}

// Register patient
void registerPatient(sqlite3* db) {
    string name, gender, disease, phone;

    name = readName("Enter Name: ");
    int age = readInt("Enter Age: ", 1, 100);

    cout << "Enter Gender: ";
    getline(cin >> ws, gender);

    cout << "Enter Disease: ";
    getline(cin, disease);

    cout << "Enter Phone: ";
    getline(cin, phone);

    int emergency = readInt("Emergency? (1 = Yes, 0 = No): ", 0, 1);

    const char* sql =
        "INSERT INTO patients(name, age, gender, disease, phone, emergency) "
        "VALUES(?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to prepare registration!\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, age);
    sqlite3_bind_text(stmt, 3, gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, disease.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, emergency);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        cout << "Patient registered successfully!\n";
        cout << "Patient ID: " << sqlite3_last_insert_rowid(db) << '\n';
    } else {
        cout << "Registration failed!\n";
    }

    sqlite3_finalize(stmt);
}

// Display one patient record
void printPatient(sqlite3_stmt* stmt) {
    cout << "\nID: " << sqlite3_column_int(stmt, 0);
    cout << "\nName: " << sqlite3_column_text(stmt, 1);
    cout << "\nAge: " << sqlite3_column_int(stmt, 2);
    cout << "\nGender: " << sqlite3_column_text(stmt, 3);
    cout << "\nDisease: " << sqlite3_column_text(stmt, 4);
    cout << "\nPhone: " << sqlite3_column_text(stmt, 5);
    cout << "\nEmergency: "
         << (sqlite3_column_int(stmt, 6) == 1 ? "Yes" : "No");
    cout << "\n----------------------\n";
}

// Display all patients
void displayPatients(sqlite3* db) {
    const char* sql = "SELECT * FROM patients ORDER BY id";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to retrieve patients!\n";
        return;
    }

    cout << "\n--- Patient Records ---\n";
    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = true;
        printPatient(stmt);
    }

    if (!found) cout << "No patients found!\n";

    sqlite3_finalize(stmt);
}

// Search patient
void searchPatient(sqlite3* db) {
    int id = readInt("Enter Patient ID: ", 1, 2147483647);

    const char* sql = "SELECT * FROM patients WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Search failed!\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        printPatient(stmt);
    else
        cout << "Patient not found!\n";

    sqlite3_finalize(stmt);
}

// Update patient
void updatePatient(sqlite3* db) {
    int id = readInt("Enter Patient ID to update: ", 1, 2147483647);

    if (!patientExists(db, id)) {
        cout << "Patient not found!\n";
        return;
    }

    string name = readName("Enter New Name: ");
    int age = readInt("Enter New Age (1-100): ", 1, 100);

    string gender, disease, phone;

    cout << "Enter New Gender: ";
    getline(cin >> ws, gender);

    cout << "Enter New Disease: ";
    getline(cin, disease);

    cout << "Enter New Phone: ";
    getline(cin, phone);

    const char* sql =
        "UPDATE patients SET name=?, age=?, gender=?, disease=?, phone=? "
        "WHERE id=?";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to prepare update!\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, age);
    sqlite3_bind_text(stmt, 3, gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, disease.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, id);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        cout << "Patient updated successfully!\n";
    else
        cout << "Update failed!\n";

    sqlite3_finalize(stmt);
}

// Add patient to queue
void addToQueue(sqlite3* db) {
    int id = readInt("Enter Patient ID: ", 1, 2147483647);

    if (!patientExists(db, id)) {
        cout << "Patient ID does not exist!\n";
        return;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* checkSQL =
        "SELECT 1 FROM patient_queue WHERE patient_id = ?";

    if (sqlite3_prepare_v2(db, checkSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Database error!\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << "Patient is already in the queue!\n";
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    int priority = 0;
    const char* prioritySQL =
        "SELECT emergency FROM patients WHERE id = ?";

    if (sqlite3_prepare_v2(db, prioritySQL, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Database error!\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        priority = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    const char* insertSQL =
        "INSERT INTO patient_queue(patient_id, priority) VALUES(?, ?)";

    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to prepare queue entry!\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, priority);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        cout << "Patient added to queue successfully!\n";
    else
        cout << "Failed to add patient to queue!\n";

    sqlite3_finalize(stmt);
}

// Display queue
void displayQueue(sqlite3* db) {
    const char* sql =
        "SELECT q.queue_id, p.id, p.name, q.priority "
        "FROM patient_queue q "
        "JOIN patients p ON q.patient_id = p.id "
        "ORDER BY q.priority DESC, q.queue_id ASC";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to display queue!\n";
        return;
    }

    cout << "\n--- Patient Queue ---\n";
    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = true;

        cout << "\nQueue ID: " << sqlite3_column_int(stmt, 0);
        cout << "\nPatient ID: " << sqlite3_column_int(stmt, 1);
        cout << "\nName: " << sqlite3_column_text(stmt, 2);
        cout << "\nPriority: "
             << (sqlite3_column_int(stmt, 3) == 1 ? "Emergency" : "Normal");
        cout << "\n----------------------\n";
    }

    if (!found) cout << "Queue is empty!\n";

    sqlite3_finalize(stmt);
}

// Treat next patient

void treatNextPatient(sqlite3* db) {
    const char* selectSQL =
        "SELECT queue_id, patient_id, priority "
        "FROM patient_queue "
        "ORDER BY priority DESC, queue_id ASC LIMIT 1";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to retrieve next patient!\n";
        return;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cout << "Queue is empty!\n";
        sqlite3_finalize(stmt);
        return;
    }

    int queueID = sqlite3_column_int(stmt, 0);
    int patientID = sqlite3_column_int(stmt, 1);
    int priority = sqlite3_column_int(stmt, 2);

    sqlite3_finalize(stmt);

    const char* deleteSQL =
        "DELETE FROM patient_queue WHERE queue_id = ?";

    if (sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Unable to prepare treatment!\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, queueID);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        cout << "\nTreating "
             << (priority == 1 ? "Emergency" : "Normal")
             << " Patient ID: " << patientID << '\n';
        cout << "Patient removed from waiting queue.\n";

        // Check whether the queue is empty
        sqlite3_stmt* countStmt = nullptr;
        const char* countSQL = "SELECT COUNT(*) FROM patient_queue";

        if (sqlite3_prepare_v2(db, countSQL, -1,
                               &countStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(countStmt) == SQLITE_ROW) {
                int remaining = sqlite3_column_int(countStmt, 0);

                // Reset queue ID when queue becomes empty
                if (remaining == 0) {
                    sqlite3_exec(db,
                        "DELETE FROM sqlite_sequence "
                        "WHERE name='patient_queue'",
                        nullptr, nullptr, nullptr);
                }
            }

            sqlite3_finalize(countStmt);
        }
    } else {
        cout << "Unable to remove patient from queue!\n";
    }

    sqlite3_finalize(stmt);
}

// Main function
int main() {
    sqlite3* db = nullptr;

    if (sqlite3_open("hospital.db", &db) != SQLITE_OK) {
        cout << "Database connection failed!\n";
        if (db) sqlite3_close(db);
        return 1;
    }

    char* errorMessage = nullptr;

    const char* createPatients =
        "CREATE TABLE IF NOT EXISTS patients ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "age INTEGER NOT NULL,"
        "gender TEXT NOT NULL,"
        "disease TEXT NOT NULL,"
        "phone TEXT NOT NULL,"
        "emergency INTEGER NOT NULL DEFAULT 0)";

    if (sqlite3_exec(db, createPatients, nullptr, nullptr,
                     &errorMessage) != SQLITE_OK) {
        cout << "Patient table error: " << errorMessage << '\n';
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return 1;
    }

    const char* createQueue =
        "CREATE TABLE IF NOT EXISTS patient_queue ("
        "queue_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "patient_id INTEGER NOT NULL UNIQUE,"
        "priority INTEGER NOT NULL DEFAULT 0)";

    if (sqlite3_exec(db, createQueue, nullptr, nullptr,
                     &errorMessage) != SQLITE_OK) {
        cout << "Queue table error: " << errorMessage << '\n';
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return 1;
    }

    while (true) {
        cout << "\n===== Hospital Patient Management =====\n";
        cout << "1. Register Patient\n";
        cout << "2. Display Patients\n";
        cout << "3. Search Patient\n";
        cout << "4. Update Patient\n";
        cout << "5. Exit\n";
        cout << "6. Add Patient to Queue\n";
        cout << "7. Display Patient Queue\n";
        cout << "8. Treat Next Patient\n";

        int choice = readInt("Enter choice: ", 1, 8);

        switch (choice) {
            case 1:
                registerPatient(db);
                break;
            case 2:
                displayPatients(db);
                break;
            case 3:
                searchPatient(db);
                break;
            case 4:
                updatePatient(db);
                break;
            case 5:
                sqlite3_close(db);
                cout << "Program closed.\n";
                return 0;
            case 6:
                addToQueue(db);
                break;
            case 7:
                displayQueue(db);
                break;
            case 8:
                treatNextPatient(db);
                break;
        }
    }
}