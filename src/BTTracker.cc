#include "BTTracker.h"

Define_Module(BTTracker);

void BTTracker::initialize() {
    EV_INFO << "Initializing BTTracker..." << endl;

    // Get the number of pieces from parameters
    numPieces = par("numPieces");

    EV_INFO << "BTTracker initialized with " << numPieces << " pieces" << endl;
}

void BTTracker::handleMessage(cMessage *msg) {
    EV_INFO << "BTTracker received message: " << msg->getName() << endl;

    if (msg->arrivedOn("trackerIn")) {
        if (strcmp(msg->getName(), "announce") == 0) {
            processAnnounce(msg);   //Register
        }
        else if (strcmp(msg->getName(), "pieceUpdate") == 0) {
            processPieceUpdate(msg);    //Update
        }
        else {
            EV_WARN << "BTTracker received unknown message type: " << msg->getName() << endl;
            delete msg;
        }
    }
    else {
        EV_WARN << "BTTracker received message on unknown gate" << endl;
        delete msg;
    }
}

void BTTracker::processAnnounce(cMessage *msg) {
    int peerId = msg->par("peerId");
    int arrivalGateIndex = msg->getArrivalGate()->getIndex();

    EV_INFO << "BTTracker processing announce from peer " << peerId << " on gate " << arrivalGateIndex << endl;

    // Add peer to active list if not already present
    if (std::find(activePeers.begin(), activePeers.end(), peerId) == activePeers.end()) {
        activePeers.push_back(peerId);
        EV_INFO << "Added peer " << peerId << " to active peers" << endl;
    }

    // Convert vector to comma-separated string
    std::ostringstream oss;
    for (size_t i = 0; i < activePeers.size(); ++i) {
        if (i > 0) oss << ",";
        oss << activePeers[i];
    }

    cMessage *resp = new cMessage("trackerResponse");
    resp->addPar("peerList").setStringValue(oss.str().c_str());

    // Check if gate index is valid
    if (arrivalGateIndex >= 0 && arrivalGateIndex < gateSize("trackerOut")) {
        EV_INFO << "BTTracker sending response to peer on gate " << arrivalGateIndex << endl;
        send(resp, "trackerOut", arrivalGateIndex);
    } else {
        EV_ERROR << "Invalid arrival gate index: " << arrivalGateIndex << endl;
        delete resp;
    }

    delete msg;
}

void BTTracker::processPieceUpdate(cMessage *msg) {
    int peerId = msg->par("peerId");
    int pieceIdx = msg->par("pieceIdx");

    // Validate piece index
    if (pieceIdx >= 0 && pieceIdx < numPieces) {
        EV_INFO << "BTTracker received piece update: peer " << peerId << " has piece " << pieceIdx << endl;
        peerPieces[peerId][pieceIdx] = true;
    } else {
        EV_ERROR << "Received invalid piece index in update: " << pieceIdx << endl;
    }

    delete msg;
}
