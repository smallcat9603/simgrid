#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// #define N (1024 * 1024 * 1)
#define absErrBound         0.000001 //default 0.0001=2^{-12} (-13?), 0.000001=2^{-20}, 0.00001=2^{-16}, 0.001=2^{-10}, 0.01=2^{-7}
#define absErrBound_binary  20 //bitwise, SZ, equal to above
#define CT                  1 //compress type for pingpong & himeno & k-means, 0 no compress, 1 mycompress, 2 no-lossy-performance, 3 no-lossy-area, 4 sz, 5 bitwise
#define byte_or_bit         2 //1 byte, 2 bit
#define tp                  0 //0 uniform, 1 matrix, 2 reversal

int reversal(int src, int num);
int matrix(int src, int num, int array_size, int dimension);
int uniform(int src, int num);

int myCompress_double(double[], double**, char**, int**, int);
double* myDecompress_double(double[], char[], int[], int);
// static void alltoalls0c(pfftss_plan);
// static void alltoalls1c(pfftss_plan);
double* myDecompress_bitwise_double(unsigned char*, int, int);
double decompress_bitwise_double(char*, int, double, double, double);
void myCompress_bitwise_double(double[], int, unsigned char**, int*, int*);
void compress_bitwise_double(double, unsigned char**, int*, int*);
double toSmallDataset_double(double[], double**, int);
void doubletostr(double*, char*);
double strtodbl(char*);
void add_bit_to_bytes(unsigned char**, int*, int*, int);
void bit_set(unsigned char*, unsigned char, int);
float calcCompressionRatio_sz_double(double[], int);
float calcCompressionRatio_nolossy_performance_double(double[], int);
float calcCompressionRatio_nolossy_area_double(double[], int);
void getDoubleBin(double,char[]);

