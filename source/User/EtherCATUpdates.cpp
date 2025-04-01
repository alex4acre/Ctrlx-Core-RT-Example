#include "EtherCATUpdates.h"
#include "../impl/Logger.h"

long int m_ticks = 0;    
int16_t* StatusWord;
int16_t controlWord = 0x0100;
int16_t DigitalOutputs = 0xFF;
int16_t* PLCINT;

namespace RTUpdate{
    void MDT(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap)
    {
        m_ticks++; 

        //turn on/off some outputs just to show how it is done
        if(0 == m_ticks%500)
        {
            DigitalOutputs = DigitalOutputs ^ 0xFFFF;
        }
        //Enable the only drive  
        //Check to see if the drive is in Ab and Error free
        if (((*StatusWord & 0x8000) != 0) && ((*StatusWord & 0x2000) == 0)) 
        {
            controlWord = controlWord | 0xE000; //Enable bits 15,14,13 to enable (0xE000). 
            controlWord = controlWord | 0x0100; //Set the secondary operation mode
        }
        else
        {   //if there is an error or if the drive is not ready then clear the control bits
            controlWord = controlWord & 0x0100; 
        }
        controlWord = controlWord ^ 0x0400; //toggle the control bit, the 10th bit in the control word
        //copy over the IO
        if (m_outMap.contains("DO_16_1/Channel_1.Value"))
        {
          std::memcpy(&outData[m_outMap["DO_16_1/Channel_1.Value"]/8], &DigitalOutputs, 2); 
        }
        //copy over the control word
        if (m_outMap.contains("Axis1/MDT.Master_control_word"))
        {
          std::memcpy(&outData[m_outMap["Axis1/MDT.Master_control_word"]/8], &controlWord, 2); 
        }
        
        //copy over the velocity commands
        int32_t Velocity = 300000;
        if (m_outMap.contains("Axis1/MDT.VelocityCommand"))
        {
          std::memcpy(&outData[m_outMap["Axis1/MDT.VelocityCommand"]/8], &Velocity, 4);
        }  
    }

    void AT(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap)
    {
        //Read in the status word
        if (m_inMap.contains("Axis1/AT.Drive_status_word"))
        {
          StatusWord = (int16_t*)(&inData[m_inMap["Axis1/AT.Drive_status_word"]/8]); 
        if(0 == m_ticks%500)
          {
            LOG_INFO("Status Word: %i",(int)*StatusWord); 
          }  
        }
    }


    void PLC_OUT(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap)
    {
      if (m_outMap.contains("iTest"))
        {
          PLCINT = (int16_t*)(&outData[m_outMap["iTest"]/8]);
          //std::memcpy(&PLCINT, &outData[m_outMap["iTest"]/8], 2); 
          LOG_INFO("iTest Word: %i",(int)*PLCINT); 
        }
    }   

    void PLC_IN(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap)
    {
        if (PLCINT != 0)
        {
            //(*PLCINT)++;
            if (m_inMap.contains("iTest"))
            {
              std::memcpy(&inData[m_inMap["iTest"]/8], PLCINT, 2); 
            }
        }
    }
}
