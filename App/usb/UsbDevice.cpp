//
// Created by Capitang7 on 24/05/2026.
//

#include "UsbDevice.h"



UsbDevice::UsbDevice() {
  qDebug() << "UsbDevice constructor called";
  int rc = libusb_init(&context);
  assert(rc == LIBUSB_SUCCESS);
  libusb_set_option(context, LIBUSB_OPTION_USE_USBDK);
  libusb_set_debug(context, LIBUSB_LOG_LEVEL_INFO);


}

UsbDevice::~UsbDevice() {
  qDebug() << "UsbDevice destructor called";

  libusb_exit(context);
}

void UsbDevice::connect_device() {
  qDebug() << "UsbDevice connect_device called";

  libusb_device_handle *device = libusb_open_device_with_vid_pid(context, VID, PID);

  // struct libusb_config_descriptor *config;
  // int ret = libusb_get_active_config_descriptor(device, &config);

  auto data = new unsigned char[4];
  int read, counter = 0;
  if (libusb_has_capability (LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER) && libusb_kernel_driver_active(device, INTERFACE) == 1) { //find out if kernel driver is attached
    libusb_detach_kernel_driver(device, INTERFACE); //detach it
    qDebug() << "Detached";
  } else if (!libusb_has_capability (LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER)) {
    qDebug() << "Detaching kernel driver not supported on this platform";
  }


  libusb_claim_interface(device, INTERFACE); //claim interface 0 (the first) of device (desired device FX3 has only 1)
  qDebug() << "Claimed interface";

  while (libusb_interrupt_transfer(device, (ENDPOINT | LIBUSB_ENDPOINT_IN), data, sizeof(data), &read, 1000) == 0 && counter++ < 5) {
    qDebug() << "Data read: " << data[0] << " " << data[1] << " " << data[2] << " " << data[3];
  }

  libusb_release_interface(device, INTERFACE); //release the claimed interface
  delete[] data; //delete the allocated memory for data
  libusb_close(device); //close the device we opened

}

std::list<VendorProduct> UsbDevice::list_devices() {

  libusb_device **list = NULL;
  const ssize_t count = libusb_get_device_list(context, &list);
  assert(count > 0);

  int rc = 0;
  std::list<VendorProduct> devicesLocal;
  std::list<VendorProduct>::iterator it;
  for (size_t idx = 0; idx < count; ++idx) {
      libusb_device *device = list[idx];
      libusb_device_descriptor desc = {0};

      rc = libusb_get_device_descriptor(device, &desc);
      assert(rc == LIBUSB_SUCCESS);

      qDebug() << "DeviceClass: " << (int) desc.bDeviceClass ;
      qDebug()  << "IdVendor: "  << int_to_hex(desc.idVendor) << " IdProduct: " << int_to_hex(desc.idProduct);

      it = devicesLocal.begin();
      it ++;
      devicesLocal.insert(it, VendorProduct(desc.idVendor, desc.idProduct));

      libusb_config_descriptor *config;

      rc = libusb_get_active_config_descriptor(device, &config);
      assert(rc == LIBUSB_SUCCESS);

      print_configuration(config);

      libusb_free_config_descriptor(config);
  }

  libusb_free_device_list(list, 1);

  return devicesLocal;
}


void UsbDevice::print_configuration(libusb_config_descriptor *config) {

  qDebug() << "Configuration descriptor called";
  qDebug() <<"    wTotalLength:            " << config->wTotalLength;
  qDebug() <<"    bNumInterfaces:          " << config->bNumInterfaces;
  qDebug() <<"    bConfigurationValue:     " << config->bConfigurationValue;
  qDebug() <<"    iConfiguration:          " << config->iConfiguration;
  qDebug() <<"    bmAttributes:            " << config->bmAttributes;
  qDebug() <<"    MaxPower:                 " << config->MaxPower;

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
