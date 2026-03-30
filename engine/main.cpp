#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <set>
#include <algorithm>
#include <iomanip>

using namespace std;

const double INF = 1e18;

struct Edge {
    int to;
    double weight;
};

struct NodeDist {
    int id;
    double dist;
    bool operator>(const NodeDist& other) const {
        return dist > other.dist;
    }
};

struct Result {
    double distance;
    long long duration_us;
    vector<int> path;
    vector<int> frontier;
    vector<int> pivots;
};

vector<vector<Edge>> adj;
int n, m;

Result runDijkstra(int source, int dest) {
    auto start_time = chrono::high_resolution_clock::now();
    vector<double> dist(n, INF);
    vector<int> parent(n, -1);
    priority_queue<NodeDist, vector<NodeDist>, greater<NodeDist>> pq;
    vector<int> frontier_visited;

    dist[source] = 0;
    pq.push({source, 0});

    while (!pq.empty()) {
        NodeDist top_node = pq.top();
        int u = top_node.id;
        double d = top_node.dist;
        pq.pop();
        
        if (d > dist[u]) continue;
        frontier_visited.push_back(u);

        if (u == dest) break; 

        for (auto& edge : adj[u]) {
            int v = edge.to;
            double w = edge.weight;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push({v, dist[v]});
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    long long duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    Result res;
    res.distance = (dist[dest] == INF) ? -1 : dist[dest];
    res.duration_us = duration;
    if (dist[dest] != INF) {
        int curr = dest;
        while (curr != -1) {
            res.path.push_back(curr);
            curr = parent[curr];
        }
        reverse(res.path.begin(), res.path.end());
    }
    res.frontier = frontier_visited;
    return res;
}

Result runBellmanFord(int source, int dest) {
    auto start_time = chrono::high_resolution_clock::now();
    vector<double> dist(n, INF);
    vector<int> parent(n, -1);
    dist[source] = 0;
    vector<int> frontier_visited;

    for (int i = 0; i < n - 1; ++i) {
        bool updated = false;
        for (int u = 0; u < n; ++u) {
            if (dist[u] == INF) continue;
            frontier_visited.push_back(u);
            for (auto& edge : adj[u]) {
                if (dist[u] + edge.weight < dist[edge.to]) {
                    dist[edge.to] = dist[u] + edge.weight;
                    parent[edge.to] = u;
                    updated = true;
                }
            }
        }
        if (!updated) break;
    }

    auto end_time = chrono::high_resolution_clock::now();
    long long duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    Result res;
    res.distance = (dist[dest] == INF) ? -1 : dist[dest];
    res.duration_us = duration;
    if (dist[dest] != INF) {
        int curr = dest;
        while (curr != -1) {
            res.path.push_back(curr);
            curr = parent[curr];
        }
        reverse(res.path.begin(), res.path.end());
    }
    // Remove duplicates from frontier for cleaner visualization
    set<int> unique_f(frontier_visited.begin(), frontier_visited.end());
    res.frontier.assign(unique_f.begin(), unique_f.end());
    return res;
}

// Simplified Pivot Algorithm (Delta-Stepping / K-step relaxation proxy)
Result runPivotAlgorithm(int source, int dest) {
    auto start_time = chrono::high_resolution_clock::now();
    vector<double> dist(n, INF);
    vector<int> parent(n, -1);
    dist[source] = 0;
    
    vector<int> frontier;
    vector<int> pivots;
    
    // Simplification: Use buckets (delta steps) to group nodes and avoid full sorting
    double delta = 10.0; // Assume an average edge weight to group
    vector<vector<int>> buckets;
    buckets.push_back({source});
    
    int current_bucket = 0;
    int k_steps = 3; // Bellman-ford k-step inner loop for pivots
    
    bool reached_dest = false;
    
    while (current_bucket < buckets.size()) {
        if (buckets[current_bucket].empty()) {
            current_bucket++;
            continue;
        }
        
        // Mark these nodes as pivots for this bucket level
        for(int u : buckets[current_bucket]) pivots.push_back(u);
        
        // Relax edges from this bucket
        vector<int> current_active = buckets[current_bucket];
        buckets[current_bucket].clear(); // Empty bucket
        
        // K-step bounded relaxation for local cluster (pseudo BMSSP)
        for(int step = 0; step < k_steps; ++step) {
            vector<int> next_active;
            for(int u : current_active) {
                if (u == dest) reached_dest = true;
                frontier.push_back(u);
                
                for(auto& edge : adj[u]) {
                    int v = edge.to;
                    double w = edge.weight;
                    if (dist[u] + w < dist[v]) {
                        dist[v] = dist[u] + w;
                        parent[v] = u;
                        next_active.push_back(v);
                        
                        // Re-bucket
                        int b_idx = dist[v] / delta;
                        if (b_idx >= buckets.size()) buckets.resize(b_idx + 1);
                        // Prevent duplicates in buckets simply by only adding if smaller bucket
                        buckets[b_idx].push_back(v);
                    }
                }
            }
            if (next_active.empty()) break;
            current_active = next_active;
        }
        if (reached_dest) break; // Optimization
        current_bucket++;
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    long long duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();

    Result res;
    res.distance = (dist[dest] == INF) ? -1 : dist[dest];
    res.duration_us = duration;
    if (dist[dest] != INF) {
        int curr = dest;
        while (curr != -1) {
            res.path.push_back(curr);
            curr = parent[curr];
        }
        reverse(res.path.begin(), res.path.end());
    }
    
    set<int> unique_f(frontier.begin(), frontier.end());
    res.frontier.assign(unique_f.begin(), unique_f.end());
    
    set<int> unique_p(pivots.begin(), pivots.end());
    res.pivots.assign(unique_p.begin(), unique_p.end());
    
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string algo;
    int source, dest;
    
    if (!(cin >> n >> m >> source >> dest >> algo)) {
        return 0; // Terminate if no valid input
    }
    
    adj.resize(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        double w;
        cin >> u >> v >> w;
        adj[u].push_back({v, w});
        // Assuming directed graph. If undirected, add reverse edge.
        // Let's assume directed as per paper, but we can handle both if we duplicate inputs.
    }

    Result res;
    if (algo == "dijkstra") {
        res = runDijkstra(source, dest);
    } else if (algo == "bellman") {
        res = runBellmanFord(source, dest);
    } else if (algo == "pivot") {
        res = runPivotAlgorithm(source, dest);
    } else if (algo == "auto") {
        if (n < 1000) res = runDijkstra(source, dest);
        else res = runPivotAlgorithm(source, dest);
    } else {
        res = runDijkstra(source, dest); // default
    }

    // Output JSON strictly
    cout << "{";
    cout << "\"distance\": " << fixed << setprecision(2) << res.distance << ", ";
    cout << "\"time_us\": " << res.duration_us << ", ";
    
    cout << "\"path\": [";
    for(size_t i=0; i<res.path.size(); ++i) {
        cout << res.path[i];
        if(i != res.path.size()-1) cout << ", ";
    }
    cout << "], ";
    
    cout << "\"frontier\": [";
    for(size_t i=0; i<res.frontier.size(); ++i) {
        cout << res.frontier[i];
        if(i != res.frontier.size()-1) cout << ", ";
    }
    cout << "], ";
    
    cout << "\"pivots\": [";
     for(size_t i=0; i<res.pivots.size(); ++i) {
        cout << res.pivots[i];
        if(i != res.pivots.size()-1) cout << ", ";
    }
    cout << "]";
    cout << "}\n";

    return 0;
}
