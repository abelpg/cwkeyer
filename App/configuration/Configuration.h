
#ifndef CWKEYERAPP_CONFIGURATION_H
#define CWKEYERAPP_CONFIGURATION_H

#include <QFile>
#include <stdexcept>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

class Configuration {

  public:

    Configuration();

    static void removeValue(std::string key);

    static void putObject(std::string key,  QJsonObject  object);
    static void putValueInt(std::string key,  int  value);
    static void putValueDouble(std::string key, double value);
    static void putValueBool(std::string key, bool value);

    static QJsonObject * getValue(std::string key);
    static int getValueInt(std::string key);
    static double getValueDouble(std::string key);
    static bool getValueBool(std::string key);

  private:
    inline static const auto CONFIGURATION_FILE_NAME = QStringLiteral("configuration.json");

};


#endif //CWKEYERAPP_CONFIGURATION_H
