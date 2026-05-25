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

template<typename T>
T Configuration::get(std::string key) {

  QString keyName = QString::fromStdString(key);

  if (jsonObject != nullptr && jsonObject->contains(keyName)) {
    QJsonValue value = jsonObject->value(keyName);
    if (value.isDouble()) {
      return static_cast<T>(value.toDouble());
    } else if (value.isString()) {
      return value.toString().toStdString();
    } else if (value.isBool()) {
      return value.toBool();
    }
  }

  return nullptr;

}

template<typename T>
void Configuration::put(std::string key, T i) {

  qDebug() <<  "Type id" << typeid(T).name();

  //
  // jsonObject[QString::fromStdString(key)] = QJsonValue::fromVariant(QVariant::fromValue(i));
  //
  // QFile file(CONFIGURATION_FILE_NAME);
  // if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
  //   throw std::runtime_error("Could not open file");
  // }
  //
  // QJsonDocument document(*jsonObject);
  // file.write( document.toJson());
  //
  // file.close();

}

Configuration::~Configuration() {
  delete jsonObject;
  delete instance;
}