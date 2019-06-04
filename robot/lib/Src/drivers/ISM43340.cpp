#include "drivers/ISM43340.hpp"


namespace ISMConstants {
    static const unsigned int RECEIVE_BUFF_SIZE = 1000;
    
    static const uint8_t CHIP_SELECT = 0;
    static const uint8_t CHIP_DESELECT = 1;
    
    static const uint8_t RESET_TURN_OFF = 0;
    static const uint8_t RESET_TURN_ON  = 1;

    static const unsigned int SPI_FREQ = 1'000'000;

    static const unsigned int RESET_DELAY = 500;
    static const unsigned int SPI_DELAY = 1;

    // RETURN VALUES (Not including error)
    static const std::string OK     = "OK\r\n> ";
    static const std::string PROMPT = "\r\n> ";
    static const uint8_t     NACK   = 0x15;

    // DELIMITERS
    static const std::string EVEN_DELIMITER = "\r\n";
    static const std::string ODD_DELIMITER  = "\r";

    // MISC COMMANDS
    static const uint8_t     READ           = 0x0A;
    static const std::string EMPTY_TRANSFER = "\n\n";
    static const std::string CMD_INVALID    = "FO";

    // GENERAL
    static const std::string CMD_PRINT_HELP           = "?";
    static const std::string CMD_SET_HUMAN_READABLE   = "$$$";
    static const std::string CMD_SET_MACHINE_READABLE = "---";

    // ACCESS POINT
    static const std::string CMD_SHOW_ACCESS_POINT_SETTINGS                = "A?";
    static const std::string CMD_ACTIVATE_ACCESS_POINT                     = "A0";
    static const std::string CMD_SET_ACCESS_POINT_SECURITY_MODE            = "A1";
    static const std::string CMD_SET_SECURITY_KEY                          = "A2=";
    static const std::string CMD_GET_AP_DHCP_CACHED_ADDRESS                = "AA=";
    static const std::string CMD_SET_ACCESS_POINT_CHANNEL                  = "AC=";
    static const std::string CMD_ACTIVATE_ACCESS_POINT_DIRECT_CONNECT_MODE = "AD";
    static const std::string CMD_EXIT_ACCESS_POINT_DIRECT_CONNECT_MODE     = "AE";
    static const std::string CMD_SET_ACCESS_POINT_DHCP_LEASE_TIME          = "AL=";
    static const std::string CMD_GET_CLIENT_RSSI                           = "AR";
    static const std::string CMD_SET_ACCESS_POINT_SSID                     = "AS=";
    static const std::string CMD_SET_MAXIMUM_NUMBER_AP_CLIENTS             = "AT=";

    // SELECT COMMUNICATION INTERFACE
    static const std::string CMD_SHOW_COMMUNICATION_INTERFACE_SETTINGS = "B?";
    static const std::string CMD_SET_SPI_MODE                          = "B2=";
    static const std::string CMD_SET_SPI_READY_PIN                     = "B3=";

    // CONFIGURE NETWORK SETTINGS
    static const std::string CMD_JOIN_NETWORK                     = "C0";
    static const std::string CMD_SET_NETWORK_SSID                 = "C1=";
    static const std::string CMD_SET_NETWORK_PASSWORD             = "C2=";
    static const std::string CMD_SET_NETWORK_SECURITY_TYPE        = "C3=";
    static const std::string CMD_SET_NETWORK_DHCP                 = "C4=";
    static const std::string CMD_SET_NETWORK_IP_VERSION           = "C5=";
    static const std::string CMD_SET_NETWORK_IP_ADDRESS           = "C6=";
    static const std::string CMD_SET_NETWORK_IP_MASK              = "C7=";
    static const std::string CMD_SET_NETWORK_GATEWAY              = "C8=";
    static const std::string CMD_SET_NETWORK_PRIMARY_DNS          = "C9=";
    static const std::string CMD_SET_NETWORK_SECONDARY_DNS        = "CA=";
    static const std::string CMD_SET_NETWORK_JOIN_RETRY_COUNT     = "CB=";
    static const std::string CMD_NETWORK_AUTO_CONNECT             = "CC=";
    static const std::string CMD_DISCONNECT_NETWORK               = "CD";
    static const std::string CMD_SET_AUTHORIZATION_TYPE           = "CE=";
    static const std::string CMD_JOIN_LEAVE_IGMP_GROUP            = "CJ=";
    static const std::string CMD_SET_COUNTRY_CODE                 = "CN=";
    static const std::string CMD_GET_COUNTRY_CODE                 = "CN";
    static const std::string CMD_GET_RSSI_ASSOCIATED_ACCESS_POINT = "CR";
    static const std::string CMD_CONNECTION_STATUS                = "CS";
    static const std::string CMD_GET_CONNECTED_RATE               = "CV";
    static const std::string CMD_SET_WPS_PUSH_BUTTON              = "CT=";
    static const std::string CMD_CONNECT_WPS_USING_PIN_PBC        = "CW=";
    static const std::string CMD_GET_CONNECTION_INFO              = "C?";

