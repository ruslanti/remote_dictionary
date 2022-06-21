//
// Created by ruslan on 6/21/22.
//
#include "async_client.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

nanoseconds call(std::function<void()> f) {
    auto start = steady_clock::now();
    f();
    return steady_clock::now() - start;
}


int main(int argc, char *argv[]) {
    boost::asio::io_context io_context;
    AsyncClient client(io_context, "127.0.0.1");
    int insert_records = 1;
    int insert_data_size = 256;
    int get_count = insert_records + 100000;

    std::vector<char> data(insert_data_size, 'a');

    std::vector<std::thread> threads;
    auto io_run = [&io_context]() {
        for (;;) {
            try {
                io_context.run();
                break;
            } catch (std::exception &ex) {
                std::cerr << ex.what() << std::endl;
            }
        }
    };

    for (int i = 0; i < 1; ++i) { threads.emplace_back(std::thread(io_run)); }


    std::mutex mtx;
    nanoseconds min_latency{100000};
    nanoseconds max_latency{0};
    nanoseconds sum_latency{0};

    std::cout<<"SET "<< insert_records<<" records"<<std::endl;
    auto start = steady_clock::now();
    for (auto i = 0; i < insert_records; i++) {
        auto start = steady_clock::now();
        client.set("test"+ std::to_string(i), std::string(data.begin(), data.end()), [&](Response _res) {
            auto latency = steady_clock::now() - start;
            std::scoped_lock<std::mutex> _lock(mtx);
            if (latency < min_latency) min_latency = latency;
            if (latency > max_latency) max_latency = latency;
            sum_latency += latency;
        });
    }
    std::cout<<"e"<<std::endl;
    auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
/*
    std::cout << "Min latency: " << duration_cast<microseconds>(min_latency).count() << " μs, " <<
              "Max latency: " << duration_cast<microseconds>(max_latency).count() << " μs, " <<
              "Average latency: " << duration_cast<microseconds>(sum_latency / insert_records).count() << " μs, " <<
              "Throughput: " << (insert_records/elapsed)<<" inserts/ms"<<std::endl;
*/
    io_context.run();
    sleep(1000);
    for (auto &thread: threads) thread.join();
    threads.clear();

    /*min_latency = nanoseconds(100000);
    max_latency = nanoseconds(0);
    sum_latency = nanoseconds(0) ;

    std::cout<<"GET "<< get_count<<" records"<<std::endl;
    start = steady_clock::now();
    for (auto i = 0; i < get_count; i++) {
        auto latency = call([&client, i, &data]() {
            client.get("test"+ std::to_string(i));
        });
        if (latency < min_latency) min_latency = latency;
        if (latency > max_latency) max_latency = latency;
        sum_latency += latency;
    }
    elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
    std::cout << "Min latency: " << duration_cast<microseconds>(min_latency).count() << " μs, " <<
              "Max latency: " << duration_cast<microseconds>(max_latency).count() << " μs, " <<
              "Average latency: " << duration_cast<microseconds>(sum_latency / insert_records).count() << " μs, " <<
              "Throughput: " << (insert_records/elapsed)<<" inserts/ms"<<std::endl;

    auto stats = client.stats();
    std::cout << "Totals: " << stats.total << ", " <<
              "Success: " << stats.success << ", " <<
              "Fail: " << stats.fail<<std::endl;*/
}