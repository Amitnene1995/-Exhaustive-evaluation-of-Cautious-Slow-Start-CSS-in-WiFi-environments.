# Exhaustive evaluation of Cautious Slow Start in WiFi environments.

## Course Code: CS821

## Assignment No 12

### Overview about Cautious Slow Start

In normal Slow Start algorithm, the congestion window (cwnd) is increases by 1 on receipt of acknowledgement of every successful packet transfer.
This means that twice the number of packets can be sent in the next RTT. But, by doing this, there can be bulk consecutive packet losses. 
Thus, to avoid such losses, Cautious Slow Start algorithm has been introduced in which the Congestion window is increased cautiously.

There are total 4 scenarios in the Cautious Slow Start Algorithm.

#### Scenario 1 : When Average RTT < Current RTT.

In this case, we can safely conclude that there is no congestion in the network.

Thus, congestion window size is incremented by 1.

Average RTT is calculated as 

avgRTT = ((avgRTT∗(n−1))+curRTT)/n

where n is the number of packets whose acknowledgements have been received.

#### Scenario 2 : When Current RTT >=Average RTT and k >= cwnd/2

k is defined as Average RTT / (Current RTT - Average RTT)

Thus, higher the value of k, lower will be the difference between Current RTT and Average RTT.

Thus, Current RTT is only marginally higher than Average RTT.

Thus, Congestion window is incremented by 0.5.

#### Scenario 3 : When Current RTT >=Average RTT and 1<k<cwnd/2

This indicates that the congestion present in the network is moderate and Congestion window is Incremented by a fraction of k/cwnd.

#### Scenario 4 : When Current RTT >=Average RTT and 0<k<1

Low value of k indicates that there is huge difference between Current RTT and Average RTT.

Thus , Congestion window size remains the same.
