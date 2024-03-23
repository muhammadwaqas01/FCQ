
#include <omnetpp.h>

using namespace omnetpp;

namespace fifo {


class Sink : public cSimpleModule
{
  private:
    simsignal_t msgProcessedFromSourceSignal;
    simsignal_t msgProcessedFromSource1Signal;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Sink);

void Sink::initialize()
{
    msgProcessedFromSourceSignal = registerSignal("msgProcessedFromSource");
    msgProcessedFromSource1Signal = registerSignal("msgProcessedFromSource1");

}

void Sink::handleMessage(cMessage *msg)
{

    const char *origin = msg->par("origin").stringValue();
    if (strcmp(origin, "Source") == 0) {
        emit(msgProcessedFromSourceSignal, 1);
    } else if (strcmp(origin, "Source1") == 0) {
        emit(msgProcessedFromSource1Signal, 1);
    }


    delete msg;
}


}; //namespace

