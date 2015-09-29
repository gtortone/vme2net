#include <iostream>
#include <string>

#include "vme2net.h"
#include "zhelpers.hpp"

#define BASEADDR	0xF0910000

void usage(void);
int getHex(std::string);

int main(int argc, char *argv[]) {

   if(argc < 3) {

      usage();
      return 1;
   }

   request_t req;
   init(req);

   if(std::string(argv[1]) == "R") {

      req.op = READ;

   } else if(std::string(argv[1]) == "W") {

      req.op = WRITE;
 
   } else {

      std::cout << "ERROR: opcode not valid" << std::endl;
      usage();
      return 1;
   }

   std::string string_req, string_res;

   response_t reply;

   std::cout << "SEND setup to vme2net 1.0" << std::endl;

   zmq::context_t zmq_ctx(1);
   zmq::socket_t zmq_client(zmq_ctx, ZMQ_REQ);

   zmq_client.connect("tcp://exodaq01.na.infn.it:5555");	// from arguments
      

   if(req.op == READ) {

      std::cout << "op code = " << req.op << " : address = " << getHex(std::string(argv[2])) << std::endl;
      req.seqnum = 128;
      req.address = (BASEADDR + getHex(std::string(argv[2])));
      req.data = 0;
      req.am = cvA32_U_DATA;
      req.dw = cvD16;
   }

   if(req.op == WRITE) {

      req.seqnum = 128;
      req.address = (BASEADDR + getHex(std::string(argv[2])));
      req.data = getHex(std::string(argv[3]));;
      req.am = cvA32_U_DATA;
      req.dw = cvD16;
   }

   debug(req);

   string_req = serialize_req(req);

   s_send(zmq_client, string_req);
   
   std::cout << "- - - -" << std::endl;

   string_res = s_recv(zmq_client);

   init(reply);
   reply = deserialize_res(string_res);
   debug(reply); 

   zmq_client.close();
   zmq_ctx.close();

   return(0);
 }

void usage(void) {

   std::cout << "sendvme R|W address [value]" << std::endl;
   std::cout << "example: sendvme R 0xF09100E8" << std::endl;
}

int getHex(std::string hexstr) {
    return (int)strtol(hexstr.c_str(), 0, 16);
}
