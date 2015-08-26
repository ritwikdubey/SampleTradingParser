#include "parser.h"
#include <fstream>
#include <map>
using namespace std;

#include <climits>

void PopulateOrderEntry
(
  std::ifstream &inputFile, 
  const int msg_len, 
  std::map<std::string,int> &traderOrderMap, 
  std::map<std::string,int> &traderLiquidityMap, 
  uint64_t sequence_id, 
  std::map<std::string,vector<uint64_t>> &instrumentOrderIdMap)
{
  char dummy[8];
      OrderEntry order;
  inputFile.read(reinterpret_cast<char*>(&order.price),sizeof(order.price));
  inputFile.read(reinterpret_cast<char*>(&order.qty),sizeof(order.qty));
  inputFile.read(reinterpret_cast<char*>(order.instrument),10);
  inputFile.read(reinterpret_cast<char*>(&order.side),sizeof(order.side));
  inputFile.read(reinterpret_cast<char*>(&order.client_assigned_id),sizeof(order.client_assigned_id));
  inputFile.read(reinterpret_cast<char*>(&order.time_in_force),sizeof(order.time_in_force));
  inputFile.read(reinterpret_cast<char*>(order.trader_tag),3);
  inputFile.read(reinterpret_cast<char*>(&order.firm_id),sizeof(order.firm_id));
  int firmNameLen = msg_len - 44;
  inputFile.read(reinterpret_cast<char*>(&order.firm),firmNameLen);
  inputFile.read(reinterpret_cast<char*>(dummy),8);
  
  std::pair<std::map<std::string,int>::iterator,bool> ret;
  ret = traderOrderMap.insert(std::pair<std::string,int>(order.trader_tag,order.qty));
  
  if(ret.second==false)
    ret.first->second += order.qty;

  std::pair<std::map<std::string,int>::iterator,bool> retLiq;
  if(order.time_in_force==2)
  {
    retLiq = traderLiquidityMap.insert(std::pair<std::string,int>(order.trader_tag,order.qty));
    if(retLiq.second==false)
      retLiq.first->second += order.qty;
  }

  std::map<std::string,vector<uint64_t>>::iterator retOrder;
  retOrder = instrumentOrderIdMap.find(order.instrument);
  
  if(retOrder==instrumentOrderIdMap.end())
  {
    vector<uint64_t> orderId;
    orderId.push_back(sequence_id);
    instrumentOrderIdMap.insert(std::pair<std::string,vector<uint64_t>>(order.instrument,orderId));
  }
  else
    retOrder->second.push_back(sequence_id);
}

void PopulateOrderAck(std::ifstream &inputFile, const int msg_len)
{
  char dummy[8];
  OrderAcknowledge orderAck;
  inputFile.read(reinterpret_cast<char*>(&orderAck.order_id),sizeof(orderAck.order_id));
  inputFile.read(reinterpret_cast<char*>(&orderAck.client_id),sizeof(orderAck.client_id));
  inputFile.read(reinterpret_cast<char*>(&orderAck.order_status),sizeof(orderAck.order_status));
  inputFile.read(reinterpret_cast<char*>(&orderAck.reject_code),sizeof(orderAck.reject_code));
  inputFile.read(reinterpret_cast<char*>(dummy),8);
}

void PopulateOrderFill
(
  std::ifstream &inputFile, 
  const int msg_len, 
  std::map<string,int> &traderFillMap, 
  std::map<std::string,int>instrumentTradeMap, 
  std::map<uint64_t,uint32_t> &orderIdVolumeMap
)
{
  char dummy[8];
  OrderFill orderFill;
  inputFile.read(reinterpret_cast<char*>(&orderFill.order_id),sizeof(orderFill.order_id));
  inputFile.read(reinterpret_cast<char*>(&orderFill.fill_price),sizeof(orderFill.fill_price));
  inputFile.read(reinterpret_cast<char*>(&orderFill.fill_qty),sizeof(orderFill.fill_qty));
  inputFile.read(reinterpret_cast<char*>(&orderFill.no_of_contras),sizeof(orderFill.no_of_contras));
  
  for(int i=0;i<orderFill.no_of_contras;i++)
  {
    CounterParty cp;
    inputFile.read(reinterpret_cast<char*>(&cp.firm_id),sizeof(cp.firm_id));
    inputFile.read(reinterpret_cast<char*>(cp.trader_tag),3);
    inputFile.read(reinterpret_cast<char*>(&cp.qty),sizeof(cp.qty));
    orderFill.repeatingGroup.push_back(cp);
    
    std::pair<std::map<std::string,int>::iterator,bool> ret;
    ret = traderFillMap.insert(std::pair<std::string,int>(cp.trader_tag,cp.qty));
    
    if(ret.second==false)
      ret.first->second += cp.qty;
    
    orderIdVolumeMap.insert(std::pair<uint64_t, uint32_t>(orderFill.order_id,orderFill.fill_qty));
  }
  inputFile.read(reinterpret_cast<char*>(dummy),8);
}

