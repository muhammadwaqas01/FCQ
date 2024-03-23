#include "Fifo.h"

namespace fifo {

Define_Module(Fifo);

simtime_t Fifo::startService(cMessage *msg)
{
    simtime_t serviceTime;
    if (msg->hasPar("serviceTime")) {
        serviceTime = msg->par("serviceTime").doubleValue();

    }
    EV << "Starting service of " << msg->getName() << " with service time: " << serviceTime << endl;
    return serviceTime;
}

void Fifo::endService(cMessage *msg)
{
    EV << "Completed service of " << msg->getName() << endl;
    send(msg, "out");
}

}; //namespace

