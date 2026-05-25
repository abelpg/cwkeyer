
#ifndef CWKEYERAPP_CONFIGURATION_H
#define CWKEYERAPP_CONFIGURATION_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>


template<typename T>
struct TypeParseTraits;

#define REGISTER_PARSE_TYPE(X) template <> struct TypeParseTraits<X> \
{ static const char* name; } ; const char* TypeParseTraits<X>::name = #X;

REGISTER_PARSE_TYPE(int);
REGISTER_PARSE_TYPE(double);
REGISTER_PARSE_TYPE(std::string);

class Configuration {

  public:
    static Configuration getInstance();
    ~Configuration();

    template<typename T> void put(std::string key, T i);

    template<typename T> T get(std::string key);

  private:
    inline static const auto CONFIGURATION_FILE_NAME = QStringLiteral("configuration.json");

    Configuration();

    static Configuration *instance;

    QJsonObject * jsonObject;

};


#endif //CWKEYERAPP_CONFIGURATION_H
