#include <iostream>
#include <string>
#include <cstring>
#include <bits/stdc++.h>
#include <curl/curl.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <fstream>  // Include for ofstream
#include "preprocess.hpp"
#include "util.hpp"

using namespace std;

// Function to print a line separator for the table
void printSeparator(const vector<int>& columnWidths) {
    for (int width : columnWidths) {
        cout << std::setw(width + 4) << std::setfill('-') << "+";
    }
    cout << "+" << endl;
}

// Function to export data to Excel (CSV format)
// Function to export data to Excel (CSV format)
void exportToExcel(const vector<string>& columnLabels, const vector<vector<string>>& data, const string& filename) {
    ofstream outfile(filename, ios::out | ios::app);  // Open in append mode

    // If the file is empty, write column labels
    if (outfile.tellp() == 0) {
        for (size_t i = 0; i < columnLabels.size(); ++i) {
            outfile << columnLabels[i];
            if (i < columnLabels.size() - 1) {
                outfile << ",";
            }
        }
        outfile << endl;
    }

    // Write data
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            outfile << row[i];
            if (i < row.size() - 1) {
                outfile << ",";
            }
        }
        outfile << endl;
    }

    outfile.close();
}


// Dummy definition of URL and WriteCallback
const char *URL = "https://api-inference.huggingface.co/models/tscholak/3vnuv1vf";

// Return the size of the readBuffer
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int main(void) {
    CURL *curl, *curldup;
    CURLcode res;
    char errbuf[CURL_ERROR_SIZE];
    string readBuffer;
    string text;
    string schema = SCHEMA;
    string input;
    string sqlquery;
    int start_pos, end_pos;

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *slist1 = NULL;
        slist1 = curl_slist_append(slist1, "Content-Type: application/json");
        slist1 = curl_slist_append(slist1, API_TOKEN);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);

        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

        while (true) {
            cout << "Enter text: ";
            getline(cin, text);
            cout << "Processing..." << endl;

            // Perform preprocessing to convert date to iso format
            text = convertDate(text);
            input = "{\"inputs\": \"" + text + schema + "\", \"wait_for_model\" : \"true\"}";
            const char *json_input = input.c_str();

            curldup = curl_easy_duphandle(curl);
            curl_easy_setopt(curldup, CURLOPT_POSTFIELDS, json_input);
            errbuf[0] = 0;
            readBuffer.erase();

            res = curl_easy_perform(curldup);
            curl_easy_reset(curldup);
            curl_easy_cleanup(curldup);

            if (res != CURLE_OK) {
                cout << "libcurl: (" << res << ")";
                if (strlen(errbuf))
                    cout << errbuf << endl;
                exit(0);
            }

            // Remove '\' from the sql query
            readBuffer.erase(remove(readBuffer.begin(), readBuffer.end(), '\\'), readBuffer.end());

            start_pos = readBuffer.find("|");
            end_pos = readBuffer.find("}");
            sqlquery = readBuffer.substr(start_pos + 2, end_pos - start_pos - 3);
            cout << "SQL Query: " << sqlquery << ";" << endl << endl;

            // Connect to MySQL and execute the query
            try {
                sql::mysql::MySQL_Driver *driver;
                sql::Connection *con;

                driver = sql::mysql::get_mysql_driver_instance();
                con = driver->connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASSWD);

                con->setSchema(MYSQL_TABLE);

                sql::Statement *stmt;
                sql::ResultSet *res;

                stmt = con->createStatement();
                res = stmt->executeQuery(sqlquery);

                int numColumns = res->getMetaData()->getColumnCount();
                vector<int> columnWidths(numColumns, 0);

                // Calculate maximum width for each column
                for (int i = 1; i <= numColumns; ++i) {
                    int maxColumnWidth = std::max((int)res->getMetaData()->getColumnLabel(i).length(), 10); // Minimum width 10
                    columnWidths[i - 1] = maxColumnWidth;
                }

                // Print table header
                printSeparator(columnWidths);
                cout << "|";
                for (int i = 1; i <= numColumns; ++i) {
                    cout << std::setw(columnWidths[i - 1]) << std::left << res->getMetaData()->getColumnLabel(i) << "|";
                }
                cout << endl;
                printSeparator(columnWidths);

                // Reset result set to the beginning
                res->beforeFirst();

                // Print table data to console
                vector<vector<string>> rowData;
                while (res->next()) {
                    vector<string> row;
                    for (int i = 1; i <= numColumns; ++i) {
                        row.push_back(res->getString(i));
                    }
                    rowData.push_back(row);
                }

                // Export data to Excel (CSV format)
                vector<string> columnLabels;
                for (int i = 1; i <= numColumns; ++i) {
                    columnLabels.push_back(res->getMetaData()->getColumnLabel(i));
                }

                exportToExcel(columnLabels, rowData, "output.csv");

                // Print table data to console
                for (const auto& row : rowData) {
                    cout << "|";
                    for (size_t i = 0; i < row.size(); ++i) {
                        cout << std::setw(columnWidths[i]) << std::left << row[i] << "|";
                    }
                    cout << endl;
                }

                // Clean up
                delete res;
                delete stmt;
                delete con;

            } catch (sql::SQLException &e) {
                cout << "# ERR: " << e.what() << endl;
            }

            // Ask the user if they want to continue
            cout << endl << "Do you have another Query? Y/y to continue, any other key to exit: ";
            char choice;
            cin >> choice;
            cin.ignore(); // Ignore the newline character left in the input buffer
            if (choice != 'y' && choice != 'Y') {
                break; // Exit the loop if the user doesn't want to continue
            }
        }

        // Clean up
        curl_easy_cleanup(curl);
    }

    return 0;
}
//g++ -Wall -I/usr/include/cppconn -o ct curl4.cpp -L/usr/lib -lmysqlcppconn -lcurl
