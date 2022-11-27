#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

const int DIST_INF = 2147483647;

typedef int Node;
struct Edge {
    Node node;
    int weight;

    bool operator<(const Edge &other) const {
        // We want the edges to be ordered by weight decreasing,
        // so that less weight means greater value of the edge,
        // because the priority queue will choose always the greatest edge.
        return this->weight > other.weight;
    }
};

typedef vector< vector<Edge> > Graph;

bool dijkstraCore(const Graph &graph, vector<bool> &visited, Node start, Node goal, vector<Edge> &parent) {
    // Setup a priorty queue of visited nodes (edges) waiting to have their neighbours searched. Begin with the start node only.
    priority_queue<Edge> prQu;
    prQu.push({start, 0});
    // Pop nodes from the queue until it's empty
    while (!prQu.empty()) {
        const Edge curr = prQu.top();
        prQu.pop();
        // Traverse the neighbours of each popped node
        for (const Edge &neigh : graph[curr.node]) {
            // Skip it if it's already visited
            if (visited[neigh.node]) {
                continue;
            }
            // Calculate the distance to the neighbour through this edge
            int neighDist = parent[curr.node].weight + neigh.weight;
            // If the new distance is smaller than the currently best distance
            if (neighDist < parent[neigh.node].weight) {
                // Update the parent and the distance of the neighbour
                parent[neigh.node] = { curr.node, neighDist };
                prQu.push(neigh);
            }
        }
        visited[curr.node] = true;
    }
    return parent[goal].node > -1;
}

/// Search with Dijkstra algorithm the shortest path from start node to goal node of the given graph.
/// The function returns true if a path is found, and fills the path vector with it.
bool dijkstra(const Graph &graph, Node start, Node goal, vector<Node> &path) {
    // Setup parents array that keeps track of the parent of each node, -1 for no parent.
    vector<Edge> parent(graph.size(), { -1, DIST_INF });
    parent[start] = { -2, 0 };
    vector<bool> visited(graph.size(), false);
    // Perform the actual BFS to search for a path and fill the parent array
    if (dijkstraCore(graph, visited, start, goal, parent)) {
        path.clear();
        // Start with the goal state and go back the path
        Node curr = goal;
        // until we reach the start state
        while (curr != start) {
            // by following the parent of each next node
            path.push_back(curr);
            curr = parent[curr].node;
        }
        // In the end we have to add the start node and reverse the path so that it begins with the start and ends at the goal
        path.push_back(start);
        reverse(path.begin(), path.end());
        return true;
    }
    return false;
}

// Prints the given path to the console as a list of nodes, separated with commas.
void printPath(const vector<Node> &path) {
    for (int i = 0; i < path.size(); i++) {
        cout << path[i];
        if (i < path.size() - 1) {
            cout << ", ";
        }
    }
    cout << "\n";
}

int main() {
    // Create graph
    const Graph graph {
        { { 1, 2 }, { 2, 12 }, { 4, 4 } },
        { { 3, 2 }, { 4, 10 } },
        {},
        { { 0, 1 }, { 2, 3 } },
        {}
    };
    const Node start = 0;
    const Node goal = 2;//graph.size() - 1;
    vector<Node> path;
    // Perform BFS to find the shortest path
    if (dijkstra(graph, start, goal, path)) {
        cout << "Path found: ";
        printPath(path);
    }
    else {
        cout << "No path :(\n";
    }
    return 0;
}