    namespace TYPE_NETWORK_SECURITY {
        static const std::string OPEN      = "0";
        static const std::string WEP       = "1";
        static const std::string WPA       = "2";
        static const std::string WPA2_AES  = "3";
        static const std::string WPA_WPA2  = "4";
        static const std::string WPA2_TKIP = "5";
    };

    namespace TYPE_NETWORK_DHCP {
        static const std::string DISABLED = "0";
        static const std::string ENABLED  = "1";
    };

    namespace TYPE_NETWORK_IP_VERSION {
        static const std::string IPV4 = "0";
        static const std::string IPV6 = "1";
    };

    namespace TYPE_NETWORK_AUTO_CONNECT {
        static const std::string DISABLED            = "0";
        static const std::string AUTO_JOIN           = "1";
        static const std::string AUTO_RECONNECT      = "2";
        static const std::string AUTO_JOIN_RECONNECT = "3";
    };

    namespace TYPE_AUTHORIZATION {
        static const std::string OPEN   = "0";
        static const std::string SHARED = "1";
    };

    namespace TYPE_IGMP_GROUP {
        static const std::string LEAVE = "0";
        static const std::string JOIN  = "1";
    };

    namespace TYPE_WPS_PIN_PBC {
        static const std::string PIN = "0";
        static const std::string PBC = "1";
    };

    // DNS COMMANDS
    static const std::string CMD_DNS_LOOKUP          = "D0=";
    static const std::string CMD_SET_MDNS_STATE_NAME = "D1=";
    static const std::string CMD_SET_MDNS_SERVICES   = "D2=";

    // SCAN FOR NETWORK ACCESS POINTS
    static const std::string CMD_SCAN_ACCESS_POINTS    = "F0";
    static const std::string CMD_SET_SCAN_REPEAT_COUNT = "F1=";
    static const std::string CMD_SET_SCAN_DELAY        = "F2=";
    static const std::string CMD_SET_SCAN_CHANNEL      = "F3=";
    static const std::string CMD_SET_SCAN_BSSID        = "F4=";
    static const std::string CMD_SET_SCAN_SSID         = "F5=";
    static const std::string CMD_SHOW_SCAN_SETTINGS    = "F?";

    // GPIO / ADC
    static const std::string CMD_READ_GPIO_ADC      = "G2=";
    static const std::string CMD_WRITE_GPIO         = "G3=";
    static const std::string CMD_GPIO_SETUP         = "G4=";
    static const std::string CMD_GET_UTC_TIME       = "GT";
    static const std::string CMD_SHOW_GPIO_SETTINGS = "G?";

    namespace TYPE_READ_GPIO_PIN {
        static const std::string PIN_BUTTON     = "2";
        static const std::string PIN_DIGITAL_IN = "3";
        static const std::string PIN_ADC        = "5";
    };

    namespace TYPE_WRITE_GPIO_PIN {
        static const std::string PIN_LED = "1";
        static const std::string PIN_DIGITAL_OUT = "4";
    };

    namespace TYPE_WRITE_GPIO_VALUE {
        static const std::string LOW = "0";
        static const std::string HIGH = "1";
    };

    namespace TYPE_GPIO_SETUP {
        static const std::string PIN_LED         = "1";
        static const std::string PIN_BUTTON      = "2";
        static const std::string PIN_DIGITAL_IN  = "3";
        static const std::string PIN_DIGITAL_OUT = "4";
        static const std::string PIN_ADC         = "5";
    };

    // SOFTWARE AND CONFIGURATION INFORMATION
    static const std::string CMD_IS_ENDPOINT_CONFIGURED       = "IC=";
    static const std::string CMD_SHOW_APPLICATION_INFORMATION = "I?";

