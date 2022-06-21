//
// Created by ruslan on 6/20/22.
//

#include "client.h"
#include <boost/asio.hpp>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <iostream>

using namespace boost::asio;

using SerializeBuffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;
using InputAdapter = bitsery::InputBufferAdapter<SerializeBuffer>;

Client::Client(std::string address) : m_socket(m_io) {
    m_data.reserve(1024);
    ip::address ip_address =
            ip::address::from_string(address);
    ip::tcp::endpoint endpoint(ip_address, 2222);
    m_socket.connect(endpoint);
};

std::optional<std::string> Client::get(std::string key) {
    Response res;
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(m_data, Request{GetRequest{key}});
    auto received = send(writtenSize);
    auto state = bitsery::quickDeserialization<InputAdapter>({m_data.begin(), received}, res);
    if (state.first == bitsery::ReaderError::NoError && state.second) {
        if (std::holds_alternative<GetResponse>(res)) {
            return std::get<GetResponse>(res).value;
        } else if (std::holds_alternative<NotFound>(res)) {
            return std::nullopt;
        } else if (std::holds_alternative<ErrorResponse>(res)) {
            throw std::runtime_error(std::get<ErrorResponse>(res).error);
        } else {
            throw std::runtime_error("wrong response type");
        }
    } else {
        throw std::runtime_error(ReaderError_to_string(state.first));
    }
}

void Client::set(std::string key, std::string value) {
    Response res;
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(m_data, Request{SetRequest{key, value}});
    auto received = send(writtenSize);
    auto state = bitsery::quickDeserialization<InputAdapter>({m_data.begin(), received}, res);
    if (state.first == bitsery::ReaderError::NoError && state.second) {
        if (!std::holds_alternative<SetResponse>(res)) {
            if (std::holds_alternative<ErrorResponse>(res)) {
                throw std::runtime_error(std::get<ErrorResponse>(res).error);
            } else {
                throw std::runtime_error("wrong response type");
            }
        }
    } else {
        throw std::runtime_error(ReaderError_to_string(state.first));
    }
}

StatsResponse Client::stats() {
    Response res;
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(m_data, Request{StatsRequest{}});
    auto received = send(writtenSize);
    auto state = bitsery::quickDeserialization<InputAdapter>({m_data.begin(), received}, res);
    if (state.first == bitsery::ReaderError::NoError && state.second) {
        if (std::holds_alternative<StatsResponse>(res)) {
            auto stats =  std::get<StatsResponse>(res);
            return StatsResponse{stats.total,stats.success,stats.fail};
        } else if (std::holds_alternative<ErrorResponse>(res)) {
            throw std::runtime_error(std::get<ErrorResponse>(res).error);
        } else {
            throw std::runtime_error("wrong response type");
        }
    } else {
        throw std::runtime_error(ReaderError_to_string(state.first));
    }
}

size_t Client::send(size_t size) {
    size_t sent = m_socket.send(boost::asio::buffer(m_data, size));
    assert(sent == size);
    return m_socket.receive(boost::asio::buffer(m_data));
}

