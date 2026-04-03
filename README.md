<h1 align="center">PivotPath</h1>
<h3 align="center">A Next-Generation Routing Engine Beyond Dijkstra</h3>

<p align="center">
High-performance shortest-path computation using pivot-based optimization and hybrid graph algorithms
</p>

---

## 📌 Overview

**PivotPath** is a high-performance routing engine designed to optimize shortest-path computation in large-scale graphs.  
It extends traditional approaches by introducing a **pivot-based bucketed relaxation algorithm**, reducing unnecessary computations and improving execution efficiency.

The system provides **comparative evaluation** between:

- Dijkstra’s Algorithm  
- Bellman-Ford Algorithm  
- Pivot-Based Routing Algorithm (Proposed)

---

## 🧠 Key Innovation

Unlike traditional algorithms that rely on full graph traversal or priority queue sorting, PivotPath introduces:

### 🔹 Pivot-Based Relaxation
- Nodes are grouped into **distance buckets**
- Selected nodes act as **pivots for local exploration**
- Reduces global search complexity

### 🔹 K-Step Local Relaxation
- Performs bounded relaxation (`k_steps = 3`)  
- Inspired by Bellman-Ford but **limited to local clusters**

### 🔹 Bucket-Based Processing (Delta-Stepping Inspired)
- Avoids full priority queue sorting (like Dijkstra)
- Uses **distance ranges (delta buckets)**

👉 Result: Faster execution in large graphs with reduced overhead

---

## ⚙️ Algorithms Implemented

### 1. Dijkstra Algorithm
- Uses priority queue
- Guarantees optimal shortest path
- Time Complexity: `O((V + E) log V)`

---

### 2. Bellman-Ford Algorithm
- Handles negative weights
- Iterative edge relaxation
- Time Complexity: `O(V × E)`

---

### 3. PivotPath Algorithm (Proposed)

Core logic (from implementation :contentReference[oaicite:0]{index=0}):

- Bucket-based node grouping (`delta = 10`)
- Local k-step relaxation
- Dynamic pivot selection
- Early termination when destination reached

#### Key Components:
- **Frontier Tracking** → visited nodes
- **Pivot Nodes** → key nodes per bucket
- **Selective Relaxation** → avoids redundant updates

---

## 🏗️ System Architecture
Frontend (React + Vite)
↓
Backend (FastAPI - Python)
↓
C++ Engine (PivotPath Core)


---

## 📂 Project Structure
backend/
├── main.py # FastAPI server
├── requirements.txt

engine/
├── main.cpp # Core routing algorithms
├── engine.exe # Compiled binary

frontend/
├── src/ # React components
├── index.html
├── package.json


---

## ▶️ How It Works

### 🔹 Execution Flow

1. User inputs graph data (nodes, edges, weights)
2. Backend receives request
3. C++ engine executes selected algorithm



<img width="675" height="713" alt="image" src="https://github.com/user-attachments/assets/892a6a08-46c8-4c39-a758-2616bd061db7" />