int main(int argc, char *argv[])
{
  int size, rank;
  struct timeval start, end;
  char hostname[256];
  int hostname_len;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Get_processor_name(hostname,&hostname_len);

  //src-dst mapping (tag = src rank)
  int *srcdst = malloc(sizeof(int) * size);
  if(tp == 0) //uniform
  {
    for(int i=0; i<size; i++)
    {
      srcdst[i] = (i+1)%size;
    }
  }
  else if(tp == 1) //matrix
  {
    for(int i=0; i<size; i++)
    {
      srcdst[i] = matrix(i, size, 4, 3); //array_size = 4, dimension = 3
      if(rank == 0) printf("%d, ", srcdst[i]);
    }    
  }
  else if(tp == 2) //reversal
  {
    for(int i=0; i<size; i++)
    {
      srcdst[i] = reversal(i, size);
    }    
  }

  // read data file
  char output_filename[32];
  sprintf(output_filename, "testdouble_8_8_128.txt");
  FILE *fp = fopen(output_filename, "r");
  double *data = NULL; //data array
  int n; //data number = n-1
  for (n=0; !feof(fp); n++) 
  {
    data = (double *)(data?realloc(data,sizeof(double)*(n+1)):malloc(sizeof(double)));
    fscanf(fp, "%lf", data+n);
    // printf("%f\t", data[n]);
  }
  fclose(fp);
  // free(data);
  int data_num = n - 1; 

  if(rank == 0)
  {
    float sz_comp_ratio = calcCompressionRatio_sz_double(data, data_num);
    float nolossy_performance = calcCompressionRatio_nolossy_performance_double(data, data_num);
    float nolossy_area = calcCompressionRatio_nolossy_area_double(data, data_num);
    printf("compression ratio: sz %f, nolossy_performance %f, nolossy_area %f \n", 1/sz_comp_ratio, 1/nolossy_performance, 1/nolossy_area);
  }

  int dst = srcdst[rank];
  int src;
  for(int i = 0; i < size; i++)
  {
    if(srcdst[i] == rank)
    {
      src = i;
    }
  }

  double* data_small = NULL;
  double data_min = toSmallDataset_double(data, &data_small, data_num);

  if(CT == 0)
  {
    double *buffer = malloc(sizeof(double) * data_num);

    MPI_Request* mr = (MPI_Request *)malloc(sizeof(MPI_Request) * 2); 
    MPI_Status* ms = (MPI_Status *)malloc(sizeof(MPI_Status) * 2); 

    MPI_Irecv(buffer, data_num, MPI_DOUBLE, src, src, MPI_COMM_WORLD, &mr[0]);
    MPI_Isend(data, data_num, MPI_DOUBLE, dst, rank, MPI_COMM_WORLD, &mr[1]);
    MPI_Waitall(2, mr, ms);
  }
  else if(CT == 1)
  {
    int array_double_len_send;
    int array_double_len_recv;

    double* array_double_send = NULL;
    char* array_char_send = NULL;
    int* array_char_displacement_send = NULL;

    MPI_Request* mr0 = (MPI_Request *)malloc(sizeof(MPI_Request) * 2); 
    MPI_Status* ms0 = (MPI_Status *)malloc(sizeof(MPI_Status) * 2); 

    MPI_Irecv(&array_double_len_recv, 1, MPI_INT, src, src, MPI_COMM_WORLD, &mr0[0]);
    int array_double_len = myCompress_double(data, &array_double_send, &array_char_send, &array_char_displacement_send, data_num);
    array_double_len_send = array_double_len;
    MPI_Isend(&array_double_len, 1, MPI_INT, dst, rank, MPI_COMM_WORLD, &mr0[1]); 
    MPI_Waitall(2, mr0, ms0);

    double* array_double_recv; 
    char* array_char_recv; 
    int* array_char_displacement_recv; 

    MPI_Request* mr1 = (MPI_Request *)malloc(sizeof(MPI_Request) * 6); 
    MPI_Status* ms1 = (MPI_Status *)malloc(sizeof(MPI_Status) * 6); 

    int num_p = array_double_len_recv, num_c = data_num - array_double_len_recv;
    array_double_recv = (double*) malloc(sizeof(double)*num_p);
    array_char_recv = (char*) malloc(sizeof(char)*num_c);
    array_char_displacement_recv = (int*) malloc(sizeof(int)*num_c);
    MPI_Irecv(array_double_recv, num_p, MPI_DOUBLE, src, src, MPI_COMM_WORLD, &mr1[0]);
    MPI_Irecv(array_char_recv, num_c, MPI_CHAR, src, src, MPI_COMM_WORLD, &mr1[1]);
    MPI_Irecv(array_char_displacement_recv, num_c, MPI_INT, src, src, MPI_COMM_WORLD, &mr1[2]);    
    num_p = array_double_len_send;
    num_c = data_num - array_double_len_send;
    if(rank == 0)
    {
      float compress_ratio = (float)(num_c*sizeof(char)+num_p*sizeof(double))/((num_c+num_p)*sizeof(double));
      printf("compress ratio = %f \n", 1/compress_ratio);
    }
    MPI_Isend(array_double_send, num_p, MPI_DOUBLE, dst, rank, MPI_COMM_WORLD, &mr1[3]); 
    MPI_Isend(array_char_send, num_c, MPI_CHAR, dst, rank, MPI_COMM_WORLD, &mr1[4]); 
    MPI_Isend(array_char_displacement_send, num_c, MPI_INT, dst, rank, MPI_COMM_WORLD, &mr1[5]); 

    MPI_Waitall(6, mr1, ms1);

    double* decompressed_data = myDecompress_double(array_double_recv, array_char_recv, array_char_displacement_recv, data_num);
    double gosa = 0;
    if(rank == 0)
    {
      for(int j = 0; j < data_num; j++)
      {
        gosa += decompressed_data[j] - data[j];
      }
      gosa = gosa/data_num;
      printf("gosa is %.10f\n", gosa);   
    }
  }
  else if(CT == 5)
  {
    MPI_Request* mr0 = (MPI_Request *)malloc(sizeof(MPI_Request) * 2); 
    MPI_Status* ms0 = (MPI_Status *)malloc(sizeof(MPI_Status) * 2);     

    int data_bytes_recv;
    MPI_Irecv(&data_bytes_recv, 1, MPI_INT, src, src, MPI_COMM_WORLD, &mr0[0]);

    unsigned char* data_bits_send = NULL;
    //int flag = 0; //0, 1
    int bytes = 0; //total bytes of compressed data
    int pos = 8; //position of filled bit in last byte --> 87654321

    myCompress_bitwise_double(data_small, data_num, &data_bits_send, &bytes, &pos);
    if(rank == 0)
    {
      float compress_ratio = bytes*8.0/(data_num*sizeof(double)*8); 
      printf("compress ratio = %f \n", 1/compress_ratio);
    }
    MPI_Isend(&bytes, 1, MPI_INT, dst, rank, MPI_COMM_WORLD, &mr0[1]); 

    MPI_Waitall(2, mr0, ms0);

    MPI_Request* mr1 = (MPI_Request *)malloc(sizeof(MPI_Request) * 2); 
    MPI_Status* ms1 = (MPI_Status *)malloc(sizeof(MPI_Status) * 2); 

    unsigned char* data_bits_recv = (unsigned char*) malloc(sizeof(unsigned char)*data_bytes_recv);
    MPI_Irecv(data_bits_recv, data_bytes_recv, MPI_UNSIGNED_CHAR, src, src, MPI_COMM_WORLD, &mr1[0]);
    MPI_Isend(data_bits_send, bytes, MPI_UNSIGNED_CHAR, dst, rank, MPI_COMM_WORLD, &mr1[1]); 

    MPI_Waitall(2, mr1, ms1); 

    //bitwise decompress
    double* decompressed_data = myDecompress_bitwise_double(data_bits_recv, data_bytes_recv, data_num);
    double gosa = 0;
    if(rank == 0)
    {
      for(int j = 0; j < data_num; j++)
      {
        gosa += decompressed_data[j] + data_min - data[j];
      }
      gosa = gosa/data_num;
      printf("gosa is %.10f\n", gosa);         
    }
  }

  // // Allocate a 1 MiB buffer
  // char *buffer = malloc(sizeof(char) * N);

  // // Communicate along the ring
  // if (rank == 0) {
  //       gettimeofday(&start,NULL);
  //       printf("Rank %d (running on '%s'): sending the message rank %d\n",rank,hostname,1);
	// MPI_Send(buffer, N, MPI_BYTE, 1, 1, MPI_COMM_WORLD);
  //      	MPI_Recv(buffer, N, MPI_BYTE, size-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //       printf("Rank %d (running on '%s'): received the message from rank %d\n",rank,hostname,size-1);
  // 	gettimeofday(&end,NULL);
  // 	printf("%f\n",(end.tv_sec*1000000.0 + end.tv_usec -
	// 	 	start.tv_sec*1000000.0 - start.tv_usec) / 1000000.0);

  // } else {
  //      	MPI_Recv(buffer, N, MPI_BYTE, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //       printf("Rank %d (running on '%s'): receive the message and sending it to rank %d\n",rank,hostname,(rank+1)%size);
	// MPI_Send(buffer, N, MPI_BYTE, (rank+1)%size, 1, MPI_COMM_WORLD);
  // }

  MPI_Finalize();
  return 0;
}

