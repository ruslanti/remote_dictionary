//
// Created by ruslan on 6/21/22.
//

#include "async_client.h"
#include <boost/asio.hpp>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <boost/bind/bind.hpp>
#include <iostream>

using namespace boost::asio;

using SerializeBuffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;

AsyncClient::AsyncClient(boost::asio::io_context& io, std::string address) : m_io(io), m_socket(m_io) {
    ip::address ip_address =
            ip::address::from_string(address);
    ip::tcp::endpoint endpoint(ip_address, 2222);

    ip::tcp::endpoint local_endpoint;
    m_socket.open(local_endpoint.protocol());
   m_socket.non_blocking(true);
    m_socket.async_connect(endpoint, [](const boost::system::error_code& error) {
        std::cout<<"connected "<<error.message() <<std::endl;
        if (!error)
        {

        }
    });
};


RequestHandler::RequestHandler(boost::asio::ip::tcp::socket& socket, std::function<void(Response)> callback):  m_socket(socket), m_callback(callback) {
        m_data.reserve(1024);

        std::cout<<"handler construct "<<m_socket.non_blocking()<<std::endl;
}

RequestHandler::~RequestHandler() {
    std::cout<<"handler destruct"<<std::endl;
}

void RequestHandler::start(size_t size) {
    std::cout<<"handler handle start "<<size<<std::endl;
    std::cout<< "size = "<<m_data.size()<<std::endl;
    boost::asio::async_write(m_socket,
                             boost::asio::buffer(m_data, size),
                             boost::bind(&RequestHandler::handle_write,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
    //boost::system::error_code error;
    //handle_write(error, 0);
    std::cout<<"handler handle end "<<size<<std::endl;
}

void RequestHandler::handle_read(const boost::system::error_code &error,
                 size_t bytes_transferred) {
    std::cout<<"handler handle read"<<std::endl;
    if (!error) {
        if (bytes_transferred > 0) {
            Response res;
            auto state = bitsery::quickDeserialization<InputAdapter>({m_data.begin(), bytes_transferred}, res);
            if (state.first == bitsery::ReaderError::NoError && state.second) {
                m_callback(res);
            } else {
                throw std::runtime_error(ReaderError_to_string(state.first));
            }
        }
    } else {
        std::cerr << "read error: " << error.message() << std::endl;
    }
}

void RequestHandler::handle_write(const boost::system::error_code &error, size_t bytes_transferred) {
    std::cout<<"handler handle write2"<<std::endl;
    auto self = shared_from_this();
    if (!error) {
        m_socket.async_read_some(boost::asio::buffer(self->m_data),
                                 boost::bind(&RequestHandler::handle_read,
                                             self,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "write error: " << error.message() << std::endl;
    }
}

void AsyncClient::get(std::string key, CallbackResponse callback) {
    std::cout<<"handler handle write1"<<std::endl;
    auto handler = std::make_shared<RequestHandler>(m_socket, callback);
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(handler->m_data, Request{GetRequest{key}});
    std::cout<<"handler handle write"<<std::endl;
    handler->start(writtenSize);
}

void AsyncClient::set(std::string key, std::string value,  CallbackResponse callback) {
    auto handler = std::make_shared<RequestHandler>(m_socket, callback);
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(handler->m_data, Request{SetRequest{key, value}});
    std::cout<<"handler handle write1"<<std::endl;
    handler->start(writtenSize);
}

void AsyncClient::stats( CallbackResponse callback) {
    auto handler = std::make_shared<RequestHandler>(m_socket, callback);
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(handler->m_data, Request{StatsRequest{}});
    handler->start(writtenSize);
}

