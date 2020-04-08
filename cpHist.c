#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <png.h>
#include <cpDefs.h>

#define BUF 20
#define max(x, y) (x) < (y) ? (y) : (x)
#define min(x, y) (x) < (y) ? (x) : (y)

void read_zfile(FILE *, FILE *);
void write_zfile(FILE *, FILE *);


int main( int argc, char **argv)
{
	int i, j, k, c;
	int block, sup;
	int fd;
	FILE *fopen(), *fpz, *fpdata, *fpchunk; 
	long cel[BUF];
	char fname[BUFSIZ], basename[BUFSIZ] = "/tmp/xxx.XXXXXX", *str;

	unsigned short int *data, *data0;
	cpHeader *fig;
	int ix, iy, maximum = 0, minimum = 0;

	for (i = 0; i < BUF; i++){ cel[i] = 0; }
	if (argc != 2) {
		fprintf(stderr, "usage: %s cpzfile\n", argv[0]);
		exit(0);
	}
	
	strcpy(fname, argv[1]);
	str = rindex(fname, '.');
	if (str == NULL){
		strcat(fname, ".cpz");
	} else {
		str++;
		if (!strcmp(str, ".cpz")){
			fprintf(stderr, "%s is not cpz file\n", argv[1]);
			exit(-1);
		}
	}

	if((fpz = fopen(fname, "r"))==NULL){
		perror("fopen ");
		exit(-1);
	}
    if((fd = mkstemp(basename)) == -1){
		perror("mkstemp ");
		exit(-1);
	}

	if((fpchunk = fdopen(fd, "w")) == NULL){
		perror("fdopen ");
		exit(-1);
	}

	read_zfile(fpz, fpchunk);

	fclose(fpchunk);

	if((fpchunk = fopen(basename, "r"))==NULL){
		perror("fopen");
		exit(-1);
	}
	unlink(basename);

	if ((fig = (cpHeader *)calloc(1, sizeof (cpHeader))) == NULL){
		perror("calloc ");
		exit(-1);
	}
	fread(fig, sizeof (cpHeader), 1, fpchunk);

	if (fig->id != CP_MAGIC){
		printf("magic = %x\n", fig->id);
		fprintf(stderr, "%s is not figdata\n", argv[1]);
		exit(0);
	}

	ix = fig->width;
	iy = fig->height;

	if ((data = (unsigned short int *)calloc(ix * iy,
		sizeof (unsigned short int))) == NULL){
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}

	fread(data, sizeof (unsigned short int), ix * iy, fpchunk);
	fclose(fpchunk);

	data0 = data;
	for (i = 0; i < ix; i++){
		for (j = 0; j < iy; j++){
			register int ite;
			/* ite = *(data + i * ix + j);
			*/
			ite = *data0++;
			{int m; m = max(maximum, ite); maximum = m;}
		}
	}
	printf("maximum = %d\n", maximum);
	if (maximum < BUF){
		printf("vely spreaded data.\n");
		exit(-1);
	}
	data0 = data;
	printf("sup? = ");
	scanf("%d", &sup);
	block = sup / BUF;
	for (i = 0; i < ix; i++){
		for (j = 0; j < iy; j++){
			register int ite, c;
			/*
			ite = *(data + i * ix + j);
			*/
			ite = *data0++;
			c = ite / block; 
			if (c < BUF) cel[c]++;
		}
	}
	printf("  n  : frequancy\n");

	for (i = 0; i < BUF; i++){
		printf(" %6d : %10ld\n", sup / BUF * (i+1), cel[i]);
	}
}	

