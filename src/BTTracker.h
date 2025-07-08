#ifndef __BTTRACKER_H_
#define __BTTRACKER_H_

#include <omnetpp.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>

using namespace omnetpp;

class BTTracker : public cSimpleModule {
  private:
    std::vector<int> activePeers;
    std::map<int, std::map<int, bool>> peerPieces;  // [peerId][pieceIdx] = hasPiece
    int numPieces;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void processAnnounce(cMessage *msg);
    void processPieceUpdate(cMessage *msg);
};

#endif
