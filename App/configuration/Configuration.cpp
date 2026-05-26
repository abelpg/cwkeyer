
#include "Configuration.h"

#include <qjsondocument.h>


Configuration::Configuration() {
}

 ConfigurationValue * Configuration::getValue(std::string key) {
  ConfigurationValue * configuration = nullptr;
  QFile file(CONFIGURATION_FILE_NAME);
  if (file.exists()) {
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      throw std::runtime_error("Could not open file");
    }
    try {
      QJsonDocument document = QJsonDocument::fromJson(file.readAll());
      QString keyName = QString::fromStdString(key);
      if (document.object().contains(keyName)) {
        configuration = new ConfigurationValue( document.object().value(keyName).toObject());
      }
    } catch (const std::exception &e) {
      throw std::runtime_error("Could not parse JSON");
    }
    file.close();
  }

  return configuration;

}
void Configuration::removeValue(std::string key) {
  QFile file(CONFIGURATION_FILE_NAME);

  QJsonObject *mainJson = open_file_to_write(key, &file);
  if (mainJson != nullptr) {
    QString keyName = QString::fromStdString(key);
    if (mainJson->contains(keyName)) {
      mainJson->remove(keyName);
    }

    file.write(QJsonDocument(*mainJson).toJson());
    file.close();
    delete mainJson;
  }

}


void Configuration::putObject(std::string key, QJsonObject value) {
    QFile file(CONFIGURATION_FILE_NAME);

    QJsonObject *mainJson = open_file_to_write(key, &file);
    if (mainJson == nullptr) {
      mainJson = new QJsonObject();
    }
    mainJson->operator[](QString::fromStdString(key)) = value;

    file.write(QJsonDocument(*mainJson).toJson());
    file.close();
    delete mainJson;

}


QJsonObject * Configuration::open_file_to_write(std::string key, QFile * file) {
  if (!file->open(QIODevice::ReadWrite | QIODevice::Text)) {
    throw std::runtime_error("Could not open file");
  }

  QJsonObject * mainJson = nullptr;
  if (file->exists()) {
    try {
      QJsonDocument document = QJsonDocument::fromJson(file->readAll());
      QString keyName = QString::fromStdString(key);
      if (document.object().contains(keyName)) {
        mainJson = new QJsonObject( document.object().value(keyName).toObject());
      }
    } catch (const std::exception &e) {
      throw std::runtime_error("Could not parse JSON");
    }
  }

  return mainJson;
}
