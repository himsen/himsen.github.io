#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>

/*
* Input: The program will prompt for input (>1).
* Output: Number of primes below input (including the input)
* and execution time. 
*
* The program is meant as a auxilliary method that can be
* used in several ways; number of primes below a limit,
* expanded to return an array of the primes between 2 and the
* input. 
* We only represent odd numbers using the representation 
* i |-> 2*i+1 where i is the index in the sieving array 
* sieving_array, with the convention that 0 |-> 2. With 
* that convention we have 2*i+1 is a prime if and only if
* sieving_array[i]=1, when we exit the method. 
*/
main(){
  clock_t start = clock(); //start time

  long n; 
  long sieving_array_size = 0;
  char *sieving_array;
  int number_of_primes_below_n = 1; //2 is prime

  printf("Input limit:\n");
  scanf("%d",&n);

  if(n<2){
    printf("No primes below 2\n");
    exit(0);
  }
  else{
    sieving_array_size = (n+1)/2;
    sieving_array = (char*)malloc(sizeof(char) * sieving_array_size);
    if (!sieving_array) { 
    /* If sieving_array == 0 after the call to malloc, 
    allocation failed for some reason */
      perror("Error allocating memory\n");
      exit(0);
    }
    memset(sieving_array, 1, sizeof(char)*sieving_array_size);
  }

  int i;
  int loop = 1;
  int prime;
  while(loop < (int)(sqrt(n)-1)/2+2 ){
    if(sieving_array[loop] == 1){
      number_of_primes_below_n++;
      prime = 2*loop+1; //Sieving with this prime
      for(i = loop*(2*loop+2); i < sieving_array_size; i = i + prime){
        /* We start at prime^2=2*(2*loop^2+2*loop)+1 and since we do not
        *  need to sieve the even primes we sieve at every double 
        *  multiple of prime. */
        sieving_array[i] = 0;
      }		
    }	
    loop++;
  }
  while(loop < sieving_array_size){
    if(sieving_array[loop] == 1)
      number_of_primes_below_n++;
    loop++;
  }

  printf("Number of primes below (including) %d is %d\n", n, number_of_primes_below_n);

  clock_t end = clock();
  printf("Time: %f sec.\n", (float)(end - start)/CLOCKS_PER_SEC);
  //One should be a little bit careful with CLOCKS_PER_SEC.
  //It is not completely portable!
  return 0;
}
