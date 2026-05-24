//
// Created by Capitang7 on 24/05/2026.
//

#include "UsbDevice.h"

UsbDevice::UsbDevice() {
  qDebug() << "UsbDevice constructor called";
}

UsbDevice::~UsbDevice() {
  qDebug() << "UsbDevice destructor called";
}

void UsbDevice::list_devices() {
  libusb_device **devs;
  int r = libusb_init_context(/*ctx=*/NULL, /*options=*/NULL, /*num_options=*/0);
  if (r >= 0) {
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt >= 0) {
      libusb_exit(NULL);


      libusb_free_device_list(devs, 1);

      libusb_exit(NULL);
    }
  }
}

void UsbDevice::print_devs(libusb_device **devs, int verbose) {
  libusb_device *dev;
  int i = 0, j = 0;
  uint8_t path[8];
  char string_buffer[LIBUSB_DEVICE_STRING_BYTES_MAX];

  while ((dev = devs[i++]) != NULL) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      fprintf(stderr, "failed to get device descriptor");
      return;
    }

    printf("%04x:%04x (bus %d, device %d)",
      desc.idVendor, desc.idProduct,
      libusb_get_bus_number(dev), libusb_get_device_address(dev));

    r = libusb_get_port_numbers(dev, path, sizeof(path));
    if (r > 0) {
      printf(" path: %d", path[0]);
      for (j = 1; j < r; j++)
        printf(".%d", path[j]);
    }

    if (verbose) {
      r = libusb_get_device_string(dev, LIBUSB_DEVICE_STRING_MANUFACTURER,
        string_buffer, sizeof(string_buffer));
      if (r >= 0) {
        printf("\n    manufacturer = %s", string_buffer);
      }

      r = libusb_get_device_string(dev, LIBUSB_DEVICE_STRING_PRODUCT,
        string_buffer, sizeof(string_buffer));
      if (r >= 0) {
        printf("\n    product = %s", string_buffer);
      }

      r = libusb_get_device_string(dev, LIBUSB_DEVICE_STRING_SERIAL_NUMBER,
        string_buffer, sizeof(string_buffer));
      if (r >= 0) {
        printf("\n    serial_number = %s", string_buffer);
      }
    }
    printf("\n");
  }
}
