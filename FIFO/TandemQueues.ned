network TandemQueues
{
    parameters:
        @display("i=block/network");
    submodules:
        source: Source {
            @display("p=100,100");
        }
        fifo1: Fifo {
            @display("p=200,100");
        }
        fifo2: Fifo {
            @display("p=300,100");
        }
        sink: Sink {
            @display("p=400,100");
        }
    connections:
        source.out --> fifo1.in[0];
        fifo1.out --> fifo2.in[1];
        fifo2.out --> sink.in;
}
