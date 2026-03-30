from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import subprocess
import json
import random
import os
import math

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Graph representation
# We will generate a grid-like map with some randomness for coordinates
ROWS = 30
COLS = 40
NUM_NODES = ROWS * COLS

nodes = []
edges = []

def get_node_id(r, c):
    return r * COLS + c

# Initialize the graph
def init_graph():
    global nodes, edges
    nodes.clear()
    edges.clear()

    # Create nodes with Lat/Lon around a base coordinate
    # Base: 40.7128N, 74.0060W (New Yorkish)
    base_lat = 40.7128
    base_lon = -74.0060

    lat_step = 0.005
    lon_step = 0.005

    for r in range(ROWS):
        for c in range(COLS):
            n_id = get_node_id(r, c)
            # Add some jitter to make it look less perfectly rigid
            jitter_lat = random.uniform(-0.001, 0.001)
            jitter_lon = random.uniform(-0.001, 0.001)
            
            lat = base_lat + r * lat_step + jitter_lat
            lon = base_lon + c * lon_step + jitter_lon
            nodes.append({"id": n_id, "lat": lat, "lon": lon})

    # Create edges (Grid connected: up, down, left, right)
    for r in range(ROWS):
        for c in range(COLS):
            u = get_node_id(r, c)
            neighbors = []
            if r > 0: neighbors.append(get_node_id(r - 1, c))
            if r < ROWS - 1: neighbors.append(get_node_id(r + 1, c))
            if c > 0: neighbors.append(get_node_id(r, c - 1))
            if c < COLS - 1: neighbors.append(get_node_id(r, c + 1))

            # Optional: Add some diagonal paths or missing links
            if random.random() > 0.05: # 95% of grid edges exist
                for v in neighbors:
                    # Calculate Haversine distance as base weight
                    lat1, lon1 = nodes[u]['lat'], nodes[u]['lon']
                    lat2, lon2 = nodes[v]['lat'], nodes[v]['lon']
                    
                    # Rough distance calculation in meters
                    dlat = math.radians(lat2 - lat1)
                    dlon = math.radians(lon2 - lon1)
                    a = math.sin(dlat/2)**2 + math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.sin(dlon/2)**2
                    dist = 6371000 * 2 * math.asin(math.sqrt(a))
                    
                    edges.append({"u": u, "v": v, "weight": dist})

# Initialize once at startup
init_graph()

class RouteRequest(BaseModel):
    src: int
    dest: int
    algo: str # "dijkstra", "bellman", "pivot", "auto"
    traffic_factor: float = 1.0

@app.get("/map")
def get_map():
    return {
        "nodes": nodes,
        "edges": edges
    }

@app.post("/route")
def calculate_route(req: RouteRequest):
    if req.src < 0 or req.src >= NUM_NODES or req.dest < 0 or req.dest >= NUM_NODES:
        raise HTTPException(status_code=400, detail="Invalid source or destination")

    engine_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "engine"))
    engine_path = os.path.join(engine_dir, "engine.exe")
    
    if not os.path.exists(engine_path):
        raise HTTPException(status_code=500, detail=f"Engine not found at {engine_path}")

    # Prepare input for engine
    # Format: 
    # n m src dest algo
    # u v w for each edge
    
    input_lines = []
    input_lines.append(f"{NUM_NODES} {len(edges)} {req.src} {req.dest} {req.algo}")
    
    for edge in edges:
        # Apply traffic factor dynamically
        # Let's say we add some random delay to some edges based on traffic factor
        traffic_weight_multiplier = 1.0
        if req.traffic_factor > 1.0 and random.random() < 0.3:
            traffic_weight_multiplier = random.uniform(1.0, req.traffic_factor)
        
        final_weight = edge["weight"] * traffic_weight_multiplier
        input_lines.append(f"{edge['u']} {edge['v']} {final_weight}")

    engine_input = "\n".join(input_lines) + "\n"

    try:
        process = subprocess.Popen(
            [engine_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        stdout, stderr = process.communicate(input=engine_input, timeout=10)
        
        if process.returncode != 0:
            raise Exception(f"Engine failed: {stderr}")

        # Parse output JSON
        try:
            result = json.loads(stdout.strip())
            return result
        except json.JSONDecodeError:
            raise Exception(f"Failed to parse engine output: {stdout}")

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