    // MISCELLANEOUS COMMANDS
    static const std::string CMD_TEST_EXTERNAL_SERIAL_FLASH  = "MF";
    static const std::string CMD_MANUFACTURING_TEST          = "MJ";
    static const std::string CMD_MANUFACTURING_TEST_CUSTOM   = "MJ=";
    static const std::string CMD_MESSAGE_READ                = "MR";
    static const std::string CMD_SUPPRESS_ASYNC_MESSAGE_DCHP = "MS=";
    static const std::string CMD_SET_MESAGE_TYPE             = "MT=";

    namespace TYPE_SUPPRESS_ASYNC {
        static const std::string DISABLED = "0";
        static const std::string SUPPRESS = "1";
    };

    namespace TYPE_MESSAGE_TYPE {
        static const std::string DISABLED = "0";
        static const std::string SIMPLE   = "1";
    };

    // TRANSPORT COMMUNICATION
    static const std::string CMD_GET_COMMUNICATION_SOCKET             = "P0";
    static const std::string CMD_SET_COMMUNICATION_SOCKET             = "P0=";
    static const std::string CMD_SET_TRANSPORT_PROTOCOL               = "P1=";
    static const std::string CMD_SET_TRANSPORT_LOCAL_PORT_NUMBER      = "P2=";
    static const std::string CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS = "P3=";
    static const std::string CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER     = "P4=";
    static const std::string CMD_START_STOP_TRANSPORT_SERVER          = "P5=";
    static const std::string CMD_START_STOP_TRANSPORT_CLIENT          = "P6=";
    static const std::string CMD_START_STOP_REQUEST_TCP_LOOP          = "P7=";
    static const std::string CMD_SET_LISTEN_BACKLOGS                  = "P8=";
    static const std::string CMD_SSL_CERTIFICATION_VERIFICATION_LEVEL = "P9=";
    static const std::string CMD_SET_CUSTOM_CERTIFICATE_AUTHORITY     = "PA=";
    static const std::string CMD_SET_ROOT_CA_VERIFICATION_RESULTS     = "PB=";
    static const std::string CMD_WRITE_SECURITY_CERTIFICATES          = "PC=";
    static const std::string CMD_WRITE_SECURITY_KEY                   = "PD=";
    static const std::string CMD_GET_CERTIFICATE_SET_AVAILABILITY     = "PE";
    static const std::string CMD_SET_CERTIFICATE_SET_AVAILABILITY     = "PE=";
    static const std::string CMD_SET_ACTIVE_CERTIFICATE               = "PF=";
    static const std::string CMD_PROGRAM_CA_CERTIFICATE_KEY           = "PG=";
    static const std::string CMD_TCP_KEEP_ALIVE                       = "PK=";
    static const std::string CMD_GET_MQTT_ATTRIBUTES                  = "PM";
    static const std::string CMD_SET_MQTT_ATTRIBUTES                  = "PM=";
    static const std::string CMD_SET_TCP_API_MESSAGE_TIMEOUT          = "PY=";
    static const std::string CMD_SET_TCP_STREAMING_MODE               = "PX=";
    static const std::string CMD_SHOW_TRANSPORT_SETTINGS              = "P?";

    namespace TYPE_TRANSPORT_PROTOCOL {
        static const std::string TCP_ENABLED      = "0";
        static const std::string UDP_ENABLED      = "1";
        static const std::string UDP_LITE_ENABLED = "2";
        static const std::string TCP_SSL          = "3";
        static const std::string MQTT             = "4";
    };

    namespace TYPE_TRANSPORT_SERVER_MODE {
        static const std::string SERVER_DISABLE            = "0";
        static const std::string SERVER_ENABLE             = "1";
        static const std::string MULTI_ACCEPT_CLOSE_SOCKET = "10";
        static const std::string MULTI_ACCEPT_ENABLE       = "11";
    };

    namespace TYPE_TRANSPORT_CLIENT {
        static const std::string DISABLE = "0";
        static const std::string ENABLE  = "1";
    };

    namespace TYPE_REQUEST_TCP_LOOP {
        static const std::string STOP            = "0";
        static const std::string START           = "1";
        static const std::string CLOSE_SOCKET    = "2";
        static const std::string NEXT_CONNECTION = "3";
    };

    // RECEIVE TRANSPORT DATA
    static const std::string CMD_READ_TRANSPORT_DATA            = "R0";
    static const std::string CMD_SET_READ_TRANSPORT_PACKET_SIZE = "R1=";
    static const std::string CMD_SET_READ_TRANSPORT_TIMEOUT     = "R2=";
    static const std::string CMD_SET_RECEIVE_MODE               = "R3=";
    static const std::string CMD_SHOW_READ_TRANSPORT_SETTINGS   = "R?";

