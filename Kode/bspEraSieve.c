#include "bspedupack.h"

#define SZLONG (sizeof(long))

/*  This program computes the number of primes betwen 2 and n (including). 
    In addition the user may chose to save the primes on each processor
    or both save and print out (standard output) all primes. 

    Options: 
      0 - Ordinary parallel sieve that count primes below n.
      1 - Store all primes found, on each processor.
      2 - Same as 1 but includes a printout of all primes found.
          This is done by processor 0. 

    Notes:
      - We only represent odd numbers. 
      - Block distribution. Invariant: Processor 0 contains all odd
        numbers from 3 to at least sqrt(n). 
        Block size on processor s: 
        floor((s+1)*(floor((n-1)/2) / p)) - floor(s*(floor((n-1)/2) / p))
      - Processor zero takes care of finding the next sieving prime.
*/

/* Global fields */
int P; /* number of processors requested */
long n; /* limit */
char flagOption; /* Self explaining */


void bspSoe(){

  void Soe(int s, int leftIndex, int rightIndex, int szArray, int primeIndex, char sievingArray[]);
  int szLocalSievingArray = 0; /* Local sieving array size */
  char *sievingArray; /* Local sieving array */
  int *Primes; /* Stores primes - if option is flagged */
  int *Accumulate; /* To distribute results later */
  //long *Multiples; /* For becnhmarking */
  long number_of_flops=0; /* Local number of flops */
  int i, j, p, primeIndex, s, leftEnd, rightEnd, number_of_primes_below_n = 0;
  double time0, time1;

  bsp_begin(P);
  p= bsp_nprocs(); /* Number of processors obtained */ 
  s= bsp_pid();    /* Processor number */ 

  /* Make sure every processor knows everything */
  bsp_push_reg(&n,SZLONG);
  bsp_push_reg(&flagOption,sizeof(char));
  bsp_push_reg(&primeIndex,SZINT);
  bsp_sync();

  bsp_get(0,&n,0,&n,SZLONG);
  bsp_get(0,&flagOption,0,&flagOption,sizeof(char));
  bsp_sync();
  bsp_pop_reg(&n);
  bsp_pop_reg(&flagOption);

  /* Define size of arrays for distributing results */
  Accumulate = vecalloci(p);
  //Multiples = (long *)malloc(sizeof(long) * p); 
  bsp_push_reg(Accumulate, p*SZINT);
  //bsp_push_reg(Multiples, p*SZLONG);

  if(s==0)
    printf("Setting up distribution\n"); fflush(stdout);

  /* Setup distribution. We might need to leave out some
     processors because we enforce that processor 0 contain
     odd integers from 3 up to at least sqrt(n). 'p' will 
     be the number of processors used. This situation will
     not happen very often. Infact if n>10000 we need more
     than 100 processers before any restriction will take place.
  */
  
  /* Avoid multiple computations */
  int szGlobalArray = floor((n-1)/2); 
  leftEnd = 2*s*(szGlobalArray / p)+3; 
  rightEnd = 2*(s+1)*(szGlobalArray / p)+1; 

  /* Allocate sieving arrays */
  szLocalSievingArray = floor((s+1)*(szGlobalArray / p)) - floor(s*(szGlobalArray / p));
  sievingArray = (char*)malloc(sizeof(char) * szLocalSievingArray);
  if (!sievingArray && n > 2) { 
    /* If sievingArray == 0 after the call to malloc, 
    allocation failed for some reason */
    bsp_abort("Error allocating memory\n");
  }
  /* Set all cells to 1 */
  memset(sievingArray, 1, sizeof(char)*szLocalSievingArray);

  if(s==0)
    printf("Sieving\n"); fflush(stdout);

  primeIndex = 0; /* First sieving prime(3) index  */

  time0 = bsp_time();

  /* Below is the main sieving.
     Benchmark: Each time we hit a prime, we do at most 
     12 flops to calculate the offset and initialize 
     the sieving plus the number of multiples we need 
     to calculate for the prime.
  */
  while(primeIndex < (int)((sqrt(n)-3)/2)+1){
    //number_of_flops += (rightEnd % (2*primeIndex+3))+12 - (leftEnd % (2*primeIndex+3)); 
    Soe(s, leftEnd, rightEnd, szLocalSievingArray, primeIndex, sievingArray); 
    if(s==0){ /* Find next sieving prime. This is done in processor 0. */
      do{
        primeIndex++;
      }
      while(sievingArray[primeIndex] == 0 && primeIndex < (int)((sqrt(n)-3)/2)+1);
    }
    bsp_sync(); /* To enforce the BSP scheme. */
    if(s==0){
      for(i=1; i<p; i++){
        bsp_put(i,&primeIndex,&primeIndex,0,SZINT);
      }
    }
    bsp_sync();
  }
  bsp_pop_reg(&primeIndex);
  
  if(s==0)
    printf("Counting primes\n"); fflush(stdout);

  /* Count non-flagged elements - these are the primes */
  for(i=0; i<szLocalSievingArray; i++){
    if(sievingArray[i]==1)
      number_of_primes_below_n += 1;
  }
  bsp_sync():

  /* Distribute results */
  for(i=0;i<p;i++){
    bsp_put(i, &number_of_primes_below_n, Accumulate, s*SZINT, SZINT);
    //bsp_put(i, &number_of_flops, Multiples, s*SZLONG, SZLONG);
  }
  bsp_sync();

  /* Calculate results on each processor */
  number_of_primes_below_n = 0;
  //number_of_flops = 0;
  for(i=0;i<p;i++){
    number_of_primes_below_n += Accumulate[i];
    //number_of_flops += Multiples[i];
  }
  number_of_primes_below_n += 1; //Two is a prime

  /* If flagged: Store primes from 1 to n on all processors. */
  if(flagOption > 0){ 
    if(s==0)
      printf("Storing primes\n"); fflush(stdout);

    Accumulate[0] -= 1; //Store 2 adhoc. 
    int temp = 0;
    for(i=0;i<p;i++){
       temp += Accumulate[i];
       Accumulate[i] = -Accumulate[i]+temp;
    }

    Primes = vecalloci(number_of_primes_below_n); 
    bsp_push_reg(Primes, (number_of_primes_below_n)*SZINT);
    bsp_sync();

    int index = 0, pr = 2, k;
    for(i=0;i<p;i++)
      bsp_put(i, &pr, Primes, 0, SZINT);

    for(j=0;j<szLocalSievingArray;j++){
      if(sievingArray[j]==1){
        pr=leftEnd+2*j;
        for(k=0;k<p;k++){
	  bsp_put(k,&pr,Primes, SZINT*(Accumulate[s]+index+1),SZINT);
        }
        index++;
      }
    }
    bsp_sync();

    /* If flagged: Print out primes to standard output */
    if(flagOption > 1){
      if(s==0){
        int size = 10;
        if(n > 1000000)
          size = 5;
        printf("Primes from 2 to %ld:",n); fflush(stdout);
        for(i=0;i<number_of_primes_below_n;i++){
          if(i % size == 0)
            printf("\n");
          printf("%d, ",Primes[i]); fflush(stdout);
        }
      }
      printf("The end.\n");
      bsp_sync();
    }
  }
  
  time1 = bsp_time();
  printf("Processor %d: Number of primes from 2 to %ld is %d\n", s, n, number_of_primes_below_n); fflush(stdout);
  if(s==0){
    printf("This took %.6lf seconds.\n",time1-time0);
   // printf("Number of FLOPS (approx): %ld\n", number_of_flops);
  }

  bsp_end();
} /* End bspSoe*/


