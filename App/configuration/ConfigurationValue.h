//
// Created by Capitang7 on 26/05/2026.
//

#ifndef CWKEYERAPP_CONFIGURATIONVALUE_H
#define CWKEYERAPP_CONFIGURATIONVALUE_H


#include <QJsonObject>

class ConfigurationValue {

  public:
    ConfigurationValue(QJsonObject value);
    QJsonObject * value;

  private:
    ~ConfigurationValue();

};


#endif //CWKEYERAPP_CONFIGURATIONVALUE_H