    namespace TYPE_RECEIVE_MODE {
        static const std::string NORMAL        = "0";
        static const std::string NO_DELIEATION = "1";
    };

    // WRITE TRANSPORT DATA
    static const std::string CMD_WRITE_TRANSPORT_DATA            = "S0";
    static const std::string CMD_SET_WRITE_TRANSPORT_PACKET_SIZE = "S1=";
    static const std::string CMD_SET_WRITE_TRANSPORT_TIMEOUT     = "S2=";
    static const std::string CMD_WRITE_TRANSPORT_DATA_INLINE     = "S3=";
    static const std::string CMD_SPI_FLASH_CS_PIN                = "SF=";
    static const std::string CMD_SHOW_WRITE_TRANSPORT_SETTINGS   = "S?";

    // PING IP TARGET ADDRESS
    static const std::string CMD_PING_TARGET_ADDRESS     = "T0";
    static const std::string CMD_SET_PING_TARGET_ADDRESS = "T1=";
    static const std::string CMD_SET_PING_REPEAT_COUNT   = "T2=";
    static const std::string CMD_SET_PING_DELAY          = "T3=";
    static const std::string CMD_SHOW_PING_SETTINGS      = "T?";

    // WLAN
    static const std::string CMD_SET_GPIO_LINK_STATUS_ACTIVITY = "WL=";

    // SYSTEM INFORMATION FLASH
    static const std::string CMD_RESET_FACTORY               = "Z0";
    static const std::string CMD_SAVE_SETTINGS               = "Z1";
    static const std::string CMD_CLEAR_SAVED_SETTINGS        = "Z2";
    static const std::string CMD_SET_FACTORY_USER_SPACE      = "Z3=";
    static const std::string CMD_SET_MAC_ADDRESS             = "Z4=";
    static const std::string CMD_GET_MAC_ADDRESS             = "Z5";
    static const std::string CMD_SET_ACCESS_POINT_IP_ADDRESS = "Z6=";
    static const std::string CMD_SET_WPS_PIN                 = "Z7=";
    static const std::string CMD_GET_WPS_PIN                 = "Z8";
    static const std::string CMD_CLEAR_FACTORY_LOCK_SWITCH   = "ZC=";
    static const std::string CMD_FLASH_DUMP                  = "ZD";
    static const std::string CMD_SET_FACTORY_LOCK_SWITCH     = "ZF=";
    static const std::string CMD_SET_PRODUCT_NAME            = "ZN=";
    static const std::string CMD_OTA_FIRMWARE_UPDATE         = "ZO=";
    static const std::string CMD_POWER_MANAGEMENT            = "ZP=";
    static const std::string CMD_RESET_MODULE                = "ZR";
    static const std::string CMD_GET_SERIAL_NUMBER           = "ZS";
    static const std::string CMD_SET_SERIAL_NUMBER           = "ZT=";
    static const std::string CMD_FIRMWARE_UPDATE             = "ZU";
    static const std::string CMD_SET_OTA_METHOD              = "ZV=";
    static const std::string CMD_SHOW_SYSTEM_SETTINGS        = "Z?";
};

ISM43340::ISM43340(std::shared_ptr<SPI> radioSPI, PinName nCsPin, PinName nResetPin,
                   PinName dataReadyPin)
    : radioSPI(radioSPI),
      nCs(nCsPin),
      nReset(nResetPin),
      dataReady(dataReadyPin) {
    nCs = ISMConstants::CHIP_DESELECT;

    reset();
}

int ISM43340::writeToSpi(uint8_t* command, int length) {
    HAL_Delay(ISMConstants::SPI_DELAY);
    while (dataReady.read() != 1);
    HAL_Delay(ISMConstants::SPI_DELAY);

    nCs = ISMConstants::CHIP_SELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    for (int i = 0; i < length; i += 2) {
        uint8_t c1 = command[i];
        uint8_t c2 = 0;
        if (i + 1 < length) {
            c2 = command[i + 1];
        }

        // Swap endianess
        radioSPI->transmit(c2);
        radioSPI->transmit(c1);
    }

    HAL_Delay(ISMConstants::SPI_DELAY);
    nCs = ISMConstants::CHIP_DESELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    return 0;
}

