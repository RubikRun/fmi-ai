#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

typedef int Node;

typedef vector< vector<Node> > Graph;

bool bfsCore(const Graph &graph, Node start, Node goal, vector<Node> &parent) {
    // Setup a queue of visited nodes waiting to have their neighbours searched. Begin with the start node only.
    queue<Node> visQu;
    visQu.push(start);
    // Pop nodes from the queue until it's empty
    while (!visQu.empty()) {
        const Node curr = visQu.front();
        visQu.pop();
        // Traverse the neighbours of each popped node
        for (const Node neigh : graph[curr]) {
            // Push each unvisited neighbour to the queue and mark its parent to be the current node
            if (parent[neigh] == -1) {
                parent[neigh] = curr;
                // If this neighbour happens to be the goal node, then we are done
                if (neigh == goal) {
                    return true;
                }
                visQu.push(neigh);
            }
        }
    }
    // At this point there is no path to the goal state
    return false;
}

/// Breadth-first search the shortest path from start node to goal node of the given graph.
/// The function returns true if a path is found, and fills the path vector with it.
bool bfs(const Graph &graph, Node start, Node goal, vector<Node> &path) {
    // Setup parents array that keeps track of the parent of each node, -1 for no parent.
    vector<Node> parent(graph.size(), -1);
    parent[start] = -2;
    // Perform the actual BFS to search for a path and fill the parent array
    if (bfsCore(graph, start, goal, parent)) {
        path.clear();
        // Start with the goal state and go back the path
        Node curr = goal;
        // until we reach the start state
        while (curr != start) {
            // by following the parent of each next node
            path.push_back(curr);
            curr = parent[curr];
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
        { 1, 4 },
        { 3 },
        { 0, 1 },
        { 4 },
        {}
    };
    const Node start = 0;
    const Node goal = graph.size() - 1;
    vector<Node> path;
    // Perform BFS to find the shortest path
    if (bfs(graph, start, goal, path)) {
        cout << "Path found: ";
        printPath(path);
    }
    else {
        cout << "No path :(\n";
    }
    return 0;
}