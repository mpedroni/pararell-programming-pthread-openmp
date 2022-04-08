#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include <time.h>

#define MATRIX_MAX_VALUE 100
#define MATRIX_MAX_SIZE 8
#define MAX_THREADS 8

typedef enum
{
  Sequential = 1,
  ParallelPthread,
  ParallelOpenMP
} processing_methods;

typedef enum
{
  Matrix = 1,
  Positional,
} multiplication_type;

typedef char matrix_type;

matrix_type A[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE] /* = {
     {1, 2, 3},
     {4, 5, 6},
     {7, 8, 9}}*/
    ;
matrix_type B[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE] /* = {
     {9, 8, 7},
     {6, 5, 4},
     {3, 2, 1}}*/
    ;

matrix_type C[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];

void print_matrix(matrix_type matrix[][MATRIX_MAX_SIZE], char label[])
{
  printf("%s:\n", label);

  for (int i = 0; i < MATRIX_MAX_SIZE; i++)
  {
    for (int j = 0; j < MATRIX_MAX_SIZE; j++)
      printf("%d\t", (matrix_type)matrix[i][j]);

    printf("\n");
  }
}

void clear_matrix(matrix_type matrix[][MATRIX_MAX_SIZE])
{
  for (int i = 0; i < MATRIX_MAX_SIZE; i++)
    for (int j = 0; j < MATRIX_MAX_SIZE; j++)
      matrix[i][j] = 0;
}

void *get_resultant_matrix_row(void *row)
{
  int i = *((int *)row);

  for (int j = 0; j < MATRIX_MAX_SIZE; j++)
    C[i][j] = A[i][j] * B[j][i];

  free(row);
}

void *get_resultant_matrix_row_using_positional_multiplication(void *row)
{
  int i = *((int *)row);

  for (int j = 0; j < MATRIX_MAX_SIZE; j++)
    C[i][j] = A[i][j] * B[i][j];

  free(row);
}

void multiply_matrices_sequential(multiplication_type type)
{
  for (int i = 0; i < MATRIX_MAX_SIZE; i++)
    for (int j = 0; j < MATRIX_MAX_SIZE; j++)
      C[i][j] =
          type == Positional
              ? A[i][j] * B[i][j]
              : A[i][j] * B[j][i];
}

void multiply_matrices_pthreads(multiplication_type type)
{
  pthread_t threads[MAX_THREADS];

  for (int i = 0; i < MAX_THREADS; i++)
  {
    int *arg = malloc(sizeof(*arg));
    *arg = i;
    pthread_create(
        &threads[i],
        NULL,
        type == Positional
            ? get_resultant_matrix_row_using_positional_multiplication
            : get_resultant_matrix_row,
        arg);
  }

  for (int i = 0; i < MAX_THREADS; i++)
    pthread_join(threads[i], NULL);

  return;
}

void multiply_matrices_openmp(multiplication_type type)
{
  if (type == Positional)
  {

#pragma omp parallel for
    for (int i = 0; i < MATRIX_MAX_SIZE; i++)
    {
      int *arg = malloc(sizeof(*arg));
      *arg = i;
      get_resultant_matrix_row_using_positional_multiplication(arg);
    }
  }
  else
  {
#pragma omp parallel for
    for (int i = 0; i < MATRIX_MAX_SIZE; i++)
    {
      int *arg = malloc(sizeof(*arg));
      *arg = i;
      get_resultant_matrix_row(arg);
    }
  }
}

double multiply_matrices(processing_methods method, multiplication_type type)
{

  time_t start, finish;
  start = clock();

  switch (method)
  {
  case Sequential:
    multiply_matrices_sequential(type);
    break;

  case ParallelPthread:
    multiply_matrices_pthreads(type);
    break;

  case ParallelOpenMP:
    multiply_matrices_openmp(type);
    break;

  default:
    break;
  }

  finish = clock();

  // return the operation time in miliseconds
  return (double)(finish - start) / CLOCKS_PER_SEC * 1000;
}

int main()
{
  // unsigned int seed = 1;
  // srand(seed);

  for (int i = 0; i < MATRIX_MAX_SIZE; i++)
    for (int j = 0; j < MATRIX_MAX_SIZE; j++)
    {
      // A[i][j] = (matrix_type)rand() % MATRIX_MAX_VALUE;
      // B[i][j] = (matrix_type)rand() % MATRIX_MAX_VALUE;
      A[i][j] = (matrix_type)random();
      B[i][j] = (matrix_type)random();
    }

  print_matrix(A, "");

  double duration_in_miliseconds;

  duration_in_miliseconds = multiply_matrices(Sequential, Matrix);
  printf("sequential\n\t%fms\n", duration_in_miliseconds);

  duration_in_miliseconds = multiply_matrices(ParallelPthread, Matrix);
  printf("\npthread\n\t%fms\n", duration_in_miliseconds);

  duration_in_miliseconds = multiply_matrices(ParallelOpenMP, Matrix);
  printf("\nopenmp\n\t%fms\n", duration_in_miliseconds);

  ///////////

  duration_in_miliseconds = multiply_matrices(Sequential, Positional);
  printf("\n\nsequential with positional multi\n\t%fms\n", duration_in_miliseconds);

  duration_in_miliseconds = multiply_matrices(ParallelPthread, Positional);
  printf("\npthread with positional multi\n\t%fms\n", duration_in_miliseconds);

  duration_in_miliseconds = multiply_matrices(ParallelOpenMP, Positional);
  printf("\nopenmp with positional multi\n\t%fms\n", duration_in_miliseconds);

  print_matrix(A, "A");
  printf("\n");

  print_matrix(B, "B");
  printf("\n");

  print_matrix(C, "C");

  return 0;
}