
#ifndef CWKEYERAPP_CONFIGURATION_H
#define CWKEYERAPP_CONFIGURATION_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>



class Configuration {

  public:
    static Configuration getInstance();
    ~Configuration();

    void putObject(std::string key,  QJsonObject  object);

    QJsonObject* getObject(std::string key);

  private:
    inline static const auto CONFIGURATION_FILE_NAME = QStringLiteral("configuration.json");

    Configuration();

    static Configuration* instance;

    QJsonObject* jsonObject;

};


#endif //CWKEYERAPP_CONFIGURATION_H
