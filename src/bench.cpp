//
// Created by ruslan on 6/21/22.
//
#include "client.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

nanoseconds call(std::function<void()> f) {
    auto start = steady_clock::now();
    f();
    return steady_clock::now() - start;
}


int main() {
    Client client("127.0.0.1");
    int insert_records = 100000;
    int insert_data_size = 256;
    int get_count = insert_records + 100000;

    std::vector<char> data(insert_data_size, 'a');

    nanoseconds min_latency{100000};
    nanoseconds max_latency{0};
    nanoseconds sum_latency{0};

    std::cout << "SET " << insert_records << " records" << std::endl;
    auto start = steady_clock::now();
    for (auto i = 0; i < insert_records; i++) {
        auto latency = call([&client, i, &data]() {
            client.set("test" + std::to_string(i), std::string(data.begin(), data.end()));
        });
        if (latency < min_latency) min_latency = latency;
        if (latency > max_latency) max_latency = latency;
        sum_latency += latency;
    }
    auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
    std::cout << "Min latency: " << duration_cast<microseconds>(min_latency).count() << " μs, " << "Max latency: "
              << duration_cast<microseconds>(max_latency).count() << " μs, " << "Average latency: "
              << duration_cast<microseconds>(sum_latency / insert_records).count() << " μs, " << "Throughput: "
              << (insert_records / elapsed) << " inserts/ms" << std::endl;


    min_latency = nanoseconds(100000);
    max_latency = nanoseconds(0);
    sum_latency = nanoseconds(0);

    std::cout << "GET " << get_count << " records" << std::endl;
    start = steady_clock::now();
    for (auto i = 0; i < get_count; i++) {
        auto latency = call([&client, i, &data]() {
            client.get("test" + std::to_string(i));
        });
        if (latency < min_latency) min_latency = latency;
        if (latency > max_latency) max_latency = latency;
        sum_latency += latency;
    }
    elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
    std::cout << "Min latency: " << duration_cast<microseconds>(min_latency).count() << " μs, " << "Max latency: "
              << duration_cast<microseconds>(max_latency).count() << " μs, " << "Average latency: "
              << duration_cast<microseconds>(sum_latency / insert_records).count() << " μs, " << "Throughput: "
              << (insert_records / elapsed) << " gets/ms" << std::endl;

    auto stats = client.stats();
    std::cout << "Totals: " << stats.total << ", " << "Success: " << stats.success << ", " << "Fail: " << stats.fail
              << std::endl;

}