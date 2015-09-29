#ifndef VME2NET_H_ 
#define VME2NET_H_

#define LINUX
#include <CAENVMElib.h>
#include <sstream>

// VME access type 
typedef enum { READ = 'r', WRITE = 'w' } vmeaccess_t;
typedef enum { NONE = 0, LOW_PARAM, HIGH_PARAM, OP_ERR, AM_ERR, DW_ERR, SEQ_ERR } ProtErrorCodes;

char const* const ProtErrorCodes_str[] = { "NONE", "missing parameters", "too many parameters", "opcode error", "address modifier error", "data width error" , "sequence error" };

// REQUEST:  vmeaccess_t op = WRITE | unsigned char seqnum | uint32_t address | uint32_t data | CVAddressModifier am | CVDataWidth dw
// REQUEST:  vmeaccess_t op = READ  | unsigned char seqnum | uint32_t address | CVAddressModifier am | CVDataWidth dw

struct _req {

   vmeaccess_t op;
   unsigned int seqnum;
   uint32_t address;
   uint32_t data;
   CVAddressModifier am;
   CVDataWidth dw;

   ProtErrorCodes perr;
};

// REPLY:    vmeaccess_t op = WRITE | unsigned char seqnum | CVErrorCodes verr 
// REPLY:    vmeaccess_t op = READ  | unsigned char seqnum | CVErrorCodes verr | uint32_t data 

struct _res {

   vmeaccess_t op;
   unsigned int seqnum;
   CVErrorCodes verr;
   uint32_t data;

   ProtErrorCodes perr;
};

typedef struct _req request_t;
typedef struct _res response_t;

ProtErrorCodes validate_param(vmeaccess_t op) {

   if( (op == READ) || (op == WRITE) )
      return NONE;
   else
      return OP_ERR;
}

ProtErrorCodes validate_param(CVAddressModifier am) {

   if( (am == cvA16_S) || (am == cvA16_U) || (am == cvA24_S_DATA) || (am == cvA24_U_DATA) || (am == cvA32_S_DATA) || (am == cvA32_U_DATA))
      return NONE;
   else
      return AM_ERR;
}

ProtErrorCodes validate_param(CVDataWidth dw) {

   if( (dw == cvD8) || (dw == cvD16) || (dw == cvD32) || (dw == cvD64))
      return NONE;
   else
      return DW_ERR;
}

void init(request_t &req) {

   req.op = (vmeaccess_t) 0;
   req.seqnum = 0;
   req.address = 0;
   req.data = 0;
   req.am = (CVAddressModifier) 0;
   req.dw = (CVDataWidth) 0;
   req.perr = (ProtErrorCodes) 0;
}

void init(response_t &res) {

   res.op = (vmeaccess_t) 0;
   res.seqnum = 0;
   res.verr = (CVErrorCodes) 0;
   res.data = 0;
   res.perr = (ProtErrorCodes) 0;
}

request_t deserialize_req(std::string string_req) {

   request_t req;
   int nconv;
   vmeaccess_t op;
   unsigned int seqnum;
   uint32_t address;
   uint32_t data;
   CVAddressModifier am;
   CVDataWidth dw;

   if((validate_param((vmeaccess_t) string_req[0]) != NONE)) {
    
      req.perr = OP_ERR;
      return(req);

   } else req.op = (vmeaccess_t) string_req[0];

   if(req.op == READ) {

      std::istringstream iss(string_req);
      std::string token;

      nconv = 0;
      while(getline(iss, token, '|')) {

         std::istringstream ss(token);

         if(nconv == 0) op = (vmeaccess_t) token[0];
         else if(nconv == 1) ss >> seqnum;
         else if(nconv == 2) ss >> address;
         else if(nconv == 3) am = (CVAddressModifier) atoi(token.c_str());
         else if(nconv == 4) dw = (CVDataWidth) atoi(token.c_str()); 

         nconv++;
      }

      if(nconv < 5)
         req.perr = LOW_PARAM;
      else if(nconv > 5) 
         req.perr = HIGH_PARAM;
      else {	// nconv == 5

         // parameters range validation
         if(validate_param(am) != NONE)
            req.perr = AM_ERR;
         else if(validate_param(dw) != NONE)
            req.perr = DW_ERR;
         else {

            req.seqnum = seqnum;
            req.address = address;
            req.am = am;
            req.dw = dw;
            req.perr = NONE;
         }
      }

   } else if(req.op == WRITE) {

      std::istringstream iss(string_req);
      std::string token;

      nconv = 0;
      while(getline(iss, token, '|')) {

         std::istringstream ss(token);

         if(nconv == 0) op = (vmeaccess_t) token[0];
         else if(nconv == 1) ss >> seqnum;
         else if(nconv == 2) ss >> address;
         else if(nconv == 3) ss >> data;
         else if(nconv == 4) am = (CVAddressModifier) atoi(token.c_str());
         else if(nconv == 5) dw = (CVDataWidth) atoi(token.c_str()); 

         nconv++;
      }

      if(nconv < 6)
         req.perr = LOW_PARAM;
      else if(nconv > 6)
         req.perr = HIGH_PARAM;
      else {    // nconv == 6

         // parameters range validation
         if(validate_param(am) != NONE)
            req.perr = AM_ERR;
         else if(validate_param(dw) != NONE)
            req.perr = DW_ERR;
         else {

            req.seqnum = seqnum;
            req.address = address;
            req.data = data;
            req.am = am;
            req.dw = dw;
            req.perr = NONE;
         }
      }
   } 

   return(req);
}

