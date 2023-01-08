#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <random>
#include <ctime>
#include <limits>
#include <cassert>
using namespace std;

#define DO_DEBUG 0
#define DEBUG(msg) if (DO_DEBUG) cout << msg

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef unsigned char Byte;
#define RGB_CHANNELS_COUNT 3

const int FILEPATH_MAX_LENGTH = 300;
const int MAX_INPUT_LINE_LENGTH = 45;

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

struct Color {
    int r;
    int g;
    int b;
};

const int CLUSTER_COLORS_COUNT = 10;
const Color clusterColors[CLUSTER_COLORS_COUNT] = {
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 },
    { 0, 255, 255 },
    { 255, 0, 255 },
    { 255, 255, 0 },
    { 255, 255, 255 },
    { 120, 60, 10 },
    { 30, 30, 140 },
    { 80, 160, 30 }
};

struct Point {
    float x;
    float y;
};

struct ColoredPoint {
    Point point;
    int colorIdx;
};

struct Box {
    Point min;
    Point max;

    float getWidth() const {
        return max.x - min.x;
    }
    float getHeight() const {
        return max.y - min.y;
    }
};

float calcDistSq(const Point &aPoint, const Point &bPoint) {
    const float xDelta = fabsf(aPoint.x - bPoint.x);
    const float yDelta = fabsf(aPoint.y - bPoint.y);
    return xDelta * xDelta + yDelta * yDelta;
}

float calcDist(const Point &aPoint, const Point &bPoint) {
    return sqrtf(calcDistSq(aPoint, bPoint));
}

Point getPointInSpace(const Point &point, const Box &inSpace, const Box &outSpace) {
    return {
        ((point.x - inSpace.min.x) / inSpace.getWidth()) * outSpace.getWidth() + outSpace.min.x,
        ((point.y - inSpace.min.y) / inSpace.getHeight()) * outSpace.getHeight() + outSpace.min.y,
    };
}

void drawColoredPoints(
    vector<ColoredPoint> coloredPoints,
    const char *filepath,
    const Box &pointSpace,
    int imgWidth = 378,
    int imgHeight = 248,
    int pointRadius = 3
) {
    Byte* rgb_image = new Byte[imgWidth * imgHeight * RGB_CHANNELS_COUNT];
    const int bytesPerRow = imgWidth * RGB_CHANNELS_COUNT;
    for (int i = 0; i < imgWidth * imgHeight * RGB_CHANNELS_COUNT; i++) {
        rgb_image[i] = 0;
    }

    const Box pixelSpace = { { 0.f, 0.f }, { float(imgWidth), float(imgHeight) } };
    for (const ColoredPoint &coloredPoint : coloredPoints) {
        const Point pointInPixSpace = getPointInSpace(coloredPoint.point, pointSpace, pixelSpace);
        const int pointRow = imgHeight - int(roundf(pointInPixSpace.y));
        const int pointCol = int(roundf(pointInPixSpace.x));

        for (int row = pointRow - pointRadius; row <= pointRow + pointRadius; row++) {
            for (int col = pointCol - pointRadius; col <= pointCol + pointRadius; col++) {
                const bool inCircle = (row - pointRow) * (row - pointRow) + (col - pointCol) * (col - pointCol) <= pointRadius * pointRadius;
                if (inCircle) {
                    rgb_image[row * bytesPerRow + col * RGB_CHANNELS_COUNT + 0] = clusterColors[coloredPoint.colorIdx % CLUSTER_COLORS_COUNT].r;
                    rgb_image[row * bytesPerRow + col * RGB_CHANNELS_COUNT + 1] = clusterColors[coloredPoint.colorIdx % CLUSTER_COLORS_COUNT].g;
                    rgb_image[row * bytesPerRow + col * RGB_CHANNELS_COUNT + 2] = clusterColors[coloredPoint.colorIdx % CLUSTER_COLORS_COUNT].b;
                }
            }
        }
    }

    stbi_write_png(filepath, imgWidth, imgHeight, RGB_CHANNELS_COUNT, rgb_image, bytesPerRow);
}

vector<Point> readPointsFromFile(const char *filepath) {
    std::fstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open data file.");
    }

    vector<Point> points;
    Point currPoint;
    while (file >> currPoint.x && file >> currPoint.y) {
        points.push_back(currPoint);
    }

    file.close();
    if (file.is_open()) {
        throw std::runtime_error("Cannot close data file.");
    }

    return points;
}

