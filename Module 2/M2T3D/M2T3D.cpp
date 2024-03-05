#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <ctime>
#include <random>
#include <algorithm>
#include <map>
#include <condition_variable>

using namespace std;

const int NUM_TRAFFIC_LIGHTS = 4;
const int BUFFER_SIZE = 12;
const int MEASUREMENT_INTERVAL_SECONDS = 300;
const int TOP_N_CONGESTED = 3;
const int NUM_MEASUREMENTS = 12;

mutex buffer_lock;
condition_variable buffer_not_full, buffer_not_empty;
queue<tuple<string, int, int>> traffic_buffer;
map<int, int> congested_traffic_lights;
ofstream output_file("data.txt");

void produce_traffic_data() {
    while (true) {
        for (int i = 0; i < NUM_MEASUREMENTS; ++i) {
            auto now = chrono::system_clock::now();
            auto now_time = chrono::system_clock::to_time_t(now);
            string timestamp = ctime(&now_time);
            
            int traffic_light_id = rand() % NUM_TRAFFIC_LIGHTS + 1;
            int cars_passed = rand() % 50 + 1;

            unique_lock<mutex> lock(buffer_lock);
            buffer_not_full.wait(lock, []{ return traffic_buffer.size() < BUFFER_SIZE; });

            traffic_buffer.push(make_tuple(timestamp, traffic_light_id, cars_passed));
            cout << "Produced: " << timestamp << ", Traffic Light ID: " << traffic_light_id << ", Cars Passed: " << cars_passed << endl;

            lock.unlock();
            buffer_not_empty.notify_one();

            this_thread::sleep_for(chrono::minutes(5));
        }
        this_thread::sleep_for(chrono::minutes(60 - (5 * NUM_MEASUREMENTS)));
    }
}

void consume_traffic_data() {
    while (true) {
        unique_lock<mutex> lock(buffer_lock);
        buffer_not_empty.wait(lock, []{ return !traffic_buffer.empty(); });

        while (!traffic_buffer.empty()) {
            auto data = traffic_buffer.front();
            traffic_buffer.pop();
            string timestamp = get<0>(data);
            int traffic_light_id = get<1>(data);
            int cars_passed = get<2>(data);
            if (congested_traffic_lights.find(traffic_light_id) != congested_traffic_lights.end()) {
                congested_traffic_lights[traffic_light_id] += cars_passed;
            } else {
                congested_traffic_lights[traffic_light_id] = cars_passed;
            }
        }

        lock.unlock();
        buffer_not_full.notify_one();

        vector<pair<int, int>> sorted_congested(congested_traffic_lights.begin(), congested_traffic_lights.end());
        partial_sort(sorted_congested.begin(), sorted_congested.begin() + min(TOP_N_CONGESTED, (int)sorted_congested.size()), sorted_congested.end(),
                     [](const pair<int, int> &a, const pair<int, int> &b) { return a.second > b.second; });

        output_file << "Top " << TOP_N_CONGESTED << " congested traffic lights at " << chrono::system_clock::to_time_t(chrono::system_clock::now()) << " - ";
        for (int i = 0; i < min(TOP_N_CONGESTED, (int)sorted_congested.size()); ++i) {
            output_file << sorted_congested[i].first << " (Cars Passed: " << sorted_congested[i].second << "), ";
        }
        output_file << endl;

        congested_traffic_lights.clear();

        this_thread::sleep_for(chrono::hours(1));
    }
}

int main() {
    srand(time(nullptr));

    thread producer_thread(produce_traffic_data);
    thread consumer_thread(consume_traffic_data);

    producer_thread.join();
    consumer_thread.join();

    output_file.close();

    return 0;
}
