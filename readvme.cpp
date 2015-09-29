#include <iostream>

#include "vme2net.h"
#include "zhelpers.hpp"

int main(int argc, char *argv[]) {

   int i = 0;
   std::string string_req, string_res;

   request_t req;
   response_t reply;

   std::cout << "SEND setup to vme2net 1.0" << std::endl;

   zmq::context_t zmq_ctx(1);
   zmq::socket_t zmq_client(zmq_ctx, ZMQ_REQ);

   //zmq_client.connect("tcp://localhost:5555");	// from arguments
   zmq_client.connect("tcp://exodaq01.na.infn.it:5555");	// from arguments
      
   std::cout << "press a key to send a request" << std::endl;

   while(1) {
 
      getchar();

      std::cout << "request #" << i++ << std::endl;

      init(req);

      req.op = READ;
      req.seqnum = i % 128;
      //req.address = (0xACAC0000 + i);
      req.address = (0xF0910000 + 0x00E8);
      req.data = 0;
      req.am = cvA32_U_DATA;
      req.dw = cvD16;

      debug(req);

      string_req = serialize_req(req);

      //std::cout << "REQ string: " << string_req << std::endl;

      s_send(zmq_client, string_req);
   
      std::cout << "- - - -" << std::endl;

      string_res = s_recv(zmq_client);

      //std::cout << "REP string: " << string_res.data() << " [" << string_res.size() << "]" << std::endl;

      init(reply);
      reply = deserialize_res(string_res);
      debug(reply); 

   } // end while(1)

   zmq_client.close();
   zmq_ctx.close();

   return(0);
 }
