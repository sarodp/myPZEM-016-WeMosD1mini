//----PZEM data and calculation 

typedef struct
 {
   float_t V;
   float_t A;
   float_t PF;
   float_t W;
   float_t HZ;
   float_t WH;
   word ALARM; 
   long tstampHEX;        //"123456789012345678901"
   char tstampDT[21];     //"yyyy-mmm-ddThh:mm:ssZ"
 }  PZEMrec;

long   TstampHEX;
String TstampDT;

uint16_t Hregs[10] = { 0 };
uint16_t Iregs[10] = { 0 };

PZEMrec PZEMdata1;

PZEMrec calcPZEMdata(uint16_t xIreg[], long xthex, String xtstampDT){
  PZEMrec xval ;
  xval.V = (float_t) xIreg[0]*.1 ;
  xval.A = (float_t)  ((xIreg[2] * 65536) +xIreg[1])*.001 ; 
  xval.W = (float_t)  ((xIreg[4] * 65536) +xIreg[3])*.1 ;   
  xval.WH = (float_t) ((xIreg[6] * 65536) +xIreg[5])*1.0 ;  
  xval.HZ = (float_t) xIreg[7] *.1 ;
  xval.PF = (float_t) xIreg[8]*.01 ;
  xval.ALARM = (word) xIreg[9] ;
  xval.tstampHEX = xthex ;
  xtstampDT.toCharArray(xval.tstampDT,21) ;
  return xval ;
}



bool cbIread(Modbus::ResultCode event, uint16_t transactionId, void* data) { 
  //calc PZEMdata1
  PZEMdata1 = calcPZEMdata(Iregs,TstampHEX,TstampDT);

  // Callback to monitor errors
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  if (event == Modbus::EX_SUCCESS) {
    //digitalWrite(16, !digitalRead(16));
    Serial.print("DataRead OK\n");
  } 
  else {
    Serial.print("DataRead ERROR\n");  
  }
  
  Serial.print("TimeStamp = ");
  Serial.println(PZEMdata1.tstampDT);   //timestamp
  Serial.print("V = ");
  Serial.println(PZEMdata1.V,1);   //220.0V
  Serial.print("A = ");
  Serial.println(PZEMdata1.A,3);  // 1mA
  Serial.print("W = ");
  Serial.println(PZEMdata1.W,1); //  1watt
  Serial.print("WH = ");
  Serial.println(PZEMdata1.WH,0); // 0.1WH
  Serial.print("Hz = ");
  Serial.println(PZEMdata1.HZ,1);  // 49.0Hz
  Serial.print("PF = ");
  Serial.println(PZEMdata1.PF,2);   // 90.00%
  Serial.print("ALARM = 0b");
  Serial.println( (PZEMdata1.ALARM),BIN);  //bit list

  Serial.print("\n");
  
  return true;
}


bool cbIread_debug(Modbus::ResultCode event, uint16_t transactionId, void* data) { 
  // Callback to monitor errors
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  if (event == Modbus::EX_SUCCESS) {
    //digitalWrite(16, !digitalRead(16));
    Serial.print("DataRead OK\n");
  } 
  else {
    Serial.print("DataRead ERROR\n");  
  }
  
  Serial.print("DATAIreg\n");
  for(int i = 0; i < 10; i++) { 
    Serial.print(Iregs[i]);
    Serial.print(",");
  }
  Serial.print("\n");
  
  return true;
}

bool cbHread_debug(Modbus::ResultCode event, uint16_t transactionId, void* data) { 
// Callback to monitor errors
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  if (event == Modbus::EX_SUCCESS) {
    //digitalWrite(16, !digitalRead(16));
    Serial.print("DataRead OK\n");
  } 
  else {
    Serial.print("DataRead ERROR\n");  
  }
  
  Serial.print("DATA Hreg\n");
  for(int i = 0; i < 7; i++) { 
    Serial.print(Hregs[i]);
    Serial.print(",");
  }
  Serial.print("\n");
  
  return true;
}

/* --------------------------------------------------------------------
       enum FunctionCode {
            FC_READ_COILS       = 0x01, // Read Coils (Output) Status
            FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs)
            FC_READ_REGS        = 0x03, // Read Holding Registers
            FC_READ_INPUT_REGS  = 0x04, // Read Input Registers
            FC_WRITE_COIL       = 0x05, // Write Single Coil (Output)
            FC_WRITE_REG        = 0x06, // Preset Single Register
            FC_DIAGNOSTICS      = 0x08, // Not implemented. Diagnostics (Serial Line only)
            FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs)
            FC_WRITE_REGS       = 0x10, // Write block of contiguous registers
            FC_READ_FILE_REC    = 0x14, // Not implemented. Read File Record
            FC_WRITE_FILE_REC   = 0x15, // Not implemented. Write File Record
            FC_MASKWRITE_REG    = 0x16, // Not implemented. Mask Write Register
            FC_READWRITE_REGS   = 0x17  // Not implemented. Read/Write Multiple registers
        };
       enum ResultCode {
            EX_SUCCESS              = 0x00, // Custom. No error
            EX_ILLEGAL_FUNCTION     = 0x01, // Function Code not Supported
            EX_ILLEGAL_ADDRESS      = 0x02, // Output Address not exists
            EX_ILLEGAL_VALUE        = 0x03, // Output Value not in Range
            EX_SLAVE_FAILURE        = 0x04, // Slave or Master Device Fails to process request
            EX_ACKNOWLEDGE          = 0x05, // Not used
            EX_SLAVE_DEVICE_BUSY    = 0x06, // Not used
            EX_MEMORY_PARITY_ERROR  = 0x08, // Not used
            EX_PATH_UNAVAILABLE     = 0x0A, // Not used
            EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
            EX_GENERAL_FAILURE      = 0xE1, // Custom. Unexpected master error
            EX_DATA_MISMACH         = 0xE2, // Custom. Inpud data size mismach
            EX_UNEXPECTED_RESPONSE  = 0xE3, // Custom. Returned result doesn't mach transaction
            EX_TIMEOUT              = 0xE4, // Custom. Operation not finished within reasonable time
            EX_CONNECTION_LOST      = 0xE5, // Custom. Connection with device lost
            EX_CANCEL               = 0xE6  // Custom. Transaction/request canceled
         };      
------------------------------------------------------------------------------------------------ 
*/ 
