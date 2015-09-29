#include <iostream>
#include <stdint.h>

#define LINUX
#include <CAENVMElib.h>
#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {

   CVBoardTypes VMEBoard;
   short Link;
   short Device;
   int32_t BHandle;

   VMEBoard = cvV2718;   
   Device = 0;
   Link = 0;

   if( CAENVME_Init(VMEBoard, Link, Device, &BHandle) != cvSuccess ) {

      std::cout << "ERROR: fail to open VME bus adapter" << std::endl;
      exit(1);
   }

   std::cout << "VME board V2818 opened successfully !" << std::endl;

   CAENVME_SystemReset(BHandle);
   CAENVME_SetInputConf(BHandle, cvInput0, cvDirect, cvActiveHigh); 
   CAENVME_SetOutputConf(BHandle, cvOutput0, cvDirect, cvActiveHigh, cvManualSW); 
   CAENVME_SetOutputConf(BHandle, cvOutput4, cvDirect, cvActiveHigh, cvMiscSignals); 

   CAENVME_SetScalerConf(BHandle, 16, 0, cvInputSrc0, cvManualSW, cvManualSW);

   CAENVME_ResetScalerCount(BHandle);
   CAENVME_EnableScalerGate(BHandle);

   unsigned int rval = 0;
   unsigned int sval = 0;
   unsigned int oldrval = 0;

   CAENVME_ReadRegister(BHandle, cvScaler0, &sval);
   std::cout << "cvScaler0 = " << sval << std::endl;

   while(1) {	// main loop

     CAENVME_ReadRegister(BHandle, cvInputReg, &rval); 
  
     std::cout << "IN0 => " << (rval & cvIn0Bit) << std::endl;
     std::cout << "SCALEREND => " << (rval & cvScalEndCntBit) << std::endl;
     //std::cout << "IN1 => " << ((rval & cvIn1Bit)>>1) << std::endl;
     std::cout << std::endl;

     CAENVME_EnableScalerGate(BHandle);
     sval = 0;
     while( sval == 0)
        CAENVME_ReadRegister(BHandle, cvScaler1, &sval);

     if(sval) {
 
        CAENVME_ResetScalerCount(BHandle);
        std::cout << "-- reset scaler - counter = " << sval << std::endl;
        CAENVME_SetOutputRegister(BHandle, cvOut0Bit);
        //CAENVME_ClearOutputRegister(BHandle, cvOut0Bit);
     }

     //usleep(1);
     
     oldrval = rval;
   }

   CAENVME_End(BHandle);

   return(0);
}
