
// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//
//used above topology and converted it in dumbbell.
//
//
//
//


#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-classifier.h"




                                     //LAN 10.1.2.0

using namespace ns3;
Ptr<PacketSink> sink1,sink2;
uint64_t lastTotalRx = 0; 
NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         
  double cur = (sink2->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink2->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}


/*static void RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
  std::cout<<"RxDrop at " << Simulator::Now ().GetSeconds ();
  file->Write (Simulator::Now (), p);
}*/


int 
main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; 
  bool verbose = true;
  //uint32_t nCsma = 0;
  uint32_t nWifi = 2;
  bool tracing = true;
  std::string dataRate = "100Mbps";                  /* Application layer datarate. */
  std::string tcpVariant = "TcpVeno";             /* TCP variant type. */                    /* Physical layer bitrate. */
  std::string wifiManager ("Arf");
 double simulationTime = 100;                        /* Simulation time in seconds. */                        /* PCAP Tracing is enabled or not. */
  std::string linkbw = "500";

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("linkbw", "Bottleneck Link Bandwidth", linkbw);

  cmd.Parse (argc,argv);
  linkbw=linkbw+"Kbps";

  tcpVariant = std::string ("ns3::") + tcpVariant;

  /* No fragmentation and no RTS/CTS */
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("999999"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));

  // Select TCP variant
  if (tcpVariant.compare ("ns3::TcpWestwoodPlus") == 0)
    { 
      // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
    }
  else
    {
      TypeId tcpTid;
      NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
    }

  /* Configure TCP Options */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));


  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

//Device Creation:
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  NodeContainer p1;
  p1.Create(1);
  p1.Add(p2pNodes.Get(1));
  PointToPointHelper p1helper;
  p1helper.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  p1helper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p1Devices;
  p1Devices = p1helper.Install (p1);

  NodeContainer p2;
  p2.Create(1);
  p2.Add(p2pNodes.Get(1));
  PointToPointHelper p2helper;
  p2helper.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  p2helper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2Devices;
  p2Devices = p2helper.Install (p2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue (linkbw));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

 /*Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.001));
  p2pDevices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  p2pDevices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); */

//wifi:
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  WifiMacHelper wifiMac ;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211b);
  
  

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  //channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //channel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (590000));
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  
   

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  
 

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);


Ptr<WifiNetDevice> myWifiNetDevice = DynamicCast<WifiNetDevice> (staDevices.Get (0)); //
//Converts a Ptr<NetDevice> to a Ptr<WifiNetDevice>
 Ptr<WifiPhy> myWifiPhy1 = myWifiNetDevice->GetPhy (); //works. dev->GetPhy () doesn't

 Ptr<YansWifiPhy> myWifiPhy = DynamicCast<YansWifiPhy> (myWifiPhy1); // phy
  myWifiPhy->SetAttribute ("RxNoiseFigure", DoubleValue(11));
  staDevices.Get (0)->SetAttribute ("Phy", PointerValue(myWifiPhy));
  staDevices.Get (1)->SetAttribute ("Phy", PointerValue(myWifiPhy));

//mobility:
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (100, -50,   20));
  positionAlloc->Add (Vector (100, 50,   20));
  positionAlloc->Add (Vector (50,  0,   20));
  positionAlloc->Add (Vector (-50,  50,   20));
  positionAlloc->Add (Vector (0,  0,   20));
  positionAlloc->Add (Vector (-50,  -50,   20));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);
  mobility.Install (wifiApNode);
  mobility.Install (p1);
  mobility.Install (p2.Get(0));

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);
  stack.Install (p1);
  stack.Install (p2.Get(0));

//IP Addressing:
  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  Ipv4InterfaceContainer wwe;
  wwe=address.Assign (staDevices);
  address.Assign (apDevices);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p1Interfaces;
  p1Interfaces = address.Assign (p1Devices);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer p2Interfaces;
  p2Interfaces = address.Assign (p2Devices);

  uint16_t port = 9; 
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


//Transmission:
//t1:
    // Discard port (RFC 863)
  //uint16_t port1 = 9;
  OnOffHelper onoff ("ns3::TcpSocketFactory", 
                     Address (InetSocketAddress (wwe.GetAddress (1), port)));                //d 4
  onoff.SetAttribute ("PacketSize", UintegerValue (1472));
  onoff.SetConstantRate (DataRate ("500Kb/s"));
  ApplicationContainer apps = onoff.Install (p2.Get(0));                       //s 0
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sinker ("ns3::TcpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  ApplicationContainer apper= sinker.Install (wifiStaNodes.Get (1)); 
  sink1 = StaticCast<PacketSink>(apper.Get (0));                                                                    //4
  apper.Start (Seconds (1.0));
  apps.Start(Seconds(0.0));
  apps.Stop (Seconds (15.0));

//t2:
  OnOffHelper onoff1 ("ns3::TcpSocketFactory", 
                     Address (InetSocketAddress (wwe.GetAddress (0), port)));                //d 4
  onoff1.SetAttribute ("PacketSize", UintegerValue (1472));
  onoff1.SetConstantRate (DataRate ("500Kb/s"));
  ApplicationContainer apps1 = onoff1.Install (p1.Get(0));                       //s 0
  apps1.Start (Seconds (4.0));
  apps1.Stop (Seconds (12.0));

  // Create a packet sink to receive these packets
 PacketSinkHelper sinker1 ("ns3::TcpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  ApplicationContainer apper1= sinker1.Install (wifiStaNodes.Get (0)); 
  sink2 = StaticCast<PacketSink>(apper1.Get (0));                                                                    //4
  apper1.Start (Seconds (1.0));
  apps1.Start(Seconds(4.0));
  apps1.Stop (Seconds (12.0));




  Simulator::Stop (Seconds (10.0));
  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", p2pDevices.Get (1));
    //  csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }

   FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll(); 
   
/* Start Simulation */
 Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
  for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i!= stats.end();i++)
  {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
          std::cout<<"Flow"<<i->first <<"("<<t.sourceAddress<<"->"<<t.destinationAddress <<")\n";
          std::cout<<" Tx Bytes: "<<i->second.txBytes<<"\n";
          std::cout<<" Rx Bytes: "<<i->second.rxBytes<<"\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  lostPackets: " << i->second.lostPackets << "\n";
  }



 /*    PcapHelper pcapHelper;
  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile ("TS5_5.pcap", std::ios::out, PcapHelper::DLT_PPP);
  p2pDevices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&RxDrop, file));*/


   double averageThroughput = ((sink2->GetTotalRx () * 8) / (1e6  * simulationTime));
   std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;
   //Simulator::Run ();
   Simulator::Destroy ();
  return 0;
}
