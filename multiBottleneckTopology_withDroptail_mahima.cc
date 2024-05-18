/**
 * Author : Mahima Gupta
 * Last Modified : 21/Feb/2024
 * Title : Multi Bottleneck Topology with DropTail Queues
*/
 
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cerrno> // for errno
#include <cstring> // for strerror
#include <sys/stat.h> // for mkdir

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/config-store-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SocketBoundTcpRoutingExample");

static const uint32_t totalTxBytes = 10000000;
static uint32_t currentTxBytes = 0;
static const uint32_t writeSize = 1446;
uint8_t data[writeSize];
// std::string dir;


void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t);
void StartFlow2 (Ptr<Socket>, Ipv4Address, uint16_t);
void WriteUntilBufferFull (Ptr<Socket>, uint32_t);

void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port);
void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev);

int queue_number = 120;

uint64_t queueSize;
uint64_t queueSize2;
uint64_t queueSize3;
uint64_t queueSize4;
uint64_t queueSize5;
uint64_t queueSize6;

Ptr<OutputStreamWrapper> qSize_stream;

uint64_t droppedPackets;
uint64_t droppedPackets2;
uint64_t droppedPackets3;
uint64_t droppedPackets4;
uint64_t droppedPackets5;
uint64_t droppedPackets6;
uint64_t previously_droppedPackets;
uint64_t previously_droppedPackets2;
uint64_t previously_droppedPackets3;
uint64_t previously_droppedPackets4;
uint64_t previously_droppedPackets5;
uint64_t previously_droppedPackets6;
//float transmittedPackets;
Ptr<OutputStreamWrapper> dropped_stream;

uint32_t pkt_count1 = 0;
uint32_t pkt_count2 = 0;
uint32_t pkt_count3 = 0;
uint32_t pkt_count4 = 0;
uint32_t pkt_count5 = 0;
uint32_t pkt_count6 = 0;
uint32_t pkt_count7 = 0;
uint32_t pkt_count8 = 0;

uint32_t prev_pkt_count1 = 0;
uint32_t prev_pkt_count2 = 0;
uint32_t prev_pkt_count3 = 0;
uint32_t prev_pkt_count4 = 0;
uint32_t prev_pkt_count5 = 0;
uint32_t prev_pkt_count6 = 0;
uint32_t prev_pkt_count7 = 0;
uint32_t prev_pkt_count8 = 0;

Time prevTime1 = Seconds (0);
Time prevTime2 = Seconds (0);

Time prevTime3 = Seconds (0);
Time prevTime4 = Seconds (0);
Time prevTime5= Seconds (0);
Time prevTime6= Seconds (0);
Time prevTime7 = Seconds (0);
Time prevTime8= Seconds (0);

Ptr<OutputStreamWrapper> bottleneckTransmittedStream1;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream2;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream3;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream4;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream5;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream6;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream7;
Ptr<OutputStreamWrapper> bottleneckTransmittedStream8;


uint64_t packetsTransmitted;
uint64_t previous_transmitted_packets = 0;
uint64_t packetsTransmitted2;
uint64_t previous_transmitted_packets2 = 0;
uint64_t packetsTransmitted3;
uint64_t previous_transmitted_packets3 = 0;
uint64_t packetsTransmitted4;
uint64_t previous_transmitted_packets4 = 0;
uint64_t packetsTransmitted5;
uint64_t previous_transmitted_packets5 = 0;
uint64_t packetsTransmitted6;
uint64_t previous_transmitted_packets6 = 0;
Time prevTime02 = Seconds (0);
Ptr<OutputStreamWrapper> utilization;

/**
 * --------------------------------------------------------------------- 
 *                  Functions for tracing Queue Size
 * --------------------------------------------------------------------- 
*/

static void plotQsizeChange (uint32_t oldQSize, uint32_t newQSize){
    queueSize = newQSize;
}
static void plotQsizeChange2 (uint32_t oldQSize, uint32_t newQSize){
    queueSize2 = newQSize;
}
static void plotQsizeChange3 (uint32_t oldQSize, uint32_t newQSize){
    queueSize3 = newQSize;
}
static void plotQsizeChange4 (uint32_t oldQSize, uint32_t newQSize){
    queueSize4 = newQSize;
}
static void plotQsizeChange5 (uint32_t oldQSize, uint32_t newQSize){
    queueSize5 = newQSize;
}
static void plotQsizeChange6 (uint32_t oldQSize, uint32_t newQSize){
    queueSize6 = newQSize;
}

static void TraceQueueSize(){
    *qSize_stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << queueSize << "\t" << queueSize2 << "\t" << queueSize3 <<
    "\t" << queueSize4 << "\t" << queueSize5 << "\t" << queueSize6 << std::endl;
}

