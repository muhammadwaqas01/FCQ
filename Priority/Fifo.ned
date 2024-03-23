simple Fifo
{
    parameters:
       // volatile double serviceTime @unit(s);
        @display("i=block/activeq;q=queue");
        @signal[msgDroppedFromSource](type="long");
        @statistic[msgDroppedFromSource](title="messages dropped from source"; record= count);
        @signal[msgDroppedFromSource1](type="long");
        @statistic[msgDroppedFromSource1](title="messages dropped from source1"; record= count);
        
        int bufferSize = default(10);
        int resourceCapacity = default(20);
        
        @signal[avgWaitTimeFromSource](type="double");
  //      @statistic[avgWaitTimeFromSource](title="Average Wait Time From Source"; record=vector, timeavg);
        @signal[avgWaitTimeFromSource1](type="double");
//        @statistic[avgWaitTimeFromSource1](title="Average Wait Time From Source1"; record=vector, timeavg);
    	double checkInterval @unit(s) = default(0.5s);  // Define default value as 5s or whatever is suitable


        
    gates:
        input in[2];
        output out;
}
