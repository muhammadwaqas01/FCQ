simple Fifo
{
    parameters:
       // volatile double serviceTime @unit(s);
        @display("i=block/activeq;q=queue");
        @signal[msgDroppedFromSource](type="long");
        @statistic[msgDroppedFromSource](title="messages dropped from source"; record=count);
        @signal[msgDroppedFromSource1](type="long");
        @statistic[msgDroppedFromSource1](title="messages dropped from source1"; record= count);
        double checkInterval @unit(s) = default(0.5s);
        
        int bufferSize = default(10);
        int resourceCapacity = default(20);
    gates:
        input in[2];
        output out;
}
