#include <iostream>
#include <vector>
using namespace std;

typedef int Node;

typedef vector< vector<Node> > Graph;

bool dfsCore(const Graph &graph, vector<bool> &visited, int start, int goal, vector<int> &path) {
    // Add start node to path and mark it as visited
    path.push_back(start);
    visited[start] = true;
    // If start node is the goal, then we are done
    if (start == goal) {
        return true;
    }
    // Traverse neighbours of the start node
    for (int neigh : graph[start]) {
        // Recursively perform DFS on each unvisited neighbour
        if (!visited[neigh] && dfsCore(graph, visited, neigh, goal, path)) {
            // If the goal is reached through any of them, we are done
            return true;
        }
    }
    // At this point no path is found, so remove start node from path and mark it as unvisited
    path.pop_back();
    visited[start] = false;
    return false;
}

/// Depth-first search a path from start node to goal node of the given graph.
/// The function returns true if a path is found, and fills the path vector with it.
bool dfs(const Graph &graph, int start, int goal, vector<int> &path) {
    vector<bool> visited(graph.size(), false);
    return dfsCore(graph, visited, start, goal, path);
}

// Prints the given path to the console as a list of nodes, separated with commas.
void printPath(const vector<int> &path) {
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
        { 1 },
        { 3 },
        { 0, 1 },
        { 4 },
        {}
    };
    // Setup DFS parameters
    const int start = 0;
    const int goal = graph.size() - 1;
    vector<int> path;
    // Perform DFS
    if (dfs(graph, start, goal, path)) {
        cout << "Path found: ";
        printPath(path);
    }
    else {
        cout << "No path :(\n";
    }
    return 0;
}