Box getSpaceOfPoints(const vector<Point> &points, float expandSpaceFactor = 0.1f) {
    Box space = {
        { numeric_limits<float>::max(), numeric_limits<float>::max() },
        { numeric_limits<float>::min(), numeric_limits<float>::min() }
    };

    for (const Point &point : points) {
        space.min.x = min(space.min.x, point.x);
        space.min.y = min(space.min.y, point.y);
        space.max.x = max(space.max.x, point.x);
        space.max.y = max(space.max.y, point.y);
    }

    const float ogWidth = space.getWidth();
    const float ogHeight = space.getHeight();
    space.min.x -= ogWidth * expandSpaceFactor;
    space.min.y -= ogHeight * expandSpaceFactor;
    space.max.x += ogWidth * expandSpaceFactor;
    space.max.y += ogHeight * expandSpaceFactor;

    return space;
}

Point getRandPointInSpace(const Box &space) {
    Point point = {
        getRandFloat(space.min.x, space.max.x),
        getRandFloat(space.min.y, space.max.y)
    };
    return point;
}

vector<Point> getCentroids(const Box &space, int clustersCount) {
    vector<Point> centroids(clustersCount);
    for (Point &centroid : centroids) {
        centroid = getRandPointInSpace(space);
    }
    return centroids;
}

bool colorPoints(vector<ColoredPoint> &points, const vector<Point> &centroids) {
    bool changed = false;
    for (ColoredPoint &point : points) {
        int minIdx = -1;
        float minDistSq = numeric_limits<float>::max();
        for (int cIdx = 0; cIdx < centroids.size(); cIdx++) {
            const float distSq = calcDistSq(point.point, centroids[cIdx]);
            if (distSq < minDistSq) {
                minDistSq = distSq;
                minIdx = cIdx;
            }
        }

        if (point.colorIdx != minIdx) {
            point.colorIdx = minIdx;
            changed = true;
        }
    }
    return changed;
}

void changeCentroids(vector<Point> &centroids, const vector<ColoredPoint> &points) {
    vector<Point> ogCentroids = centroids;
    vector<int> pointCounts(centroids.size(), 0);
    for (int cIdx = 0; cIdx < centroids.size(); cIdx++) {
        centroids[cIdx].x = 0.f;
        centroids[cIdx].y = 0.f;
    }

    DEBUG("groups:");
    for (const ColoredPoint &point : points) {
        assert(point.colorIdx >= 0 && point.colorIdx < centroids.size());
        DEBUG(point.colorIdx);
        centroids[point.colorIdx].x += point.point.x;
        centroids[point.colorIdx].y += point.point.y;
        pointCounts[point.colorIdx]++;
    }
    DEBUG("\n");
    DEBUG("pointCounts: ");
    for (int cIdx = 0; cIdx < centroids.size(); cIdx++) {
        DEBUG(pointCounts[cIdx] << ", ");
        if (pointCounts[cIdx] < 1) {
            centroids[cIdx] = ogCentroids[cIdx];
            continue;
        }
        centroids[cIdx].x /= float(pointCounts[cIdx]);
        centroids[cIdx].y /= float(pointCounts[cIdx]);
    }
    DEBUG("\n");
}

vector<ColoredPoint> kMeans(const vector<Point> &ogPoints, int clustersCount, int iterations = -1) {
    Box space = getSpaceOfPoints(ogPoints);
    vector<Point> centroids = getCentroids(space, clustersCount);

    vector<ColoredPoint> points(ogPoints.size());
    for (int i = 0; i < ogPoints.size(); i++) {
        points[i].point = ogPoints[i];
        points[i].colorIdx = -1;
    }

    for (int iter = 0; iter < iterations || iterations == -1; iter++) {
        DEBUG("Iteration " << iter << "\n");
        if (!colorPoints(points, centroids)) {
            return points;
        }
        for (int cIdx = 0; cIdx < centroids.size(); cIdx++) {
            DEBUG("Centroid " << cIdx << " : " << centroids[cIdx].x << ", " << centroids[cIdx].y << "\n");
        }
        changeCentroids(centroids, points);
    }
}

void run(const char *filepath, int clustersCount) {
    vector<Point> points = readPointsFromFile(filepath);
    const Box space = getSpaceOfPoints(points);
    vector<ColoredPoint> coloredPoints = kMeans(points, clustersCount, 30);
    drawColoredPoints(coloredPoints, "points.png", space);
}

int main() {
    // Read path to data file from user
    char filepath[FILEPATH_MAX_LENGTH + 1];
    cin >> filepath;
    // Read number of clusters from user
    int clustersCount;
    cin >> clustersCount;

    run(filepath, clustersCount);
}