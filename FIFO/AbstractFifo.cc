//FIFO
#include <fstream>

#include "AbstractFifo.h"

namespace fifo {  // Start of the 'fifo' namespace

// Destructor
AbstractFifo::~AbstractFifo()
{
    delete msgServiced;
    cancelAndDelete(endServiceMsg);
}

// Initialization function
void AbstractFifo::initialize()
{
    endServiceMsg = new cMessage("end-service");
    queue.setName("queue");

    msgDroppedFromSourceSignal = registerSignal("msgDroppedFromSource");
    msgDroppedFromSource1Signal = registerSignal("msgDroppedFromSource1");

    bufferSize = par("bufferSize");
    resourceCapacity = par("resourceCapacity").intValue();

    avgWaitTimeFromSourceSignal = registerSignal("avgWaitTimeFromSource");
    avgWaitTimeFromSource1Signal = registerSignal("avgWaitTimeFromSource1");

    totalWaitTimeFromSource = 0;
    totalWaitTimeFromSource1 = 0;
    msgProcessedFromSource = 0;
    msgProcessedFromSource1 = 0;

    avgResourceOccupationSignal = registerSignal("avgResourceOccupation");
    checkInterval = par("checkInterval").doubleValue();
    scheduleAt(simTime() + checkInterval, new cMessage("checkResource"));
    cumulativeWaitTimePenaltySignal = registerSignal("cumulativeWaitTimePenalty");


}

void printQueueDetails(cQueue &queue)
{
    EV << "Current jobs in the queue (" << queue.getLength() << "):\n";
    for (cQueue::Iterator iter(queue); !iter.end(); ++iter) {
        cMessage *job = (cMessage *) *iter;
        int requiredResource = static_cast<int>(job->par("requiredResource").longValue());
        EV << "Source Module: " << job->getSenderModule()->getFullName()
           << ", ID: " << job->getId()
           << ", Required Resources: " << requiredResource << '\n';
    }
}

void printActiveJobsDetails(const std::vector<cMessage*>& activeJobs)
{
    EV << "Currently active jobs (" << activeJobs.size() << "):\n";
    for (const auto& job : activeJobs) {
        EV << "Source Module: " << job->getSenderModule()->getFullName()
           << ", ID: " << job->getId()
           << ", Required Resources: " << static_cast<int>(job->par("requiredResource").longValue()) << '\n';
    }
}
const double roundingFactor = 1000.0;

void AbstractFifo::CheckCumulativeWait()
{
    // Track resources acquired by Source1 messages
    long source1AcquiredResources = 0;
    EV << "Checking Source1 acquired resources...\n";

    // Check if a "Source1" message has acquired resources
    bool source1Acquired = false;
    for (const auto& job : activeJobs) {
        if (strcmp(job->getSenderModule()->getFullName(), "source1") == 0) {
            source1Acquired = true;
            source1AcquiredResources += static_cast<long>(job->par("requiredResource").longValue());
        }
    }
    EV << "Total resources acquired by Source1: " << source1AcquiredResources << '\n';

    // Check if a "Source" message is waiting in the buffer and its required resources
    bool sourceWaiting = false;
    long sourceRequiredResources = 0;
    EV << "Checking if a Source message is waiting...\n";

    for (cQueue::Iterator iter(queue); !iter.end(); ++iter) {
        cMessage *job = (cMessage *) *iter;
        if (strcmp(job->getSenderModule()->getFullName(), "source") == 0) {
            sourceWaiting = true;
            sourceRequiredResources = static_cast<long>(job->par("requiredResource").longValue());
            break;
        }
    }
    EV << "Required resources for waiting Source message: " << sourceRequiredResources << '\n';

    // Check the sub-conditions
    bool subConditionA = source1AcquiredResources + resourceCapacity >= sourceRequiredResources;
    bool subConditionB = source1AcquiredResources >= sourceRequiredResources;
    EV << "Checking sub-conditions...\n";
    EV << "Sub-Condition A: " << (subConditionA ? "True" : "False") << '\n';
    EV << "Sub-Condition B: " << (subConditionB ? "True" : "False") << '\n';

    if (source1Acquired && sourceWaiting && (subConditionA || subConditionB)) {
        if (!conditionWasMet) {  // Condition just became true
            lastConditionMetTime = simTime();
            conditionWasMet = true;
        }
        EV << "Condition met: 'Source1' has acquired resources AND 'Source' is waiting in the buffer with the new subconditions.\n";
    } else     if (conditionWasMet) {  // Condition just became false
        cumulativeWaitTimePenalty += simTime() - lastConditionMetTime;
        EV << "CumulativeWaitTimePenalty: " << cumulativeWaitTimePenalty << '\n';
        emit(cumulativeWaitTimePenaltySignal, cumulativeWaitTimePenalty);
        conditionWasMet = false;
    }
}

// Message handling function
void AbstractFifo::handleMessage(cMessage *msg)
{
    simtime_t serviceTime;

    if (endServiceMsgs.count(msg) > 0) {
        cMessage *job = endServiceMsgs[msg];
        int releasedResources = static_cast<int>(job->par("requiredResource").longValue());
        resourceCapacity += releasedResources;

        EV << "Job completed: "
           << "Source Module: " << job->getSenderModule()->getFullName()
           << ", Message ID: " << job->getId()
           << ", Resources Released: " << releasedResources
           << ", Total Resources now: " << resourceCapacity << '\n';

        endService(job);
        activeJobs.erase(std::remove(activeJobs.begin(), activeJobs.end(), job), activeJobs.end());
        cancelAndDelete(msg);
        endServiceMsgs.erase(msg);
    }
    else if (strcmp(msg->getName(), "checkResource") == 0) {
        cumulativePacketsInProgress += activeJobs.size();
        checkCounts++;
        EV << "Time: " << simTime() << ", Active Jobs: " << activeJobs.size()
           << ", Cumulative Packets in Progress: " << cumulativePacketsInProgress << endl;
        logQueueDetails();
        // Calculate occupied resources
        long occupiedResources = par("resourceCapacity").intValue() - resourceCapacity;
        sumOfOccupiedResources += occupiedResources;
        numOfCheckIntervals++;

        // Reschedule the next check
        scheduleAt(simTime() + checkInterval, new cMessage("checkResource"));
        delete msg;
        return;
    }

    else {
        if (queue.getLength() >= bufferSize) {
            EV << "Buffer is full. Dropping message from " << msg->getSenderModule()->getFullName() << " with ID: " << msg->getId() << "\n";

            const char *origin = msg->par("origin").stringValue();
            if (strcmp(origin, "Source") == 0) {
                emit(msgDroppedFromSourceSignal, 1);
            } else if (strcmp(origin, "Source1") == 0) {
                emit(msgDroppedFromSource1Signal, 1);
            }

            delete msg;
            return;
        }

        if (msg->hasPar("serviceTime")) {
            serviceTime = msg->par("serviceTime").doubleValue();
        }

        EV << "Incoming job details: "
           << "Source Module: " << msg->getSenderModule()->getFullName()
           << ", Message ID: " << msg->getId()
           << ", Required Resource: " << static_cast<int>(msg->par("requiredResource").longValue()) << '\n';

        msg->setTimestamp();
        arrival(msg);
        queue.insert(msg);
    }

    while (!queue.isEmpty()) {
        cMessage *nextJob = (cMessage *)queue.front();
        long requiredResource = static_cast<long>(nextJob->par("requiredResource"));

        if (requiredResource <= resourceCapacity) {
            simtime_t waitTime = simTime() - nextJob->getTimestamp();
            waitTime = round(waitTime.dbl() * roundingFactor) / roundingFactor;
            const char *origin = nextJob->par("origin").stringValue();
            if (strcmp(origin, "Source") == 0) {
                totalWaitTimeFromSource += static_cast<double>(waitTime.dbl());
                msgProcessedFromSource++;
            } else if (strcmp(origin, "Source1") == 0) {
                totalWaitTimeFromSource1 += static_cast<double>(waitTime.dbl());
                msgProcessedFromSource1++;
            }

            queue.pop();
            resourceCapacity -= requiredResource;
            activeJobs.push_back(nextJob);

            if (nextJob->hasPar("serviceTime")) {
                serviceTime = nextJob->par("serviceTime").doubleValue();
            }

            simtime_t serviceEndTime = startService(nextJob);
            cMessage *newEndServiceMsg = new cMessage("end-service");
            scheduleAt(simTime() + serviceEndTime, newEndServiceMsg);
            endServiceMsgs[newEndServiceMsg] = nextJob;
        }
        else {
            break;
        }
    }
    CheckCumulativeWait();

    printActiveJobsDetails(activeJobs);
    printQueueDetails(queue);
}

void AbstractFifo::logQueueDetails() {
    logDetailsCount++; // Increment the counter

    int messagesInServiceSource = 0;
    int messagesInServiceSource1 = 0;
    int totalMessagesInService = activeJobs.size();  // Assuming activeJobs contains all jobs currently being serviced

    int messagesInBufferSource = 0;
    int messagesInBufferSource1 = 0;
    int totalMessagesInBuffer = queue.getLength();

    // Count messages in service from each source
    for (const auto& job : activeJobs) {
        if (strcmp(job->getSenderModule()->getFullName(), "source") == 0) {
            messagesInServiceSource++;
        } else if (strcmp(job->getSenderModule()->getFullName(), "source1") == 0) {
            messagesInServiceSource1++;
        }
    }

    // Update service count maps
    serviceCountSource[messagesInServiceSource]++;
    serviceCountSource1[messagesInServiceSource1]++;

    // Count messages in buffer from each source
    for (cQueue::Iterator iter(queue); !iter.end(); ++iter) {
        cMessage *job = (cMessage *) *iter;
        if (strcmp(job->getSenderModule()->getFullName(), "source") == 0) {
            messagesInBufferSource++;
        } else if (strcmp(job->getSenderModule()->getFullName(), "source1") == 0) {
            messagesInBufferSource1++;
        }
    }

    // Update buffer count maps
    bufferCountSource[messagesInBufferSource]++;
    bufferCountSource1[messagesInBufferSource1]++;

    // Logging the counts
    EV << "Messages in Service from Source: " << messagesInServiceSource << '\n';
    EV << "Messages in Service from Source1: " << messagesInServiceSource1 << '\n';
    EV << "Total Messages in Service: " << totalMessagesInService << '\n';
    EV << "Messages in Buffer from Source: " << messagesInBufferSource << '\n';
    EV << "Messages in Buffer from Source1: " << messagesInBufferSource1 << '\n';
    EV << "Total Messages in Buffer: " << totalMessagesInBuffer << '\n';
    EV << "Occurrences of " << messagesInServiceSource << " messages in service from Source: " << serviceCountSource[messagesInServiceSource] << '\n';
    EV << "Occurrences of " << messagesInServiceSource1 << " messages in service from Source1: " << serviceCountSource1[messagesInServiceSource1] << '\n';
    EV << "Occurrences of " << messagesInBufferSource << " messages in buffer from Source: " << bufferCountSource[messagesInBufferSource] << '\n';
    EV << "Occurrences of " << messagesInBufferSource1 << " messages in buffer from Source1: " << bufferCountSource1[messagesInBufferSource1] << '\n';
    EV << "Counted Log Details: " << logDetailsCount << '\n';
}


void AbstractFifo::finish()
{
    if (checkCounts > 0) {
        double avgPacketsInProgress = static_cast<double>(cumulativePacketsInProgress) / checkCounts;
        recordScalar("AveragePacketsInProgress", avgPacketsInProgress);
    }
    if (msgProcessedFromSource > 0) {
        double avgWaitTimeFromSource = totalWaitTimeFromSource / msgProcessedFromSource;
        emit(avgWaitTimeFromSourceSignal, avgWaitTimeFromSource);
        recordScalar("SourceAvgWaitTime", avgWaitTimeFromSource);
        recordScalar("SourceThrougput", msgProcessedFromSource/5000000);

    }

    if (msgProcessedFromSource1 > 0) {
        double avgWaitTimeFromSource1 = totalWaitTimeFromSource1 / msgProcessedFromSource1;
        emit(avgWaitTimeFromSource1Signal, avgWaitTimeFromSource1);
        recordScalar("Source1AvgWaitTime", avgWaitTimeFromSource1);
        recordScalar("Source1Througput", msgProcessedFromSource1/5000000);

    }

    if (numOfCheckIntervals > 0) {
        double avgResourceOccupation = (double) sumOfOccupiedResources / (double) numOfCheckIntervals;
//        avgResourceOccupation /= par("resourceCapacity").intValue();
        emit(avgResourceOccupationSignal, avgResourceOccupation);
        recordScalar("avgResourceOccupation", avgResourceOccupation);
        recordScalar("Resource Utilization", avgResourceOccupation/256); //256 is the total resources available
        recordScalar("Cumulative Wait", cumulativeWaitTimePenalty);
        recordScalar("Wait Penalty Percentage", cumulativeWaitTimePenalty/5000000);



    }


    int runNumber = getSimulation()->getActiveEnvir()->getConfigEx()->getActiveRunNumber();

    // Construct unique filenames for each run
    std::string filename = "FIFOQueueStatistics_run" + std::to_string(runNumber) + ".csv";
    std::string adjustedFilename = "AdjustedFIFOQueueStatistics_run" + std::to_string(runNumber) + ".csv";

    // Writing the first portion of statistics
    std::ofstream outputFile;
    outputFile.open(filename);

    outputFile << "Message Count, Occurrence in Buffer from Source, Occurrence in Buffer from Source1, Occurrence in Service from Source, Occurrence in Service from Source1\n";

    int maxCount = std::max({serviceCountSource.rbegin()->first, serviceCountSource1.rbegin()->first,
                             bufferCountSource.rbegin()->first, bufferCountSource1.rbegin()->first});

    for (int i = 0; i <= maxCount; ++i) {
        outputFile << i << ", "
                   << bufferCountSource[i] << ", "
                   << bufferCountSource1[i] << ", "
                   << serviceCountSource[i] << ", "
                   << serviceCountSource1[i] << "\n";
    }

    outputFile.close();

    // Writing the adjusted statistics
    std::ofstream adjustedOutputFile;
    adjustedOutputFile.open(adjustedFilename);

    adjustedOutputFile << "Adjusted Message Count, Adjusted Occurrence in Buffer from Source, Adjusted Occurrence in Buffer from Source1, Adjusted Occurrence in Service from Source, Adjusted Occurrence in Service from Source1\n";

    for (int i = 0; i <= maxCount; ++i) {
        adjustedOutputFile << i << ", "
                           << static_cast<double>(bufferCountSource[i]) / logDetailsCount << ", "
                           << static_cast<double>(bufferCountSource1[i]) / logDetailsCount << ", "
                           << static_cast<double>(serviceCountSource[i]) / logDetailsCount << ", "
                           << static_cast<double>(serviceCountSource1[i]) / logDetailsCount << "\n";
    }

    adjustedOutputFile.close();
}}
