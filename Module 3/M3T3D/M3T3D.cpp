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