static void StartTracingQueueSize(){
    std::string query = "/NodeList/" + std::to_string(queue_number)  + "/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/PacketsInQueue";
    Config::ConnectWithoutContext(query, MakeCallback(&plotQsizeChange));
    query = "/NodeList/" + std::to_string(queue_number+2)  + "/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/PacketsInQueue";
    Config::ConnectWithoutContext(query, MakeCallback(&plotQsizeChange2));
    query = "/NodeList/" + std::to_string(queue_number+4)  + "/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/PacketsInQueue";
    Config::ConnectWithoutContext(query, MakeCallback(&plotQsizeChange3));
    query = "/NodeList/" + std::to_string(queue_number+6)  + "/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/PacketsInQueue";
    Config::ConnectWithoutContext(query, MakeCallback(&plotQsizeChange4));
    query = "/NodeList/" + std::to_string(queue_number+8)  + "/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/PacketsInQueue";
    Config::ConnectWithoutContext(query, MakeCallback(&plotQsizeChange5));
    query = "/NodeList/" + std::to_string(queue_number+10)  + "/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/PacketsInQueue";
    Config::ConnectWithoutContext(query, MakeCallback(&plotQsizeChange6));
}

/**
 * --------------------------------------------------------------------- 
 *                  Functions for tracing Packet Loss 
 * --------------------------------------------------------------------- 
*/

static void RxDrop(Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p){
   droppedPackets++;
}
static void RxDrop2(Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p){
   droppedPackets2++;
} 
static void RxDrop3(Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p){
   droppedPackets3++;
} 
static void RxDrop4(Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p){
   droppedPackets4++;
} 
static void RxDrop5(Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p){
   droppedPackets5++;
} 
static void RxDrop6(Ptr<OutputStreamWrapper> stream,  Ptr<const Packet> p){
   droppedPackets6++;
} 

static void TraceDroppedPacket(std::string droppedTrFileName){
    std::string query = "/NodeList/" + std::to_string(queue_number) + "/DeviceList/*/TxQueue/Drop";
    Config::ConnectWithoutContext(query, MakeBoundCallback(&RxDrop, dropped_stream));
    query = "/NodeList/" + std::to_string(queue_number+2) + "/DeviceList/*/TxQueue/Drop";
    Config::ConnectWithoutContext(query, MakeBoundCallback(&RxDrop2, dropped_stream));
    query = "/NodeList/" + std::to_string(queue_number+4) + "/DeviceList/*/TxQueue/Drop";
    Config::ConnectWithoutContext(query, MakeBoundCallback(&RxDrop3, dropped_stream));
    query = "/NodeList/" + std::to_string(queue_number+6) + "/DeviceList/*/TxQueue/Drop";
    Config::ConnectWithoutContext(query, MakeBoundCallback(&RxDrop4, dropped_stream));
    query = "/NodeList/" + std::to_string(queue_number+8) + "/DeviceList/*/TxQueue/Drop";
    Config::ConnectWithoutContext(query, MakeBoundCallback(&RxDrop5, dropped_stream));
    query = "/NodeList/" + std::to_string(queue_number+10) + "/DeviceList/*/TxQueue/Drop";
    Config::ConnectWithoutContext(query, MakeBoundCallback(&RxDrop6, dropped_stream));
}

static void TraceDroppedPkts(){
    float packetLoss = droppedPackets - previously_droppedPackets;
    float packetLoss2 = droppedPackets2 - previously_droppedPackets2;
    float packetLoss3 = droppedPackets3 - previously_droppedPackets3;
    float packetLoss4 = droppedPackets4 - previously_droppedPackets4;
    float packetLoss5 = droppedPackets5 - previously_droppedPackets5;
    float packetLoss6 = droppedPackets6 - previously_droppedPackets6;

    *dropped_stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << packetLoss << "\t" << packetLoss2 << "\t" << packetLoss3 << "\t" << packetLoss4 << "\t" << packetLoss5 << "\t" << packetLoss6 <<std::endl;
    previously_droppedPackets = droppedPackets;
    previously_droppedPackets2 = droppedPackets2;
    previously_droppedPackets3 = droppedPackets3;
    previously_droppedPackets4 = droppedPackets4;
    previously_droppedPackets5 = droppedPackets5;
    previously_droppedPackets6 = droppedPackets6;
}

/**
 * --------------------------------------------------------------------- 
 *                  Functions for tracing Network Throughput
 * --------------------------------------------------------------------- 
*/

// For saving packets received at the sink at each time instant into the defined ascii stream

//Route1 R1->R2
static void
TraceBottleneckTx1(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr1 = ((pkt_count1-prev_pkt_count1)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime1.GetSeconds()));
    *bottleneckTransmittedStream1->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr1 << std::endl;
    prev_pkt_count1 = pkt_count1;
    prevTime1 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount1(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count1++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink1(){
    for (int i = 1; i <= 9; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount1));
        // Initialize prevTime for each sink if needed
    }
}


//Route2 R1->R2->R4-> R6

static void
TraceBottleneckTx2(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr2= ((pkt_count2-prev_pkt_count2)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime2.GetSeconds()));\
    //std::cout<<btl_thr2;
    *bottleneckTransmittedStream2->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr2 << std::endl;
    prev_pkt_count2 = pkt_count2;
    prevTime2 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount2(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count2++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink2(){
    for (int i = 11; i <= 19; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount2));
        // Initialize prevTime for each sink if needed
    }
}


//Route3  R1->R2->R6

