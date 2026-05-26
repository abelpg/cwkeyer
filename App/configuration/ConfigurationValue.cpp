#include "ConfigurationValue.h"

#include <QObject>


ConfigurationValue::ConfigurationValue(const QJsonObject value) {
  this->value = new QJsonObject(value);
}

ConfigurationValue::~ConfigurationValue() {
  delete value;
}