void PrintMostActiveTrader
(
  std::map<std::string,int> &traderOrderMap, 
  std::map<std::string,int> &traderFillMap
)
{
  std::map<std::string,int>::iterator it = traderFillMap.begin();
  std::map<std::string,int>::iterator it1 = traderOrderMap.begin();
  string mostActiveTrader;
  int maxTrade = 0;
  for(;it!=traderFillMap.end();++it)
  {
    it1 = traderOrderMap.find(it->first);
    if(it1 != traderOrderMap.end())
    {
      if((it->second + it1->second)>maxTrade)
      {
        maxTrade = it->second + it1->second;
        mostActiveTrader = it->first;
      }
    }
    else
    cout << "iterator does not match"<<endl;
  }
  cout << ", "<<mostActiveTrader;
}

void PrintMostLiquidTrader(std::map<std::string,int> &traderLiquidityMap)
{
  int maxLiq = 0;
  string mostLiqTrader;
  std::map<std::string,int>::iterator it = traderLiquidityMap.begin();
  for(;it!=traderLiquidityMap.end();++it)
  {
    if(it->second > maxLiq)
    {
      maxLiq = it->second;
      mostLiqTrader = it->first;
    }
  }
  
  cout <<", "<< mostLiqTrader;
}

void PrintInstrumentVolume
(
  std::map<std::string,vector<uint64_t>> &instrumentOrderIdMap,
  std::map<uint64_t,uint32_t> &orderIdVolumeMap
)
{
  std::map<std::string,vector<uint64_t>>::iterator r = instrumentOrderIdMap.begin();
  for(;r!=instrumentOrderIdMap.end();r++)
  {
    int trade = 0;
    cout<<", "<< r->first<<":";
    std::vector<uint64_t>::iterator t=r->second.begin();
    for(;t!=r->second.end();t++)
    {
      std::map<uint64_t,uint32_t>::iterator it = orderIdVolumeMap.begin();
      it = orderIdVolumeMap.find(*t);
      if(it != orderIdVolumeMap.end())
      {
        trade += it->second;
      }
    }
    cout<<trade;
  }
  cout<<endl; 
}

main()
{
  int numOfPackets = 0;
  int numOfOrderEntry = 0;
  int numOfOrderAck = 0;
  int numOfOrderFill = 0;
  std::map<std::string,int>traderFillMap;
  std::map<std::string,int>traderOrderMap;
  std::map<std::string,int>traderLiquidityMap;
  std::map<std::string,int>instrumentTradeMap;
  std::map<std::string,vector<uint64_t>>instrumentOrderIdMap;
  std::map<uint64_t,uint32_t>orderIdVolumeMap;
  
  ifstream inputFile;
  inputFile.open("example_data_file.bin", ios::binary|ios::in);
  if(!inputFile)
      cerr << "Not able to read" << endl;
  
  while(!inputFile.eof())
  {
    RecordHeader hdr;
    inputFile.read(reinterpret_cast<char*>(&hdr.marker),sizeof(hdr.marker));
    inputFile.read(reinterpret_cast<char*>(&hdr.msg_type),sizeof(hdr.msg_type));
    inputFile.read(reinterpret_cast<char*>(&hdr.sequence_id),sizeof(hdr.sequence_id));
    inputFile.read(reinterpret_cast<char*>(&hdr.timestamp),sizeof(hdr.timestamp));
    inputFile.read(reinterpret_cast<char*>(&hdr.msg_direction),sizeof(hdr.msg_direction));
    inputFile.read(reinterpret_cast<char*>(&hdr.msg_len),sizeof(hdr.msg_len));
    
    if(hdr.marker == 0x5453)
      numOfPackets++;
    
    if(hdr.msg_type == 1){
      numOfOrderEntry++;
      PopulateOrderEntry(inputFile, hdr.msg_len, traderOrderMap,traderLiquidityMap, hdr.sequence_id, instrumentOrderIdMap);
      }
    else if(hdr.msg_type == 2){
      numOfOrderAck++;
      PopulateOrderAck(inputFile, hdr.msg_len);
      }
    else if(hdr.msg_type == 3){
      numOfOrderFill++;
      PopulateOrderFill(inputFile, hdr.msg_len, traderFillMap,instrumentTradeMap, orderIdVolumeMap);
      }
  }
  
  // Print Output (Q 1 and 2)
  cout<<dec<<numOfPackets<<", "<<numOfOrderEntry<<", "<<numOfOrderAck<<", "<<numOfOrderFill;
  
  // Q3
  PrintMostActiveTrader(traderOrderMap, traderFillMap);
  
  // Q4
  PrintMostLiquidTrader(traderLiquidityMap);
  
  // Q5
  PrintInstrumentVolume(instrumentOrderIdMap, orderIdVolumeMap);
}
