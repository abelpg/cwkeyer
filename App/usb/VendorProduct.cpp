
#include "VendorProduct.h"

VendorProduct::VendorProduct(int vendor_id, int product_id)   {
  this->vendor_id = vendor_id;
  this->product_id = product_id;
}

bool operator< (const VendorProduct &left, const VendorProduct &right) {
  return left.vendor_id == right.vendor_id && left.product_id < right.product_id;
}
