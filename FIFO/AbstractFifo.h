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

    simsignal_t queueingTimeSignal;

    simsignal_t msgDroppedFromSourceSignal;
    simsignal_t msgDroppedFromSource1Signal;
    simsignal_t cumulativeWaitTimePenaltySignal;

    long resourceCapacity;  // Add this line
    int bufferSize;

    std::vector<cMessage*> servicedJobs;
    std::map<cMessage*, cMessage*> endServiceMsgs;
    std::vector<cMessage*> activeJobs;

    simsignal_t avgWaitTimeFromSourceSignal;  // New signal
    simsignal_t avgWaitTimeFromSource1Signal; // New signal

    double totalWaitTimeFromSource = 0.0;  // To store the sum of waiting times from Source
    double totalWaitTimeFromSource1 = 0.0; // To store the sum of waiting times from Source1
    long msgProcessedFromSource = 0;  // New
    long msgProcessedFromSource1 = 0; // New

    simsignal_t avgResourceOccupationSignal; // New signal
    simtime_t checkInterval; // New member to store check interval
    long sumOfOccupiedResources = 0; // To store the sum of occupied resources
    long numOfCheckIntervals = 0;  // To store the number of intervals

    simtime_t lastConditionMetTime = 0;  // Time when the condition was last met
    simtime_t cumulativeWaitTimePenalty = 0;  // Cumulative wait time penalty
    bool conditionWasMet = false;  // Flag to keep track of whether the condition was previously met
    long cumulativePacketsInProgress = 0;
    long checkCounts = 0;
    int logDetailsCount = 0;

    std::map<int, int> serviceCountSource;
    std::map<int, int> serviceCountSource1;
    std::map<int, int> bufferCountSource;
    std::map<int, int> bufferCountSource1;

  public:
    virtual ~AbstractFifo();
    int getLogDetailsCount() const { return logDetailsCount; } // Add this getter method


  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void CheckCumulativeWait();
    virtual void arrival(cMessage *msg) {}
    virtual simtime_t startService(cMessage *msg) = 0;
    virtual void endService(cMessage *msg) = 0;
    void logQueueDetails();
    virtual void finish() override;  // Add this line
};

};

#endif
