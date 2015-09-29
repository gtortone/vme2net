#include <iostream>

#define LINUX
#include <CAENVMElib.h>

#define BASEADDR        0xF0910000

int32_t BHandle;

void bs_enable(void);
void bs_reset(void);
int bs_read(void);

int main(int argc, char *argv[]) {

   CVBoardTypes VMEBoard;
   short Link;
   short Device;

   std::cout << "Trigger handling from TSV board" << std::endl << std::endl;

   VMEBoard = cvV2718;
   Device = 0;
   Link = 0;

   if( CAENVME_Init(VMEBoard, Link, Device, &BHandle) != cvSuccess ) {

         std::cout << "ERROR: fail to open VME bus adapter" << std::endl;
         exit(1);
   }

   std::cout << "VME board V2818 opened successfully !" << std::endl;

   //CAENVME_SystemReset(BHandle);

   bs_enable();
   bs_reset();

   while(1) {

      if(bs_read()) {	// we got a trigger...

         std::cout << "TRIGGER detected" << std::endl;
         bs_reset();  
      }
   }

   return 0;	// never reached
}

void bs_enable(void) {

   int data = 1; 
   CAENVME_WriteCycle(BHandle, BASEADDR + 0xE8, &data, cvA32_U_DATA, cvD16);
}

void bs_reset(void) {

   int data;
   //data = 0;
   //CAENVME_WriteCycle(BHandle, BASEADDR + 0xF8, &data, cvA32_U_DATA, cvD16);
   data = 1;
   CAENVME_WriteCycle(BHandle, BASEADDR + 0xF8, &data, cvA32_U_DATA, cvD16);
   //data = 0;
   //CAENVME_WriteCycle(BHandle, BASEADDR + 0xF8, &data, cvA32_U_DATA, cvD16);
}

int bs_read(void) {

   int data;
   CAENVME_ReadCycle(BHandle, BASEADDR + 0x108, &data, cvA32_U_DATA, cvD16);
   return(data);
}
