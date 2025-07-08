#ifndef __BTCLIENT_H_
#define __BTCLIENT_H_

#include <omnetpp.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>

using namespace omnetpp;

class BTClient : public cSimpleModule {
  private:
    int peerId;
    bool isSeeder;
    int numPieces;
    std::vector<bool> pieces;     //tracks the piece peer has
    std::vector<int> peerList;    //List of active peers
    std::map<int, double> uploadRates;

    cMessage *startMsg;
    cMessage *chokeTimer;

    simsignal_t downloadCompletedSignal;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void sendTrackerAnnounce();
    int findMissingPiece();
    int findRarestPiece();
    void requestPiece();
    void updateChoking();
    void unchokePeer(int neighbor);
    void chokePeer(int neighbor);

  public:
    virtual ~BTClient();
};

#endif
