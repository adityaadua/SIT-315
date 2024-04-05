#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <chrono>

#define PRINT 0 // Set to 1 to print arrays, 0 to disable

int SZ = 100000000; // Size of the arrays

int *v1, *v2, *v_out; // Arrays for input and output vectors

// OpenCL memory buffers
cl_mem bufV1, bufV2, bufV_out;
cl_device_id device_id; // OpenCL device id
cl_context context; // OpenCL context
cl_program program; // OpenCL program
cl_kernel kernel; // OpenCL kernel
cl_command_queue queue; // OpenCL command queue
cl_event event = NULL; // OpenCL event

int err; // OpenCL error variable

cl_device_id create_device(); // Function prototype to create OpenCL device
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname); // Function prototype to setup OpenCL environment
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename); // Function prototype to build OpenCL program

void setup_kernel_memory(); // Function prototype to setup kernel memory
void copy_kernel_args(); // Function prototype to copy kernel arguments
void free_memory(); // Function prototype to free allocated memory
void init(int *&A, int size); // Function prototype to initialize arrays with random integers
void print(int *A, int size); // Function prototype to print arrays

int main(int argc, char **argv) {
    if (argc > 1) {
        SZ = atoi(argv[1]); // Set size of arrays if provided as command-line argument
    }

    init(v1, SZ); // Initialize input vector 1
    init(v2, SZ); // Initialize input vector 2
    v_out = (int *)malloc(sizeof(int) * SZ); // Allocate memory for output vector

    size_t global[1] = {(size_t)SZ}; // Global size for OpenCL kernel execution

    // Print input vectors if enabled
    print(v1, SZ);
    print(v2, SZ);
   
    // Setup OpenCL environment
    setup_openCL_device_context_queue_kernel((char *)"./vector_add_ocl.cl", (char *)"vector_add_ocl");
    // Setup kernel memory buffers
    setup_kernel_memory();
    // Copy kernel arguments
    copy_kernel_args();
    
    auto start = std::chrono::high_resolution_clock::now(); // Start timer for kernel execution
    // Enqueue kernel for execution
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event); // Wait for kernel execution to finish
   
    // Read output vector from device to host
    clEnqueueReadBuffer(queue, bufV_out, CL_TRUE, 0, SZ * sizeof(int), &v_out[0], 0, NULL, NULL);
    // Print output vector if enabled
    print(v_out, SZ);
    auto stop = std::chrono::high_resolution_clock::now(); // Stop timer
    std::chrono::duration<double, std::milli> elapsed_time = stop - start; // Calculate elapsed time

    printf("OpenCL Kernel Execution Time: %f ms\n", elapsed_time.count()); // Print kernel execution time
    free_memory(); // Free allocated memory and release OpenCL resources

    // Free output vector
    free(v_out);

    return 0;
}

// Function to initialize arrays with random integers
void init(int *&A, int size) {
    A = (int *)malloc(sizeof(int) * size); // Allocate memory for array

    for (long i = 0; i < size; i++) {
        A[i] = rand() % 100; // Initialize array elements with random integers between 0 and 99
    }
}

// Function to print arrays
void print(int *A, int size)
{
    if (PRINT == 0)
    {
        return;
    }

    if (PRINT == 1 && size > 15)
    {
        for (long i = 0; i < 5; i++)
        {                      
            printf("%d ", A[i]); // Print first 5 elements
        }
        printf(" ..... ");
        for (long i = size - 5; i < size; i++)
        {                        
            printf("%d ", A[i]); // Print last 5 elements
        }
    }
    else
    {
        for (long i = 0; i < size; i++)
        {                        
            printf("%d ", A[i]); // Print all elements
        }
    }
    printf("\n----------------------------\n");
}

// Function to free allocated memory
void free_memory() {
    // Release OpenCL memory buffers
    clReleaseMemObject(bufV1);
    clReleaseMemObject(bufV2);
    clReleaseMemObject(bufV_out);

    // Release OpenCL kernel, command queue, program, and context
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v1); // Free input vector 1
    free(v2); // Free input vector 2
}

// Function to copy kernel arguments
void copy_kernel_args()
{
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ); // Set kernel argument for array size
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV1); // Set kernel argument for input vector 1
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&bufV2); // Set kernel argument for input vector 2
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&bufV_out); // Set kernel argument for output vector

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

// Function to setup kernel memory buffers
void setup_kernel_memory() {
    // Create OpenCL memory buffers for input and output vectors
    bufV1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, SZ * sizeof(int), v1, NULL);
    bufV2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, SZ * sizeof(int), v2, NULL);
    bufV_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SZ * sizeof(int), NULL, NULL);
}

// Function to setup OpenCL device, context, queue, and kernel
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device(); // Create OpenCL device
    cl_int err;

    // Create OpenCL context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    // Build OpenCL program
    program = build_program(context, device_id, filename);

    // Create OpenCL command queue
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };

    // Create OpenCL kernel
    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

// Function to build OpenCL program
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    // Read OpenCL program source from file
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // Create OpenCL program from source
    program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    // Build OpenCL program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {
        // If build fails, print program build log
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,  log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

// Function to create OpenCL device
cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   // Get OpenCL platform
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Get OpenCL GPU device
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      printf("GPU not found\n");
      // If GPU not found, get OpenCL CPU device
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}
