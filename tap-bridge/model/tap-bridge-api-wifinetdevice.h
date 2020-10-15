/*
 * Created by Advanced Networks Research Group, PUCP - GIRA PUCP                
 *                                                                              
 *            ___                                                                
 *  ___      |   |      ___                                                      
 * |   |     '---'     |   |    .------.  .--.  .-----.     .------.            
 * '---'       |       '---'   '        ' |  | '        \  /        \
 *   |         .         |    |   .---.  ||  ||   .--.   |'    /\    '          
 *   ` --. . '   ' . .-- '    |   |   '--'|  ||   |   |  ||   |  |   |          
 *        |         |         |   | .----.|  ||   '--'  / |   '--'   |          
 *        |    .    |         |   | '-.  ||  ||    __  \  |    __    |          
 *        |. '   ` .|         |   '---'  ||  ||   |  \  \ |   |  |   |          
 *   . --' ` .   . ' '-- .    '.        .'|  ||   |   \  \|   |  |   |          
 *   |         '         |      `------'  '--''---'    '-''---'  '---'          
 * .---.       |       .---.                                                     
 * |   |     .---.     |   |        P        U        C        P               
 *  ¯¯¯      |   |      ¯¯¯                                                      
 *            ¯¯¯                                                                
 *                                                                               
 * Author: Alejandro Huamantuma, <alejandro.huamantuma@pucp.edu.pe>
 */

#ifndef TAP_BRIDGE_API_WIFINETDEVICE_H
#define TAP_BRIDGE_API_WIFINETDEVICE_H

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/wifi-net-device.h"
#include "ns3/mac48-address.h"

namespace gira {

class ApiWifiNetDevice
{
public:
  ApiWifiNetDevice();
  virtual ~ApiWifiNetDevice();
  
  /*
   * \param command the Command to process
   */
  void CallApiWifi (char* command, int nodeId, int deviceId, int sock,
      ns3::Mac48Address src, ns3::Mac48Address dst);

private:
  // Methods:
  void ProcessCommand (char * command_tmp);
  bool IdentifyRequest(void);
  void SendBackMessage (const char * message);
  bool ValidateIsNumber (char * value);
  
  // Methods for Set values:
  bool ConfigWifiNet (void);
  bool ConfigWifiPhy (void);
  bool ConfigConstRate (void);

  // Methods for Get values:
  bool GetInformation(void);

  // Constant list with allowed attributes
  const std::map<const std::string, const std::string> wifinetAllowedAttributes;
  const std::map<const std::string, const std::string> wifiphyAllowedAttributes;
  const std::map<const std::string, const std::string> constRateAllowedAttributes;
  
  char * command;
  int nodeId;
  int deviceId;
  int sock;
  ns3::Mac48Address src;
  ns3::Mac48Address dst;
  std::vector<char*> tokens;
};

}

/* 
 * ========================
 * Initialize const list in constructors
 * ========================
 */ 


#endif /* TAP_BRIDGE_API_WIFINETDEVICE_H */
