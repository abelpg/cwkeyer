#ifndef CWKEYERAPP_DEVICEINTERFACE_H
#define CWKEYERAPP_DEVICEINTERFACE_H


class DeviceInterface {

  public:
    int interface;
    int endpoint;
    int packetSize;

    DeviceInterface(int interface, int endpoint, int packetSize);

};


#endif //CWKEYERAPP_DEVICEINTERFACE_H