/// nonuniform(bit reversal) ///
int reversal(int src, int num)
{
	int	dst = 0;
	int int_length = sizeof(int)*8;
	int bit_length = 1;

	// node bit length is decided by network size
	for (int x = 1; x < int_length; x++)
	{
		if ( (pow(2,x) <= num-1) && (pow(2,x+1) > num-1) )
			bit_length = x+1;
	}

	// bit order reverse
	for (int i = bit_length - 1,j=0; i >= 0; i--,j++)
	{
		if (src & (1 << i))
			dst |= 1 << j;
	}

	if (dst == src)
	{
		dst = ~dst;
		// adjust other bits
		for (int i = int_length -1; i > bit_length-1; i--)
		{
			dst ^= (1 << i);
		}
	}

  return dst;
}

/// nonuniform(matrix transpose) ///
int matrix(int src, int num, int array_size, int dimension)
{
	int	b_coord[dimension+1]; // before adjustment
	int	x,i;
	int dst = 0;

	// coordinates before adjustment
	for (i=1; i <= dimension; i++)
	{
		x = src/(int)pow(array_size, i-1);
		if (i != dimension)
		{
			x %= array_size;
		}
		b_coord[i] = x;
	}

	// coordinate adjustment
	for (i=1; i <= dimension; i++)
	{
		x = array_size - 1 - b_coord[dimension-i+1];
		dst += (x % array_size)*(int)pow(array_size, i-1);
	}

	if (src == dst)
	{
		dst = 0;
		for (i=1; i <= dimension; i++)
		{
			x = array_size - 1 - b_coord[i];
			dst += (x % array_size)*(int)pow(array_size, i-1);
		}
	}

	if (src == dst)
	{
		printf("Coordinate was not changed!!");
		exit(1);
	}

  return dst;
}

/// uniform(random) ///
int uniform(int src, int num)
{
  int dst;
  do
  {
    time_t t;
    srand((unsigned) time(&t));
    //srand((unsigned int)time(0));
    dst = rand()%num;
  } while (dst == src);
  return dst;
}

//myDecompress for k-means (double)
double* myDecompress_double(double array_double[], char array_char[], int array_char_displacement[], int num)
{
  double* data = (double*) malloc(sizeof(double)*num);
  int array_double_p = 0, array_char_p = 0, array_char_displacement_p = 0;
  for(int i=0; i<num; i++)
  {
    if(array_char_displacement[array_char_displacement_p] - 1 == i)
    {
      if(array_char[array_char_p] == 'a')
      {
        data[i] = data[i-1];
      }
      else if(array_char[array_char_p] == 'b')
      {
        data[i] = 2*data[i-1] - data[i-2];
      }
      else if(array_char[array_char_p] == 'c')
      {
        data[i] = 3*data[i-1] - 3*data[i-2] + data[i-3];
      }
      else if(array_char[array_char_p] == 'd')
      {
        data[i] = 4*data[i-1] - 6*data[i-2] + 4*data[i-3] - data[i-4];
      }      
      array_char_p++;
      array_char_displacement_p++;
    }
    else
    {
      data[i] = array_double[array_double_p];
      array_double_p++;
    }
  }
  return data;
}

