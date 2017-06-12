#include "../include/fpga_api.h"
#include <cstring>
#include <iostream>
using namespace std;
#include <stdio.h>
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
    //float* vec = this->vector();
    //float* mat = this->matrix();

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
    memcpy(temvec, input, sizeof(float) * M);
    for (int i = M; i<SIZE*rowsplit; i++)
        temvec[i]=0;

    //for(int i = 0; i<SIZE*rowsplit; i++)
    //	cout<<"temvec["<<i<<"] : "<<temvec[i]<<endl;

    //float matvec[N][SIZE*rowsplit];
    float matvec[SIZE*colsplit][SIZE*rowsplit];
    for(int i=0; i<N; i++)  {
        memcpy(matvec[i], large_mat+M*i, sizeof(float) * M);
        for(int j=M; j<SIZE*rowsplit; j++)
            matvec[i][j]=0;
    }
    for(int i=N; i<SIZE*colsplit; i++)
        for(int j=0; j<SIZE*rowsplit; j++)
            matvec[i][j]=0;

    //for(int i=0; i<SIZE*rowsplit; i++) {
    //	for(int j=0; j<SIZE*colsplit; j++)
    //		cout<<"matvec["<<i<<"]["<<j<<"]: "<<matvec[i][j]<<"|";
    //	cout<<endl;
    //}

    float resultvec[SIZE*colsplit];
    for(int i=0; i<SIZE*colsplit; i++){
        resultvec[i]=0;
        //	cout<<"resultvec["<<i<<"] : "<<resultvec[i]<<endl;
    }


    float tempmat[SIZE*SIZE];
    for(int i=0; i<rowsplit; i++) {
        for(int j=0; j<colsplit; j++) {
            for(int k=0; k<SIZE; k++) {
                memcpy(tempmat+SIZE*k, matvec[j*SIZE+k]+SIZE*i, sizeof(float)*SIZE);
                //cout<<"copy to tempmat["<<SIZE*k<<"]~["<<SIZE*(k+1)-1<<"] from matvec["<<j*SIZE+k<<"]["<<SIZE*i<<"]"<<endl;
            }

            /*int counter = 0;
            cout<<"["<<j<<":"<<i<<"]"<<endl;
            for(int t=0; t<SIZE; t++) {
                for(int s=0; s<SIZE; s++)
                    if(tempmat[s*SIZE+t]!=0){
                        //cout<<"["<<s*SIZE+t<<"] : "<<tempmat[s*SIZE+t]<<" ";
                        printf("[%d] : %0.3f",s*SIZE+t, tempmat[s*SIZE+t]);
                        counter++;
                    }
                cout<<endl;
            }
            cout<<counter<<"\n\n\n"<<endl;*/


            memcpy(data_+SIZE, tempmat, sizeof(float)*(SIZE*SIZE));
            memcpy(data_, temvec+SIZE*i, sizeof(float)*SIZE);


            /*for(int t=0; t<SIZE; t++)
                cout<<"temvec["<<i<<"]  "<<data_[t]<<"     "<<temvec[SIZE*i+t]<<endl;
            cout<<"\n\n\n"<<endl;*/

            /*for(int t=0; t<SIZE; t++)
                for(int s=0; s<SIZE; s++)
                cout<<t<<":"<<s<<"-"<<"tempmat["<<i<<j<<"]  "<<data_[SIZE+t*SIZE+s]<<"     "<<tempmat[t*SIZE+s]<<endl;
            cout<<"\n\n\n"<<endl;*/

            this->run();

            /*cout<<"index["<<i<<"]"<<endl;
            for(int t=0; t<SIZE; t++)
                cout<<"data_["<<t<<"] : "<<this->data_[t]<<endl;
            cout<<endl;*/

            for(int k=0; k<SIZE; k++)
                resultvec[SIZE*j+k] +=this->data_[k];
        }
    }
    //cout<<"index["<<i<<"]"<<endl;
//    for(int t=0; t<SIZE*colsplit; t++)
//        cout<<"resultvec["<<t<<"] : "<<resultvec[t]<<endl;
//    cout<<endl;
    memcpy(output, resultvec, sizeof(float)*N);
}
