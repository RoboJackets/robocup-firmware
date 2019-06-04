#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>
#include "rc-fshare/rtp.hpp"

#define LOG_DEBG "DEBG"
#define LOG_INFO "INFO"
#define LOG_WARN "WARN"
#define LOG_ERRO "ERRO"

#define LOG_MAIN  "MAIN"
#define LOG_RADIO "RADIO"

#define LOG(level, module, f_, ...) { \
        HAL_Delay(10); \
        printf("[%s] %s :: ", level, module); \
        fflush(stdout); \
        HAL_Delay(10); \
        printf((f_), ##__VA_ARGS__); \
        fflush(stdout); \
        HAL_Delay(10); \
    }

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
        static const std::string BUTTON     = "2";
        static const std::string DIGITAL_IN = "3";
        static const std::string ADC        = "5";
    };

    namespace TYPE_WRITE_GPIO_PIN {
        static const std::string LED = "1";
        static const std::string DIGITAL_OUT = "4";
    };

    namespace TYPE_WRITE_GPIO_VALUE {
        static const std::string LOW = "0";
        static const std::string HIGH = "1";
    };

    namespace TYPE_GPIO_SETUP {
        static const std::string LED         = "1";
        static const std::string BUTTON      = "2";
        static const std::string DIGITAL_IN  = "3";
        static const std::string DIGITAL_OUT = "4";
        static const std::string ADC         = "5";
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


class ISM43340 {
public:

    ISM43340(SPI radioSPI, PinName nCsPin, PinName nResetPin, PinName dataReadyPin);

    //I'm assuming we are still using RTP packets
    int32_t sendPacket(const rtp::Packet* pkt);

    std::vector<uint8_t> getData();

    std::vector<uint8_t> getReadBuffer() { return readBuffer; }

    void reset();

    int32_t selfTest();

    void pingRouter();

    bool isConnected() const { return isInit; }

    int writeToSpi(uint8_t* command, int length);

    //Printing isn't implemented  yet
    //int32_t testPrint();

    uint32_t readFromSpi();

    //I could add a status return to this but meh
    void sendCommand(std::string command, std::string arg = "");

private:

    SPI radioSPI;

    DigitalIn dataReady;
    DigitalOut nCs;

    DigitalOut nReset;

    std::vector<uint8_t> readBuffer;

    bool isInit = false;
};
