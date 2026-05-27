#include "Keyer.h"

Keyer::Keyer() {
  qDebug() << "Keyer constructor called";
}

void IDitDah::on_dit(bool pressed) {
  qDebug() << "DIT"<<pressed;
}
void IDitDah::on_dah(bool pressed) {
  qDebug() << "DAH" <<pressed;
}