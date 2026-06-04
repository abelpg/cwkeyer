#ifndef CWKEYERAPP_KEYBOARD_H
#define CWKEYERAPP_KEYBOARD_H


#include <QObject>
#ifdef _WIN32
#include <windows.h>
#endif

#include "../utils/Logger.h"
#include "../utils/IDitDah.h"

class Keyboard : public QObject, public IDitDah {
  Q_OBJECT
  public:
    explicit Keyboard(QObject *parent = nullptr);
    ~Keyboard();

    void onDit(bool pressed) override;
    void onDah(bool pressed) override;
    void onStraight(bool pressed) override;

    bool enabled() const { return m_enabled; };
    void setEnabled(bool enabled);

  signals:
      void ditChanged(bool pressed);
      void dahChanged(bool pressed);

  private slots:
      void pressDit(bool pressed);
      void pressDah(bool pressed);
  private:
      bool m_enabled = false;
};

#endif //CWKEYERAPP_KEYBOARD_H
