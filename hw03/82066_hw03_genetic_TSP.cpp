#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <cassert>
using namespace std;

typedef vector<int> IntList;
typedef vector<float> FloatList;
typedef vector<bool> BoolList;

// Constants for the bounds of the cities in 2D space
const float CITY_MIN_X = -1000.f;
const float CITY_MAX_X = 1000.f;
const float CITY_MIN_Y = -1000.f;
const float CITY_MAX_Y = 1000.f;

// Seeds the rand() function with the current time
void setupRand() {
    const unsigned timeNow = unsigned(time(nullptr));
    srand(timeNow);
}

// Returns a random float in given bounds [a, b]
float getRandFloat(float a, float b) {
    float rZeroOne = float(rand()) / float(RAND_MAX);
    return a + (b - a) * rZeroOne;
}

// Returns a random integer in given bounds [a, b]
int getRandInt(int a, int b) {
    return a + rand() % (b - a + 1);
}

// Returns a random permutation of the numbers from 0 to permSize - 1
IntList getRandomPermutation(int permSize) {
    // Array indicating which numbers are used for the permutation so far
    BoolList used(permSize, false);
    int unusedCount = permSize;
    // Constructing the final permutation in this list
    IntList perm;
    while (perm.size() < permSize) {
        // Choosing a random number of steps to walk along unused numbers.
        int steps = getRandInt(0, unusedCount - 1);
        // Start with the chosen number at the first unused number
        int chosen = 0;
        while (used[chosen]) {
            chosen = (chosen + 1) % permSize;
        }
        // At each step increase the chosen number to the next unused number
        while (steps > 0) {
            do {
                chosen = (chosen + 1) % permSize;
            } while (used[chosen]);
            steps--;
        }
        // At the end add the chosen number to the permutation
        perm.push_back(chosen);
        // and mark it as used
        used[chosen] = true;
        unusedCount--;
    }
    return perm;
}

// Structure for a city with an index and 2D float coordinates
struct City {
    City(){}
    City(int index, float x, float y) : index(index), x(x), y(y) {}

    int index = -1;
    float x = 0.f;
    float y = 0.f;
};

typedef vector<const City*> Route;
typedef vector<City> CityList;
typedef vector<Route> Population;

// Calculates the distance between two cities
float calcDist(const City &aCity, const City &bCity) {
    const float xDelta = fabsf(aCity.x - bCity.x);
    const float yDelta = fabsf(aCity.y - bCity.y);
    return sqrtf(xDelta * xDelta + yDelta * yDelta);
}

// Calculates the lengt of a given route, which is the sum of distances of every two consecutive cities from the route
float calcRouteLength(const Route &route) {
    float routeLength = 0.f;
    for (int rIdx = 0; rIdx < route.size() - 1; rIdx++) {
        assert(route[rIdx] != nullptr);
        assert(route[rIdx + 1] != nullptr);
        routeLength += calcDist(*route[rIdx], *route[rIdx + 1]);
    }
    return routeLength;
}

// Returns city with a random position with the given index
City getRandomCity(int index) {
    return City(
        index,
        getRandFloat(CITY_MIN_X, CITY_MAX_X),
        getRandFloat(CITY_MIN_Y, CITY_MAX_Y)
    );
}

// Returns a population consisting of random routes
Population getRandomPopulation(int popSize, const CityList &cities) {
    Population population(popSize);
    for (int pIdx = 0; pIdx < popSize; pIdx++) {
        // Get a random permutation of the cities' indices
        IntList perm = getRandomPermutation(cities.size());
        // Create the route by using the indices to get pointers to cities from the cities list
        Route route(cities.size());
        for (int rIdx = 0; rIdx < cities.size(); rIdx++) {
            route[rIdx] = &cities[perm[rIdx]];
        }
        // Add the new route to the population
        population[pIdx] = route;
    }
    return population;
}

// Operator for comparing two routes, needed for sort() function to sort populations of routes by the lengths of the routes
bool operator<(const Route &aRoute, const Route &bRoute) {
    return calcRouteLength(aRoute) < calcRouteLength(bRoute);
}

// Reads the number of cities from standard input and returns a list of that many cities with random positions
CityList getCities() {
    int citiesCount;
    cin >> citiesCount;

    CityList cities(citiesCount);
    for (int cityIdx = 0; cityIdx < citiesCount; cityIdx++) {
        cities[cityIdx] = getRandomCity(cityIdx);
    }

    return cities;
}

