#ifndef CWKEYERAPP_DEVICE_H
#define CWKEYERAPP_DEVICE_H

/*
*TESTED_DEVICES = [
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 0, "endpoint": 0x81, "max_packet_size": 8}, # Vail
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 1, "endpoint": 0x82, "max_packet_size": 4}  # Left click/right
]
*/
#include <DeviceInterface.h>
#include <list>

class Device {
  public:
    int vendor_id{};
    int product_id{};
    DeviceInterface * device_interface = nullptr;

    std::list<DeviceInterface> *available_interfaces{};

    Device(int vendor_id, int product_id);
    Device(int vendor_id, int product_id, int interface, int endpoint, int packet_size);
    ~Device();
    void addInterface(int interface, int endpoint, int packetSize);

    friend bool operator< (const Device &left, const Device &right);

};


#endif //CWKEYERAPP_DEVICE_H
