
#ifndef CWKEYERAPP_CONFIGURATION_H
#define CWKEYERAPP_CONFIGURATION_H

#include <QFile>
#include <stdexcept>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

static constexpr const char* CONFIGURATION_FILE_NAME   = "configuration.json";
static constexpr const char* CFG_AMPLITUDE             = "amplitude";
static constexpr const char* CFG_FREQUENCY             = "frequency";
static constexpr const char* CFG_WPM                   = "wpm";
static constexpr const char* CFG_SELECTED_AUDIO_DEVICE = "selected_audio_device";
static constexpr const char* CFG_MODE                  = "mode";
static constexpr const char* CFG_COMM_OUT              = "commout";

class Configuration {

  public:

    Configuration();

    static void removeValue(std::string key);

    static void putObject(std::string key,  QJsonObject  object);
    static void putValueInt(std::string key,  int  value);
    static void putValueDouble(std::string key, double value);
    static void putValueBool(std::string key, bool value);
    static void putValueString(std::string key, std::string value);

    static QJsonObject * getValue(std::string key);
    static int getValueInt(std::string key);
    static double getValueDouble(std::string key);
    static bool getValueBool(std::string key);
    static std::string getValueString(std::string key);

  private:
    void load_configuration();

};


#endif //CWKEYERAPP_CONFIGURATION_H