void Soe(int s, int leftEnd, int rightEnd, int szLocalSievingArray, int primeIndex, char sievingArray[]){

  int offset, prime;
  int calcOffset(int prime, int leftEnd, int rightEnd);

  prime = 2*primeIndex+3; /* Sieving prime */
  if(s == 0)
    offset = 2*primeIndex*(primeIndex+3)+3;
  else /* Save some time in proc 0. Might be redundant if the bottleneck is somewhere else */
    offset = calcOffset(prime, leftEnd, rightEnd);

  if(offset != -1) {  
    while(offset < szLocalSievingArray){
      /* Since we do not need to sieve the even 
         primes we sieve at every double multiple 
         of the sieving prime. 
      */
      sievingArray[offset] = 0;
      offset = offset + prime;
    }	
  }
} /* End Soe */


/* Calculate offset into processer s with left endpoint 'leftEnd'
   (recall we only represent odd numbers). RETURN -1 if prime 
   does not hit the interval represented in s.
*/
int calcOffset(int prime, int leftEnd, int rightEnd){
  if(leftEnd % prime == 0)
    return 0;
  int res = -(leftEnd % prime) + prime;
  int value = res + leftEnd;
  if(value > rightEnd)
    /* For small n we might skip a whole interval. e.g. 
       when n=25, p=4 and when sieving with 5 we skip 
       the second interval.
    */ 
    return -1;
  if(value % 2 == 0) 
    /* We need this to be odd because we only represent odd numbers */
    return (res+prime)/2;
  else 
    return res/2;
} /* End calcOffset */


int main(int argc, char **argv){

    int temp;

    bsp_init(&bspSoe, argc, argv);

    /* sequential part */
    printf("How many processors do you want to use?\n"); fflush(stdout);
    scanf("%d",&P);
    if (P > bsp_nprocs()){
        printf("Sorry, not enough processors available.\n"); fflush(stdout);
        exit(1);
    }

    printf("Please enter n:\n"); fflush(stdout);
    scanf("%ld",&n);
    if(n<2){
      printf("Error in input: No primes stricly smaller than 2!!\n");
      exit(1);
    }

    temp = (floor((n-1)/2)) / (floor(sqrt(n)));

    if(P > temp && n > 2){ 
      P = temp;
      printf("Can only use %d processor/s!\n", P); fflush(stdout);
    }

    printf("Options: 0 nothing, 1 store primes, 2 store and print out primes.\n"); fflush(stdout);
    scanf("%d",&flagOption);
    if(flagOption < 0 || 2 < flagOption){
      printf("Error in input: Options are 0, 1 or 2 only!\n");
      exit(1);
    }

    /* SPMD part */
    bspSoe();

    /* sequential part */
    exit(0);
} /* end main */

