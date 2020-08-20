//Copyright 2020, Bradley Peterson, Weber State University, all rights reserved. (7/2020)

#include <cstdio>
#include <random> 
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::stringstream;
using std::mutex;
using std::thread;

//*** Prototypes ***
void sortOneVector(vector<unsigned int>& bucket);
void _sortOneVector(vector<unsigned int>& arr, int first, int last);
int _quickSortPartition(vector<unsigned int>& arr, int first, int last);
void printArray(const string& msg);
void printAllBuckets(const string& msg);
void pressEnterToContinue();
void stepOne();
void stepTwo();
void stepThree();

//***GLOBAL VARIABLES***  (These are global as they will help with an upcoming multithreaded assignment)
unsigned int numBuckets{ 0 };
unsigned int numThreads{ 0 }; //num of work units
const unsigned int UINTMAX = 4294967295;
unsigned int* arr{ nullptr };
unsigned int arrSize{ 0 };
vector<unsigned int>* buckets{ nullptr };
bool useMultiThreading{ true }; // To turn off multithreading for any debugging purposes, set this to false.
unsigned int currentWorkBucket{ 0 }; //index of the next bucket currently to be worked on


mutex myMutex;

void bucketSort() {

  // TODO: Sort everything in arr using a step 1 / step 2 / step 3 process.
    stepOne();// Step 1 is copying values from arr into buckets.
    
    //create an array of thread tracking objects array should be sized to numThreads
    thread* threads = new thread[numThreads];
    currentWorkBucket = 0;
    //Fork by creating threads
    for (int i = 0; i < numThreads; i++) {
        threads[i] = thread(stepTwo);
    }

    //Join--so the master thread has to wait for all the child threads to finish
    for (int i = 0; i < numThreads; i++) {
        threads[i].join();
    }
    
    stepThree();// Step 3 is copying values from buckets into arr.
  // I would strongly advise using printArray("some message") and printAllBuckets("some message") to debug before, between, and after each step.
  // Note that printArray and printAllBuckets only print when arrSize is <= 100, and won't print for the larger array tests.
    delete[] threads;
}

void stepOne()
{
    //stepOne copy values from arr into a new array with buckets
    unsigned int whichBucket = 0;
    unsigned int formulaDivisor = (UINTMAX / numBuckets);

    for (unsigned int i = 0; i < arrSize; i++) {
        whichBucket = arr[i] / formulaDivisor;
        buckets[whichBucket].push_back(arr[i]);
    }

}

void stepTwo()
{
    unsigned int workUnitAssignment{ 0 };
    do {
        myMutex.lock();//lock this section to one thread at a time

        workUnitAssignment = currentWorkBucket;
        currentWorkBucket++;

        myMutex.unlock();//unlock this section for multiple threads at a time

        if (workUnitAssignment < numBuckets) {
            //yes, this is a valid work unit. Go forth and sort it.
            sortOneVector(buckets[workUnitAssignment]);
        }
        else {
            return;
        }
    } while (true);
}

void stepThree()
{
    // Step 3 is copying values from buckets into arr.
    unsigned int index = 0;
    for (int i = 0; i < numBuckets; i++) {
        for (int j = 0; j < buckets[i].size(); j++) {
            arr[index] = buckets[i].at(j);
            index++;
        }
    }
}

// The function you want to use.  Just pass in a vector, and this will sort it.
void sortOneVector(vector<unsigned int>& bucket) {
  _sortOneVector(bucket, 0, bucket.size());
}

// A function used by sortOneVector().  You won't call this function.
void _sortOneVector(vector<unsigned int>& bucket, int first, int last) {
  //first is the first index
  //last is the one past the last index (or the size of the array
  //if first is 0)

  if (first < last) {
    //Get this subarray into two other subarrays, one smaller and one bigger
    int pivotLocation = _quickSortPartition(bucket, first, last);
    _sortOneVector(bucket, first, pivotLocation);
    _sortOneVector(bucket, pivotLocation + 1, last);
  }
}

// A function used by sortOneVector().  You won't call this function.
int _quickSortPartition(vector<unsigned int>& arr, int first, int last) {
  unsigned long pivot;
  int index, smallIndex;

  unsigned long temp;

  //Take the middle value as the pivot.
  //swap(first, (first + last) / 2);
  pivot = arr[first];
  smallIndex = first;
  for (index = first + 1; index < last; index++) {
    if (arr[index] < pivot) {
      smallIndex++;
      //swap the two
      temp = arr[smallIndex];
      arr[smallIndex] = arr[index];
      arr[index] = temp;
    }
  }

  //Move pivot into the sorted location
  temp = arr[first];
  arr[first] = arr[smallIndex];
  arr[smallIndex] = temp;

  //Tell where the pivot is
  return smallIndex;

}