// Returns a list of cities from the example given in the homework
CityList getCitiesFromExample() {
    CityList cities = {
        City(0, 0.190032E-03,-0.285946E-03),
        City(1, 383.458,-0.608756E-03),
        City(2, -27.0206,-282.758),
        City(3, 335.751,-269.577),
        City(4, 69.4331,-246.780),
        City(5, 168.521,31.4012),
        City(6, 320.350,-160.900),
        City(7, 179.933,-318.031),
        City(8, 492.671,-131.563),
        City(9, 112.198,-110.561),
        City(10, 306.320,-108.090),
        City(11, 217.343,-447.089),
    };
    return cities;
}

// Prints a list of cities to standard output
void printCities(const CityList &cities) {
    for (const City &city : cities) {
        cout << "City #" << city.index << " with coordinates (" << city.x << ", " << city.y << ")\n";
    }
}

// Prints a population to standard output
void printPopulation(const Population &population) {
    cout << "Routes in population:\n";
    for (const Route &route : population) {
        cout << "    Cities: ";
        for (int rIdx = 0; rIdx < route.size(); rIdx++) {
            cout << route[rIdx]->index;
            if (rIdx + 1 < route.size()) {
                cout << ", ";
            }
        }
        cout << "\n";
        cout << "        Length: " << calcRouteLength(route) << "\n";
    }
}

// Select a mating pool from a SORTED population with some elite size,
// meaning that that number of elite individuals will be selected from the best.
// The rest will be given a random chance based on their relative fitness.
Population selectMatingPool(const Population &population, int eliteSize) {
    // First add the first elite individuals to the population
    Population selection(eliteSize);
    for (int eIdx = 0; eIdx < eliteSize; eIdx++) {
        selection[eIdx] = population[eIdx];
    }
    // Calculate cumulative sum of the route lengths in the population
    FloatList cumSum(population.size());
    cumSum[0] = calcRouteLength(population[0]);
    for (int pIdx = 1; pIdx < cumSum.size(); pIdx++) {
        cumSum[pIdx] = cumSum[pIdx - 1] + calcRouteLength(population[pIdx]);
    }
    // Calculate percentiles of the routes in the population
    FloatList percentiles(population.size());
    for (int pIdx = 0; pIdx < percentiles.size(); pIdx++) {
        percentiles[pIdx] = 100.f * cumSum[pIdx] / cumSum.back();
    }
    // Give random chance to the rest of the population by their percentiles
    for (int i = 0; i < population.size() - eliteSize; i++) {
        const float randPerc = getRandFloat(0.f, 100.f);
        for (int pIdx = 0; pIdx < population.size(); pIdx++) {
            if (percentiles[pIdx] > randPerc) {
                selection.push_back(population[pIdx]);
                break;
            }
        }
    }
    return selection;
}

// Breeds two routes to get a new child route.
// The breeding is done as follows:
// First an index range is selected at random with indices into a route.
// Then the cities from that index range of the first route are directly copied to the same index range of the resulting route.
// And then the rest of the cities are retrieved from the second route in the same order they appear there,
// and added to the remaining free inidices of the resulting route
Route breedRoutes(const Route &aRoute, const Route &bRoute) {
    // Select a range of indices into a route at random
    const int randIdx1 = getRandInt(0, aRoute.size() - 1);
    const int randIdx2 = getRandInt(0, aRoute.size() - 1);
    const int startIdx = min(randIdx1, randIdx2);
    const int endIdx = max(randIdx1, randIdx2);

    // Mark which cities are used so far
    BoolList cityUsed(aRoute.size(), false);
    int unusedCount = aRoute.size();
    // This will be the resulting route
    Route resRoute(aRoute.size());
    // Copy the cities from the chosen index range directly from the first route
    for (int aIdx = startIdx; aIdx <= endIdx; aIdx++) {
        resRoute[aIdx] = aRoute[aIdx];
        cityUsed[resRoute[aIdx]->index] = true;
        unusedCount--;
    }
    // For the rest of the indices, choose cities from the second route consecutively
    // by skipping the cities that are already used from the first route,
    // and add those cities to the resulting route by skipping the index range which is already filled.
    // ---
    // An index into the resulting route
    int resIdx = 0;
    // Traverse indices of the second route
    for (int bIdx = 0; bIdx < bRoute.size() && unusedCount > 0; bIdx++) {
        // If the resulting index reaches the index range, skip the index range and continue after it
        if (resIdx >= startIdx && resIdx <= endIdx) {
            resIdx = endIdx + 1;
        }
        assert(resIdx < resRoute.size());
        // If the city at the second route is not used
        if (!cityUsed[bRoute[bIdx]->index]) {
            // add it to the resulting route
            resRoute[resIdx] = bRoute[bIdx];
            // mark it as used
            cityUsed[resRoute[resIdx]->index] = true;
            unusedCount--;
            // and move the resulting index to the next one
            resIdx++;
        }
    }
    return resRoute;
}

