#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <cstring>

struct RecordHeader
{
  uint16_t marker;
  uint8_t msg_type;
  uint64_t sequence_id;
  uint64_t timestamp;
  uint8_t msg_direction;
  uint16_t msg_len;
  
  RecordHeader():marker(0),msg_type(0),sequence_id(0),timestamp(0),msg_direction(0),msg_len(0)
  {};
};

struct OrderEntry
{
  RecordHeader* header;
  uint64_t price;
  uint32_t qty;
  char instrument[11];
  uint8_t side;
  uint64_t client_assigned_id;
  uint8_t time_in_force;
  char trader_tag[4];
  uint8_t firm_id;
  char firm[256];
  
  OrderEntry():header(NULL),price(0),qty(0),side(255),client_assigned_id(9999),time_in_force(0),firm_id(0)
  {
    memset(instrument,'\0',11);
    memset(firm,'\0',256);
    memset(trader_tag,'\0',4);
  }
};

struct OrderAcknowledge
{
  RecordHeader* header;
  uint32_t order_id;
  uint64_t client_id;
  uint8_t order_status;
  uint8_t reject_code;
  OrderAcknowledge():header(NULL),order_id(0),client_id(0),order_status(0),reject_code(0){};
};

struct CounterParty
{
  uint8_t firm_id;
  char trader_tag[4];
  uint32_t qty;
  CounterParty():firm_id(0),qty(0)
  {
    memset(trader_tag,'\0',4);
  }
};

struct OrderFill
{
  RecordHeader* header;
  uint32_t order_id;
  uint64_t fill_price;
  uint32_t fill_qty;
  uint8_t no_of_contras;
  std::vector<CounterParty> repeatingGroup;
};
