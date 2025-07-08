package bittorrentsim;
simple BTClient {
    parameters:
        int peerId;
        bool isSeeder = default(false);
        int numPieces;
        string pieceSelection = default("rarest"); // rarest|sequential
        @display("i=device/pc");
    gates:
        input in[];  // Peer-to-peer input gates
        output out[]; // Peer-to-peer output gates
        input trackerIn; // Tracker input gate (single)
        output trackerOut; // Tracker output gate (single)
}
