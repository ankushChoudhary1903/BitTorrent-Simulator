#include "BTClient.h"
#include <algorithm>
#include <climits>

Define_Module(BTClient);

BTClient::~BTClient() {
    cancelAndDelete(startMsg);
    cancelAndDelete(chokeTimer);   //cancels pending messages to prevent leaks
}

void BTClient::initialize() {
    EV_INFO << "Initializing BTClient..." << endl;

    peerId = par("peerId");
    isSeeder = par("isSeeder");
    numPieces = par("numPieces");        //reads from ned,ini
    pieces.resize(numPieces, false);

    EV_INFO << "BTClient " << peerId << " initializing with " << numPieces << " pieces, isSeeder=" << (isSeeder ? "true" : "false") << endl;
    // Seeders start with all pieces
    if (isSeeder)
        std::fill(pieces.begin(), pieces.end(), true);

    // Protocol initialization
    startMsg = new cMessage("start");
    scheduleAt(simTime() + uniform(0,1), startMsg);   //1st piece request after a random delay

    chokeTimer = new cMessage("chokeUpdate");
    scheduleAt(simTime() + 30, chokeTimer);    //schedule choking update

    // Register with tracker
    sendTrackerAnnounce();

    // Statistics
    downloadCompletedSignal = registerSignal("downloadCompleted");

    EV_INFO << "BTClient " << peerId << " initialized successfully" << endl;
}

void BTClient::handleMessage(cMessage *msg) {
    EV_INFO << "BTClient " << peerId << " received message: " << msg->getName() << endl;

    if (msg == startMsg) {
        requestPiece();
        scheduleAt(simTime() + 1, startMsg);
    }
    else if (msg == chokeTimer) {
        updateChoking();
        scheduleAt(simTime() + 30, chokeTimer);
    }
    else if (msg->arrivedOn("trackerIn")) {
        // Messages from tracker
        if (msg->isName("trackerResponse")) {
            EV_INFO << "BTClient " << peerId << " received tracker response" << endl;  //parse the message and update the piece list

            const char *peerListStr = msg->par("peerList").stringValue();
            std::istringstream iss(peerListStr);
            std::string token;
            peerList.clear();

            // Split string into integers
            while (std::getline(iss, token, ',')) {
                try {
                    peerList.push_back(std::stoi(token));
                } catch (const std::exception& e) {
                    EV_ERROR << "Error parsing peer list: " << e.what() << endl;
                }
            }

            EV_INFO << "BTClient " << peerId << " updated peer list, size: " << peerList.size() << endl;
            delete msg;
        } else {
            EV_WARN << "BTClient " << peerId << " received unknown message type from tracker: " << msg->getName() << endl;
            delete msg;
        }
    }
    else if (msg->arrivedOn("in")) {
        // Messages from peers
        if (msg->isName("piece")) {
            int pieceIdx = msg->par("pieceIdx");

            // Validate piece index
            if (pieceIdx >= 0 && pieceIdx < numPieces) {
                if (!pieces[pieceIdx]) {
                    pieces[pieceIdx] = true;
                    EV_INFO << "Peer " << peerId << " received piece " << pieceIdx << endl;

                    // Notify tracker
                    cMessage *update = new cMessage("pieceUpdate");
                    update->addPar("peerId") = peerId;
                    update->addPar("pieceIdx") = pieceIdx;
                    send(update, "trackerOut");

                    if (std::all_of(pieces.begin(), pieces.end(), [](bool v){return v;})) {
                        EV_INFO << "Peer " << peerId << " download completed!" << endl;
                        emit(downloadCompletedSignal, simTime());
                    }
                }
            } else {
                EV_ERROR << "Received invalid piece index: " << pieceIdx << endl;
            }
            delete msg;
        }
        else if (msg->isName("request")) {
            int pieceIdx = msg->par("pieceIdx");

            // Validate piece index and gate index
            if (pieceIdx >= 0 && pieceIdx < numPieces) {
                if (pieces[pieceIdx]) {
                    int arrivalGateIndex = msg->getArrivalGate()->getIndex();

                    cMessage *pieceMsg = new cMessage("piece");
                    pieceMsg->addPar("pieceIdx") = pieceIdx;

                    // Check if gate index is valid
                    if (arrivalGateIndex >= 0 && arrivalGateIndex < gateSize("out")) {
                        send(pieceMsg, "out", arrivalGateIndex);

                        // Update upload rate
                        uploadRates[arrivalGateIndex] += 1.0; // Track uploaded pieces
                        EV_INFO << "Peer " << peerId << " sent piece " << pieceIdx << " to peer on gate " << arrivalGateIndex << endl;
                    } else {
                        EV_ERROR << "Invalid arrival gate index: " << arrivalGateIndex << endl;
                        delete pieceMsg;
                    }
                } else {
                    EV_INFO << "Peer " << peerId << " doesn't have requested piece " << pieceIdx << endl;
                }
            } else {
                EV_ERROR << "Received request for invalid piece index: " << pieceIdx << endl;
            }
            delete msg;
        } else {
            EV_WARN << "BTClient " << peerId << " received unknown message type from peer: " << msg->getName() << endl;
            delete msg;
        }
    } else {
        EV_WARN << "BTClient " << peerId << " received message on unknown gate" << endl;
        delete msg;
    }
}

