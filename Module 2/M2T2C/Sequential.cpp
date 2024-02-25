#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

// Function to partition the array and return the pivot index
int partition(vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }

    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Sequential QuickSort algorithm
void quicksortSequential(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pivotIdx = partition(arr, low, high);
        quicksortSequential(arr, low, pivotIdx - 1);
        quicksortSequential(arr, pivotIdx + 1, high);
    }
}

int main() {
    srand(time(0));
    int n = 1000000; // Adjust the size of the array as needed
    vector<int> arr(n);

    // Fill the array with random values
    for (int i = 0; i < n; i++) {
        arr[i] = rand();
    }

    // Sequential sorting
    vector<int> arrSequential = arr;
    double startTime = clock();
    quicksortSequential(arrSequential, 0, n - 1);
    double endTime = clock();
    cout << "Sequential QuickSort took " << (endTime - startTime) / CLOCKS_PER_SEC << " seconds." << endl;

    return 0;
}