// A function to create and load the array with random values.  The tests call this method, you won't call it directly.
void createArray() {
  arr = new unsigned int[arrSize];

  //std::random_device rd;
  //std::mt19937 gen(rd());
  std::mt19937 gen(0);
  std::uniform_int_distribution<unsigned long> dis(0, UINTMAX);

  for (unsigned int i = 0; i < arrSize; i++) {
    arr[i] = dis(gen);
  }
}

// A function to delete the array
void deleteArray() {
  delete[] arr;
}

// Print the array in hexadecimal.  Printing in hex is beneficial for the next function, printAllBuckets()
void printArray(const string& msg) {
  if (arrSize <= 100) {
    printf("%s\n", msg.c_str());
    for (unsigned int i = 0; i < arrSize; i++) {
      printf("%08x ", arr[i]);
    }
    printf("\n");
  }
}

// A function to determine how many threads to use on a given machine, depending on its number of cores
unsigned int getNumThreadsToUse() {
  unsigned int numThreadsToUse{ 0 };

  if (useMultiThreading) {
    //Find out how many threads are supported
    unsigned int threadsSupported = std::thread::hardware_concurrency();
    printf("This machine has %d cores.\n", threadsSupported);
    if (threadsSupported == 1 && numBuckets > 1) {
      numThreadsToUse = 2;
    }
    else if (numBuckets < threadsSupported) {
      numThreadsToUse = numBuckets;
    }
    else {
      numThreadsToUse = threadsSupported;
    }
    printf("For the upcoming problem, %d threads will be used\n", numThreadsToUse);
  }
  else {
    numThreadsToUse = 1;
  }
  return numThreadsToUse;
}

// A function to print the array in hexadecimal.  Hex is incredibly useful as an output over base 10/decimal.
// For example, suppose numBuckets = 2.  Then bucket 0 should have all values starting with digit 0-7, and bucket 1 should have all values starting with digit 8-f.
// Also, suppose numBuckets = 4.  Bucket = 0's first digits should be 0-3, bucket 1's first digits should be 4-7, bucket 2's first digits should be 8-b, bucket 3's first digits should be c-f
void printAllBuckets(const string& msg) {

  //Displays the contents of all buckets to the screen.
  if (arrSize <= 100) {
    printf("%s\n", msg.c_str());
    // just uncomment this code when you have arr properly declared as a data member
    printf("******\n");
    for (unsigned int bucketIndex = 0; bucketIndex < numBuckets; bucketIndex++) {
      printf("bucket number %d\n", bucketIndex);
      for (unsigned int elementIndex = 0; elementIndex < buckets[bucketIndex].size(); elementIndex++) {
        printf("%08x ", buckets[bucketIndex][elementIndex]);

      }
      printf("\n");
    }
    printf("\n");
  }
}


// A helper function to verify if the sort is correct.  The test code calls this for you.
void verifySort(unsigned int* arr, unsigned int arraySize, std::chrono::duration<double, std::milli>& diff, const string& sortTest) {
  double val = diff.count();
  for (unsigned int i = 1; i < arraySize; i++) {
    if (arr[i] < arr[i - 1]) {
      printf("------------------------------------------------------\n");
      printf("SORT TEST %s\n", sortTest.c_str());

      if (val != 0.0) {
        printf("Finished bucket sort in %1.16lf milliseconds\n", diff.count());
      }
      printf("ERROR - This list was not sorted correctly.  At index %d is value %08X.  At index %d is value %08X\n", i - 1, arr[i - 1], i, arr[i]);
      printf("------------------------------------------------------\n");


      return;
    }
  }
  printf("------------------------------------------------------\n");
  printf("SORT TEST %s\n", sortTest.c_str());
  if (val != 0.0) {
    printf("Finished bucket sort in %1.16lf milliseconds\n", diff.count());
  }
  printf("PASSED SORT TEST %s - The list was sorted correctly\n", sortTest.c_str());
  printf("------------------------------------------------------\n");
}


void pressEnterToContinue() {
  printf("Press Enter key to continue\n");
  std::cin.get();

}

//Copyright 2020, Bradley Peterson, Weber State University, all rights reserved. (7/2020)