//myCompress for k-means (double)
int myCompress_double(double data[], double** array_double, char** array_char, int** array_char_displacement, int num)
{
  double real_value, before_value1=-1, before_value2=-1, before_value3=-1, before_value4=-1, predict_value1, predict_value2, predict_value3, predict_value4;
  double diff1, diff2, diff3, diff4, diff_min, selected_predict_value;
  int array_double_len = 0, array_char_len = 0;
  char compress_type;
  double* array_double_more = NULL;
  char* array_char_more = NULL;
  int* array_char_displacement_more = NULL;

  for(int n=0; n<num; n++)
  {
    real_value = data[n];

    if(before_value4 == -1 || before_value3 == -1 || before_value2 == -1 || before_value1 == -1)
    {
      array_double_len++;
      array_double_more = (double*)realloc(*array_double, sizeof(double)*array_double_len);
      if (array_double_more != NULL) 
      {
        *array_double = array_double_more;
        (*array_double)[array_double_len-1] = real_value;
      }
      else 
      {
        free(*array_double);
        printf("Error (re)allocating memory1");
        exit(1);
      }        

      if(before_value4 == -1) 
      {
        before_value4 = real_value; 
      }      
      else if(before_value3 == -1) 
      {
        before_value3 = real_value; 
      }
      else if(before_value2 == -1) 
      {
        before_value2 = real_value;
      }
      else if(before_value1 == -1) 
      {
        before_value1 = real_value;
      }        
    }
    else
    {
      predict_value1 = before_value1;
      predict_value2 = 2*before_value1 - before_value2;
      predict_value3 = 3*before_value1 - 3*before_value2 + before_value3;
      predict_value4 = 4*before_value1 - 6*before_value2 + 4*before_value3 - before_value4;

      diff1 = fabs(predict_value1-real_value);
      diff2 = fabs(predict_value2-real_value);
      diff3 = fabs(predict_value3-real_value);
      diff4 = fabs(predict_value4-real_value);

      diff_min = diff1;
      compress_type = 'a';
      selected_predict_value = predict_value1;
      if(diff2<diff_min)
      {
        diff_min = diff2;
        compress_type = 'b';
        selected_predict_value = predict_value2;
      }
      if(diff3<diff_min)
      {
        diff_min = diff3;
        compress_type = 'c';
        selected_predict_value = predict_value3;
      }   
      if(diff4<diff_min)
      {
        diff_min = diff4;
        compress_type = 'd';
        selected_predict_value = predict_value4;
      }            

      before_value4 = before_value3;
      before_value3 = before_value2;
      before_value2 = before_value1;
      before_value1 = real_value;
      
      if(diff_min<=absErrBound) 
      {
        array_char_len++;
        array_char_more = (char*)realloc(*array_char, sizeof(char)*array_char_len);
        array_char_displacement_more = (int*)realloc(*array_char_displacement, sizeof(int)*array_char_len);
        if (array_char_more != NULL && array_char_displacement_more != NULL) 
        {
          *array_char = array_char_more;
          (*array_char)[array_char_len-1] = compress_type;
          *array_char_displacement = array_char_displacement_more;
          (*array_char_displacement)[array_char_len-1] = array_double_len + array_char_len;
        }
        else 
        {
          free(*array_char);
          free(*array_char_displacement);
          printf("Error (re)allocating memory2");
          exit(1);
        } 
      }
      else 
      {
        array_double_len++;
        array_double_more = (double*)realloc(*array_double, sizeof(double)*array_double_len);
        if (array_double_more != NULL) 
        {
          *array_double = array_double_more;
          (*array_double)[array_double_len-1] = real_value;
        }
        else 
        {
          free(*array_double);
          printf("Error (re)allocating memory3");
          exit(1);
        }             
      }
    }
  }
  return array_double_len;
}

//100.0 --> 0100000001011001000000000000000000000000000000000000000000000000
//str should have at least 65 byte.
void doubletostr(double* a, char* str){
	long long c;
	c= ((long long*)a)[0]; 
	for(int i=0;i<64;i++){
		str[63-i]=(char)(c&1)+'0';
		c>>=1;
	}
	str[64] = '\0';
}

//0100000001011001000000000000000000000000000000000000000000000000 --> 100.0
double strtodbl(char * str){
	long long dbl = 0;
	for(int i=0;i<63;i++){
		dbl += (str[i]-'0');
		dbl <<= 1;
	}
	dbl +=(str[63]-'0');
	double* db = (double*)&dbl;
	return *db;
} 

