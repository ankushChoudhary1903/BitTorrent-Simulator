# 🌐 BitTorrent Protocol Simulator using OMNeT++

This project simulates the core behavior of the **BitTorrent protocol** using the OMNeT++ simulation framework. It models tracker-client interactions, peer-to-peer file sharing, and various topologies like chain and mesh to analyze performance, bandwidth usage, and swarm efficiency.

---

## 📌 Features

- 🧩 **Modular Peer Design**: Implements seeder and leecher behavior using OMNeT++ modules (`BTClient`, `BTTracker`)
- 🔄 **Rarest-First Piece Selection**: Efficient piece sharing with high swarm utilization
- 💬 **Tit-for-Tat Bandwidth Allocation**: Implements BitTorrent's choking and unchoking logic
- 🚀 **Endgame Mode Support**: Parallel final requests to ensure fast completion
- 📊 **Performance Metrics**: Measures propagation time, seed load, and peer completion percentages

---

## 🛠️ Technologies Used

- **Simulation Framework**: [OMNeT++](https://omnetpp.org)
- **Languages**: C++, NED
- **Topology Configuration**: `.ned` files
- **Simulation Config**: `.ini` file with runtime settings