static void
TraceBottleneckTx3(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr3 = ((pkt_count3-prev_pkt_count3)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime3.GetSeconds()));
    *bottleneckTransmittedStream3->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr3 << std::endl;
    prev_pkt_count3 = pkt_count3;
    prevTime3 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount3(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count3++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink3(){
    for (int i = 21; i <= 47; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount3));
        // Initialize prevTime for each sink if needed
    }
}

//Route4 R1->R3->R4->R6

static void
TraceBottleneckTx4(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr4 = ((pkt_count4-prev_pkt_count4)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime4.GetSeconds()));
    *bottleneckTransmittedStream4->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr4 << std::endl;
    prev_pkt_count4 = pkt_count4;
    prevTime4 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount4(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count4++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink4(){
    for (int i = 49; i <= 57; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount4));
        // Initialize prevTime for each sink if needed
    }
}


//Route5 R1->R3->R4->R5

static void
TraceBottleneckTx5(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr5 = ((pkt_count5-prev_pkt_count5)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime5.GetSeconds()));
    *bottleneckTransmittedStream5->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr5 << std::endl;
    prev_pkt_count5 = pkt_count5;
    prevTime5 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount5(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count5++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink5(){
    for (int i = 59; i <= 63; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount5));
        // Initialize prevTime for each sink if needed
    }
}


//Route6 R1-> R3-> R4-> R5-> R6

static void
TraceBottleneckTx6(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr6 = ((pkt_count6-prev_pkt_count6)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime6.GetSeconds()));
    *bottleneckTransmittedStream6->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr6 << std::endl;
    prev_pkt_count6 = pkt_count6;
    prevTime6 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount6(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count6++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink6(){
    for (int i = 65; i <= 105; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount6));
        // Initialize prevTime for each sink if needed
    }
}


//Route7  R1-> R3-> R5->R6

static void
TraceBottleneckTx7(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr7 = ((pkt_count7-prev_pkt_count7)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime7.GetSeconds()));
    *bottleneckTransmittedStream7->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr7 << std::endl;
    prev_pkt_count7 = pkt_count7;
    prevTime7 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount7(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count7++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink7(){
    for (int i = 107; i <= 113; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount7));
        // Initialize prevTime for each sink if needed
    }
}


//Route8 R1->R3
static void
TraceBottleneckTx8(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr8 = ((pkt_count8-prev_pkt_count8)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime8.GetSeconds()));
    *bottleneckTransmittedStream8->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr8 << std::endl;
    prev_pkt_count8 = pkt_count8;
    prevTime8 = currrent_time;
}


//For counting the number of packets received at the sink
static void SinkRxCount8(Ptr<const Packet> p, const Address &ad )
{  
  //std::cout << "One packet received" << std::endl;
  pkt_count8++;
  //std::cout << pkt_count << std::endl; const Address &ad
}

static void 
StartTracingSink8(){
    for (int i = 115; i <= 119; i += 2) {
        std::string query = "/NodeList/" + std::to_string(i) + "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::ConnectWithoutContext(query, MakeCallback(&SinkRxCount8));
        // Initialize prevTime for each sink if needed
    }
}



// Call SinkRxCount function everytime a packet is received at the application layer of the sink node 
// static void 
// StartTracingSink(){
//     Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&SinkRxCount));
// }
///

/* Congestion window Tracing*/
// static void CwndTracer(Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval) {
//     std::ostream *out_stream = stream->GetStream();
//     *out_stream << Simulator::Now().GetSeconds() << "\t" << oldval << "\t" << newval << std::endl;
//     out_stream->flush(); 
// }

// // Function to trace congestion window for a specific node and socket
// void TraceCwnd(uint32_t nodeId, uint32_t socketId) {
//     AsciiTraceHelper ascii;
//     std::string dir = "congestion Window Droptail"; // Directory to store trace files
//     std::string fileName = dir + "/cwnd_" + std::to_string(nodeId) + ".txt";
//     Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream(fileName, std::ios_base::app);
//     Config::ConnectWithoutContext("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string(socketId) + "/CongestionWindow", MakeBoundCallback(&CwndTracer, stream));
// }


static void CwndTracer(Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval) {
    std::ostream *out_stream = stream->GetStream();
    *out_stream << Simulator::Now().GetSeconds() << "\t" << oldval << "\t" << newval << std::endl;
    out_stream->flush(); 
}

