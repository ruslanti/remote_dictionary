//
// Created by ruslan on 6/20/22.
//

#ifndef DICTIONARY_CLIENT_H
#define DICTIONARY_CLIENT_H

#include <string>
#include <optional>
#include <boost/asio.hpp>
#include "types.hpp"

class Client {
public:
    Client(std::string);

    std::optional<std::string> get(std::string);

    void set(std::string, std::string);

    StatsResponse stats();

private:
    size_t send(size_t);

    boost::asio::io_context m_io;
    boost::asio::ip::tcp::socket m_socket;
    std::vector<uint8_t> m_data;
};

#endif //DICTIONARY_CLIENT_H
