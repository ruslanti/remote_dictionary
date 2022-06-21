//
// Created by ruslan on 6/20/22.
//

#ifndef DICTIONARY_TYPES_H
#define DICTIONARY_TYPES_H

#include <variant>
#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/ext/std_map.h>
#include <bitsery/ext/std_optional.h>
#include <bitsery/ext/std_set.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/ext/std_variant.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

constexpr size_t MAX_LEN = 1024;

struct GetRequest {
    std::string key;
};

struct SetRequest {
    std::string key;
    std::string value;
};

struct StatsRequest {
};

struct GetResponse {
    std::string value;
};

struct SetResponse {
};

struct StatsResponse {
    size_t total;
    size_t success;
    size_t fail;
};

struct NotFound {
};

struct ErrorResponse {
    std::string error;
};

using Request = std::variant<GetRequest, SetRequest, StatsRequest>;
using Response = std::variant<GetResponse, SetResponse, StatsResponse, NotFound, ErrorResponse>;


template<typename S>
void serialize(S &s, Request &o) {
    s.ext(o,
          bitsery::ext::StdVariant{
                  [](S &s, GetRequest &o) {
                      s.text1b(o.key, MAX_LEN);
                  },

                  [](S &s, SetRequest &o) {
                      s.text1b(o.key, MAX_LEN);
                      s.text1b(o.value, MAX_LEN);
                  },

                  [](S &s, StatsRequest &o) {
                  },
          });
}

template<typename S>
void serialize(S &s, Response &o) {
    s.ext(o,
          bitsery::ext::StdVariant{
                  [](S &s, GetResponse &o) {
                      s.text1b(o.value, MAX_LEN);
                  },

                  [](S &s, SetResponse &o) {},

                  [](S &s, StatsResponse &o) {
                      s.value8b(o.total);
                      s.value8b(o.success);
                      s.value8b(o.fail);
                  },

                  [](S &s, NotFound &o) {},

                  [](S &s, ErrorResponse &o) {
                      s.text1b(o.error, MAX_LEN);
                  },
          });
}

static constexpr char NoError[] = "NoError";
static constexpr char ReadingError[] = "ReadingError";
static constexpr char DataOverflow[] = "DataOverflow";
static constexpr char InvalidData[] = "InvalidData";
static constexpr char InvalidPointer[] = "InvalidPointer";
static constexpr char Unknown[] = "Unknown";

inline const char *ReaderError_to_string(bitsery::ReaderError error) {
    switch (error) {
        case bitsery::ReaderError::NoError:
            return NoError;
        case bitsery::ReaderError::ReadingError:
            return ReadingError;
        case bitsery::ReaderError::DataOverflow:
            return DataOverflow;
        case bitsery::ReaderError::InvalidData:
            return InvalidData;
        case bitsery::ReaderError::InvalidPointer:
            return InvalidPointer;
        default:
            return Unknown;
    }
}

#endif //DICTIONARY_TYPES_H