double* myDecompress_bitwise_double(unsigned char* data_bits, int bytes, int num)
{
  int offset_bits = 0;
  char* bits = NULL;
  char* bits_more = NULL;
  int bits_num = 0;
  int min_shift = 0;

  double before_value1=-1, before_value2=-1, before_value3=-1;
  double* decompressed = (double*) malloc(sizeof(double)*num);
  int decompressed_num = 0;

  for(int i=0; i<bytes; i++)
  {
    //if(i == bytes - 1 && pos != 8) min_shift = pos;

    for (int j=7; j>=min_shift; j--) //each bit of byte
    {
      int bit = (data_bits[i] >> j) & 1;

      //printf("%d(%d)", bit, bits_num);

      if(offset_bits == 0) //start bit
      {
        if(bits_num == 0) //not start bit of mantissa
        {
          if(bit == 0)
          {
            offset_bits = 1+11;
            bits_num++;
            bits_more = (char*)realloc(bits, sizeof(char)*bits_num);
            if (bits_more != NULL) 
            {
              bits = bits_more;
              bits[bits_num-1] = bit + '0';
            }
            else 
            {
              free(bits);
              printf("Error (re)allocating memory4\n");
              exit(1);
            }             
          }
          else if(bit == 1)
          {
            offset_bits = 3; //100, 101, 110, 111
            bits_num++;
            bits_more = (char*)realloc(bits, sizeof(char)*bits_num);
            if (bits_more != NULL) 
            {
              bits = bits_more;
              bits[bits_num-1] = bit + '0';
            }
            else 
            {
              free(bits);
              printf("Error (re)allocating memory5\n");
              exit(1);
            }             
          }
        }
        else //start bit of mantissa
        {
          int expo_value = 0;
          for(int i=1; i<12; i++)
          {
            expo_value += (bits[i]-'0')*pow(2,11-i);
          }
          expo_value -= 1023;

          int mantissa_bits_within_error_bound = absErrBound_binary + expo_value;
          if(mantissa_bits_within_error_bound > 52) //23 mantissa part of float (52 in the case of double)
          {
            mantissa_bits_within_error_bound = 52;
          }
          else if(mantissa_bits_within_error_bound < 0)
          {
            mantissa_bits_within_error_bound = 0;
          }

          offset_bits = mantissa_bits_within_error_bound;

          if(offset_bits > 0) //has mantissa bits
          {
            bits_num++;
            bits_more = (char*)realloc(bits, sizeof(char)*bits_num);
            if (bits_more != NULL) 
            {
              bits = bits_more;
              bits[bits_num-1] = bit + '0';
            }
            else 
            {
              free(bits);
              printf("Error (re)allocating memory6\n");
              exit(1);
            } 
          }
          else //no mantissa bit
          {
            decompressed_num++;
            decompressed[decompressed_num-1] = decompress_bitwise_double(bits, bits_num, before_value1, before_value2, before_value3);
            // printf("%f ", decompressed[decompressed_num-1]);

            if(before_value3 == -1) 
            {
              before_value3 = decompressed[decompressed_num-1]; 
            }
            else if(before_value2 == -1) 
            {
              before_value2 = decompressed[decompressed_num-1];
            }
            else if(before_value1 == -1) 
            {
              before_value1 = decompressed[decompressed_num-1];
            }
            else
            {
              before_value3 = before_value2;
              before_value2 = before_value1;
              before_value1 = decompressed[decompressed_num-1];
            }

            bits = NULL;
            bits_num = 0;

            if(bit == 0)
            {
              offset_bits = 1+11;
              bits_num++;
              bits_more = (char*)realloc(bits, sizeof(char)*bits_num);
              if (bits_more != NULL) 
              {
                bits = bits_more;
                bits[bits_num-1] = bit + '0';
              }
              else 
              {
                free(bits);
                printf("Error (re)allocating memory7\n");
                exit(1);
              }             
            }
            else if(bit == 1)
            {
              offset_bits = 3;
              bits_num++;
              bits_more = (char*)realloc(bits, sizeof(char)*bits_num);
              if (bits_more != NULL) 
              {
                bits = bits_more;
                bits[bits_num-1] = bit + '0';
              }
              else 
              {
                free(bits);
                printf("Error (re)allocating memory8\n");
                exit(1);
              }             
            }              
          }
        }
      }
      else
      {
        bits_num++;
        bits_more = (char*)realloc(bits, sizeof(char)*bits_num);
        if (bits_more != NULL) 
        {
          bits = bits_more;
          bits[bits_num-1] = bit + '0';
        }
        else 
        {
          free(bits);
          printf("Error (re)allocating memory9\n");
          exit(1);
        }        
      }
      offset_bits--;
      if(offset_bits == 0 && bits_num != 1+11)
      {
        decompressed_num++;
        decompressed[decompressed_num-1] = decompress_bitwise_double(bits, bits_num, before_value1, before_value2, before_value3);
        // printf("%f ", decompressed[decompressed_num-1]);
        
        if(before_value3 == -1) 
        {
          before_value3 = decompressed[decompressed_num-1]; 
        }
        else if(before_value2 == -1) 
        {
          before_value2 = decompressed[decompressed_num-1];
        }
        else if(before_value1 == -1) 
        {
          before_value1 = decompressed[decompressed_num-1];
        }
        else
        {
          before_value3 = before_value2;
          before_value2 = before_value1;
          before_value1 = decompressed[decompressed_num-1];
        }

        bits = NULL;
        bits_num = 0;
      }
    }       
  }
  return decompressed;
}

double decompress_bitwise_double(char* bits, int bits_num, double before_value1, double before_value2, double before_value3)
{
  if(bits_num == 3)
  {
    if(bits[0] == '1')
    {
      if(bits[1] == '0' && bits[2] == '0')
      {
        return 0.0;
      }
      else if(bits[1] == '0' && bits[2] == '1')
      {
        return before_value1;
      }
      else if(bits[1] == '1' && bits[2] == '0')
      {
        return 2*before_value1 - before_value2;
      }
      else if(bits[1] == '1' && bits[2] == '1')
      {
        return 3*before_value1 - 3*before_value2 + before_value3;
      }
    }
    else
    {
      printf("Error start bit of 3 bits is 0\n");
      exit(1);
    }
  }
  else
  {
    if(bits_num == sizeof(double)*8)
    {
      return strtodbl(bits);
    }
    else
    {
      char* bits64 = (char*)realloc(bits, sizeof(double)*8);
      bits64[bits_num] = '1';
      if(bits_num+1 < sizeof(double)*8)     
      {
        for(int i=bits_num+1; i< sizeof(double)*8; i++)
        {
          bits64[i] = '0';
        }
      }
      return strtodbl(bits64); 
    }
  }
}

