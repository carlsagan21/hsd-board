#include "../include/fpga_api.h"
#include <cstring>
#define DATA_SIZE SIZE*(SIZE+1) // fpga bram data size

#define min(x,y) (((x)<(y))?(x):(y))

FPGA::FPGA(off_t data_addr, off_t api_addr)
{
    api_ = new unsigned int[SIZE];    // use api_ as tempolar output 
    data_ = new float[DATA_SIZE];	
}

FPGA::~FPGA()
{
    delete[] api_;
    delete[] data_;
}

float* FPGA::matrix(void)
{
	return data_ + SIZE;
}

float* FPGA::vector(void)
{
	return data_;
}

const float* FPGA::run()
{
    float* vec = this->vector();
    float* mat = this->matrix();
    float* out  = reinterpret_cast<float*>(api_);  

    for(int i = 0 ; i < SIZE; ++i)
    {
        out[i] = 0;

        for(int j = 0 ; j < SIZE; ++j)
           out[i] += vec[j] * mat[SIZE*i + j];
    }

	for(int i = 0 ; i < SIZE; ++i)
	{
		data_[i] = out[i];
	}

    return data_;    
}

void FPGA::largeMV(const float* large_mat, const float* input,
		float* output, int M, int N)
{
//	float* vec = this->vector();
//    float* mat = this->matrix();

    unsigned int *l_api = new unsigned int[N];    // use api_ as tempolar output
    float *l_data = new float[M*(N+1)];

    float* vec = l_data;
    float* mat = l_data + M;
    float* out  = reinterpret_cast<float*>(l_api);

    int rowsplit = M/SIZE;
    if(M%SIZE)
        rowsplit++;
    int colsplit = N/SIZE;
    if(N%SIZE)
        colsplit++;

    float temvec[SIZE*rowsplit];
    memcpy(temvec, vec, sizeof(float) * M);
    for (int i = M; i<SIZE*rowsplit; i++)
        temvec[i]=0;

    float matvec[N][SIZE*rowsplit];
    for(int i=0; i<N; i++)  {
        memcpy(matvec[i], mat+M*i, sizeof(float) * M);
        for(int j=M; j<SIZE*rowsplit; j++)
            matvec[i][j]=0;
    }

}