response_t deserialize_res(std::string string_res) {

   response_t res;
   int nconv = 0;
   vmeaccess_t op;
   unsigned int seqnum;
   ProtErrorCodes perr;
   CVErrorCodes verr;
   uint32_t data;

   if((validate_param((vmeaccess_t) string_res[0]) != NONE)) {

      res.perr = OP_ERR;
      return(res);

   } else res.op = (vmeaccess_t) string_res[0];

   if(res.op == READ) {

      std::istringstream iss(string_res);
      std::string token;

      nconv = 0;
      while(getline(iss, token, '|')) {

         std::istringstream ss(token);

         if(nconv == 0) op = (vmeaccess_t) token[0];
         else if(nconv == 1) ss >> seqnum;
         else if(nconv == 2) verr = (CVErrorCodes) atoi(token.c_str());
         else if(nconv == 3) ss >> data;

         nconv++;
      }
 
      if(nconv < 4)
         res.perr = LOW_PARAM;
      else if(nconv > 4) 
         res.perr = HIGH_PARAM;
      else {	// nconv == 4

         res.seqnum = seqnum;
         res.verr = verr;
         res.data = data;
         res.perr = NONE;
      }

   } else if(res.op == WRITE) {

      std::istringstream iss(string_res);
      std::string token;

      nconv = 0;
      while(getline(iss, token, '|')) {

         std::istringstream ss(token);

         if(nconv == 0) op = (vmeaccess_t) token[0];
         else if(nconv == 1) ss >> seqnum;
         else if(nconv == 2) verr = (CVErrorCodes) atoi(token.c_str());

         nconv++;
      }
    
      if(nconv < 3)
         res.perr = LOW_PARAM;
      else if(nconv > 3) 
         res.perr = HIGH_PARAM;
      else {	// nconv == 3

         res.seqnum = seqnum;
         res.verr = verr;
         res.perr = NONE;
      }
   }

   return(res);
}

std::string serialize_req(request_t req) {

   std::stringstream sstr;

   if(req.op == WRITE)
      sstr << (char) req.op << "|" << req.seqnum << "|" << req.address << "|" << req.data << "|" << req.am << "|" << req.dw;
   else if(req.op == READ)
      sstr << (char) req.op << "|" << req.seqnum << "|" << req.address << "|" << req.am << "|" << req.dw;

   return(sstr.str());
}

std::string serialize_res(response_t res) {

   std::stringstream sstr;

   if(res.op == WRITE)
      sstr << (char) res.op << "|" << res.seqnum << "|" << res.verr;
   else if(res.op == READ) 
      sstr << (char) res.op << "|" << res.seqnum << "|" << res.verr << "|" << res.data;

   return(sstr.str());
}

void debug(request_t req) {

   std::cout << "REQUEST: " << std::endl;
   std::cout << "  op     = " << (char) req.op << std::endl;
   std::cout << "  seqnum = " << req.seqnum << std::endl;
   std::cout << "  addr   = 0x" << std::hex << req.address << std::dec << std::endl;

   if(req.op == WRITE)
      std::cout << "  data   = " << req.data << std::endl;

   std::cout << "  am     = 0x" << std::hex << req.am << std::dec << std::endl;
   std::cout << "  dw     = 0x" << std::hex << req.dw << std::dec << std::endl;

   if( (req.perr > 0) && (req.perr < 6) )
      std::cout << "format error: " << ProtErrorCodes_str[req.perr] << std::endl;

   std::cout << std::endl;
}

void debug(response_t res) {

   std::cout << "RESPONSE: " << std::endl;
   std::cout << "  op     = " << (char) res.op << std::endl;
   std::cout << "  seqnum = " << res.seqnum << std::endl;
   if(res.op == READ)
      std::cout << "  data   = " << res.data << std::endl;
   std::cout << "VME error: " << res.verr << std::endl;

   if( (res.perr > 0) && (res.perr < 6) )
      std::cout << "format error: " << ProtErrorCodes_str[res.perr] << std::endl;

   std::cout << std::endl;
}

#endif