// Function to trace congestion window for a specific node and socket
void TraceCwnd(uint32_t nodeId, uint32_t socketId, std::string dir) {
    AsciiTraceHelper ascii;
    std::string fileName = dir + "/cwnd_" + std::to_string(nodeId) + ".txt";
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream(fileName, std::ios_base::app);
    Config::ConnectWithoutContext("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string(socketId) + "/CongestionWindow", MakeBoundCallback(&CwndTracer, stream));
}









/**
 * --------------------------------------------------------------------- 
 *                  Functions for tracing Link Utilization
 * --------------------------------------------------------------------- 
*/

// static void TxxPacket(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint interface){
//     packetsTransmitted++;
// }
static void TxxPacket( Ptr<const Packet> p){
    packetsTransmitted++;
}
static void Txx2Packet( Ptr<const Packet> p){
    packetsTransmitted2++;
}
static void Txx3Packet( Ptr<const Packet> p){
    packetsTransmitted3++;
}
static void Txx4Packet( Ptr<const Packet> p){
    packetsTransmitted4++;
}
static void Txx5Packet( Ptr<const Packet> p){
    packetsTransmitted5++;
}
static void Txx6Packet( Ptr<const Packet> p){
    packetsTransmitted6++;
}

static void TraceUtilization(uint32_t pktSize){
    Time currrent_time = Now();
    float btl_thr_u1 = ((packetsTransmitted-previous_transmitted_packets)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime02.GetSeconds()));
    float btl_thr_u2= ((packetsTransmitted2-previous_transmitted_packets2)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime02.GetSeconds()));
    float btl_thr_u3 = ((packetsTransmitted3-previous_transmitted_packets3)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime02.GetSeconds()));
    float btl_thr_u4 = ((packetsTransmitted4-previous_transmitted_packets4)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime02.GetSeconds()));
    float btl_thr_u5 = ((packetsTransmitted5-previous_transmitted_packets5)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime02.GetSeconds()));
    float btl_thr_u6 = ((packetsTransmitted6-previous_transmitted_packets6)*8*pktSize)/(1000 * 1000*(currrent_time.GetSeconds() - prevTime02.GetSeconds()));
    *utilization->GetStream() << Simulator::Now().GetSeconds() << "\t" << btl_thr_u1/100 << "\t" << btl_thr_u2/100 << "\t" << btl_thr_u3/100 << "\t" << btl_thr_u4/100 <<  "\t" << btl_thr_u5/100 << "\t" << btl_thr_u6/100 << std::endl;
    previous_transmitted_packets = packetsTransmitted ;
    previous_transmitted_packets2 = packetsTransmitted2 ;
    previous_transmitted_packets3 = packetsTransmitted3 ;
    previous_transmitted_packets4 = packetsTransmitted4 ;
    previous_transmitted_packets5 = packetsTransmitted5 ;
    previous_transmitted_packets6 = packetsTransmitted6 ;
    prevTime02 = currrent_time;
}

