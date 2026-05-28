
#include "Configuration.h"

#include <qjsondocument.h>


Configuration::Configuration() {
}

// Helper interno: lee el JSON completo del archivo (sin truncar)
static QJsonObject readFullJson(QFile& file) {
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isObject()) {
      return doc.object();
    }
  }
  return QJsonObject{};
}

static void writeFullJson(QFile& file, const QJsonObject& root) {
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    throw std::runtime_error("Could not open file for writing");
  }
  file.write(QJsonDocument(root).toJson());
  file.close();
}


QJsonObject * Configuration::getValue(std::string key) {
  QJsonObject* object = nullptr;
  QFile file(CONFIGURATION_FILE_NAME);
  if (!file.exists()) {
    return nullptr;
  }

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw std::runtime_error("Could not open file");
  }

  QJsonDocument document = QJsonDocument::fromJson(file.readAll());
  file.close();

  QString keyName = QString::fromStdString(key);
  if (document.object().contains(keyName)) {
    object = new QJsonObject(document.object().value(keyName).toObject());
  }

  return object;
}

int Configuration::getValueInt(std::string key) {
  QFile file(CONFIGURATION_FILE_NAME);
  if (!file.exists()) {
    return 0;
  }
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw std::runtime_error("Could not open file");
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  QString k = QString::fromStdString(key);
  if (!doc.object().contains(k)) {
    return 0;
  }
  return doc.object().value(k).toInt();
}

double Configuration::getValueDouble(std::string key) {
  QFile file(CONFIGURATION_FILE_NAME);
  if (!file.exists()) return 0.0;
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    throw std::runtime_error("Could not open file");

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  QString k = QString::fromStdString(key);
  if (!doc.object().contains(k)) return 0.0;
  return doc.object().value(k).toDouble();
}

bool Configuration::getValueBool(std::string key) {
  QFile file(CONFIGURATION_FILE_NAME);
  if (!file.exists()) return false;
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    throw std::runtime_error("Could not open file");

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  QString k = QString::fromStdString(key);
  if (!doc.object().contains(k)) return false;
  return doc.object().value(k).toBool();
}

std::string Configuration::getValueString(std::string key) {
  QFile file(CONFIGURATION_FILE_NAME);
  if (!file.exists()) return nullptr;
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    throw std::runtime_error("Could not open file");

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  QString k = QString::fromStdString(key);
  if (!doc.object().contains(k)) return nullptr;
  return doc.object().value(k).toString().toStdString();
}

void Configuration::putObject(std::string key, QJsonObject value) {
  QFile file(CONFIGURATION_FILE_NAME);
  QJsonObject root = readFullJson(file);
  root[QString::fromStdString(key)] = value;
  writeFullJson(file, root);
}

void Configuration::putValueInt(std::string key, int value) {
  QFile file(CONFIGURATION_FILE_NAME);
  QJsonObject root = readFullJson(file);
  root[QString::fromStdString(key)] = value;
  writeFullJson(file, root);
}

void Configuration::putValueDouble(std::string key, double value) {
  QFile file(CONFIGURATION_FILE_NAME);
  QJsonObject root = readFullJson(file);
  root[QString::fromStdString(key)] = value;
  writeFullJson(file, root);
}

void Configuration::putValueBool(std::string key, bool value) {
  QFile file(CONFIGURATION_FILE_NAME);
  QJsonObject root = readFullJson(file);
  root[QString::fromStdString(key)] = value;
  writeFullJson(file, root);
}

void Configuration::putValueString(std::string key, std::string value) {
  QFile file(CONFIGURATION_FILE_NAME);
  QJsonObject root = readFullJson(file);
  root[QString::fromStdString(key)] = value.c_str();
  writeFullJson(file, root);
}

void Configuration::removeValue(std::string key) {
  QFile file(CONFIGURATION_FILE_NAME);
  QJsonObject root = readFullJson(file);
  root.remove(QString::fromStdString(key));
  writeFullJson(file, root);
}