uint32_t ISM43340::readFromSpi() {
    unsigned int bytesRead = 0;

    readBuffer.clear();

    HAL_Delay(ISMConstants::SPI_DELAY);
    while (dataReady.read() != 1);
    HAL_Delay(ISMConstants::SPI_DELAY);
    
    nCs = ISMConstants::CHIP_SELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    while (dataReady.read() == 1) {
        uint8_t data1 = radioSPI->transmitReceive(ISMConstants::READ);
        uint8_t data2 = radioSPI->transmitReceive(ISMConstants::READ);

        // Read only if both bytes are NACK's and
        // the receive buffer isn't full
        if (!(data1 == ISMConstants::NACK &&
              data2 == ISMConstants::NACK) &&
            readBuffer.size() < ISMConstants::RECEIVE_BUFF_SIZE) {

            // Swap endianess so readBuff is correct order
            readBuffer.push_back(data2);
            readBuffer.push_back(data1);
            bytesRead += 2;
        }
    }

    HAL_Delay(ISMConstants::SPI_DELAY);
    nCs = ISMConstants::CHIP_DESELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    return bytesRead;
}

void ISM43340::sendCommand(std::string command, std::string arg) {
    // Add delimiter and keep command of even length
    if (command.compare("S3=") != 0) {
        command = command + arg;
        if (command.length() % 2 == 0) {
            command += ISMConstants::EVEN_DELIMITER;
        } else {
            command += ISMConstants::ODD_DELIMITER;
        }
    } else {
        command = command + arg;
    }

    writeToSpi((uint8_t*)command.data(), command.length());
    readFromSpi();
}

int32_t ISM43340::sendPacket(const rtp::Packet* pkt) {
    // Return failure if not initialized
    if (!isInit) return -1;

    std::vector<uint8_t> txBuffer;

    // Transmit using socket #1
    // Could increase efficiency by tracking if this needs to be set
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, "1");

    // Reserve memory for AT command header, packet, and delimiter
    int bufferSize = 5 + pkt->size();
    txBuffer.reserve(bufferSize);

    const auto headerFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto headerLastPtr = headerFirstPtr + rtp::HeaderSize;

    const auto cmdFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto cmdLastPtr = headerFirstPtr + rtp::HeaderSize;

    // Insert ISM AT string and payload length
    txBuffer.insert(txBuffer.end(),
                    ISMConstants::CMD_WRITE_TRANSPORT_DATA_INLINE.begin(),
                    ISMConstants::CMD_WRITE_TRANSPORT_DATA_INLINE.end());

    char size[5];
    sprintf(size, "%d", pkt->size());

    std::string sizeButAString = size;

    txBuffer.insert(txBuffer.end(), sizeButAString.begin(),
                    sizeButAString.end());
    // Insert \r
    txBuffer.insert(txBuffer.end(), ISMConstants::ODD_DELIMITER.begin(),
                    ISMConstants::ODD_DELIMITER.end());

    // insert the rtp header
    txBuffer.insert(txBuffer.end(), headerFirstPtr, headerLastPtr);
    // insert the rtp payload
    txBuffer.insert(txBuffer.end(), pkt->payload.begin(), pkt->payload.end());

    printf("preparing to send %d bytes\r\n", txBuffer.size());

    for (int i = 0; i < txBuffer.size(); i++) {
        char c = txBuffer[i];
        if (c == '\r') {
            printf("\\r");
        } else if (c == '\n') {
            printf("\\n");
        } else {
            printf("%c", c);
        }
    }
    printf("\r\n");

    writeToSpi(txBuffer.data(), txBuffer.size());
    readFromSpi();

    return 0;
}

std::vector<uint8_t> ISM43340::getData() {
    // Receive using socket #0
    // Could increase efficiency by tracking if this needs to be set
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, "0");

    // Should add some sort of check to see if data is ready to be read here
    sendCommand(ISMConstants::CMD_READ_TRANSPORT_DATA);

    return readBuffer;
}

int32_t ISM43340::testPrint() {
    for (int i = 0; i < readBuffer.size(); i++) {
        printf("%c", (char) readBuffer[i]);
        HAL_Delay(10);
    }
    printf("\r\n");
    HAL_Delay(50);
    return 0;
}

