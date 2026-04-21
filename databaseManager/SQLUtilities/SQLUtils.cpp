#include "SQLUtils.h"

string escapeSQL(const string& value) {
    string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        if (c == '\'') escaped += "''";
        else escaped += c;
    }
    return escaped;
}
