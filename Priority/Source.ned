simple Source
{
    parameters:
        volatile double interarrivalTime @unit(s);
        volatile double serviceTime @unit(s);
        @display("i=block/source");
        @signal[msgGenerated](type="long");
        @statistic[msgGenerated](title="messages generated"; record= count);
        int requiredResource = default(10);
        
    gates:
        output out;
}
