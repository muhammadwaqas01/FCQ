

#include <omnetpp.h>

using namespace omnetpp;

namespace fifo {

class Source1 : public cSimpleModule
{
  private:
    cMessage *sendMessageEvent = nullptr;
    simsignal_t msgGeneratedSignal;

  public:
    virtual ~Source1();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Source1);

Source1::~Source1()
{
    cancelAndDelete(sendMessageEvent);
}

void Source1::initialize()
{
    sendMessageEvent = new cMessage("sendMessageEvent");
    scheduleAt(simTime(), sendMessageEvent);
    msgGeneratedSignal = registerSignal("msgGenerated");  // Register the signal here

}

void Source1::handleMessage(cMessage *msg)
{
    ASSERT(msg == sendMessageEvent);

    cMessage *job = new cMessage("job");
    int requiredResourceValue = par("requiredResource").intValue();
    job->addPar("origin").setStringValue("Source1");
    job->addPar("requiredResource").setLongValue(requiredResourceValue);  // Add this line
    job->addPar("serviceTime").setDoubleValue(par("serviceTime").doubleValue());

    // Logging message ID and required resources
    EV << "Generated message with ID: " << job->getId() << ", Required Resources: " << requiredResourceValue << endl;

    send(job, "out");
    scheduleAt(simTime()+par("interarrivalTime").doubleValue(), sendMessageEvent);
    emit(msgGeneratedSignal, 1);
}

}; //namespace

