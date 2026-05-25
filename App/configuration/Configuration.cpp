//
// Created by Capitang7 on 25/05/2026.
//

#include "Configuration.h"

#include <qjsondocument.h>


Configuration::Configuration() {

  QFile file(CONFIGURATION_FILE_NAME);
  if (file.exists()) {
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      throw std::runtime_error("Could not open file");
    }

    try {
      QJsonDocument document = QJsonDocument::fromJson(file.readAll());
      jsonObject = new QJsonObject(document.object());

    } catch (const std::exception &e) {
      throw std::runtime_error("Could not parse JSON");
    }

    file.close();

  } else {
    jsonObject = new QJsonObject();
  }

}

Configuration Configuration::getInstance() {

  if (instance != nullptr) {
    return *instance;
  }

  instance = new Configuration();
  return *instance;
}

 QJsonObject * Configuration::getObject(std::string key) {

  QString keyName = QString::fromStdString(key);

  if (jsonObject != nullptr && jsonObject->contains(keyName)) {
    return new QJsonObject(jsonObject->value(keyName).toObject());
  }

  return nullptr;

}

void Configuration::putObject(std::string key, QJsonObject value) {
  jsonObject->operator[](QString::fromStdString(key)) = value;

}

Configuration::~Configuration() {
  delete jsonObject;
  delete instance;
}