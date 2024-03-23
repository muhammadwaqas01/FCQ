#ifndef __FIFO_H
#define __FIFO_H

#include "AbstractFifo.h"

namespace fifo {

class Fifo : public AbstractFifo
{
  protected:
    virtual simtime_t startService(cMessage *msg) override;
    virtual void endService(cMessage *msg) override;
};

}; //namespace

#endif
