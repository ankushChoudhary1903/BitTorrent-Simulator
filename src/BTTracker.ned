package bittorrentsim;

simple BTTracker {
    parameters:
        int numPieces;
        @display("i=block/server");
    gates:
    input trackerIn[];  // Change to vector gate
    output trackerOut[];  // Change to vector gate
}
