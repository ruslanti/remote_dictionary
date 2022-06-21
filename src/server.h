//
// Created by ruslan on 6/20/22.
//

#ifndef DICTIONARY_SERVER_H
#define DICTIONARY_SERVER_H

#include <map>
#include <shared_mutex>
#include <boost/asio.hpp>
#include "types.hpp"
#include "dictionary.h"

class Server : public IDictionary {
public:
    Server() = delete;

    Server(boost::asio::io_context &);

    virtual ~Server();

    std::optional<std::string> get(std::string key) const override;

    void set(std::string key, std::string value) override;

    Stats stats() const override;

private:
    void accept();

    std::map<std::string, std::string> m_dictionary;
    mutable std::shared_mutex m_dictionary_mutex;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    mutable std::size_t m_total{0};
    mutable std::size_t m_success{0};
    mutable std::size_t m_fail{0};
};


#endif //DICTIONARY_SERVER_H
