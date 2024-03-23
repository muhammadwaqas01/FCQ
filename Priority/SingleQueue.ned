network SingleQueue
{
    submodules:
        source: Source {
            @display("p=100,100");
        }
        source1: Source1 {
            @display("p=100,27");
        }
        fifo: Fifo {
            @display("p=200,100");
        }
        sink: Sink {
            @display("p=300,100");
        }
    connections:
        source1.out --> fifo.in[0];
        source.out --> fifo.in[1];
        fifo.out --> sink.in;
}


