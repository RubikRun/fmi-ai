#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
using namespace std;

const int DATA_ROWS_COUNT = 435;
const int DATA_COLS_COUNT = 17;
const int CLASS_VARIABLE_INDEX = 0;

const int FEATURES_COUNT = DATA_COLS_COUNT - 1;

const int CLASS_VALUES_COUNT = 2;
const int FEATURE_VALUES_COUNT = 3;

const char CLASS_VARIABLE_VALUES[CLASS_VALUES_COUNT] = { 'd', 'r' };
const char FEATURE_VARIABLE_VALUES[FEATURE_VALUES_COUNT] = { 'n', 'y', '?' };

const int FEATURE_VALUE_MISSING = 2;

const int MAX_INPUT_LINE_LENGTH = 45;

const float LAPLACE_ALPHA = 1.0f;

const int TEST_INTERVALS_COUNT = 10;

struct Data {
    int data[DATA_ROWS_COUNT * DATA_COLS_COUNT];
    int skipBegin = -1;
    int skipEnd = -1;

    void readFromFile(const char *filepath) {
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
                data[dataIdx++] = getValueFromChar(inputLine[inpIdx], colIdx == CLASS_VARIABLE_INDEX);
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

    int getNextIndex(int index) const {
        if (index < 0 || index >= DATA_ROWS_COUNT * DATA_COLS_COUNT) {
            throw runtime_error("Invalid data index.");
        }
        const int row = (index + 1) / DATA_COLS_COUNT;
        if (row >= skipBegin && row < skipEnd) {
            return skipEnd * DATA_COLS_COUNT;
        }
        return index + 1;
    }

    int getNextRow(int row) const {
        if (row < 0 || row >= DATA_ROWS_COUNT) {
            throw runtime_error("Invalid row index.");
        }
        if (row + 1 >= skipBegin && row + 1 < skipEnd) {
            return skipEnd;
        }
        return row + 1;
    }

    int getFirstIndex() const {
        if (skipBegin == 0) {
            return skipEnd * DATA_COLS_COUNT;
        }
        return 0;
    }

    int getFirstRow() const {
        if (skipBegin == 0) {
            return skipEnd;
        }
        return 0;
    }

    int operator[](int index) const {
        if (index < 0 || index >= DATA_ROWS_COUNT * DATA_COLS_COUNT) {
            throw runtime_error("Invalid data index.");
        }
        return data[index];
    }

    void print() const {
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

private:
    static int getValueFromChar(char inputChar, bool classVariable) {
        if (classVariable) {
            for (int val = 0; val < CLASS_VALUES_COUNT; val++) {
                if (inputChar == CLASS_VARIABLE_VALUES[val]) {
                    return val;
                }
            }
            throw runtime_error("Invalid data value of a class variable.");
        }
        else {
            for (int val = 0; val < FEATURE_VALUES_COUNT; val++) {
                if (inputChar == FEATURE_VARIABLE_VALUES[val]) {
                    return val;
                }
            }
            throw runtime_error("Invalid data value of a feature variable.");
        }
        return -1;
    }
};

int countClassVariablesWithValue(const Data &data, int value) {
    int count = 0;
    int dataIdx = data.getFirstIndex();
    for (int row = data.getFirstRow(); row < DATA_ROWS_COUNT; row = data.getNextRow(row)) {
        for (int featIdx = 0; featIdx < DATA_COLS_COUNT; featIdx++) {
            if (featIdx == CLASS_VARIABLE_INDEX && data[dataIdx] == value) {
                count++;
            }
            dataIdx = data.getNextIndex(dataIdx);
        }
    }
    return count;
}

int countFeatVariablesWithValue(const Data &data, int targetFeatVal, int targetFeatIdx, int targetClassVal) {
    int count = 0;
    int dataIdx = data.getFirstIndex();
    for (int row = data.getFirstRow(); row < DATA_ROWS_COUNT; row = data.getNextRow(row)) {
        bool classMatch = false;
        bool featMatch = false;
        for (int featIdx = 0; featIdx < DATA_COLS_COUNT; featIdx++) {
            const int featVal = data[dataIdx];
            dataIdx = data.getNextIndex(dataIdx);
            if (featIdx == CLASS_VARIABLE_INDEX && featVal == targetClassVal) {
                classMatch = true;
            }
            else if (featIdx == targetFeatIdx && featVal == targetFeatVal) {
                featMatch = true;
            }
        }
        if (classMatch && featMatch) {
            count++;
        }
    }
    return count;
}

void calcProbCounts(const Data &data, int *probCounts) {
    int probIdx = 0;
    for (int classVal = 0; classVal < CLASS_VALUES_COUNT; classVal++) {
        probCounts[probIdx++] = countClassVariablesWithValue(data, classVal);
    }
    for (int classVal = 0; classVal < CLASS_VALUES_COUNT; classVal++) {
        for (int featIdx = 0; featIdx < DATA_COLS_COUNT; featIdx++) {
            if (featIdx == CLASS_VARIABLE_INDEX) {
                continue;
            }
            for (int featVal = 0; featVal < FEATURE_VALUES_COUNT; featVal++) {
                if (featVal == FEATURE_VALUE_MISSING) {
                    continue;
                }
                probCounts[probIdx++] = countFeatVariablesWithValue(data, featVal, featIdx, classVal);
            }
        }
    }
}

float calcLaplaceProb(float numerator, float denominator) {
    return (numerator + LAPLACE_ALPHA) / (denominator + LAPLACE_ALPHA * float(FEATURES_COUNT));
}

float calcProbOfClassValue(const int *probCounts, int *featureValues, int classValue) {
    const int classRowsCount = probCounts[classValue];
    const float classProb = calcLaplaceProb(float(classRowsCount), float(DATA_ROWS_COUNT));
    float classLogProb = log(classProb);

    int probIdx = classValue * FEATURES_COUNT * (FEATURE_VALUES_COUNT - 1);
    int featIdx = 0;

    for (int col = 0; col < DATA_COLS_COUNT; col++) {
        if (col == CLASS_VARIABLE_INDEX) {
            continue;
        }

        const int featCount = probCounts[probIdx + featureValues[featIdx]];
        const float featProb = calcLaplaceProb(float(featCount), float(classRowsCount));

        classLogProb += log(featProb);

        probIdx++;
        featIdx++;
    }

    return classLogProb;
}

int chooseClassValue(const int *probCounts, int *featureValues) {
    int bestClassValue = -1;
    float bestProb = 1.0f;
    for (int classValue = 0; classValue < CLASS_VALUES_COUNT; classValue++) {
        const float classProb = calcProbOfClassValue(probCounts, featureValues, classValue);
        if (bestClassValue == -1 || classProb > bestProb) {
            bestClassValue = classValue;
            bestProb = classProb;
        }
    }
    return bestClassValue;
}

void getIntervals(int *intervals) {
    const int intervalLength = DATA_ROWS_COUNT / TEST_INTERVALS_COUNT;
    for (int intervalIdx = 1; intervalIdx < TEST_INTERVALS_COUNT; intervalIdx++) {
        intervals[intervalIdx] = intervalIdx * intervalLength;
    }
}

void fillFeatureValuesFromRow(const Data &data, int row, int *featureValues, int &realClass) {
    int dataIdx = row * DATA_COLS_COUNT;
    int featIdx = 0;
    realClass = -1;
    for (int col = 0; col < DATA_COLS_COUNT; col++) {
        if (col == CLASS_VARIABLE_INDEX) {
            realClass = data[dataIdx];
        }
        else {
            featureValues[featIdx++] = data[dataIdx];
        }
        dataIdx++;
    }
}

float calcAccuracy(const Data &data, const int *probCounts) {
    int succCount = 0;
    for (int row = data.skipBegin; row < data.skipEnd; row++) {
        int featureValues[FEATURES_COUNT];
        int realClass;
        fillFeatureValuesFromRow(data, row, featureValues, realClass);
        const int chosenClass = chooseClassValue(probCounts, featureValues);
        if (chosenClass == realClass) {
            succCount++;
        }
    }
    const int testRowsCount = data.skipEnd - data.skipBegin;
    return float(succCount) / float(testRowsCount);
}

void accuracyTest(Data &data) {
    int intervals[TEST_INTERVALS_COUNT] = { 0 };
    getIntervals(intervals);

    float accuSum = 0.0f;
    for (int chosenInterval = 0; chosenInterval < TEST_INTERVALS_COUNT; chosenInterval++) {
        data.skipBegin = intervals[chosenInterval];
        data.skipEnd = (chosenInterval + 1 < TEST_INTERVALS_COUNT) ? intervals[chosenInterval + 1] : DATA_ROWS_COUNT;

        int probCounts[CLASS_VALUES_COUNT + CLASS_VALUES_COUNT * FEATURES_COUNT * (FEATURE_VALUES_COUNT - 1)];
        calcProbCounts(data, probCounts);

        const float accu = calcAccuracy(data, probCounts);
        cout << "Accuracy of training #" << chosenInterval + 1 << " ---> " << accu << "\n";
        accuSum += accu;
    }
    const float accuAvg = accuSum / float(TEST_INTERVALS_COUNT);
    cout << "Average accuracy across the trainings ---> " << accuAvg << "\n";
}

int main() {
    Data data;
    data.readFromFile("data/house-votes-84.data");

    accuracyTest(data);
}