//bitwise myCompress for k-means (double)
void myCompress_bitwise_double(double data[], int num, unsigned char** data_bits, int* bytes, int* pos)
{
  double real_value, before_value1=-1, before_value2=-1, before_value3=-1, predict_value1, predict_value2, predict_value3;
  double diff1, diff2, diff3, diff_min, selected_predict_value;
  char compress_type;
  int a=0, b=0, c=0, d=0;

  // unsigned char* data_bits = NULL;
  // int flag = 0; //0, 1
  // int bytes = 0; //total bytes of compressed data
  // int pos = 8; //position of filled bit in last byte --> 87654321

  for(int n=0; n<num; n++)
  {
    real_value = data[n];

    if(before_value3 == -1 || before_value2 == -1 || before_value1 == -1)
    {
      //if(real_value == 0)
      if(fabs(real_value) < absErrBound)
      {
        add_bit_to_bytes(data_bits, bytes, pos, 1);
        add_bit_to_bytes(data_bits, bytes, pos, 0);
        add_bit_to_bytes(data_bits, bytes, pos, 0);
        d++;
      }
      else
      {
        compress_bitwise_double(real_value, data_bits, bytes, pos);
      }       
      
      if(before_value3 == -1) 
      {
        before_value3 = real_value; 
      }
      else if(before_value2 == -1) 
      {
        before_value2 = real_value;
      }
      else if(before_value1 == -1) 
      {
        before_value1 = real_value;
      }        
    }
    else
    {
      predict_value1 = before_value1;
      predict_value2 = 2*before_value1 - before_value2;
      predict_value3 = 3*before_value1 - 3*before_value2 + before_value3;

      diff1 = fabs(predict_value1-real_value);
      diff2 = fabs(predict_value2-real_value);
      diff3 = fabs(predict_value3-real_value);

      diff_min = diff1;
      compress_type = 'a'; //101
      selected_predict_value = predict_value1;
      if(diff2<diff_min)
      {
        diff_min = diff2;
        compress_type = 'b'; //110
        selected_predict_value = predict_value2;
      }
      if(diff3<diff_min)
      {
        diff_min = diff3;
        compress_type = 'c'; //111
        selected_predict_value = predict_value3;
      }        

      before_value3 = before_value2;
      before_value2 = before_value1;
      before_value1 = real_value;
      
      if(fabs(real_value) < absErrBound)
      {
        add_bit_to_bytes(data_bits, bytes, pos, 1);
        add_bit_to_bytes(data_bits, bytes, pos, 0);
        add_bit_to_bytes(data_bits, bytes, pos, 0);
        d++;
      }
      else if(diff_min<=absErrBound) 
      {
        if(compress_type == 'a')
        {
          add_bit_to_bytes(data_bits, bytes, pos, 1);
          add_bit_to_bytes(data_bits, bytes, pos, 0);
          add_bit_to_bytes(data_bits, bytes, pos, 1);     
          a++;   
        }
        else if(compress_type == 'b')
        {
          add_bit_to_bytes(data_bits, bytes, pos, 1);
          add_bit_to_bytes(data_bits, bytes, pos, 1);
          add_bit_to_bytes(data_bits, bytes, pos, 0);  
          b++;
        }
        else if(compress_type == 'c')
        {
          add_bit_to_bytes(data_bits, bytes, pos, 1);
          add_bit_to_bytes(data_bits, bytes, pos, 1);
          add_bit_to_bytes(data_bits, bytes, pos, 1);  
          c++;
        }
        else
        {
          printf("Error compress_type\n");
          exit(1);
        }
      }
      else 
      {
        compress_bitwise_double(real_value, data_bits, bytes, pos);            
      }
    }
  }

  //printf("compression pattern: a = %d (%f), b = %d (%f), c = %d (%f), d = %d (%f), num = %d\n", a, (float)a/num, b, (float)b/num, c, (float)c/num, d, (float)d/num, num);
}

void compress_bitwise_double(double real_value, unsigned char** data_bits, int* bytes, int* pos)
{
  double double10 = real_value;
  char double_arr[64+1];
  doubletostr(&double10, double_arr);

  int expo_value = 0;
  for(int i=1; i<12; i++)
  {
    expo_value += (double_arr[i]-'0')*pow(2,11-i);
  }
  expo_value -= 1023;

  int mantissa_bits_within_error_bound = absErrBound_binary + expo_value;

  if(mantissa_bits_within_error_bound > 52) //23 mantissa part of float (52 in the case of double)
  {
    mantissa_bits_within_error_bound = 52;
  }
  else if(mantissa_bits_within_error_bound < 0)
  {
    mantissa_bits_within_error_bound = 0;
  }
  int bits_after_compress = 1+11+mantissa_bits_within_error_bound;  

  for(int i=0; i<bits_after_compress; i++)
  {
    add_bit_to_bytes(data_bits, bytes, pos, double_arr[i]-'0');
  }
}

