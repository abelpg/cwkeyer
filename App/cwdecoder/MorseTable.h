#ifndef CWKEYERAPP_MORSETABLE_H
#define CWKEYERAPP_MORSETABLE_H

#include "../utils/IKeyerCW.h"
#include <unordered_map>
#include <string>

static constexpr char NOT_FOUND = '\0';
static constexpr char SK = '\1';
static constexpr char BK = '\2';
static constexpr char CW_ERROR = '\3';


/**
 * MorseTable
 *
 * Maps sequences of DIT ('.') and DAH ('-') to their decoded characters.
 * Covers ITU-R M.1677 International Morse Code:
 *   letters A-Z, digits 0-9 and common punctuation marks.
 *
 * Usage:
 *   std::string seq;
 *   seq += MorseTable::toSymbol(item);   // '.' or '-' per KeyerItem
 *   char ch = MorseTable::decode(seq);   // '\0' if not found
 */
class MorseTable {
public:

    /** Convert a KeyerItem (DIT/DAH) to its dot/dash character. */
    static char toSymbol(KeyerItem item) {
        return (item == DIT) ? '.' : '-';
    }

    /**
     * Decode a dot-dash sequence string into the corresponding character.
     * Returns '\0' if the sequence is not recognised.
     */
    static char decode(const std::string &sequence) {
        auto it = table().find(sequence);
        return (it != table().end()) ? it->second : NOT_FOUND;
    }

    /** Full decode table: dot-dash sequence → character. */
    static const std::unordered_map<std::string, char> &table() {
        static const std::unordered_map<std::string, char> s_table = {

            // ── Letters ───────────────────────────────────────────────────
            { ".-",   'A' },
            { "-...", 'B' },
            { "-.-.", 'C' },
            { "-..",  'D' },
            { ".",    'E' },
            { "..-.", 'F' },
            { "--.",  'G' },
            { "....", 'H' },
            { "..",   'I' },
            { ".---", 'J' },
            { "-.-",  'K' },
            { ".-..", 'L' },
            { "--",   'M' },
            { "-.",   'N' },
            { "---",  'O' },
            { ".--.", 'P' },
            { "--.-", 'Q' },
            { ".-.",  'R' },
            { "...",  'S' },
            { "-",    'T' },
            { "..-",  'U' },
            { "...-", 'V' },
            { ".--",  'W' },
            { "-..-", 'X' },
            { "-.--", 'Y' },
            { "--..", 'Z' },

            // ── Digits ────────────────────────────────────────────────────
            { "-----", '0' },
            { ".----", '1' },
            { "..---", '2' },
            { "...--", '3' },
            { "....-", '4' },
            { ".....", '5' },
            { "-....", '6' },
            { "--...", '7' },
            { "---..", '8' },
            { "----.", '9' },

            // ── Punctuation (ITU-R M.1677) ────────────────────────────────
            { ".-.-.-", '.' },   // full stop
            { "--..--", ',' },   // comma
            { "..--..", '?' },   // question mark
            { ".----.", '\'' },  // apostrophe
            { "-.-.--", '!' },   // exclamation mark
            { "-..-.",  '/' },   // slash
            { "-.--.",  '(' },   // open parenthesis
            { "-.--.-", ')' },   // close parenthesis
            { ".-...",  '&' },   // ampersand / wait
            { "---...", ':' },   // colon
            { "-.-.-.", ';' },   // semicolon
            { "-...-",  '=' },   // equals / break (BT)
            { ".-.-.",  '+' },   // plus (AR)
            { "-....-", '-' },   // hyphen / minus
            { ".-..-.", '"' },   // quotation mark
            { ".--.-.", '@' },   // at sign (AC)
            { "..--.-", '_' },   // underscore

            // ── Prosigns (ITU-R M.1677) ─────────────────────────────────
            { "...-.-",  SK }, // end of work (SK)
            { "-...-.-", BK }, // end of work (BK)
            { "........", CW_ERROR }, // error (HH)
        };
        return s_table;
    }
};

#endif //CWKEYERAPP_MORSETABLE_H
