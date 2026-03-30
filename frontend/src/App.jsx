import { useEffect, useState } from 'react';
import { MapContainer, TileLayer, CircleMarker, Polyline, useMap } from 'react-leaflet';
import axios from 'axios';
import { Play, Navigation, Clock, Activity, Settings2 } from 'lucide-react';
import 'leaflet/dist/leaflet.css';
import './index.css';

const API_URL = 'http://localhost:8000';

function App() {
  const [mapData, setMapData] = useState({ nodes: [], edges: [] });
  const [src, setSrc] = useState(null);
  const [dest, setDest] = useState(null);
  const [algo, setAlgo] = useState('auto');
  const [traffic, setTraffic] = useState(1.0);
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState(null);
  const [error, setError] = useState('');

  useEffect(() => {
    // Fetch Map
    axios.get(`${API_URL}/map`)
      .then(res => setMapData(res.data))
      .catch(err => console.error("Error loading map:", err));
  }, []);

  const handleNodeClick = (id) => {
    if (src === null) {
      setSrc(id);
    } else if (dest === null && id !== src) {
      setDest(id);
    } else {
      setSrc(id);
      setDest(null);
      setResult(null);
    }
  };

  const calculateRoute = async () => {
    if (src === null || dest === null) return;
    setLoading(true);
    setError('');
    setResult(null);
    try {
      const res = await axios.post(`${API_URL}/route`, {
        src: parseInt(src),
        dest: parseInt(dest),
        algo,
        traffic_factor: parseFloat(traffic)
      });
      setResult(res.data);
    } catch (err) {
      setError(err.response?.data?.detail || "Failed to calculate route");
    }
    setLoading(false);
  };

  const getEdgeStyle = (u, v) => {
    if (!result?.path) return { color: "#ffffff", opacity: 0.1, weight: 1 };
    
    // Check if edge is in path
    for (let i = 0; i < result.path.length - 1; i++) {
        const u_p = result.path[i];
        const v_p = result.path[i+1];
        if ((u == u_p && v == v_p) || (u == v_p && v == u_p)) {
            return { color: "#10b981", opacity: 1, weight: 3, dashArray: "5, 5" }; // Path (Green)
        }
    }
    return { color: "#ffffff", opacity: 0.05, weight: 1 };
  };

  const getNodeStyle = (id) => {
    if (id === src) return { color: "#3b82f6", fillColor: "#3b82f6", fillOpacity: 1, radius: 8 }; // Source (Blue)
    if (id === dest) return { color: "#10b981", fillColor: "#10b981", fillOpacity: 1, radius: 8 }; // Dest (Green)
    
    if (result) {
        if (result.path && result.path.includes(id)) {
            return { color: "#10b981", fillColor: "#10b981", fillOpacity: 1, radius: 4 }; // Path Node
        }
        if (result.pivots && result.pivots.includes(id)) {
            return { color: "#ef4444", fillColor: "#ef4444", fillOpacity: 1, radius: 4 }; // Pivot Node
        }
        if (result.frontier && result.frontier.includes(id)) {
            return { color: "transparent", fillColor: "#3b82f6", fillOpacity: 0.4, radius: 3 }; // Frontier Node
        }
    }

    return { color: "transparent", fillColor: "#475569", fillOpacity: 0.6, radius: 2 }; // Normal Node
  };

  // Pre-compute coordinates map
  const coordsMap = {};
  mapData.nodes.forEach(n => coordsMap[n.id] = [n.lat, n.lon]);

  return (
    <div className="app-container">
      <div className="map-container">
        {mapData.nodes.length > 0 && (
          <MapContainer 
            center={[40.78, -73.9]} 
            zoom={13} 
            style={{ width: '100%', height: '100%' }}
            zoomControl={false}
          >
            {/* Dark map layer (optional if we just want space, but gives context) */}
            <TileLayer
              url="https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png"
              attribution='&copy; <a href="https://carto.com/">CartoDB</a>'
            />
            
            {/* Render Edges */}
            {mapData.edges.map((e, idx) => {
              if(!coordsMap[e.u] || !coordsMap[e.v]) return null;
              return (
                <Polyline 
                  key={`edge-${idx}`} 
                  positions={[coordsMap[e.u], coordsMap[e.v]]} 
                  pathOptions={getEdgeStyle(e.u, e.v)} 
                />
              )
            })}

            {/* Render Nodes */}
            {mapData.nodes.map(n => (
              <CircleMarker
                key={`node-${n.id}`}
                center={[n.lat, n.lon]}
                pathOptions={getNodeStyle(n.id)}
                eventHandlers={{ click: () => handleNodeClick(n.id) }}
              />
            ))}
          </MapContainer>
        )}
      </div>

      <div className="control-panel">
        <div className="panel-header">
          <h1>SmartRoute Engine</h1>
          <p>High-Performance Navigation Lab</p>
        </div>

        <div className="form-group">
          <label>Algorithm Selection</label>
          <select value={algo} onChange={e => setAlgo(e.target.value)}>
            <option value="auto">Adaptive Auto-Selection</option>
            <option value="dijkstra">Dijkstra's Algorithm</option>
            <option value="bellman">Bellman-Ford Algorithm</option>
            <option value="pivot">Simplified Pivot BMSSP</option>
          </select>
        </div>

        <div className="form-group" style={{ flexDirection: 'row', gap: '16px' }}>
          <div style={{ flex: 1 }}>
            <label>Source Node</label>
            <input 
              type="number" 
              placeholder="Click a node" 
              value={src !== null ? src : ''} 
              onChange={e => setSrc(e.target.value ? parseInt(e.target.value) : null)}
            />
          </div>
          <div style={{ flex: 1 }}>
            <label>Dest Node</label>
            <input 
              type="number" 
              placeholder="Click a node" 
              value={dest !== null ? dest : ''} 
              onChange={e => setDest(e.target.value ? parseInt(e.target.value) : null)}
            />
          </div>
        </div>

        <div className="form-group">
          <label>Traffic Factor ({traffic}x)</label>
          <input 
            type="range" 
            min="1.0" max="5.0" step="0.5" 
            value={traffic} 
            onChange={e => setTraffic(e.target.value)} 
          />
        </div>

        {error && <div style={{ color: 'var(--danger)', fontSize: '14px' }}>{error}</div>}

        <button 
          className="btn-primary" 
          onClick={calculateRoute}
          disabled={loading || src === null || dest === null}
        >
          {loading ? <div className="spinner" /> : <Play size={18} />}
          {loading ? 'Computing...' : 'Run Navigation'}
        </button>

        {result && (
          <div className="stats-container">
            <div className="stat-row">
              <span className="stat-label"><Navigation size={14} style={{display:'inline', marginRight:'4px'}}/> Distance</span>
              <span className="stat-value">{result.distance >= 0 ? `${(result.distance/1000).toFixed(2)} km` : 'Unreachable'}</span>
            </div>
            
            <div className="stat-row">
              <span className="stat-label"><Clock size={14} style={{display:'inline', marginRight:'4px'}}/> Engine Time</span>
              <span className="stat-value time">{result.time_us} μs</span>
            </div>
            
            <div className="stat-row">
              <span className="stat-label"><Activity size={14} style={{display:'inline', marginRight:'4px'}}/> Nodes Explored</span>
              <span className="stat-value" style={{color: 'var(--text-main)'}}>{result.frontier?.length || 0}</span>
            </div>
          </div>
        )}

        <div className="legend">
          <div className="legend-item">
            <div className="legend-color" style={{background: '#3b82f6'}}></div>
            <span>Explored Frontier</span>
          </div>
          <div className="legend-item">
            <div className="legend-color" style={{background: '#ef4444'}}></div>
            <span>Pivot Nodes</span>
          </div>
          <div className="legend-item">
            <div className="legend-color" style={{background: '#10b981'}}></div>
            <span>Optimal Path</span>
          </div>
        </div>

      </div>
    </div>
  );
}

export default App;
