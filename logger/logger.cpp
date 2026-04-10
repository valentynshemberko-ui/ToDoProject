#include "Logger.h"
#include <iostream>
#include <ctime>

string Logger::currentTime() const {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%d-%m %H:%M:%S", localtime(&now));
    return buf;
}

string Logger::levelToStr(Level lvl) const {
    switch (lvl) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default:    return "UNKNOWN";
    }
}

string Logger::levelToColor(Level lvl) const {
    switch (lvl) {
        case DEBUG: return GRAY;
        case INFO:  return GREEN;
        case WARN:  return YELLOW;
        case ERROR: return RED;
        default:    return RESET;
    }
}

Logger::Logger(const string& filename)
    : flags(DEBUG | INFO | WARN | ERROR)
{
    file.open(filename, ios::app);
    if (!file.is_open()) {
        cerr << "[LOGGER ERROR] Can not open logger file!" << endl;
    }
}

Logger::~Logger() {
    if (file.is_open()) file.close();
}

void Logger::enable(int lvls) {
    flags |= lvls;

    string enabledLevels;
    if (lvls & DEBUG) enabledLevels += " DEBUG";
    if (lvls & INFO)  enabledLevels += " INFO";
    if (lvls & WARN)  enabledLevels += " WARN";
    if (lvls & ERROR) enabledLevels += " ERROR";

    string msg = "Ennabled loger levels:" + enabledLevels;
    log(INFO, msg);
}

void Logger::disable(int lvls) {
    flags &= ~lvls;
    string disabledLevels;
    if (lvls & DEBUG) disabledLevels += " DEBUG";
    if (lvls & INFO)  disabledLevels += " INFO";
    if (lvls & WARN)  disabledLevels += " WARN";
    if (lvls & ERROR) disabledLevels += " ERROR";

    string msg = "Disabled loger levels:" + disabledLevels;
    log(WARN, msg);
}

bool Logger::isEnabled(Level lvl) const {
    return flags & lvl;
}

void Logger::listEnabledLevels() {
    string active;
    if (flags & DEBUG) active += " DEBUG";
    if (flags & INFO)  active += " INFO";
    if (flags & WARN)  active += " WARN";
    if (flags & ERROR) active += " ERROR";

    if (active.empty()) active = " (no active levels)";
    string msg = "Active loger levels:" + active;
    log(INFO, msg);
}


void Logger::log(Level lvl, const string& msg) {
    if (!isEnabled(lvl)) return;

    string timestamp = currentTime();
    string lvlStr = levelToStr(lvl);
    string formatted = "[" + timestamp + "] [" + lvlStr + "] " + msg;
    cout << levelToColor(lvl) << formatted << RESET << endl;

    if (file.is_open()) {
        file << formatted << endl;
    }
}

void Logger::debug(const string& msg) { log(DEBUG, msg); }
void Logger::info(const string& msg)  { log(INFO, msg); }
void Logger::warn(const string& msg)  { log(WARN, msg); }
void Logger::error(const string& msg) { log(ERROR, msg); }