//
// Created by ruslan on 6/20/22.
//

#include "connection.h"
#include "types.hpp"
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>

using namespace boost::asio;

using SerializeBuffer = std::array<uint8_t, 1024>;
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;

Connection::Connection(boost::asio::ip::tcp::socket socket, IDictionary &dictionary) : m_socket(std::move(socket)),
                                                                                       m_dictionary(dictionary) {
    m_socket.non_blocking(true);
}

Connection::~Connection() {
    std::cout << "connection closed" << std::endl;
}

void Connection::start() {
    m_socket.async_read_some(boost::asio::buffer(m_data),
                             boost::bind(&Connection::handle_read,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void Connection::handle_read(const boost::system::error_code &error,
                             size_t bytes_transferred) {
    if (!error) {
        if (bytes_transferred > 0) {
            Request req;
            auto state = bitsery::quickDeserialization<InputAdapter>({m_data.begin(), bytes_transferred}, req);
            if (state.first == bitsery::ReaderError::NoError && state.second) {
                auto response = visit(*this, req);

                auto writtenSize = bitsery::quickSerialization<OutputAdapter>(m_data, response);

                boost::asio::async_write(m_socket,
                                         boost::asio::buffer(m_data, writtenSize),
                                         boost::bind(&Connection::handle_write,
                                                     shared_from_this(),
                                                     boost::asio::placeholders::error));

            } else {
                std::cout << ReaderError_to_string(state.first) << std::endl;
            }
        }
    } else {
        std::cerr << "read error: " << error.message() << std::endl;
    }
}

void Connection::handle_write(const boost::system::error_code &error) {
    std::cerr << "write: " << error.message() << std::endl;
    if (!error) {
        m_socket.async_read_some(boost::asio::buffer(m_data),
                                 boost::bind(&Connection::handle_read,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "write error: " << error.message() << std::endl;
    }
}


template<>
Response Connection::operator()(const GetRequest &req) {
   // std::cout << "get request: " << req.key;
    auto r = m_dictionary.get(req.key);
    if (r.has_value()) {
     //   std::cout << " response: " << *r << std::endl;
        return GetResponse{*r};
    } else {
     //   std::cout << " not found" << std::endl;
        return NotFound{};
    }
}

template<>
Response Connection::operator()(const SetRequest &req) {
    std::cout << "set request: " << req.key << ":" << req.value << std::endl;
    m_dictionary.set(req.key, req.value);
    return SetResponse{};
}

template<>
Response Connection::operator()(const StatsRequest &req) {
   // std::cout << "stats request" << std::endl;
    auto stats = m_dictionary.stats();
    return StatsResponse{stats.total, stats.success, stats.fail};
}
