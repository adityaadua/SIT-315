#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <algorithm>
#include <mpi.h>

using namespace std;

struct TrafficInfo {
    string timestamp;
    int lightId;
    int carsPassed;
};

const int maxBufferSize = 10;
const int numTrafficSignals = 10;

deque<TrafficInfo> dataBuffer;

void dataProducer(const string& filename, int rank, int size) {
    ifstream inputFile(filename);
    if (!inputFile) {
        cerr << "Failed to open input file\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int chunkSize = maxBufferSize / size;
    vector<TrafficInfo> chunk(chunkSize);

    inputFile.seekg(rank * chunkSize * sizeof(TrafficInfo));
    for (int i = 0; i < chunkSize; ++i) {
        string line;
        if (!getline(inputFile, line)) {
            inputFile.clear();
            inputFile.seekg(0, ios::beg);
            getline(inputFile, line);
        }

        istringstream iss(line);
        iss >> chunk[i].timestamp >> chunk[i].lightId >> chunk[i].carsPassed;
    }

    MPI_Send(chunk.data(), chunkSize * sizeof(TrafficInfo), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
}

void dataConsumer(int rank, int size) {
    MPI_Status status;
    int chunkSize = maxBufferSize / size;
    vector<TrafficInfo> chunk(chunkSize);

    MPI_Recv(chunk.data(), chunkSize * sizeof(TrafficInfo), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    
    for (const auto& info : chunk) {
        dataBuffer.push_back(info);
    }
}

void masterProcess(int size) {
    vector<TrafficInfo> allData(maxBufferSize);
    for (int i = 1; i < size; ++i) {
        MPI_Status status;
        MPI_Recv(allData.data() + (i - 1) * (maxBufferSize / size), maxBufferSize / size * sizeof(TrafficInfo), MPI_BYTE, i, 0, MPI_COMM_WORLD, &status);
    }

    // Combine data from all processes
    for (const auto& info : allData) {
        dataBuffer.push_back(info);
    }

    // Sort and display top N most congested traffic lights
    sort(dataBuffer.begin(), dataBuffer.end(), [](const TrafficInfo& a, const TrafficInfo& b) {
        return a.carsPassed > b.carsPassed;
    });

    cout << "Top 5 most congested traffic signals:" << endl;
    for (int i = 0; i < min(5, static_cast<int>(dataBuffer.size())); ++i) {
        cout << "Time: " << dataBuffer[i].timestamp << ", Traffic Light ID: " << dataBuffer[i].lightId << ", Cars Passed: " << dataBuffer[i].carsPassed << endl;
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " <input_file>" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        masterProcess(size);
    } else {
        dataProducer(argv[1], rank, size);
    }

    MPI_Finalize();
    return 0;
}



#include <mpi.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

// Define struct for traffic signal data
struct TrafficSignal {
    int index;
    string timestamp;
    int light_id;
    int num_cars;
};

// Function to read data from file
vector<TrafficSignal> readData(const string& filename) {
    vector<TrafficSignal> data;
    ifstream infile(filename);
    string line;

    // Skip header
    getline(infile, line);

    // Read data line by line
    while (getline(infile, line)) {
        stringstream ss(line);
        TrafficSignal signal;
        ss >> signal.index >> signal.timestamp >> signal.light_id >> signal.num_cars;
        data.push_back(signal);
    }

    return data;
}

// Function to distribute data among processes
void distributeData(const vector<TrafficSignal>& all_data, vector<TrafficSignal>& local_data) {
    int num_data = all_data.size();
    int num_local_data = num_data / size;
    local_data.resize(num_local_data);

    MPI_Scatter(all_data.data(), num_local_data * sizeof(TrafficSignal), MPI_CHAR,
                local_data.data(), num_local_data * sizeof(TrafficSignal), MPI_CHAR,
                0, MPI_COMM_WORLD);
}

// Producer function
void produce(const vector<TrafficSignal>& local_data, queue<TrafficSignal>& local_queue) {
    for (const auto& signal : local_data) {
        local_queue.push(signal);
    }
}

// Consumer function
void consume(queue<TrafficSignal>& local_queue, vector<pair<int, int>>& local_statistics) {
    while (!local_queue.empty()) {
        const TrafficSignal& signal = local_queue.front();
        local_statistics[signal.light_id - 1].second += signal.num_cars; // Update local statistics
        local_queue.pop();
    }
}

// Function to gather statistics from all processes
void gatherStatistics(vector<pair<int, int>>& local_statistics, vector<pair<int, int>>& global_statistics) {
    int num_lights = local_statistics.size();
    global_statistics.resize(num_lights);

    // Gather statistics from all processes
    MPI_Allreduce(local_statistics.data(), global_statistics.data(), num_lights * 2, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
}

// Main function
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<pair<int, int>> local_statistics(4, {0, 0}); // (light_id, total_num_cars)
    vector<pair<int, int>> global_statistics;

    if (rank == 0) {
        // Read data from file
        vector<TrafficSignal> all_data = readData("traffic_data.txt");

        // Distribute data among processes
        vector<TrafficSignal> local_data;
        distributeData(all_data, local_data);

        // Producer-consumer tasks
        queue<TrafficSignal> local_queue;
        produce(local_data, local_queue);
        consume(local_queue, local_statistics);
    } else {
        // Receive local data from root process
        vector<TrafficSignal> local_data;
        distributeData({}, local_data);

        // Producer-consumer tasks
        queue<TrafficSignal> local_queue;
        produce(local_data, local_queue);
        consume(local_queue, local_statistics);
    }

    // Gather statistics from all processes
    gatherStatistics(local_statistics, global_statistics);

    // Sort global statistics to find top congested traffic lights
    sort(global_statistics.begin(), global_statistics.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.second > b.second;
    });

    // Print top N congested traffic lights (in rank order)
    const int N = 2; // Change this value to print more top traffic lights
    if (rank == 0) {
        cout << "Top " << N << " Congested Traffic Lights:" << endl;
        cout << "Traffic Light\tTotal Cars" << endl;
        for (int i = 0; i < min(N, (int)global_statistics.size()); ++i) {
            cout << global_statistics[i].first << "\t\t" << global_statistics[i].second << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
