#include "fpga_api.h"
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define DATA_SIZE SIZE*(SIZE+1)*sizeof(float) // fpga bram data size

#define min(x,y) (((x)<(y))?(x):(y))

FPGA::FPGA(off_t data_addr, off_t api_addr)
{
    fd_ = open("/dev/mem", O_RDWR);
    data_ = static_cast<float*>(mmap(NULL, DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, data_addr));
    api_ = static_cast<unsigned int*>(mmap(NULL, sizeof(unsigned int), PROT_READ|PROT_WRITE, MAP_SHARED,fd_, api_addr));
}

FPGA::~FPGA()
{
    munmap(data_, DATA_SIZE );
    munmap(api_, sizeof(unsigned int));
    close(fd_);
}

float* FPGA::matrix(void)
{
	return data_ + SIZE;
}

float* FPGA::vector(void)
{
	return data_;
}

const float* __attribute__((optimize("O0"))) FPGA::run()
{    *api_ = 0x5555;
    while(*api_ == 0x5555);

    return data_;    
}

void FPGA::largeMV(const float* large_mat, const float* input,
		float* output, int M, int N)
{
    //declaration of temp variable
    float* mat_temp = new float[SIZE*SIZE];
    
    //declaration of FPGA variable
    float* vec = this->vector();
    float* mat = this->matrix();
    
    //initialization of output
    for(int n=0; n<N; n++)
        output[n] = 0.0f;

    //declaration of large_mat, input
    int num_M = (M%SIZE == 0) ? M/SIZE : M/SIZE + 1;
    int num_N = (N%SIZE == 0) ? N/SIZE : N/SIZE + 1;

    float* input_temp = new float[SIZE*num_M];
    float* large_mat_temp = new float[SIZE*num_M*SIZE*num_N];

    //declaration of zero array
    float* zero_long = new float[SIZE*num_M];

    for(int n=0; n<SIZE*num_M; n++)
        zero_long[n] = 0.0f;

    //memcpy input_temp
    memcpy(input_temp, input, sizeof(float)*M);
    if(num_M > M/SIZE) memcpy(input_temp + M, zero_long, sizeof(float)*(SIZE*num_M - M));

    //memcpy large_mat_temp
    for(int n=0;n<N; n++){
        memcpy(large_mat_temp + (SIZE*num_M)*n, large_mat + M*n, sizeof(float)*M);
        if(num_M > M/SIZE) memcpy(large_mat_temp + (SIZE*num_M)*n + M, zero_long, sizeof(float)*(SIZE*num_M - M));
    }
    if(num_N > N/SIZE) {
        for(int n=0; n< SIZE*num_N -N; n++){
            memcpy(large_mat_temp + SIZE*num_M*N + n*SIZE*num_M, zero_long, sizeof(float)*SIZE*num_M);
        }
    }
    //calculation
    for(int i=0; i<num_M; i++){
        for(int j=0; j<num_N;j++){
            //memcpy vector
            memcpy(vec, input_temp + SIZE*i, sizeof(float)*SIZE);

            //memcpy matrix
            for(int k=0;k<SIZE;k++){
                memcpy(mat_temp + SIZE*k, large_mat_temp + SIZE*i + SIZE*num_M*(SIZE*j + k), sizeof(float)*SIZE);
            }
            memcpy(mat, mat_temp, sizeof(float)*SIZE*SIZE);

            //FPGA run
            const float* out_temp = this->run();

            //Add out_temp
            for(int l=0; l<SIZE; l++){
                if(j*SIZE + l < N) output[j*SIZE + l] += out_temp[l];
            }
        }
    }
}

