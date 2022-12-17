#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace std;

const int DATA_ROWS_COUNT = 435;
const int DATA_COLS_COUNT = 17;
const int CLASS_VARIABLE_INDEX = 0;

const int CLASS_VARIABLE_VALUES_COUNT = 2;
const int FEATURE_VARIABLE_VALUES_COUNT = 3;

const char CLASS_VARIABLE_VALUES[CLASS_VARIABLE_VALUES_COUNT] = { 'd', 'r' };
const char FEATURE_VARIABLE_VALUES[FEATURE_VARIABLE_VALUES_COUNT] = { 'n', 'y', '?' };

const int MAX_INPUT_LINE_LENGTH = 45;

int readDataValue(char inputChar, bool classVariable) {
    if (classVariable) {
        for (int val = 0; val < CLASS_VARIABLE_VALUES_COUNT; val++) {
            if (inputChar == CLASS_VARIABLE_VALUES[val]) {
                return val;
            }
        }
        throw runtime_error("Invalid data value of a class variable.");
    }
    else {
        for (int val = 0; val < FEATURE_VARIABLE_VALUES_COUNT; val++) {
            if (inputChar == FEATURE_VARIABLE_VALUES[val]) {
                return val;
            }
        }
        throw runtime_error("Invalid data value of a feature variable.");
    }
    return -1;
}

void readData(const char *filepath, int *data) {
    ifstream file(filepath);
    if (!file.is_open()) {
        throw runtime_error("Cannot open data file.");
    }

    int dataIdx = 0;
    char inputLine[MAX_INPUT_LINE_LENGTH + 1];
    while (file >> inputLine) {
        int colIdx = 0;
        int inpIdx = 0;
        while (inputLine[inpIdx] != '\0') {
            data[dataIdx++] = readDataValue(inputLine[inpIdx], colIdx == CLASS_VARIABLE_INDEX);
            colIdx++;
            while (inputLine[inpIdx] != '\0') {
                inpIdx++;
                if (inputLine[inpIdx] == ',') {
                    inpIdx++;
                    break;
                }
            }
        }
        if (colIdx < DATA_COLS_COUNT) {
            throw runtime_error("Invalid data row with fewer columns than needed.");
        }
    }

    file.close();
    if (file.is_open()) {
        throw runtime_error("Cannot close data file.");
    }
}

void printData(const int *data) {
    int dataIdx = 0;
    for (int row = 0; row < DATA_ROWS_COUNT; row++) {
        for (int col = 0; col < DATA_COLS_COUNT; col++) {
            if (col == CLASS_VARIABLE_INDEX) {
                if (CLASS_VARIABLE_VALUES[data[dataIdx]] == 'd') {
                    cout << "democrat";
                }
                else {
                    cout << "republican";
                }
            }
            else {
                cout << FEATURE_VARIABLE_VALUES[data[dataIdx]];
            }
            if (col + 1 < DATA_COLS_COUNT) {
                cout << ",";
            }
            dataIdx++;
        }
        cout << "\n";
    }
}

int main() {
    int data[DATA_ROWS_COUNT * DATA_COLS_COUNT];
    readData("data/house-votes-84.data", data);
}