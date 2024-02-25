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

// Parallel QuickSort algorithm using OpenMP
void quicksortParallel(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pivotIdx = partition(arr, low, high);
#pragma omp parallel sections
        {
#pragma omp section
            quicksortParallel(arr, low, pivotIdx - 1);
#pragma omp section
            quicksortParallel(arr, pivotIdx + 1, high);
        }
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

    // Parallel sorting
    vector<int> arrParallel = arr;
    double startTime = clock();
    quicksortParallel(arrParallel, 0, n - 1);
    double endTime = clock();
    cout << "Parallel QuickSort took " << (endTime - startTime) << " seconds." << endl;

    return 0;
}
