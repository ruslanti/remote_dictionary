//
// Created by ruslan on 6/20/22.
//

#ifndef DICTIONARY_CONNECTION_H
#define DICTIONARY_CONNECTION_H

#include "dictionary.h"
#include "types.hpp"
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::ip::tcp::socket socket, IDictionary &);

    virtual ~Connection();

    void start();

    void handle_read(const boost::system::error_code &error,
                     size_t bytes_transferred);

    void handle_write(const boost::system::error_code &error);

    template<class T>
    Response operator()(const T &t);

private:


    std::array<uint8_t, 1024> m_data;
    boost::asio::ip::tcp::socket m_socket;
    IDictionary &m_dictionary;
};


#endif //DICTIONARY_CONNECTION_H
