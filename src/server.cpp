//
// Created by ruslan on 6/20/22.
//

#include "server.h"
#include "connection.h"
#include <iostream>

using namespace boost::asio;

Server::Server(io_context &io) :
        m_acceptor(io), m_socket(io) {

    ip::tcp::endpoint endpoint(ip::tcp::v4(), 2222);
    m_acceptor.open(endpoint.protocol());
    m_acceptor.bind(endpoint);
    m_acceptor.listen();
    std::cout << "listen: " << m_acceptor.local_endpoint() << std::endl;
    accept();
}

Server::~Server() {
    m_acceptor.close();
}

void Server::accept() {
    m_acceptor.async_accept(m_socket,
                            [this](boost::system::error_code error) {
                                if (!error) {
                                    std::cout << "new connection: " << m_socket.remote_endpoint() << std::endl;
                                    auto conn = std::make_shared<Connection>(std::move(m_socket), *this);
                                    conn->start();
                                    accept();
                                } else if (error::operation_aborted != error) {
                                    auto end = m_acceptor.local_endpoint();
                                    std::cerr << "RemoteDictionary accept error: " << error.message();
                                }
                            });

}

std::optional<std::string> Server::get(std::string key) const {
    ++m_total;
    std::shared_lock lock(m_dictionary_mutex);
    auto it = m_dictionary.find(key);
    if (it != m_dictionary.end()) {
        ++m_success;
        return it->second;
    } else {
        ++m_fail;
        return std::nullopt;
    }
};

void Server::set(std::string key, std::string value) {
    std::unique_lock lock(m_dictionary_mutex);
    m_dictionary.insert(make_pair(key, value));
};

Stats Server::stats() const {
    std::shared_lock lock(m_dictionary_mutex);
    std::cout<<m_total<<std::endl;
    return Stats{m_total, m_success, m_fail};
};

int main() {
    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&io](boost::system::error_code error, int signal) {
        if (!error) {
            // handle any signal as stop
            io.stop();
        }
    });

    Server server(io);

    std::vector<std::thread> threads;
    auto io_run = [&io]() {
        for (;;) {
            try {
                io.run();
                break;
            } catch (std::exception &ex) {
                std::cerr << ex.what() << std::endl;
            }
        }
    };

    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) { threads.emplace_back(std::thread(io_run)); }

    for (auto &thread: threads) thread.join();
    threads.clear();

    return 0;
}
