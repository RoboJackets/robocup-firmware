#include <mbed.h>
#include <USBHID.h>

class PCLink {
public:
    static const uint16_t RJ_VENDOR_ID = 0x524A;
    static const uint16_t RJ_PRODUCT_ID = 0x4253;
    static const uint16_t RJ_RELEASE = 0x0000;

    PCLink(uint16_t vendorID = RJ_VENDOR_ID, uint16_t productID = RJ_PRODUCT_ID,
           uint16_t release = RJ_RELEASE);

    template <class ARRAY_TYPE>
    void send(const ARRAY_TYPE& array) {
        ASSERT(array.size() <= MAX_HID_REPORT_SIZE);
        _out.length = array.size();
        memcpy(&_out.data, array.data(), array.size());
        _usbLink.sendNB(&_out);
    }

    void setSerialDebugging(Serial* pc);
    void setLed(DigitalOut led);
    void read();

private:
    Serial* _pc;
    DigitalOut _led;
    USBHID _usbLink;
    HID_REPORT _in;
    HID_REPORT _out;
};
