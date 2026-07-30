/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#ifndef CWKEYERAPP_IDITDAH_H
#define CWKEYERAPP_IDITDAH_H

enum KeyType {
  AUTOMATIC=0x1,
  MANUAL=0x2
};

class IDitDah {
  public:
    virtual ~IDitDah() = default;

    /**Dit dah from automatic keys**/
    virtual void onDah(bool pressed) = 0;
    virtual void onDit(bool pressed) = 0;
    virtual void onStraight(bool pressed) = 0;

};


#endif //CWKEYERAPP_IDITDAH_H

