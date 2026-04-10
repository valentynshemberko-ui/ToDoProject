#ifndef LOGGER_H
#define LOGGER_H
//Sometimes it can make an error when copying to the console
/*#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#undef ERROR*/
#include <fstream>
#include <string>
using namespace std;

class Logger {
public:
    enum Level {
        DEBUG = 1 << 0,
        INFO  = 1 << 1,
        WARN  = 1 << 2,
        ERROR = 1 << 3
    };

private:
    int flags;
    ofstream file;

    // Кольори консолі
    const string RESET  = "\033[0m";
    const string GRAY   = "\033[94m";
    const string GREEN  = "\033[92m";
    const string YELLOW = "\033[93m";
    const string RED    = "\033[91m";

    string currentTime() const;
    string levelToStr(Level lvl) const;
    string levelToColor(Level lvl) const;

public:
    explicit Logger(const string& filename = "log.txt");
    ~Logger();

    void enable(int lvls);
    void disable(int lvls);
    bool isEnabled(Level lvl) const;
    void listEnabledLevels();

    void log(Level lvl, const string& msg);

    void debug(const string& msg);
    void info(const string& msg);
    void warn(const string& msg);
    void error(const string& msg);
};

#endif // LOGGER_H