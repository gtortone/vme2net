#include <iostream>
#include <stdint.h>

#define LINUX
#include <CAENVMElib.h>
#include "boost/program_options.hpp"
#include "vme2net.h"
#include "zhelpers.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {

   CVBoardTypes VMEBoard;
   short Link;
   short Device;
   int32_t BHandle;

   zmq::context_t zmq_ctx(1);
   zmq::socket_t zmq_server(zmq_ctx, ZMQ_REP);
   std::string url, port;

   std::cout << "vme2net - release 1.0" << std::endl;

   // handling of command line parameters
   po::options_description desc("\nallowed options: ");

   desc.add_options()
    ("help", "produce help message")
    ("port", po::value<std::string>(), "vme2net server port (default: 5555)")
    ("dryrun", "start server without V2718 controller setup")
   ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
   }

   if (vm.count("port")) {
      port = vm["port"].as<std::string>();
   } else {
      port = "5555";
   }
 
   url = "tcp://*:" + port;

   try {

      std::cout << "starting TCP server on port ..." << port << std::endl;
      zmq_server.bind(url.c_str());
      sleep(1);     

   } catch(zmq::error_t zmq_err) {

      std::cout << "ERROR: failed - " << zmq_err.what() << std::endl;
      return -1;
   }

   std::cout << "server OK" << std::endl;

   if(!vm.count("dryrun")) {

      VMEBoard = cvV2718;   
      Device = 0;
      Link = 0;

      if( CAENVME_Init(VMEBoard, Link, Device, &BHandle) != cvSuccess ) {

         std::cout << "ERROR: fail to open VME bus adapter" << std::endl;
         exit(1);
      }

      std::cout << "VME board V2818 opened successfully !" << std::endl;

      // CAENVME_SystemReset(BHandle);
   }

   request_t req;
   response_t res;
   std::string string_req, string_res;

   while(1) {	// main loop

      try {

         string_req = s_recv(zmq_server);

      } catch(zmq::error_t zmq_err) {

         std::cout << "socket error: " << zmq_err.what() << std::endl;
      }

      //std::cout << "data received: " << (char *) msg.data() << " [" << msg.size() << "]" << std::endl;

      init(req);
      init(res);

      req = deserialize_req(string_req);
      debug(req);

      res.seqnum = req.seqnum;
      res.data = 0;
      res.perr = req.perr;

      if(req.perr == NONE) {

         res.op = req.op;

         if(!vm.count("dryrun")) {

            // perform a VME access

            if(req.op == READ) {

               res.verr = CAENVME_ReadCycle(BHandle, req.address, &(req.data), req.am, req.dw);

               if(res.verr == cvSuccess) 
                  res.data = req.data;
            
            } else if(req.op == WRITE) {

               res.verr = CAENVME_WriteCycle(BHandle, req.address, &(req.data), req.am, req.dw);
            }

        } else {

            // dryrun

            res.verr = cvSuccess;
            res.data = 0;            
        } 

      } 

      std::string string_res = serialize_res(res);
      //std::cout << "RESPONSE string: " << string_res.data() << " [" << string_res.size() << "]" << std::endl;
      debug(res);

      s_send(zmq_server, string_res);

      std::cout << "- - - -" << std::endl;
   }

   if(!vm.count("dryrun")) {

      CAENVME_End(BHandle);
   }

   zmq_server.close();
   zmq_ctx.close();

   return(0);
}
