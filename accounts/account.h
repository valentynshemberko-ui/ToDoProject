#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <string>
#include <sstream>
using namespace std;

class Account {
private:
    string username;
    size_t passwordHash;

public:
    Account() = default;
    Account(string u, size_t hash);

    const string& getUsername() const;
    size_t getPasswordHash() const;

    string serialize() const;
    static Account deserialize(const string& line);
};


#endif //ACCOUNT_H
