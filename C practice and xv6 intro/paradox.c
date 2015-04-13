#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
typedef int bool;	//define boolean variable
#define true 1;
#define false 0;
FILE *ifp, *ofp;	//file pointer	
int opt;
while((opt = getopt(argc, argv, "i:o:"))!=-1)		//get command line argment
{
	switch(opt)
	{
	case 'i':					
	ifp = fopen(optarg, "r");			//open input file and check if it exist
	if(ifp == NULL)
	{
	fprintf(stderr, "Error: Cannot open file %s\n", optarg);
	exit(1);
	}
	
	
	int line = 0;
	int N[1000];
	double M[1000];
        while(!feof(ifp))
		{
		 fscanf(ifp, "%d", &N[line]);	//read each line into an array
		 //printf("%d\n", N[line]);     debug usage
		 int trial = 0;
		 int num_pt = 0;
		 while(trial < 1000)
		 {
		 	int individual =0;
			int b[1000];		//an array to accept those randomly generated numbers
			bool p_trial = 0;	//a boolean to identify if there is a match
			while(individual < N[line])
			{
				srand(trial + individual);  		//seed rand()
				b[individual] = rand() % 364 + 1;	//generate random number	
				int match = 0;				
				while(match < individual)
				{
					if((b[match] == b[individual])&&(match != individual)) //compare new random number with previous one.
					{
					p_trial = 1;	//if there is a match, set the flag true.
					}
					match++;				
				}
				individual++;	
			}
			trial++;
			if(p_trial)
			num_pt++;
		 }
		 M[line] = num_pt/1000.0;  //calculate probability and store it into an array
		 line++;
		}
	if(line > 1000)		       //print error massege if the file has more than 1000 numbers
		{
		fprintf(stderr, "Error: There are more than 1000 numbers in the file\n");
		exit(1);
		}


	fclose(ifp);
	break;
	
	case 'o':
	printf("o was %s\n", optarg);
	ofp = fopen(optarg, "w");			//open output file and check it
	if(ofp == NULL)
	{
	fprintf(stderr, "Error: Cannot open file %s\n", optarg);
	exit(1);
	}
	int i = 0;
	while(i < line-1){
	fprintf(ofp, "%.2f\n", M[i]);			//write array M into outputfile
	i++;
	}
	fclose(ofp);
	break;
	
	case ':':					//case that handle ilegal commands	
	fprintf(stderr, "Usage: paradox -i inputfile -o outputfile");
	exit(1);
	break;
	
	default:
	fprintf(stderr, "Usage: paradox -i inputfile -o outputfile");
	exit(1);
	break;
	}
}
return 0;
}

