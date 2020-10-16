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

#include "tap-bridge-api-wifinetdevice.h"

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/wifi-mode.h"
#include "ns3/address.h"
#include "ns3/mac48-address.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/ethernet-header.h"
#include "ns3/node.h"
#include "ns3/wifi-net-device.h"
#include "ns3/pointer.h"
#include "ns3/config.h"

#include <unistd.h>

NS_LOG_COMPONENT_DEFINE ("TapBridgeApiWifiNetDevice");

namespace gira {

ApiWifiNetDevice::ApiWifiNetDevice ():
  wifinetAllowedAttributes({
      {"Mtu","UintegerValue"}
      }),
  wifiphyAllowedAttributes({
      {"ChannelNumber", "UintegerValue"},
      {"RxSensitivity", "DoubleValue"},
      {"CcaEdThreshold", "DoubleValue"},
      {"TxPower","DoubleValue"}
      }),
  constRateAllowedAttributes({
      {"DataMode", "StringValue"},
      {"ControlMode","StringValue"},
      {"MaxSsrc","UintegerValue"},
      {"MaxSlrc","UintegerValue"},
      {"RtsCtsThreshold","UintegerValue"},
      {"FragmentationThreshold","UintegerValue"}
      })
{}

gira::ApiWifiNetDevice::~ApiWifiNetDevice () {}

void
ApiWifiNetDevice::CallApiWifi(char* command, int nodeId, int deviceId, 
    int sock, ns3::Mac48Address src, ns3::Mac48Address dst)
{
  // Guardo las variables
  this->command = command;
  this->nodeId = nodeId;
  this->deviceId = deviceId;
  this->sock = sock;
  this->src = src;
  this->dst = dst;
  
  // Creo una copia del comando recibido
  char * command_tmp = (char*)malloc(std::strlen(command));
  std::strcpy (command_tmp, command);

  // Llamamos al metodo que parsea el comando y lo divide en tokens
  ProcessCommand (command_tmp);
  
  // Llamamos al metodo que identifica el request
  bool reqidentified = IdentifyRequest();
  if (!reqidentified) {
    // No se identifico la solicitud
    // *** do some stuff
    return;
  }
  
  // Liberaciones y finalizaciones
  std::free(command_tmp);

  return;
}

void 
ApiWifiNetDevice::ProcessCommand (char * command_tmp)
{

  // Delimiters
  char const* command_puro_delimiter = "&";
  char const* token_delimiter = "@";
  
  // Separar (Tokenizer) el comando en tokens
  
  // Primero separo todo el comando detras de '&' para evitar caracteres
  // extranhos al final. Me quedo con un 'comando puro'
  char * command_puro = std::strtok(command_tmp, command_puro_delimiter);
  NS_LOG_DEBUG ("--> AH: command_puro=" << command_puro 
      << ", largo=" << std::strlen(command_puro) );
  
  // Creo vector local para guardar los tokens temporalmente
  std::vector<char*> tokens_vec_tmp;

  // Empiezo a separar los tokens por el delimiter
  char * tokens_tmp = std::strtok(command_puro, token_delimiter);
  tokens_vec_tmp.push_back( tokens_tmp );
  while (tokens_tmp !=  NULL){
    //NS_LOG_UNCOND("--> AH: token=" << tokens.back() << ", length="
    //   << std::strlen(tokens.back()));
    //tokens.push_back( tokens_tmp );
    tokens_tmp = std::strtok(NULL, token_delimiter);
    if (tokens_tmp != NULL){
      tokens_vec_tmp.push_back( tokens_tmp );
    }
  }
  
  // Copio los valores al atributo tokens
  this->tokens = tokens_vec_tmp;

  NS_LOG_DEBUG ("--> API: #tokens="<<tokens.size());
  
  return;
}

bool
ApiWifiNetDevice::IdentifyRequest ()
{
  // Detectar si se desea setear un valor o si se desea obtener un valor.
  // Ademas validar si los atributos mencionados existen
  
  // Tokens recibidos (temporal)
  for ( unsigned i=0; i< tokens.size() ; i++ ) {
    NS_LOG_DEBUG ("--> API: token=" << tokens.at(i) << ", length=" 
        << std::strlen(tokens.at(i)));
  }

  bool requestAcomplished = false;

  if ( strcmp(tokens.at(0), "set") == 0) {
    NS_LOG_INFO ("--> API: Se solicita configurar: "<< tokens.at(1));
    
    char response_config_ok[] = "ok";
    char response_config_parameter_fail[] = "parameter_fail";
    char response_config_value_fail[] = "value_fail";
    
    // Como todos los atributos tienen valores numericos

    // Validacion de atributos requeridos
    if ( wifinetAllowedAttributes.find( tokens.at(1) ) 
        != wifinetAllowedAttributes.end() ) {
      // El atributo a modificar esta en WifiNet
      requestAcomplished = ConfigWifiNet();
      if (requestAcomplished) {
        SendBackMessage(response_config_ok);
      }
      else{
        SendBackMessage(response_config_value_fail);
      }
    } 
    else if ( wifiphyAllowedAttributes.find( tokens.at(1) ) 
        != wifiphyAllowedAttributes.end() ) {
      // EL atributo a modificar esta en WifiPhy
      requestAcomplished = ConfigWifiPhy();
      if (requestAcomplished) {
        SendBackMessage(response_config_ok);
      }
      else{
        SendBackMessage(response_config_value_fail);
      }
    }
    else if ( constRateAllowedAttributes.find ( tokens.at(1) )
        != constRateAllowedAttributes.end() ) {
      // El atributo a modificar esta en ConstRate
      requestAcomplished = ConfigConstRate();
      if (requestAcomplished) {
        SendBackMessage(response_config_ok);
      }
      else{
        SendBackMessage(response_config_value_fail);
      }
    }
    else {
      // Si no se valido el atributo
      NS_LOG_INFO ("--> API: No se encontro un parametro valido");
      SendBackMessage(response_config_parameter_fail);
    }
  }
  else if ( strcmp(tokens.at(0), "get") == 0 ) {
    NS_LOG_INFO ("--> API: Se solicita devolver algo");
    requestAcomplished = GetInformation();
  }
  else {
    NS_LOG_DEBUG ("--> API: No se detecto el tipo de solicitud");
    // No deberia pasar porque el API valida el tipo de solicitud
    return false;
  }

  return requestAcomplished;
}

bool
ApiWifiNetDevice::ConfigWifiNet()
{
  // Config Path de WifiNet. // Mtu
  std::ostringstream osconf;
  osconf << "/NodeList/" << nodeId
    << "/DeviceList/" << deviceId
    << "/$ns3::WifiNetDevice/"
    << tokens.at(1);
  
  bool validvalue = ValidateIsNumber(tokens.at(2));
  if (!validvalue) return false;
  
  if ( std::stoi(tokens.at(2)) < 1 ) return false;

  // Ya se que es Uint porque solo tiene el MTU
  NS_LOG_INFO (osconf.str());
  ns3::Config::Set (osconf.str(), ns3::UintegerValue(std::stoi(tokens.at(2))));
  return true;
}

bool
ApiWifiNetDevice::ConfigWifiPhy()
{
  // Caso especial con el TxPower. EN NS3 se definen
  // TxPowerStart y TxPowerEnd. Debo setear ambos con el mismo valor
  
  bool validvalue = ValidateIsNumber(tokens.at(2));
  if (!validvalue) return false;
  
  if (std::strcmp(tokens.at(1),"TxPower")==0) {
    // Debo setear dos parametros
    std::ostringstream osconftxpower;
    osconftxpower << "/NodeList/" << nodeId
    << "/DeviceList/" << deviceId
    << "/$ns3::WifiNetDevice/Phy/";
    
    NS_LOG_INFO (osconftxpower.str());

    std::ostringstream ostmp1;
    ostmp1 << osconftxpower.str() << "TxPowerStart";
    ns3::Config::Set (ostmp1.str(), ns3::DoubleValue(std::stoi(tokens.at(2))));
    
    NS_LOG_INFO (ostmp1.str());

    std::ostringstream ostmp2;
    ostmp2 << osconftxpower.str() << "TxPowerEnd";
    ns3::Config::Set (ostmp2.str(), ns3::DoubleValue(std::stoi(tokens.at(2))));
    
    NS_LOG_INFO (ostmp2.str());
    
    NS_LOG_INFO ("-->API: Valor seteado correctamente");
    
    return true;
  }

  // Config Path de WifiPhy
  std::ostringstream osconf;
  osconf << "/NodeList/" << nodeId
    << "/DeviceList/" << deviceId
    << "/$ns3::WifiNetDevice/Phy/"
    << tokens.at(1);
  
  NS_LOG_INFO ("--> AH: El path de configuracion es:");
  NS_LOG_INFO (osconf.str());
  NS_LOG_INFO ("--> valor a actualizar: "<< tokens.at(2) );
  
  // Vamos a validar el tipo de variable
  if ( wifiphyAllowedAttributes.find(tokens.at(1))->second.compare(
        "UintegerValue") == 0) {
    
    if ( std::stoi(tokens.at(2)) < 1 ) return false;
    
    // Validacion especial para el ChannelNumber en 802.11n 2.4 GHz
    // Solo son validos del 1 al 13
    if ( std::string(tokens.at(1)).compare("ChannelNumber")==0 &&
        std::stoi(tokens.at(2)) <= 13) {
      //NS_LOG_UNCOND ("Channel no valido");
      return false;
    }
    ns3::Config::Set (osconf.str(), ns3::UintegerValue(std::stoi(tokens.at(2))));
  }
  else if ( wifiphyAllowedAttributes.find(tokens.at(1))->second.compare(
        "DoubleValue") == 0) {
    ns3::Config::Set (osconf.str(), ns3::DoubleValue(std::stoi(tokens.at(2))));
  }
  else {
    // No es niguno. Esto no deberia ocurrir por la definicion incial
  }
  
  NS_LOG_INFO ("-->API: Valor seteado correctamente");

  return true;
}

bool
ApiWifiNetDevice::ConfigConstRate()
{
  // Config Path de ConstantRateManager
  std::ostringstream osconf;
  osconf << "/NodeList/" << nodeId
    << "/DeviceList/" << deviceId
    << "/$ns3::WifiNetDevice/RemoteStationManager"
    << "/$ns3::ConstantRateWifiManager/"
    << tokens.at(1);
  
  NS_LOG_INFO ("--> API: El path de configuracion es:");
  NS_LOG_INFO (osconf.str());
  NS_LOG_INFO ("--> valor a actualizar: "<< tokens.at(2) );
  
  // Vamos a validar el tipo de variable
  if ( constRateAllowedAttributes.find(tokens.at(1))->second.compare(
        "UintegerValue") == 0) {
    //bool validvalue = ValidateIsNumber(tokens.at(2));
    //if (!validvalue) return false;
    //if ( std::stoi(tokens.at(2)) < 1 ) return false;
    ns3::Config::Set (osconf.str(), ns3::UintegerValue(std::stoi(tokens.at(2))));
  }
  else if ( constRateAllowedAttributes.find(tokens.at(1))->second.compare(
        "StringValue") == 0) {
    // Validacion para evitar errores en el set:
    // Suponiendo que estamos en el standard 802.11n
    // Entonces los valores permitidos son:
    // HtMcs1, HtMcs2, HtMcs3, HtMcs4, HtMcs5, HtMcs6, HtMcs7
    // No se puede mas porque solo estamos trabajando con 1 antena = 
    // 1 spatial stream. 
    if (  std::string(tokens.at(2)).compare("HtMcs0")!=0 && 
          std::string(tokens.at(2)).compare("HtMcs1")!=0 && 
          std::string(tokens.at(2)).compare("HtMcs2")!=0 &&
          std::string(tokens.at(2)).compare("HtMcs3")!=0 &&
          std::string(tokens.at(2)).compare("HtMcs4")!=0 &&
          std::string(tokens.at(2)).compare("HtMcs5")!=0 &&
          std::string(tokens.at(2)).compare("HtMcs6")!=0 &&
          std::string(tokens.at(2)).compare("HtMcs7")!=0   )
    {
      NS_LOG_DEBUG ("VALOR NO VALIDO");
      return false;
    }
    ns3::Config::Set (osconf.str(), ns3::StringValue( tokens.at(2) ));
    
    /*
    // Para el comportamiento del Access Point Aironet en la experiencia de
    // inalambricas. La sensitividad de recepcion tambien varia con el MCS 
    // del DataMode
    
    // Config Path de WifiPhy
    std::ostringstream osconfphy;
    osconfphy << "/NodeList/" << nodeId
      << "/DeviceList/" << deviceId
      << "/$ns3::WifiNetDevice/Phy/RxSensitivity";
    
    if (std::string(tokens.at(1)).compare("DataMode")==0){
      if (std::string(tokens.at(2)).compare("HtMcs0")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-90));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs1")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-87));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs2")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-84));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs3")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-82));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs4")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-79));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs5")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-74));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs6")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-72));
      }
      else if (std::string(tokens.at(2)).compare("HtMcs7")==0) {
        ns3::Config::Set (osconfphy.str(), ns3::DoubleValue(-71));
      }
      else {
        // No deberia pasar pero vamo a ver
      }
    }
    */
  }
  else {
    // No es niguno. Esto no deberia ocurrir por la definicion incial
    // en mi tool iwconfig en los contenedores
  }

  NS_LOG_INFO ("-->API: Valor seteado correctamente");
  
  return true;
}

bool
ApiWifiNetDevice::GetInformation()
{
  // Metodo para obtener todos los valores. 
  // Path base para obtener los atributos
  std::ostringstream ospathbase;
  ospathbase << "/NodeList/" << nodeId
    << "/DeviceList/" << deviceId
    << "/$ns3::WifiNetDevice/";
  
  std::string path_tmp;
  std::string path_base_tmp;
  
  ns3::UintegerValue uintv;
  ns3::DoubleValue doublev;
  ns3::StringValue stringv;

  std::ostringstream osback;
  osback << "IEEE 802.11n 2.4 GHz\n";
  path_base_tmp = ospathbase.str();
  // Loop sobre los valores disponibles en wifinet
  for (auto &x:wifinetAllowedAttributes) {
    path_tmp = path_base_tmp + x.first;
    ns3::Config::Get(path_tmp, uintv);
    osback << std::string(x.first) << ": " << uintv.Get() << "\n";
  }
  
  path_base_tmp = ospathbase.str() + "Phy/";
  // Loop sobre los valores disponibles en wifiphy
  for (auto &x:wifiphyAllowedAttributes) {
    //Caso Especial con TxPower
    if (x.first.compare("TxPower")==0) {
      path_tmp = path_base_tmp + "TxPowerStart";
    }
    else{
      path_tmp = path_base_tmp + x.first;
    }
    // Busqueda normal
    if (x.second.compare("UintegerValue")==0) {
      ns3::Config::Get(path_tmp, uintv);
      osback << std::string(x.first) << ": " << uintv.Get() << "\n";
    }
    else {
      ns3::Config::Get(path_tmp, doublev);
      osback << std::string(x.first) << ": " << doublev.Get() << "\n";
    }
  }
  
  path_base_tmp = ospathbase.str() + 
    "RemoteStationManager/$ns3::ConstantRateWifiManager/";
  // Loop sobre los valores disponibles en constRate
  for (auto &x:constRateAllowedAttributes) {
    path_tmp = path_base_tmp + x.first;
    //NS_LOG_UNCOND ("TEST: "<<path_tmp);
    if (x.second.compare("UintegerValue")==0) {
      ns3::Config::Get(path_tmp, uintv);
      osback << std::string(x.first) << ": " << uintv.Get() << "\n";
    }
    else {
      ns3::Config::Get(path_tmp, stringv);
      osback << std::string(x.first) << ": " << stringv.Get() << "\n";
    }
  }
  
  NS_LOG_INFO ("--> API: prueba Get: "<<osback.str());
  
  SendBackMessage( osback.str().c_str() );
  return true;
}

void
ApiWifiNetDevice::SendBackMessage (const char * message)
{    
  // Creo un buffer local
  char buf[strlen(message)];
  strcpy(buf, message);
  int size = sizeof(buf)/sizeof(buf[0]);
  
  NS_LOG_INFO ("--> API: SendBackMessage: "<< message);

  // Creamos un paquete ns3 para aprovechar sus metodos de gestion de
  // cabeceras y memoria
  ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet> 
    (reinterpret_cast<const uint8_t *> (buf), size);
  
  // Cabeceras Ethernet, el src sera el dst y el dst sera el src.
  ns3::EthernetHeader header = ns3::EthernetHeader (false);
  header.SetSource(dst);
  header.SetDestination(src);
  header.SetLengthType (2064); // 0x0810 : Mi cabecera de configuracion
  
  // Se agregan las cabeceras
  packet->AddHeader (header);
  
  // Creo un buffer para copiar la data del paquete
  uint8_t * buffer = (uint8_t *)malloc(packet->GetSize());
  packet->CopyData(buffer, packet->GetSize());

  NS_LOG_INFO ("--> API: SendBackMessage packet size: " << packet->GetSize());

  uint32_t bytesWritten = write (sock, buffer, packet->GetSize());
  std::free(buffer);

  NS_LOG_DEBUG ("--> API: Get # Bytes Written: " << bytesWritten);

  return;
}

bool
ApiWifiNetDevice::ValidateIsNumber (char * value)
{
  return true;
  std::stringstream convertor;
  std::string numberString = std::string (value);
  int number;
  
  convertor << numberString;
  convertor >> number;

  if (convertor.fail())
  {
    // numberString is not a number!
    return false;
  }
  else 
  {
    return true;
  }
}

} // NAMESPACE GIRA
