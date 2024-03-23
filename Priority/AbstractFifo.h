#include <vector>  // Include this for std::vector
#ifndef __ABSTRACTFIFO_H
#define __ABSTRACTFIFO_H

#include <omnetpp.h>

using namespace omnetpp;

namespace fifo {

class AbstractFifo : public cSimpleModule
{
  protected:
    cMessage *msgServiced = nullptr;
    cMessage *endServiceMsg = nullptr;
    cQueue queue;

    simsignal_t msgDroppedFromSourceSignal;
    simsignal_t msgDroppedFromSource1Signal;

    long resourceCapacity;  // Add this line
    int bufferSize;

    std::vector<cMessage*> servicedJobs;
    std::map<cMessage*, cMessage*> endServiceMsgs;
    std::vector<cMessage*> activeJobs;

    double totalWaitTimeFromSource = 0;
    double totalWaitTimeFromSource1 = 0;
    int msgProcessedFromSource = 0;
    int msgProcessedFromSource1 = 0;
    simsignal_t avgWaitTimeFromSourceSignal;
    simsignal_t avgWaitTimeFromSource1Signal;

    long sumOfOccupiedResources = 0;  // Sum of occupied resources
    int numOfCheckIntervals = 0;  // Number of times resources have been checked
    simsignal_t avgResourceOccupationSignal;  // Signal for average resource occupation
    double checkInterval;  // Time interval for checking resource occupation

    simtime_t lastConditionMetTime = 0;  // Time when the condition was last met
    simtime_t cumulativeWaitTimePenalty = 0;  // Cumulative wait time penalty
    bool conditionWasMet = false;  // Flag to keep track of whether the condition was previously met

    simsignal_t cumulativeWaitTimePenaltySignal;  // New signal
    long cumulativePacketsInProgress = 0;
    int checkCount = 0;
    int logDetailsCount = 0;

    std::map<int, int> serviceCountSource;
    std::map<int, int> serviceCountSource1;
    std::map<int, int> bufferCountSource;
    std::map<int, int> bufferCountSource1;

  public:
    virtual ~AbstractFifo();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    // Hook functions to (re)define behaviour
    virtual void arrival(cMessage *msg) {}
    virtual void finish() override;
    void CheckCumulativeWait();  // Add this line

    virtual simtime_t startService(cMessage *msg) = 0;
    virtual void endService(cMessage *msg) = 0;
    void logQueueDetails();
};

};

#endif
