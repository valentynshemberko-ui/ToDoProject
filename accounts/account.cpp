#include "account.h"
#include <utility>
#include <stdexcept>
using namespace std;

Account::Account(string u, size_t hash)
    : username(move(u)), passwordHash(hash){}

const string& Account::getUsername() const { return username; }
size_t Account::getPasswordHash() const { return passwordHash; }

string Account::serialize() const {
    ostringstream oss;
    oss << username << ';' << passwordHash;
    return oss.str();
}

Account Account::deserialize(const string& line) {
    istringstream iss(line);
    string username;
    string hashStr;

    if (!getline(iss, username, ';')) throw runtime_error("bad accounts line");
    if (!getline(iss, hashStr)) throw runtime_error("bad accounts line");

    size_t hash = 0;
    try { hash = static_cast<size_t>(stoull(hashStr)); } catch(...) {}

    return Account(username, hash);
}
