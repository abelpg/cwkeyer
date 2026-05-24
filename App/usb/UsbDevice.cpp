//
// Created by Capitang7 on 24/05/2026.
//

#include "UsbDevice.h"

/*
*TESTED_DEVICES = [
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 0, "endpoint": 0x81, "max_packet_size": 8}, # Vail
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 1, "endpoint": 0x82, "max_packet_size": 4}  # Left click/right
]
 */
UsbDevice::UsbDevice() {
  qDebug() << "UsbDevice constructor called";
}

UsbDevice::~UsbDevice() {
  qDebug() << "UsbDevice destructor called";
}

void UsbDevice::connect_device() {

  qDebug() << "UsbDevice connect_device called";
}

void UsbDevice::list_devices() {
  libusb_context *context = NULL;
  libusb_device **list = NULL;
  int rc = 0;
  ssize_t count = 0;

  rc = libusb_init(&context);
  assert(rc == 0);

  count = libusb_get_device_list(context, &list);
  assert(count > 0);

  for (size_t idx = 0; idx < count; ++idx) {
      libusb_device *device = list[idx];
      libusb_device_descriptor desc = {0};

      rc = libusb_get_device_descriptor(device, &desc);
      assert(rc == 0);

      qDebug() << "DeviceClass: " << (int) desc.bDeviceClass ;
      qDebug()  << "IdVendor: "  << int_to_hex(desc.idVendor) << " IdProduct: " << int_to_hex(desc.idProduct);

      //printf("Vendor:Device = %04x:%04x\n", desc.idVendor, desc.idProduct);
  }

  libusb_free_device_list(list, 1);
  libusb_exit(context);
}

/**
 * Transforms a number to a hexadecimal representation
 * @tparam T With number
 * @param i with value
 * @return String with hex value.
 */
template< typename T > std::string UsbDevice::int_to_hex( T i ) {
  std::stringstream stream;
  stream << "0x"
         << std::setfill ('0') << std::setw(sizeof(T)*2)
         << std::hex << i;
  return stream.str();
}