int main() {

  std::chrono::duration<double, std::milli> diff{ 0 };
  double baselineTime{ 9999999.0 };
  double bestMultiThreadedTime{ 9999999.0 };
  int bestMultiThreadedBuckets{ 0 };
  double bestSingleThreadedTime{ 9999999.0 };
  int bestSingleThreadedBuckets{ 0 };

  //Set the listSize, numBuckets, and numThreads global variables.  
  arrSize = 100;

  numBuckets = 2;
  createArray();
  buckets = new vector<unsigned int>[numBuckets];
  numThreads = getNumThreadsToUse();
  printf("\nStarting bucket sort for listSize = %d, numBuckets = %d, numThreads = %d, number of cores = %d\n", arrSize, numBuckets, numThreads, std::thread::hardware_concurrency());
  // printf("Displaying the unsorted list array:\n");
  // printArray(); //useful for debugging small amounts of numbers.  
  pressEnterToContinue();
  auto start = std::chrono::high_resolution_clock::now();
  bucketSort();
  auto end = std::chrono::high_resolution_clock::now();
  diff = end - start;
  verifySort(arr, arrSize, diff, "2 buckets");
  delete[] buckets;
  deleteArray();
  pressEnterToContinue();

  numBuckets = 4;
  createArray();
  buckets = new vector<unsigned int>[numBuckets];
  numThreads = getNumThreadsToUse();
  printf("\nStarting bucket sort for listSize = %d, numBuckets = %d, numThreads = %d, number of cores = %d\n", arrSize, numBuckets, numThreads, std::thread::hardware_concurrency());
  pressEnterToContinue();
  bucketSort();
  verifySort(arr, arrSize, diff, "4 buckets");
  delete[] buckets;
  deleteArray();
  pressEnterToContinue();

  printf("\n\n*** Note, remember to run on performance mode for useful data (for VS, RELEASE instead of DEBUG.  For gcc, /O3 instead of just /g)\n\n");
  pressEnterToContinue();

  arrSize = 4000000;
  numBuckets = 1;
  createArray();
  numThreads = getNumThreadsToUse();
  buckets = new vector<unsigned int>[numBuckets];
  printf("\nStarting bucket sort for listSize = %d, numBuckets = %d, numThreads = %d, number of cores = %d\n", arrSize, numBuckets, numThreads, std::thread::hardware_concurrency());
  start = std::chrono::high_resolution_clock::now();
  bucketSort();
  end = std::chrono::high_resolution_clock::now();
  diff = end - start;
  baselineTime = diff.count();
  verifySort(arr, arrSize, diff, "4000000 items in 1 bucket with 1 thread - BASELINE");
  delete[] buckets;
  deleteArray();

  for (int mode = 0; mode < 2; mode++) {

    useMultiThreading = (bool)mode; // Run all tests without multithreading, then run all with multithreading.  

    for (numBuckets = 2; numBuckets <= 1024; numBuckets *= 2) {
      arrSize = 4000000;
      createArray();
      numThreads = getNumThreadsToUse();
      buckets = new vector<unsigned int>[numBuckets];
      printf("\nStarting bucket sort for listSize = %d, numBuckets = %d, numThreads = %d, number of cores = %d\n", arrSize, numBuckets, numThreads, std::thread::hardware_concurrency());
      start = std::chrono::high_resolution_clock::now();
      bucketSort();
      end = std::chrono::high_resolution_clock::now();
      diff = end - start;
      if (useMultiThreading && (diff.count() < bestMultiThreadedTime)) {
        bestMultiThreadedTime = diff.count();
        bestMultiThreadedBuckets = numBuckets;
      }
      else if (!useMultiThreading && (diff.count() < bestSingleThreadedTime)) {
        bestSingleThreadedTime = diff.count();
        bestSingleThreadedBuckets = numBuckets;
      }
      stringstream ss;
      ss << arrSize << " items in " << numBuckets << " buckets";
      verifySort(arr, arrSize, diff, ss.str());
      delete[] buckets;
      deleteArray();
    }
  }

  printf("\n-----------------------------------------------------------\n");
  printf("              FINAL RESULTS                      \n");
  printf("The baseline (quicksort on 1 thread/1 bucket):  completed in %g ms\n", baselineTime);
  printf("The best singlethreaded result:     %d buckets completed in %g ms\n", bestSingleThreadedBuckets, bestSingleThreadedTime);
  if (useMultiThreading) {
    printf("The best multithreaded result:      %d buckets completed in %g ms\n", bestMultiThreadedBuckets, bestMultiThreadedTime);
  }
  printf("\n-----------------------------------------------------------\n");

  pressEnterToContinue();
  return 0;
}