int32_t ISM43340::selfTest() {
    // I don't really have anything for this right now
    sendCommand(ISMConstants::CMD_SET_HUMAN_READABLE);

    sendCommand(ISMConstants::CMD_GET_CONNECTION_INFO);

    // This isn't a good measure of correctness
    isInit = readBuffer.size() > 0;

    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);

    if (isInit) {
        return 0;
    }

    return -1;
}

void ISM43340::pingRouter() {
    sendCommand(ISMConstants::CMD_SET_HUMAN_READABLE);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_PING_TARGET_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_PING_TARGET_ADDRESS, "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_PING_DELAY\r\n");
    sendCommand(ISMConstants::CMD_SET_PING_DELAY, "10");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_PING_TARGET_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_PING_REPEAT_COUNT, "10");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_PING_TARGET_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_PING_TARGET_ADDRESS);

    testPrint();

    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);
}

// Essentially the init function
void ISM43340::reset() {
    LOG(LOG_INFO, LOG_RADIO, "Resetting\r\n");

    nReset = ISMConstants::RESET_TURN_OFF;
    HAL_Delay(ISMConstants::RESET_DELAY);
    nReset = ISMConstants::RESET_TURN_ON;
    HAL_Delay(ISMConstants::RESET_DELAY);

    radioSPI->frequency(ISMConstants::SPI_FREQ);

    // Wait for device to turn on
    int i = 0;
    while (dataReady.read() != 1 && i < 100) {
        HAL_Delay(10);
        i++;
    }

    isInit = !dataReady.read();

    if (isInit) {
        LOG(LOG_ERRO, LOG_RADIO, "Could not initialize radio\r\n");
        return;
    }

    // Get the first prompt
    readFromSpi();

    // Send invalid command
    // This is needed to get the device to connect to a network???
    //LOG(LOG_INFO, LOG_RADIO, "CMD_INVALID\r\n");
    sendCommand(ISMConstants::CMD_INVALID);

    sendCommand(ISMConstants::CMD_POWER_MANAGEMENT, "0");


    // Configure Network

    // Disconnect from a network if power doesn't toggle
    //LOG(LOG_INFO, LOG_RADIO, "CMD_DISCONNECT_NETWORK\r\n");
    sendCommand(ISMConstants::CMD_DISCONNECT_NETWORK);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_SSID\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_SSID,
                "rj-rc-field");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_PASSWORD\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_PASSWORD,
                "r0b0jackets");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_SECURITY_TYPE\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_SECURITY_TYPE,
                ISMConstants::TYPE_NETWORK_SECURITY::WPA2_AES);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_DHCP\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_DHCP,
                ISMConstants::TYPE_NETWORK_DHCP::ENABLED);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_IP_VERSION\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_IP_VERSION,
                ISMConstants::TYPE_NETWORK_IP_VERSION::IPV4);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_GATEWAY\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_GATEWAY,
                "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_PRIMARY_DNS\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_PRIMARY_DNS,
                "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_SECONDARY_DNS\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_SECONDARY_DNS,
                "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_JOIN_RETRY_COUNT\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_JOIN_RETRY_COUNT,
                "3");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_NETWORK_AUTO_CONNECT\r\n");
    sendCommand(ISMConstants::CMD_NETWORK_AUTO_CONNECT,
                ISMConstants::TYPE_NETWORK_AUTO_CONNECT::AUTO_JOIN_RECONNECT);

    // Connect to network
    //LOG(LOG_INFO, LOG_RADIO, "CMD_JOIN_NETWORK\r\n");
    sendCommand(ISMConstants::CMD_JOIN_NETWORK);

    if (readBuffer.size() == 0 || (int)readBuffer[0] == 0) {
        // Failed to connect to network
        // not sure what to have it do here

        LOG(LOG_INFO, LOG_RADIO, "Failed to connect to network\r\n");
        return;
    }

    // Port initialization
    // UDP receive
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_COMMUNICATION_SOCKET\r\n");
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET,
                "0");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_PROTOCOL\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                "172.16.1.22");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                "25566");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_START_STOP_TRANSPORT_CLIENT\r\n");
    sendCommand(ISMConstants::CMD_START_STOP_TRANSPORT_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    // UDP send
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_COMMUNICATION_SOCKET\r\n");
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, "1");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_PROTOCOL\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                "172.16.1.22");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                "25565");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_START_STOP_TRANSPORT_CLIENT\r\n");
    sendCommand(ISMConstants::CMD_START_STOP_TRANSPORT_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    LOG(LOG_INFO, LOG_RADIO, "Radio initialized\r\n");
}