double toSmallDataset_double(double data[], double** data_small, int num)
{
  *data_small = malloc(sizeof(double) * num);
  double min = data[0];

  for(int i=1; i<num; i++)
  {
    if(data[i]<min)
    {
      min = data[i];
    }
  }

  for(int i=0; i<num; i++)
  {
    (*data_small)[i] = data[i] - min;
  }

  return min;
}

float calcCompressionRatio_sz_double(double data[], int num)
{
  double real_value, before_value1=-1, before_value2=-1, before_value3=-1, predict_value1, predict_value2, predict_value3;
  double diff1, diff2, diff3, diff_min, predict_diff, selected_predict_value;
  char compress_type;
  float compress_ratio;
  long origin_bits=0, compressed_bits=0;

  for(int n=0; n<num; n++)
  {
    real_value = data[n]; 

    origin_bits += sizeof(double)*8;     

    if(before_value3 == -1 || before_value2 == -1 || before_value1 == -1)
    { 
      compressed_bits += sizeof(double)*8; 
      if(before_value3 == -1) 
      {
        before_value3 = real_value; 
      }
      else if(before_value2 == -1) 
      {
        before_value2 = real_value;
      }
      else if(before_value1 == -1) 
      {
        before_value1 = real_value;
      }        
    }
    else
    {
      predict_value1 = before_value1;
      predict_value2 = 2*before_value1 - before_value2;
      predict_value3 = 3*before_value1 - 3*before_value2 + before_value3;

      diff1 = fabs(predict_value1-real_value);
      diff2 = fabs(predict_value2-real_value);
      diff3 = fabs(predict_value3-real_value);

      diff_min = diff1;
      compress_type = 'a';
      selected_predict_value = predict_value1;
      if(diff2<diff_min)
      {
        diff_min = diff2;
        compress_type = 'b';
        selected_predict_value = predict_value2;
      }
      if(diff3<diff_min)
      {
        diff_min = diff3;
        compress_type = 'c';
        selected_predict_value = predict_value3;
      }        

      before_value3 = before_value2;
      before_value2 = before_value1;
      before_value1 = real_value;

      if(diff_min<=absErrBound) 
      {
        if(byte_or_bit == 1)
        {
          compressed_bits += sizeof(char)*8; 
        }
        else if(byte_or_bit == 2)
        {
          compressed_bits += 2; 
        }
      }
      else 
      {
        double max, min;
        if(predict_value1 > predict_value2)
        {
          max = predict_value1;
          min = predict_value2;
        }
        else
        {
          max = predict_value2;
          min = predict_value1;
        }
        if(predict_value3 > max)
        {
          max = predict_value3;
        }
        else if(predict_value3 < min)
        {
          min = predict_value3;
        }
        
        predict_diff = max-min;

        char c[sizeof(double)*8];
        getDoubleBin(predict_diff/2, c);
        int expo_value = 0;
        int mantissa_bits_within_error_bound;

        for(int i=1;i<12;i++) //1-9 exponential part of float (1-12 in the case of double)
        {
          if(c[i] != 0) 
          {
            expo_value += pow(2, 11-i);
          }  
        }
        expo_value -= 1023;
        mantissa_bits_within_error_bound = absErrBound_binary + expo_value;
        if(mantissa_bits_within_error_bound > 52) //23 mantissa part of float (52 in the case of double)
        {
          mantissa_bits_within_error_bound = 52;
        }
        else if(mantissa_bits_within_error_bound < 0)
        {
          mantissa_bits_within_error_bound = 0;
        }
        if(byte_or_bit == 1)
        {
          if(mantissa_bits_within_error_bound%8 != 0) compressed_bits += 1+11+(mantissa_bits_within_error_bound/8+1)*8;  
          else compressed_bits += 1+11+mantissa_bits_within_error_bound;
        }
        else if(byte_or_bit == 2)
        {
          compressed_bits += 1+11+mantissa_bits_within_error_bound;  
        }
      }
    }
  }
  compress_ratio = (float)compressed_bits/origin_bits;
  return compress_ratio;
}

float calcCompressionRatio_nolossy_performance_double(double data[], int num)
{
  double real_value, before_value1=-1, before_value2=-1, before_value3=-1, before_value4=-1, predict_value4;
  double diff4;
  float compress_ratio;
  long origin_bits=0, compressed_bits=0;

  for(int n=0; n<num; n++)
  {
    real_value = data[n]; 

    origin_bits += sizeof(double)*8;

    if(before_value4 == -1 || before_value3 == -1 || before_value2 == -1 || before_value1 == -1)
    {
      compressed_bits += sizeof(double)*8;       
      
      if(before_value4 == -1) 
      {
        before_value4 = real_value; 
      }
      else if(before_value3 == -1) 
      {
        before_value3 = real_value; 
      }
      else if(before_value2 == -1) 
      {
        before_value2 = real_value;
      }
      else if(before_value1 == -1) 
      {
        before_value1 = real_value;
      }        
    }
    else
    {
      predict_value4 = 4*before_value1 - 6*before_value2 + 4*before_value3 - before_value4;

      diff4 = predict_value4-real_value;     

      before_value4 = before_value3;
      before_value3 = before_value2;
      before_value2 = before_value1;
      before_value1 = real_value;

      char c[sizeof(double)*8];
      getDoubleBin(diff4, c);
      for(int i=1;i<sizeof(double)*8;i++)
      {
        if(c[i] != 0) 
        {
          if(byte_or_bit == 1)
          {
            if((sizeof(double)*8 - i + 3 + 1)%8 != 0) compressed_bits += ((sizeof(double)*8 - i + 3 + 1)/8+1)*8;  
            else compressed_bits += sizeof(double)*8 - i + 3 + 1;
          }
          else if(byte_or_bit == 2)
          {
            compressed_bits += sizeof(double)*8 - i + 3 + 1;
          }
          break;
        } 
      }
    }    
  }
  compress_ratio = (float)compressed_bits/origin_bits;
  return compress_ratio;  
}

