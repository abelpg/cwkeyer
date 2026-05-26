
#ifndef CWKEYERAPP_CONFIGURATION_H
#define CWKEYERAPP_CONFIGURATION_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

#include "ConfigurationValue.h"


class Configuration {

  public:

    Configuration();

    static void removeValue(std::string key);

    static void putObject(std::string key,  QJsonObject  object);

    static  ConfigurationValue* getValue(std::string key);


  private:
    inline static const auto CONFIGURATION_FILE_NAME = QStringLiteral("configuration.json");
    static QJsonObject *open_file_to_write(std::string key, QFile * file);

};


#endif //CWKEYERAPP_CONFIGURATION_H
