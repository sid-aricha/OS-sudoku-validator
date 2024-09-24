
/*
Programming Assignment 2: Validating Sudoku Solution
Roll no.: CO21BTECH11008
With pthreads
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define mini 1e-30

int N, K;

int *sudoku;

void *runner(void *param);

int sqroot(double square) {
  double root = square / 3, last, diff = 1;
  if (square <= 0) return 0;
  do {
    last = root;
    root = (root + square / root) / 2;
    diff = root - last;
  } while (diff > mini || diff < -mini);
  return root;
}


typedef struct {
  int nrow;
  int start_row;
  int ncol;
  int start_col;
  int nsub;
  int start_sub;
  int thread_number;
  int *should_terminate;
  int in_index;
} thread_info;

typedef struct{
 char arr[100];
}sentence;

sentence *output;



int sub_valid(int s) {
  int sq_root = sqroot(N);
  int *valid_s = (int *)calloc(N, sizeof(int));
  int r = (s / sq_root) * sq_root;
  int c = (s % sq_root) * sq_root;
  for (int i = r; i < r + sq_root; i++) {
    for (int j = c; j < c + sq_root; j++) {
      if (sudoku[i * N + j] < 1 || sudoku[i * N + j] > 9 ||
          valid_s[sudoku[i * N + j] - 1] == 1) {
        return -1;
      } else {
        valid_s[sudoku[i * N + j] - 1] = 1;
      }
    }
  }
  free(valid_s);
  return s;
}

int row_valid(int r) {
  int *valid_r = (int *)calloc(N, sizeof(int));
  for (int j = 0; j < N; j++) {
    if (sudoku[r * N + j] < 1 || sudoku[r * N + j] > N ||
        valid_r[sudoku[r * N + j] - 1] == 1) {
      free(valid_r);
      return -1;
    } else {
      valid_r[sudoku[r * N + j] - 1] = 1;
    }
  }
  free(valid_r);
  return r;
}

int col_valid(int c) {
  int *valid_c = (int *)calloc(N, sizeof(int));
  for (int i = 0; i < N; i++) {
    if (sudoku[i * N + c] < 1 || sudoku[i * N + c] > N ||
        valid_c[sudoku[i * N + c] - 1] == 1) {
      free(valid_c);
      return -1;
    } else {
      valid_c[sudoku[i * N + c] - 1] = 1;
    }
  }
  free(valid_c);
  return c;
}

int main() {
  FILE *in_file = fopen("input.txt", "r");
  if (in_file == NULL) {
    printf("oops, file can't be read\n");
    exit(-1);
  }

  fscanf(in_file, "%d %d", &K, &N);
  sudoku = (int *)malloc(sizeof(int) * N * N);

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      fscanf(in_file, "%d", &sudoku[i * N + j]);
    }
  }
  fclose(in_file);
     clock_t start, end;
     double cpu_time_used;
     
     start = clock();

  pthread_t thread_id[K];
  
  thread_info threads[K];
  int n = N / K;
  int remainder = N - n * K;
  threads[0].start_row = 0;
  threads[0].start_col = 0;
  threads[0].start_sub = 0;
  int can_terminate=0;
  for (int i = 0; i < K; i++) {
    threads[i].nrow = n;
    threads[i].ncol = n;
    threads[i].nsub = n;
    threads[i].thread_number = i;
    threads[i].should_terminate=&can_terminate;
    threads[i].in_index=0;
  }
  for (int i = 0; i < remainder; i++) {
    threads[i].nrow++;
    threads[i].ncol++;
    threads[i].nsub++;
  }
  for (int i = 1; i < K; i++) {
    threads[i].start_row = (threads[i - 1].start_row) + (threads[i - 1].nrow);
    threads[i].start_col = (threads[i - 1].start_col) + (threads[i - 1].ncol);
    threads[i].start_sub = (threads[i - 1].start_sub) + (threads[i - 1].nsub);
  }
/*
  for (int i = 0; i < K; i++) {
    printf("%d %d %d %d %d %d\n", threads[i].start_row, threads[i].start_col,
           threads[i].start_sub, threads[i].nrow, threads[i].ncol,
           threads[i].nsub);
  }*/
  for (int i = 0; i < K; i++) {
    pthread_create(&thread_id[i], NULL, runner, (void *)&threads[i]);
  }
  
  for (int i = 0; i < K; i++) {
    pthread_join(thread_id[i], NULL);
  }

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  
  FILE *out_file = fopen("output.txt", "w"); 
  if (out_file == NULL) {
    printf("oops, output file can't be created\n");
    exit(-1);
  }
    int total=0;
  int validity=0;
  for(int i=0; i<K; i++){
 	if(*(threads[i].should_terminate)==0){
 	validity++;
 	}
 	total=total+threads[i].in_index;
 }
 for(int i=0; i<total; i++){
  fprintf(out_file,"%s\n",output[i].arr);
 }
 if(validity==K){
  fprintf(out_file,"Sudoku is valid\n");
  }
  else{
  fprintf(out_file,"Sudoku is invalid\n");
  }
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fprintf(out_file,"The total time taken is %lf microseconds\n", cpu_time_used*(1e+6));

  free(sudoku);
  free(output);
  return 0;
}
void *runner(void *param) {
  thread_info *temp = (thread_info *)param;
  
  output=(sentence *)malloc(sizeof(sentence)*3*N);
  int th_no = temp->thread_number;
  for(int i=0; i< temp->nrow; i++) {
    if(*(temp->should_terminate)==1){
    pthread_exit(0);
    }
    int r = row_valid(temp->start_row);
    if (r == -1) {
      //fprintf(out_file,"Thread %d checks %d row and is invalid\n", th_no,temp->start_row);
      snprintf(output[temp->in_index].arr,sizeof(output[temp->in_index].arr),"Thread %d checks %d row and is invalid",th_no,temp->start_row);
      temp->in_index++;
      temp->start_row++;
      *(temp->should_terminate)=1;
      pthread_exit(0);
    } 
    else {
      //fprintf(out_file,"Thread %d checks %d row and is valid\n", th_no, temp->start_row);
      snprintf(output[temp->in_index].arr,sizeof(output[temp->in_index].arr),"Thread %d checks %d row and is valid",th_no,temp->start_row);
      temp->in_index++;
      temp->start_row++;
    }
  }
  for (int j = 0; j < temp->ncol; j++) {
    if(*(temp->should_terminate)==1){
    pthread_exit(0);
    }
    int c = col_valid(temp->start_col);
    if (c == -1) {
      //fprintf(out_file,"Thread %d checks %d column and is invalid\n", th_no, temp->start_col);
      snprintf(output[temp->in_index].arr,sizeof(output[temp->in_index].arr),"Thread %d checks %d column and is invalid",th_no,temp->start_col);
      temp->in_index++;
      temp->start_col++;
      *(temp->should_terminate)=1;
      pthread_exit(0);
    } else {
      //fprintf(out_file,"Thread %d checks %d column and is valid\n", th_no, temp->start_col);
      snprintf(output[temp->in_index].arr,sizeof(output[temp->in_index].arr),"Thread %d checks %d column and is valid",th_no,temp->start_col);
      temp->in_index++;
      temp->start_col++;
    }
  }
  for (int k = 0; k < temp->nsub; k++) {
    if(*(temp->should_terminate)==1){
    	pthread_exit(0);
    }
    int s = sub_valid(temp->start_sub);
    if (s == -1) {
      //fprintf(out_file,"Thread %d checks %d subgrid and is invalid\n", th_no, temp->start_sub);
      snprintf(output[temp->in_index].arr,sizeof(output[temp->in_index].arr),"Thread %d checks %d subgrid and is invalid",th_no,temp->start_sub);
      temp->in_index++;
      temp->start_sub++;
      *(temp->should_terminate)=1;
      pthread_exit(0);
    } else {
      //fprintf(out_file,"Thread %d checks %d subgrid and is valid\n", th_no, temp->start_sub);
      snprintf(output[temp->in_index].arr,sizeof(output[temp->in_index].arr),"Thread %d checks %d subgrid and is invalid",th_no,temp->start_sub);
      temp->in_index++;
      temp->start_sub++;
    }
  }
}