void BTClient::sendTrackerAnnounce() {
    EV_INFO << "Peer " << peerId << " sending announce to tracker" << endl;

    cMessage *announce = new cMessage("announce");
    announce->addPar("peerId") = peerId;
    send(announce, "trackerOut");
}

int BTClient::findMissingPiece() {
    if (strcmp(par("pieceSelection"), "rarest") == 0)
        return findRarestPiece();
    else {
        for (int i=0; i<numPieces; ++i)
            if (!pieces[i]) return i;
        return -1;
    }
}

int BTClient::findRarestPiece() {
    std::vector<int> counts(numPieces, 0);

    for (int peer : peerList) {
        // Skip invalid peers
        if (peer < 0) continue;

        // Simulate peer's piece availability (replace with real logic later)
        for (int i = 0; i < numPieces; ++i) {
            if (i % (peer + 1) == 0) { // Example logic using 'peer'
                counts[i]++;
            }
        }
    }

    int rarestIdx = -1, minCount = INT_MAX;
    for (int i = 0; i < numPieces; ++i) {
        if (!pieces[i] && counts[i] < minCount) {
            rarestIdx = i;
            minCount = counts[i];
        }
    }

    return rarestIdx;
}

void BTClient::requestPiece() {
    int missing = findMissingPiece();
    if (missing == -1) return;

    if (!peerList.empty()) {
        // Choose a random peer from our list
        int peerIdx = intuniform(0, peerList.size()-1);

        // Bounds check
        if (peerIdx >= 0 && peerIdx < (int)peerList.size()) {
            int neighbor = peerList[peerIdx];

            EV_INFO << "Peer " << peerId << " requesting piece " << missing << " from peer " << neighbor << endl;

            cMessage *req = new cMessage("request");
            req->addPar("pieceIdx") = missing;

            // Check if gate index is valid
            if (neighbor >= 0 && neighbor < gateSize("out")) {
                send(req, "out", neighbor);
            } else {
                EV_ERROR << "Invalid gate index for sending request: " << neighbor << endl;
                delete req;
            }
        } else {
            EV_ERROR << "Invalid peer index: " << peerIdx << endl;
        }
    }
}

void BTClient::updateChoking() {
    EV_INFO << "Peer " << peerId << " updating choking status" << endl;

    // Tit-for-tat: Unchoke top 4 uploaders
    std::vector<std::pair<int, double>> sorted(uploadRates.begin(), uploadRates.end());
    std::sort(sorted.begin(), sorted.end(),
        [](auto& a, auto& b){ return a.second > b.second; });

    for (size_t i=0; i<sorted.size() && i < 5; ++i) {  // Limit to avoid overflow
        if (i < 4) unchokePeer(sorted[i].first);
        else chokePeer(sorted[i].first);
    }

    // Optimistic unchoke
    if (!peerList.empty()) {
        int optimisticPeerIdx = intuniform(0, peerList.size()-1);

        // Bounds check
        if (optimisticPeerIdx >= 0 && optimisticPeerIdx < (int)peerList.size()) {
            int optimisticPeer = peerList[optimisticPeerIdx];
            unchokePeer(optimisticPeer);
        }
    }
}

void BTClient::unchokePeer(int neighbor) {
    // Validate neighbor index
    if (neighbor < 0) {
        EV_ERROR << "Invalid neighbor index for unchoking: " << neighbor << endl;
        return;
    }

    // Implement actual unchoking logic
    EV_DEBUG << "Unchoking peer " << neighbor << endl;
}

void BTClient::chokePeer(int neighbor) {
    // Validate neighbor index
    if (neighbor < 0) {
        EV_ERROR << "Invalid neighbor index for choking: " << neighbor << endl;
        return;
    }

    // Implement actual choking logic
    EV_DEBUG << "Choking peer " << neighbor << endl;
}
