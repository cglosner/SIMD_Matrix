#include <immintrin.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <sys/times.h>
#include <time.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

struct sysinfo memInfo;

std::vector<std::vector<float>> mmFloating(std::vector<std::vector<float>> a, std::vector<std::vector<float>> b)
{
  std::vector<std::vector<float>> c(a.size(), std::vector<float> (a.size(), 0));

  for(int i = 0; i < a.size(); i++)
  {
    for(int j = 0; j < a.size(); j++)
    {
      __m256 result = _mm256_setzero_ps();
      for(int k = 0; k < a.size(); k+=8)
      {
        __m256 a_line = _mm256_loadu_ps(&a[i][k]);
        __m256 b_line = _mm256_loadu_ps(&b[j][k]);

        result = _mm256_add_ps(result, _mm256_mul_ps(a_line, b_line));


        result = _mm256_hadd_ps(result, result);
        result = _mm256_hadd_ps(result, result);
        result[1] = result[4];
        result = _mm256_hadd_ps(result, result);

        //DEBUG
        //std::cout << "a row: " << a_line[0] << " " << a_line[1] << " "
        //                       << a_line[2] << " " << a_line[3] << " " << std::endl;

        //std::cout << "b row: " << b_line[0] << " " << b_line[1] << " "
        //                       << b_line[2] << " " << b_line[3] << " " << std::endl;

        //std::cout << "result row: " << result[0] << " " << result[1] << " "
        //                            << result[2] << " " << result[3] << " "
        //                            << result[4] << " " << result[5] << " "
        //                            << result[6] << " " << result[7] << " " << std::endl;
      }
      c[i][j] = result[0];
    }
  }
  return c;
}


//need to have -march=sandybridge, -march=haswell, -march=knl in compiler

std::vector<std::vector<float>> mmStandard(std::vector<std::vector<float>> a, std::vector<std::vector<float>> b)
{
  std::vector<std::vector<float>> c(a.size(), std::vector<float> (a.size(), 0));
  for(int i = 0; i < a.size(); i++)
  {
    for(int j = 0; j < a.size(); j++)
    {
      for(int k = 0; k < a.size(); k++)
      {
        c[i][j] += a[i][k]*b[k][j];
      }
    }
  }
  return c;
}

std::vector<std::vector<float>> transpose(std::vector<std::vector<float>> a)
{
  std::vector<std::vector<float>> c(a.size(), std::vector<float> (a.size(), 0));
  for(int i = 0; i < a.size(); i++)
  {
    for(int j = 0; j < a.size(); j+=8)
    {
      __m256 tmp = _mm256_loadu_ps(&a[i][j]);
      c[j][i] = tmp[0];
      if((j+1) < a.size())
      {
        c[j+1][i] = tmp[1];
      }
      if((j+2) < a.size())
      {
        c[j+2][i] = tmp[2];
      }
      if((j+3) < a.size())
      {
        c[j+3][i] = tmp[3];
      }
      if((j+4) < a.size())
      {
        c[j+4][i] = tmp[4];
      }
      if((j+5) < a.size())
      {
        c[j+5][i] = tmp[5];
      }
      if((j+6) < a.size())
      {
        c[j+6][i] = tmp[6];
      }
      if((j+7) < a.size())
      {
        c[j+7][i] = tmp[7];
      }
    }
  }
  return c;
}

void printMatrix(std::vector<std::vector<float>> a, std::string title)
{
  std::cout << title << std::endl;
  for(int i = 0; i < a.size(); i++)
  {
    for(int j = 0; j < a.size(); j++)
    {
      std::cout << (int)a[i][j] << " ";
    }
    std::cout << std::endl;
  }
}


int main(void)
{
  //text matrix mutiplication
  //create two random input matrices
  //and one output matrix

  int size = 1000;
  for(int k = 0; k < 2; k++)
  {
    if(k == 1)
    {
      size = 10000;
    }
    else if(k == 2)
    {
      size = 100000;
    }
    std::vector<std::vector<float>> in1(size, std::vector<float> (size, 0));
    std::vector<std::vector<float>> in2(size, std::vector<float> (size, 0));

    for(int i = 0; i < size; i++)
    {
      for(int j = 0; j < size; j++)
      {
        in1[i][j] = rand() % 10;
        in2[i][j] = rand() % 10;
      }
    }

    std::cout << "Matrix size: " << size << "x" << size << std::endl;

    /*
    sysinfo (&memInfo);

    long long physMemUsedBefore = memInfo.totalram - memInfo.freeram;
    physMemUsedBefore *= memInfo.mem_unit;
    std::cout << "Standard Memory Usage: " << std::fixed << physMemUsedBefore << std::setprecision(9);
    std::cout << std::endl;
    */
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<float>> out1 = mmStandard(in1, in2);
    auto stop = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    duration /= 1e6;

    std::cout << "Standard Implementation Runtime: " << std::fixed << duration << std::setprecision(9);
    std::cout << "sec" << std::endl;



    start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<float>> out2 = mmFloating(in1, transpose(in2));
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    duration /= 1e6;
    std::cout << "SIMD Implementation Runtime: " << std::fixed << duration << std::setprecision(9);
    std::cout << "sec" << std::endl;

    std::cout << std::endl;




    //DEBUG
    /*
    printMatrix(in1, "Input 1: ");
    printMatrix(in2, "Input 2: ");
    printMatrix(out1, "Standard Output: ");
    printMatrix(out2, "SIMD Output: ");
    */
  }
}
