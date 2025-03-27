#include <map> 
#include "comm/datalayer/datalayer.h"

namespace RTUpdate
            {
            void MDT(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap);
            void AT(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap);            
            }

