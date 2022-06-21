//
// Created by ruslan on 6/20/22.
//

#ifndef DICTIONARY_DICTIONARY_H
#define DICTIONARY_DICTIONARY_H

#include <string>
#include <optional>

struct Stats {
    size_t total;
    size_t success;
    size_t fail;
};

class IDictionary {
public:
    virtual std::optional<std::string> get(std::string key) const = 0;

    virtual void set(std::string key, std::string value) = 0;

    virtual Stats stats() const = 0;
};

#endif //DICTIONARY_DICTIONARY_H
