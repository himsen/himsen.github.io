//Can easily be appended to report primes. 

import java.io.*;

/*Calculate the number of primes below(including) n(input). n must be below or equals to 1'400'897'506.
  If you want to input a number higher than the above one can use the command -Xmxym, where y is the number of MB you want to allocate to the java virtual machine.
  For example you can calculate 1.5 billion with: java -Xmx2048m ErastosthenesSieve 1'500'000'000. -Xmx2048m is equivalent to -Xmx2G. GigaByte instead of MegaByte. 
  The highest number n you input using the command -Xmx2048 is 2'100'000'000<n<2'200'000'000. For any higher you will get a NumberFormatException.
  We save a factor 2 of space in the sieving array, because we only represent the odd numbers. 
  to represent the odd numbers we use the map array[i] |-> 2i+1.
  It is only necessary to represent the odd numbers since primes are not even numbers, and odd numbers can never be a multiple of an even number. 
*/

/*
  UPDATE 18-04-2012
  One can now go higher since we can input a Long instead of Int. The highest number n you can input using the command -Xmx3072m is 4'290'000'000<n<4'300'000'000.
*/

/*
 * Update 22-05-2012
 * Changed it completely to avoid a lot of calculations. It still apply to take a number n aprox: 4'290'000'000<n<4'300'000'000
 */
public class EratosthenesSieve {
	public static void main(String[] args) throws IOException{

		final long startTime = System.currentTimeMillis();

		long n = Long.parseLong(args[0]);

		//FileWriter fstream = new FileWriter("primes.txt");
		//BufferedWriter out = new BufferedWriter(fstream);

		if(n<=1){
			System.out.println("No primes strictly below 1");
		}

		else if(n==2){
			 System.out.println("Number of primes equal to or below 2 is 1" ); //could be out.write
		}
		else{
			int sievingArraySize = (int)((n+1)/2);
			boolean[] sievingArray = new boolean[sievingArraySize]; 

			int prime;
			int count = 1;
			int loop = 1;
			System.out.println("Sieving.....");
			while(loop < (int)(Math.sqrt(n)/2+1)){
				if(sievingArray[loop] == false){ //
					count++;
					prime = 2*loop+1;
					for(int i = loop*(2*loop+2); i < sievingArraySize; i += prime){ //(2*loop+1)^2=2(2*loop^2+2*loop)+1=2(loop*(2*loop+2))+1. Could use i = loop + prime which is easier to see; 3(2*loop+1)= 6*loop+3=6*loop+2+1=2(3*loop+1)+1=2(2*loop+1+loop)2*(prime+loop)+1
						sievingArray[i] = true;
					}
					//out.write(prime + " ");
				}
				loop++;
			}
			final long sievingTime = System.currentTimeMillis();
			System.out.println("Sieving finished. Took " + (double)(sievingTime - startTime)/1000 + "sec." );
			
			System.out.println("Counting non flagged....."); //i.e. primes above sqrt(n)
			while(loop < sievingArraySize){
				if(sievingArray[loop] == false){
					count++;
					//out.write((2*loop+1) + " ");
				}
				loop++;
			}
			final long countingTime = System.currentTimeMillis();
			System.out.println("Counting finished. Took " + (double)(countingTime - sievingTime)/1000 + "sec." );
			
			final long endTime = System.currentTimeMillis();

			System.out.println("Number of primes equal to or below " + n + " is " + count + ". Time complexity: " + (double)(endTime - startTime)/1000 + "sec." ); // could be out.write
			if(n % 2 == 1){
				System.out.println(n + " is prime? = " + !sievingArray[sievingArraySize-1]);
			}
			else{
				System.out.println(n + " is prime? = " + false + " (n is even!)");
			}
			
		}	
		
		//Close the output stream
		//out.close();

	}
	
}