// Breeds a population from a mating pool to get a next population of children.
// The first elite individuals are directly added to the children without breeding.
// The remaining individuals are shuffled and breeded together at random to get the remaining children.
Population breedPopulation(const Population &matingPool, int eliteSize) {
    // First add the first elite individuals to the children
    Population children(eliteSize);
    for (int eIdx = 0; eIdx < children.size(); eIdx++) {
        children[eIdx] = matingPool[eIdx];
    }
    // Shuffle the mating pool
    const IntList poolPerm = getRandomPermutation(matingPool.size());
    Population poolShuffled(matingPool.size());
    for (int pIdx = 0; pIdx < poolShuffled.size(); pIdx++) {
        poolShuffled[pIdx] = matingPool[poolPerm[pIdx]];
    }
    // For the non-elite population breed parents to get children
    for (int i = 0; i < poolShuffled.size() - eliteSize; i++) {
        const Route child = breedRoutes(poolShuffled[i], poolShuffled[poolShuffled.size() - 1 - i]);
        children.push_back(child);
    }
    return children;
}

// Swaps two cities in a route
void swapCitiesInRoute(Route &route, int aIdx, int bIdx) {
    const City *tCity = route[aIdx];
    route[aIdx] = route[bIdx];
    route[bIdx] = tCity;
}

// Mutates a route by randomly swapping pairs of cities with some mutation rate probability
Route mutateRoute(const Route &route, float mutationRate) {
    Route mutRoute = route;
    for (int swapIdx = 0; swapIdx < route.size(); swapIdx++) {
        if (getRandFloat(0.f, 1.f) < mutationRate) {
            const int swapWith = getRandInt(0, route.size() - 1);
            swapCitiesInRoute(mutRoute, swapIdx, swapWith);
        }
    }
    return mutRoute;
}

// Mutates a population by just mutating each route separately
Population mutatePopulation(const Population &population, float mutationRate) {
    Population mutPopulation(population.size());
    for (int pIdx = 0; pIdx < population.size(); pIdx++) {
        mutPopulation[pIdx] = mutateRoute(population[pIdx], mutationRate);
    }
    return mutPopulation;
}

// Returns the next generation population
// NOTE: the input population has to be SORTED
Population getNextGeneration(Population population, int eliteSize, float mutationRate) {
    sort(population.begin(), population.end());
    const Population matingPool = selectMatingPool(population, eliteSize);
    const Population children = breedPopulation(matingPool, eliteSize);
    const Population nextGeneration = mutatePopulation(children, mutationRate);
    return nextGeneration;
}

// Finds the best route between the given cities
Route findBestRoute(const CityList &cities, int populationSize, int eliteSize, float mutationRate, int generations) {
    Population population = getRandomPopulation(populationSize, cities);

    // Looping to generations + 1 so that after the last generation it goes inside the for-loop one more time
    // to print the results of the final generation.
    for (int gen = 0; gen < generations + 1; gen++) {
        sort(population.begin(), population.end());
        const Route &bestRoute = population[0];

        cout << "Generation #" << gen << "\n";
        cout << "    Best route: ";
        for (int rIdx = 0; rIdx < bestRoute.size(); rIdx++) {
            cout << bestRoute[rIdx]->index;
            if (rIdx + 1 < bestRoute.size()) {
                cout << ", ";
            }
        }
        cout << "\n";
        cout << "    Length: " << calcRouteLength(bestRoute) << "\n";

        // Handling the case of the last iteration that needs to only print the messages above and then exit
        if (gen < generations) {
            population = getNextGeneration(population, eliteSize, mutationRate);
        }
    }

    return population[0];
}

int main() {
    setupRand();

    const CityList cities = getCities();
    printCities(cities);

    const Route bestRoute = findBestRoute(cities, 100, 20, 0.01f, 100);

    return 0;
}