static void StartTracingUtilization(){
    packetsTransmitted = 0;
    previous_transmitted_packets = 0;
    std::string query = "/NodeList/" + std::to_string(queue_number) + "/DeviceList/61/PhyTxEnd";
    Config::ConnectWithoutContext(query, MakeCallback(&TxxPacket));
    query = "/NodeList/" + std::to_string(queue_number+2) + "/DeviceList/2/PhyTxEnd";
    Config::ConnectWithoutContext(query, MakeCallback(&Txx2Packet));
    query = "/NodeList/" + std::to_string(queue_number+4) + "/DeviceList/2/PhyTxEnd";
    Config::ConnectWithoutContext(query, MakeCallback(&Txx3Packet));
    query = "/NodeList/" + std::to_string(queue_number+6) + "/DeviceList/3/PhyTxEnd";
    Config::ConnectWithoutContext(query, MakeCallback(&Txx4Packet));
    query = "/NodeList/" + std::to_string(queue_number+8) + "/DeviceList/3/PhyTxEnd";
    Config::ConnectWithoutContext(query, MakeCallback(&Txx5Packet));
    query = "/NodeList/" + std::to_string(queue_number+10) + "/DeviceList/3/PhyTxEnd";
    Config::ConnectWithoutContext(query, MakeCallback(&Txx6Packet));
}
int 
main (int argc, char *argv[])
{


//  CommandLine cmd (__FILE__);
//     cmd.Parse (argc, argv);

//  std::string dir ="congestion_Window_Droptail";
//     if (mkdir(dir.c_str(), 0777) == -1) {
//         std::cerr << "Error creating directory: " << strerror(errno) << std::endl;
//         return 1;
//     }

      


    // DefaultValue::Bind ()s at run-time, via command-line arguments
   

    ConfigStore config;
    config.ConfigureDefaults ();
    config.ConfigureAttributes ();
    // --------------------------------------------------
    //        Variable Declaration & Configurations
    // --------------------------------------------------
    std::string flavour = "TcpNewReno";		//TCP variant considered
    std::string tcpModel ("ns3::"+flavour);
    int number_of_sources = 60;
    int simDuration = 250; // In Seconds
    		//round-trip time of each TCP flow
         std::string RTT; 

     
    int number_of_nodes = 12 + (number_of_sources * 2);
    int number_of_links = 15 + (number_of_sources * 2);
    int router_starting_index = number_of_sources * 2;
    int pktSize = 1446;
    float queue_size = 2084;


     CommandLine cmd;
 
    cmd.AddValue ("RTT", "mean RTT for random generation of delays in each link", RTT);
    //cmd.AddValue ("accessDelay", "Access link delay", accessDelay);
   
    cmd.Parse (argc, argv);

    std::string dir = RTT + "_congestion_window";
    if (mkdir(dir.c_str(), 0777) == -1) {
        std::cerr << "Error creating directory: " << strerror(errno) << std::endl;
        return 1;
    }





    //cmd.AddValue ("accessDelay", "Access link delay", accessDelay);
   





    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue(tcpModel));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (pktSize));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue (1));
    Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (20*1000));

    // --------------------------------------------------
    //            Node Initilisation
    // --------------------------------------------------
    NodeContainer nodes;
    nodes.Create(number_of_nodes);


    InternetStackHelper internet;
    internet.Install (nodes);

    // --------------------------------------------------
    //            Point-to-point links
    // --------------------------------------------------
    NodeContainer containers[number_of_links];
    int a = 0;
    for(int i = 0; i < number_of_sources; i++)   
      containers[a++] = NodeContainer (nodes.Get(i*2), nodes.Get(router_starting_index));
    
    containers[a++] = NodeContainer (nodes.Get(router_starting_index), nodes.Get(router_starting_index + 1)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 1), nodes.Get(router_starting_index + 2)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 1), nodes.Get(router_starting_index + 4)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 2), nodes.Get(router_starting_index + 3)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 4), nodes.Get(router_starting_index + 5)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 3), nodes.Get(router_starting_index + 10)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 3), nodes.Get(router_starting_index + 6)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 5), nodes.Get(router_starting_index + 6)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 5), nodes.Get(router_starting_index + 8));
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 6), nodes.Get(router_starting_index + 7)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 7), nodes.Get(router_starting_index + 10)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 10), nodes.Get(router_starting_index + 11));
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 7), nodes.Get(router_starting_index + 8)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 8), nodes.Get(router_starting_index + 9)); 
    containers[a++] = NodeContainer (nodes.Get(router_starting_index + 9), nodes.Get(router_starting_index + 10)); 


    for(int i = 0; i < 5 ; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 3), nodes.Get(i + (i+1))); // Route 1
    
    for(int i=5; i < 10; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 11), nodes.Get(i + (i+1))); // Route 2
    
    for(int i=10; i < 24; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 11), nodes.Get(i + (i+1))); // Route 3
    
    for(int i=24; i < 29; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 11), nodes.Get(i + (i+1))); // Route 4

    for(int i=29; i < 32; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 9), nodes.Get(i + (i+1))); // Route 5

    for(int i=32; i < 53; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 11), nodes.Get(i + (i+1))); // Route 6

    for(int i=53; i < 57; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 11), nodes.Get(i + (i+1))); // Route 7

    for(int i=57; i < 60; i++)
      containers[a++] = NodeContainer (nodes.Get(router_starting_index + 5), nodes.Get(i + (i+1))); // Route 8

    // We create the channels first without any IP addressing information

    // Defining the links to be used between nodes
    double range = (double) (stoi(RTT.substr(0, RTT.length()-2)));
    double min = range - (0.1 * range);
    double max = range + (0.1 * range);
    
    Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
    x->SetAttribute ("Min", DoubleValue (min));
    x->SetAttribute ("Max", DoubleValue (max));
    
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
    p2p.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(0/number_of_sources)+"p"))); // p in 1000p stands for packets
    p2p.DisableFlowControl();

    PointToPointHelper bottleneck_p2p;
    bottleneck_p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    bottleneck_p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
    bottleneck_p2p.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize ("2084p"))); // p in 1000p stands for packets
    bottleneck_p2p.DisableFlowControl();
    
    NetDeviceContainer devices[number_of_links];
    for(int i = 0 ; i< number_of_links; i++){
      double delay = (x->GetValue())/4;
      std::string delay_str = std::to_string(delay) + "ms";
      p2p.SetChannelAttribute ("Delay", StringValue(delay_str));
      switch(i){
        case 63 : 
                  bottleneck_p2p.SetDeviceAttribute("DataRate", StringValue ("40Mbps"));
                  queue_size = 2084 * (0.4);
                  bottleneck_p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(queue_size) + "p")));
                  break;
        case 64:
                  bottleneck_p2p.SetDeviceAttribute("DataRate", StringValue ("40Mbps"));
                  queue_size = 2084 * (0.4);
                  bottleneck_p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(queue_size) + "p")));
                  break; 
        case 69 :
                  bottleneck_p2p.SetDeviceAttribute("DataRate", StringValue ("60Mbps"));
                  queue_size = 2084 * (0.6);
                  bottleneck_p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(queue_size) + "p")));
                  break; 
        case 71 :
                  bottleneck_p2p.SetDeviceAttribute("DataRate", StringValue ("60Mbps"));
                  queue_size = 2084 * (0.6);
                  bottleneck_p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(queue_size) + "p")));
                  break;
        case 73 :
                  bottleneck_p2p.SetDeviceAttribute("DataRate", StringValue ("60Mbps"));
                  queue_size = 2084 * (0.6);
                  bottleneck_p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(queue_size) + "p")));
                  break;
        default :
                  bottleneck_p2p.SetDeviceAttribute("DataRate", StringValue ("100Mbps"));
                  queue_size = 2084 ;
                  bottleneck_p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (std::to_string(queue_size) + "p")));
                  break;

      }
      if(i < number_of_sources)
        devices[i] = p2p.Install (containers[i]);
      else if (i >= (number_of_sources + 15))
        devices[i] = p2p.Install (containers[i]);
      else
        devices[i] = bottleneck_p2p.Install (containers[i]);
    }


    // --------------------------------------------------
    //              IP addresses.
    // --------------------------------------------------
    Ipv4AddressHelper ipv4;
    Ipv4InterfaceContainer ip_addresses[number_of_links];
    std::string k = "1.";
    int j = 1;

    for(int i = 0; i< number_of_links; i++){
      std::string ip_address = "10." + k + std::to_string(j) + ".0";
      ipv4.SetBase (ip_address.c_str(), "255.255.255.0");
      ip_addresses[i] = ipv4.Assign (devices[i]);
      if (i < number_of_sources - 1)
            j++;
      switch (i){
        case 59 : j = 1; k = "2."; break;
        case 60 : j = 1; k = "3."; break;
        case 61 : j = 2; k = "3."; break;
        case 62 : j = 1; k = "5."; break;
        case 63 : j = 2; k = "5."; break;
        case 64 : j = 1; k = "10."; break;
        case 65 : j = 2; k = "10."; break;
        case 66 : j = 3; k = "10.";break;
        case 67 : j = 4; k = "10."; break;
        case 68 : j = 1; k = "15."; break;
        case 69 : j = 1; k = "20."; break;
        case 70 : j = 1; k = "25."; break;
        case 71 : j = 2; k = "20."; break;
        case 72 : j = 2; k = "25."; break;
        case 73 : j = 3; k = "25."; break;
        case 74 : j = 1; k = "30."; break;
        default : break;
      }
      if(i > (number_of_sources + 15 - 1))
        j++;
    }
  
    // --------------------------------------------------
    //           Source based Static Routing
    // --------------------------------------------------
    Ptr<Ipv4> ipv4_pointers[number_of_nodes];
    for(int i = 0; i < number_of_nodes ; i++){
      ipv4_pointers[i] = nodes.Get(i)->GetObject<Ipv4> ();
    }

    int interface = number_of_sources + 1;
    std::string source_ip, destination_ip;
    // 0 - Source, 1 - Destination 
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> staticRouting[number_of_nodes];
    for(int i = 0; i < number_of_nodes ; i++){
      staticRouting[i] = ipv4RoutingHelper.GetStaticRouting (ipv4_pointers[i]);
    }

    // Route No 01
    // ----------------------------------------------------------------------------------------------------
    for(int i = 0 ; i < 5; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.1.2"), 2);
      staticRouting[router_starting_index + 2]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.1.2"), 2);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 3]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.1.1"), 1);
      staticRouting[router_starting_index + 2]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.1.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }
    
    // Route No 02
    // ----------------------------------------------------------------------------------------------------
    for(int i=5; i < 10; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.1.2"), 2);
      staticRouting[router_starting_index + 2]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.1.2"), 2);
      staticRouting[router_starting_index + 3]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.2.2"), 3);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.2"), 3);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.1.2"), 2);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.2"), 3);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 11]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.1"), 1);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.1.1"), 2);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.1"), 1);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.2.1"), 1);
      staticRouting[router_starting_index + 3]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.1.1"), 1);
      staticRouting[router_starting_index + 2]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.1.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }

    // Route No 03
    // ----------------------------------------------------------------------------------------------------
    // src - > R1 -> R2 -> R6 -> des
    for(int i=10; i < 24; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.1.2"), 2);
      staticRouting[router_starting_index + 2]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.1.2"), 2);
      staticRouting[router_starting_index + 3]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.1.2"), 2);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.2"), 3);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 11]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.1"), 1);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.1.1"), 1);
      staticRouting[router_starting_index + 3]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.1.1"), 1);
      staticRouting[router_starting_index + 2]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.1.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }
    // Route No 04
    // ----------------------------------------------------------------------------------------------------
    // src - > R1 -> R3 -> R4 -> R6 -> des
    for(int i=24; i < 29; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.2"), 3);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.2"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.3.2"), 2);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.2"), 3);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.1.2"), 2);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.2"), 3);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 11]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.1"), 1);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.1.1"), 2);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.1"), 1);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.3.1"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.1"), 1);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }
    // Route No 05
    // ----------------------------------------------------------------------------------------------------
    for(int i=29; i < 32; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.2"), 3);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.2"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.3.2"), 2);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.2"), 3);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.2.2"), 3);
      staticRouting[router_starting_index + 8]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.2.2"), 3);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 9]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.2.1"), 1);
      staticRouting[router_starting_index + 8]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.2.1"), 2);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.1"), 1);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.3.1"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.1"), 1);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }
    // Route No 06
    // ----------------------------------------------------------------------------------------------------
    // src -> R1 -> R3 -> R4 -> R5 -> R6 -> des
    for(int i=32; i < 53; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.2"), 3);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.2"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.3.2"), 2);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.2"), 3);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.2.2"), 3);
      staticRouting[router_starting_index + 8]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.2.2"), 3);
      staticRouting[router_starting_index + 9]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.3.2"), 2);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.2"), 3);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 11]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.1"), 1);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.3.1"), 4);
      staticRouting[router_starting_index + 9]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.2.1"), 1);
      staticRouting[router_starting_index + 8]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.20.2.1"), 2);
      staticRouting[router_starting_index + 7]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.15.1.1"), 1);
      staticRouting[router_starting_index + 6]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.3.1"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.1"), 1);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }
    // Route No 07
    // ----------------------------------------------------------------------------------------------------
    // src -> R1 -> R3 -> R5-> R6 -> des
    for(int i=53; i < 57; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.2"), 3);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.2"), 2);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.4.2"), 3);
      staticRouting[router_starting_index + 8]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.2.2"), 3);
      staticRouting[router_starting_index + 9]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.3.2"), 2);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.2"), 3);

      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 11]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.1.1"), 1);
      staticRouting[router_starting_index + 10]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.3.1"), 4);
      staticRouting[router_starting_index + 9]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.25.2.1"), 1);
      staticRouting[router_starting_index + 8]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.10.4.1"), 1);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.1"), 1);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }
    // Route No 08
    // ----------------------------------------------------------------------------------------------------
    // src -> R1 -> R3 -> des
    for(int i=57; i < 60; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.2"), interface);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.2"), 3);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.2"), 2);


      source_ip = "10.30." + std::to_string(i+1) + ".1";
      destination_ip = "10.1." + std::to_string (i+1) + ".1";
      staticRouting[i+(i+1)]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1);
      staticRouting[router_starting_index + 5]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.5.2.1"), 1);
      staticRouting[router_starting_index + 4]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.3.2.1"), 1);
      staticRouting[router_starting_index + 1]->AddHostRouteTo (destination_ip.c_str(), Ipv4Address ("10.2.1.1"), 1);
      staticRouting[router_starting_index]->AddHostRouteTo (destination_ip.c_str(), destination_ip.c_str(), i+1);
    }

    // Two routes to same destination - setting separate metrics. 
    // You can switch these to see how traffic gets diverted via different routes
    // Adding Destinations to each source link
    // ----------------------------------------------------------------------------------------------------
    for(int i=0; i<number_of_sources; i++){
      destination_ip = "10.30." + std::to_string(i+1) + ".2";
      source_ip = "10.1." + std::to_string(i+1) + ".2";
      staticRouting[i*2]->AddHostRouteTo (destination_ip.c_str(), source_ip.c_str(), 1, 5);
    }
    
    
    // --------------------------------------------------
    //       Packet Sink and TCP Sockets Scheduling
    // --------------------------------------------------
    
    // There are no apps that can utilize the Socket Option so doing the work directly..
    // Taken from tcp-large-transfer example
    Ptr<Socket> src_socket[number_of_sources][1];
    for(int i = 0; i < number_of_sources; i++){
      for(int j = 0; j < 1; j++){
        src_socket[i][j] = Socket::CreateSocket (nodes.Get(i * 2), TypeId::LookupByName ("ns3::TcpSocketFactory"));
      }
    }
    
    uint16_t dstport = 12345;
    std::string dstaddr[number_of_sources];
    for (int  i = 0; i < number_of_sources ; i++)
      dstaddr[i] = "10.30." + std::to_string(i+1) + ".2";
    
    double mean = 0.1;   // more like a ~ 0.06
    double bound = 1;
    Ptr<ExponentialRandomVariable> expRandomVariable = CreateObject<ExponentialRandomVariable> ();
    expRandomVariable->SetAttribute ("Mean", DoubleValue (mean));
    expRandomVariable->SetAttribute ("Bound", DoubleValue (bound));

    PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dstport));
    ApplicationContainer apps[number_of_sources];
    for(int i = 0 ; i < number_of_sources; i++){
      apps[i] = sink.Install (nodes.Get(i+(i+1))); 
      apps[i].Start (Seconds (0.0));
      apps[i].Stop (Seconds (simDuration));
    }

    double stime = 0;
    for(int i = 0 ; i < number_of_sources ; i++){
        double gap = expRandomVariable->GetValue();
        Simulator::Schedule (Seconds (gap),&StartFlow, src_socket[i][0], Ipv4Address(dstaddr[i].c_str()), dstport);
    }
    
    for(int i = 1 ; i < simDuration ; i++){
        for(int j = 0 ; j < number_of_sources ; j++){
            double gap = expRandomVariable->GetValue();
            Simulator::Schedule (Seconds (i + gap),&StartFlow2, src_socket[j][0], Ipv4Address(dstaddr[j].c_str()), dstport);
            //stime += gap;
        }
    }

    Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (2), routingStream);

    /**
       * -----------------------------------------------------------------
       *                  Trace Files and Simulation Run
       * -----------------------------------------------------------------
      */

    std::string iterator = RTT;
      // Configuring file stream to write the Qsize
      AsciiTraceHelper ascii_qsize;
      qSize_stream = ascii_qsize.CreateFileStream(iterator + "DROP_QS.txt");

      AsciiTraceHelper ascii_dropped;
      dropped_stream = ascii_dropped.CreateFileStream (iterator + "DROP_Loss.txt");

    // Configuring file stream to write the no of packets transmitted by the bottleneck
      AsciiTraceHelper ascii_qsize_tx1;
            AsciiTraceHelper ascii_qsize_tx2;

      AsciiTraceHelper ascii_qsize_tx3;

      AsciiTraceHelper ascii_qsize_tx4;

      AsciiTraceHelper ascii_qsize_tx5;

      AsciiTraceHelper ascii_qsize_tx6;

      AsciiTraceHelper ascii_qsize_tx7;

      AsciiTraceHelper ascii_qsize_tx8;

      bottleneckTransmittedStream1 = ascii_qsize_tx1.CreateFileStream(iterator + "DROP_th1.txt");
            bottleneckTransmittedStream2 = ascii_qsize_tx2.CreateFileStream(iterator + "DROP_th2.txt");
      bottleneckTransmittedStream3 = ascii_qsize_tx3.CreateFileStream(iterator + "DROP_th3.txt");
      bottleneckTransmittedStream4 = ascii_qsize_tx4.CreateFileStream(iterator + "DROP_th4.txt");
       bottleneckTransmittedStream5 = ascii_qsize_tx5.CreateFileStream(iterator + "DROP_th5.txt");
            bottleneckTransmittedStream6 = ascii_qsize_tx6.CreateFileStream(iterator + "DROP_th6.txt");
      bottleneckTransmittedStream7 = ascii_qsize_tx7.CreateFileStream(iterator + "DROP_th7.txt");
      bottleneckTransmittedStream8 = ascii_qsize_tx8.CreateFileStream(iterator + "DROP_th8.txt");


      AsciiTraceHelper ascii_tx;
      utilization = ascii_tx.CreateFileStream(iterator + "U.txt");

      Simulator::Schedule( Seconds(stime), &StartTracingQueueSize);
      Simulator::Schedule( Seconds(stime), &StartTracingSink1);
            Simulator::Schedule( Seconds(stime), &StartTracingSink2);

      Simulator::Schedule( Seconds(stime), &StartTracingSink3);

      Simulator::Schedule( Seconds(stime), &StartTracingSink4);
       Simulator::Schedule( Seconds(stime), &StartTracingSink5);
            Simulator::Schedule( Seconds(stime), &StartTracingSink6);

      Simulator::Schedule( Seconds(stime), &StartTracingSink7);

      Simulator::Schedule( Seconds(stime), &StartTracingSink8);

      Simulator::Schedule( Seconds(stime), &StartTracingUtilization); 
      Simulator::Schedule( Seconds(stime), &TraceDroppedPacket, iterator + "Loss.txt");
      for (int time = stime; time < simDuration;){   
          Simulator::Schedule( Seconds(time), &TraceDroppedPkts);
          Simulator::Schedule( Seconds(time), &TraceUtilization, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx1, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx2, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx3, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx4, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx5, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx6, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx7, 1446);
          Simulator::Schedule( Seconds(time), &TraceBottleneckTx8, 1446);

          time= time+1;


          
      }
        for (float time = stime; time < simDuration;){

                Simulator::Schedule( Seconds(time), &TraceQueueSize);

time=time+.1;
        }
    //     for (uint32_t nodeId = 0; nodeId < 120; nodeId+=2 ){
    //     // Schedule trace for each socket of the node (assuming there is only one socket per node)
    //     for (uint32_t socketId = 0; socketId < 1; socketId++) {
    //         Simulator::Schedule(Seconds(stime), &TraceCwnd, nodeId, socketId);
    //     }
    // }
      for (uint32_t nodeId = 0; nodeId < 120; nodeId += 2) {
        for (uint32_t socketId = 0; socketId < 1; socketId++) {
            Simulator::Schedule(Seconds(stime), &TraceCwnd, nodeId, socketId, dir);
        }
    }


    Simulator::Run ();
    Simulator::Destroy ();
    Ptr<PacketSink> sink1 ;
    for(int i =0 ; i< number_of_sources ; i ++){
        sink1 = DynamicCast<PacketSink> (apps[i].Get (0));
        std::cout << "Total Bytes Received " << i+1 << " : " << sink1->GetTotalRx () << std::endl;
    }
    return 0;
}

void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev)
{
  sock->BindToNetDevice (netdev);
  return;
}

void StartFlow (Ptr<Socket> localSocket,
                Ipv4Address servAddress,
                uint16_t servPort)
{
  NS_LOG_INFO ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
  //NS_LOG_UNCOND("Reached here");
  currentTxBytes = 0;
  localSocket->Bind ();
  localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect
  
  // tell the tcp implementation to call WriteUntilBufferFull again
  // if we blocked and new tx buffer space becomes available
  localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
  WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void StartFlow2 (Ptr<Socket> localSocket,
                Ipv4Address servAddress,
                uint16_t servPort)
{
  NS_LOG_INFO ("Starting flow again at time " <<  Simulator::Now ().GetSeconds ());
  currentTxBytes = 0;
  localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
  WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
  while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0)
    {
      uint32_t left = totalTxBytes - currentTxBytes;
      uint32_t dataOffset = currentTxBytes % writeSize;
      uint32_t toWrite = writeSize - dataOffset;
      toWrite = std::min (toWrite, left);
      toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
      int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
      if(amountSent < 0)
        {
          // we will be called again when new tx space becomes available.
          return;
        }
      currentTxBytes += amountSent;
    }
    // localSocket->Close ();
}

 