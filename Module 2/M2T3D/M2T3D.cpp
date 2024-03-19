#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

struct TrafficInfo {
    string timestamp;
    int lightId;
    int carsPassed;
};

const unsigned int maxBufferSize = 10;
const int numTrafficSignals = 10;

std::mutex mutexLock;
std::condition_variable conditionVar;
deque<TrafficInfo> dataBuffer;

void dataProducer(ifstream& inputFile) {
    for (int i = 0; i < maxBufferSize; ++i) {
        string line;
        if (!getline(inputFile, line)) {
            inputFile.clear();
            inputFile.seekg(0, ios::beg);
            getline(inputFile, line);
        }

        istringstream iss(line);
        string timestamp;
        int lightId;
        int carsPassed;
        if (!(iss >> timestamp >> lightId >> carsPassed)) {
            cerr << "Invalid input format\n";
            continue;
        }

        TrafficInfo info = { timestamp, lightId, carsPassed };

        std::unique_lock<std::mutex> locker(mutexLock);
        dataBuffer.push_back(info);
    }

    conditionVar.notify_one();
}

void dataConsumer() {
    while (true) {
        std::unique_lock<std::mutex> locker(mutexLock);
        conditionVar.wait(locker, []() { return !dataBuffer.empty(); });

        vector<TrafficInfo> currentData;
        currentData.reserve(dataBuffer.size());
        while (!dataBuffer.empty()) {
            currentData.push_back(dataBuffer.front());
            dataBuffer.pop_front();
        }

        locker.unlock();

        sort(currentData.begin(), currentData.end(), [](const TrafficInfo& a, const TrafficInfo& b) {
            return a.carsPassed > b.carsPassed;
        });

        cout << "Top 5 most congested traffic signals:" << endl;
        for (int i = 0; i < min(5, static_cast<int>(currentData.size())); ++i) {
            cout << "Time: " << currentData[i].timestamp << ", Traffic Light ID: " << currentData[i].lightId << ", Cars Passed: " << currentData[i].carsPassed << endl;
        }

        if (dataBuffer.empty()) {
            break;
        }
    }
}

int main() {
    auto startTime = chrono::steady_clock::now();

    ifstream inputFile("testdata.txt");
    if (!inputFile) {
        cerr << "Failed to open input file\n";
        return 1;
    }

    vector<thread> producers;

    for (int i = 0; i < 8; ++i) {
        producers.emplace_back(dataProducer, std::ref(inputFile));
    }

    for (auto& producer : producers) {
        producer.join();
    }

    std::thread consumerThread(dataConsumer);
    consumerThread.join();
    auto endTime = chrono::steady_clock::now();
    chrono::duration<double, std::milli> duration = endTime - startTime;
    cout << "Time taken by the program: " << duration.count() << " milliseconds" << endl;

    return 0;
}
