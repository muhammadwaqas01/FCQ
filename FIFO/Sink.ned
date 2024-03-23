simple Sink
{
    parameters:
        @display("i=block/sink");
        // New signals for tracking messages processed from each source
        @signal[msgProcessedFromSource](type="long");
        @statistic[msgProcessedFromSource](title="messages processed from source"; record= count);
        
        @signal[msgProcessedFromSource1](type="long");
        @statistic[msgProcessedFromSource1](title="messages processed from source1"; record=count);
        
    gates:
        input in;
}
