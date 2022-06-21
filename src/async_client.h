//
// Created by ruslan on 6/21/22.
//

#ifndef DICTIONARY_ASYNCCLIENT_H
#define DICTIONARY_ASYNCCLIENT_H

#include <string>
#include <optional>
#include <boost/asio.hpp>
#include "types.hpp"

using CallbackResponse = std::function<void(Response)>;

struct RequestHandler: public std::enable_shared_from_this<RequestHandler> {
    RequestHandler( boost::asio::ip::tcp::socket& socket, CallbackResponse);
    virtual ~RequestHandler();

    void start(size_t size);
    void handle_read(const boost::system::error_code &error,
                     size_t bytes_transferred);

    void handle_write(const boost::system::error_code &error, size_t);

    std::vector<uint8_t> m_data;
    boost::asio::ip::tcp::socket& m_socket;
    CallbackResponse m_callback;
};

class AsyncClient {
public:
    AsyncClient(boost::asio::io_context&, std::string);

    void get(std::string, CallbackResponse);

    void set(std::string, std::string, CallbackResponse);

    void stats(CallbackResponse);

private:
    boost::asio::io_context& m_io;
    boost::asio::ip::tcp::socket m_socket;
};


#endif //DICTIONARY_ASYNCCLIENT_H