float calcCompressionRatio_nolossy_area_double(double data[], int num)
{
  double real_value, before_value1=-1, before_value2=-1, before_value3=-1, before_value4=-1, predict_value4;
  double diff4;
  float compress_ratio;
  long origin_bits=0, compressed_bits=0;
  int cdb = 512, cdb_num = 1, occupied_bits = 0, indication = 5, le = 285, re = 222, re1 = 2, re2 = 4, re3 = 32, llrb = 2, ex = 1;

  for(int n=0; n<num; n++)
  {
    real_value = data[n]; 

    origin_bits += sizeof(double)*8;

    if(before_value4 == -1 || before_value3 == -1 || before_value2 == -1 || before_value1 == -1)
    {
      occupied_bits += re3+llrb+ex;       
      
      if(before_value4 == -1) 
      {
        before_value4 = real_value; 
      }
      else if(before_value3 == -1) 
      {
        before_value3 = real_value; 
      }
      else if(before_value2 == -1) 
      {
        before_value2 = real_value;
      }
      else if(before_value1 == -1) 
      {
        before_value1 = real_value;
      }        
    }
    else
    {
      predict_value4 = 4*before_value1 - 6*before_value2 + 4*before_value3 - before_value4;

      diff4 = predict_value4-real_value;     

      before_value4 = before_value3;
      before_value3 = before_value2;
      before_value2 = before_value1;
      before_value1 = real_value;

      char c[sizeof(double)*8];
      getDoubleBin(diff4, c);
      for(int i=1;i<sizeof(double)*8;i++)
      {
        if(c[i] != 0) 
        {
          int nonzero = sizeof(double)*8 - i;
          int data_bits;
          if(nonzero <= re1)
          {
            data_bits = re1+llrb+ex;
          }
          else if(nonzero <= re2)
          {
            data_bits = re2+llrb+ex;
          }
          else if(nonzero <= re3)
          {
            data_bits = re3+llrb+ex;
          }
          
          if(occupied_bits + data_bits > cdb-indication)
          {
            cdb_num++;
            occupied_bits = data_bits;
          }
          else
          {
            occupied_bits += data_bits;
          }

          break;
        }  
      }
    }    
  }
  compressed_bits = cdb_num*cdb;
  compress_ratio = (float)compressed_bits/origin_bits;
  return compress_ratio;    
}

void add_bit_to_bytes(unsigned char** data_bits, int* bytes, int* pos, int flag)
{
  if(*pos > 0 && *pos < 9)
  {
    if(*pos == 8) 
    {
      (*bytes)++;
      unsigned char* data_bits_more = (unsigned char*)realloc(*data_bits, sizeof(char)*(*bytes));
      if (data_bits_more != NULL) 
      {
        *data_bits = data_bits_more;
        (*data_bits)[*bytes-1] = 0; //put all 8 bits to 0
        bit_set(&((*data_bits)[*bytes-1]), *pos, flag);
        (*pos)--;
      }
      else 
      {
        free(*data_bits);
        printf("Error (re)allocating memory0\n");
        exit(1);
      }         
    }
    else{
      bit_set(&((*data_bits)[(*bytes)-1]), *pos, flag);
      (*pos)--;     
    }
    if(*pos == 0) *pos = 8;
  }
  else
  {
    printf("Error position value\n");
    return;
  }
}

// n*8 bits, position --> 87654321, flag --> 1, 0
void bit_set(unsigned char *p_data, unsigned char position, int flag)
{
	// int i = 0;
	assert(p_data);
	if (position > 8 || position < 1 || (flag != 0 && flag != 1))
	{
		printf("输入有误！\n");
		return;
	}
	if (flag != (*p_data >> (position - 1) & 1))
	{
		*p_data ^= 1 << (position - 1);
	}
	// for (i = 7; i >= 0; i--)     //由低地址的位开始输出。
	// {
	// 	printf("%d", (*p_data >> i) & 1);
	// }
	// printf("\n");
}

void getDoubleBin(double num,char bin[])
{
    int t = 1;
    int *f = (int*)(&num);
    for(int i=0;i<64;i++)
    {
        bin[i] = (*f)&(t<<63-i)?1:0;
    }
}
