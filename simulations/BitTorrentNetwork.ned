package bittorrentsim.simulations;
import bittorrentsim.BTClient;
import bittorrentsim.BTTracker;

network BitTorrentNetwork {
    parameters:
        int numPeers = default(50);
        int numSeeders = default(1);
        string topology = default("mesh"); // mesh|chain
        double connectionProbability = default(0.3);
        int numPieces = default(100);

    submodules:
        tracker: BTTracker {
            parameters:
                numPieces = parent.numPieces;
            @display("p=150,50");
        }
        peer[numPeers]: BTClient {
            parameters:
                peerId = index;
                isSeeder = (index < parent.numSeeders);
                numPieces = parent.numPieces;
                pieceSelection = "rarest";
            @display("p=50,150,m,n");
        }

    connections allowunconnected:
        // Connect clients to tracker
        for i=0..numPeers-1 {
            peer[i].trackerOut --> tracker.trackerIn++;
            tracker.trackerOut++ --> peer[i].trackerIn;
        }

        // Peer-to-peer connections (mesh)
        for i=0..numPeers-1, for j=i+1..numPeers-1 {
            peer[i].out++ --> peer[j].in++ 
                if (topology == "mesh" && uniform(0,1) < connectionProbability);
            peer[j].out++ --> peer[i].in++ 
                if (topology == "mesh" && uniform(0,1) < connectionProbability);
        }

        // Chain topology connections
        for i=0..numPeers-2 {
            peer[i].out++ --> peer[i+1].in++ if (topology == "chain");
            peer[i+1].out++ --> peer[i].in++ if (topology == "chain");
        }
}