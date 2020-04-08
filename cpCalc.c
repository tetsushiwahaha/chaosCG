#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <cpDefs.h>


double f(double, double, double, double);
double g(double, double, double, double);

void read_zfile(FILE *, FILE *);
void write_zfile(FILE *, FILE *);

int main(int argc, char **argv)
{
	int i, j, k, c;

	FILE *fpin, *fpout;
	char pipe[BUFSIZ], fname[BUFSIZ];
	int fd;
	char basename[BUFSIZ] = "/tmp/xxx.XXXXXX";
	long count, mapmax;
	unsigned short int *data;
	int mode, inc_r, inc_g, inc_b;
	cpHeader *fig;
	double x, x0, x00, y, y0, y00,a, b, xmin, xmax, ymin, ymax;
	double u, v;
	int width, height;
	double scx, scy;
	double inc_h, inc_s, inc_v;

	if (argc != 2){
		fprintf(stderr, "usage: %s setfile outfile\n", argv[0]);
		exit(1);
	}

	sprintf(pipe, CPP_COMMAND_LINE, argv[1]);

	if ((fpin = popen(pipe, "r")) == NULL){
		perror("popen; ");
		exit(-1);
	}

	if ((fd = mkstemp(basename)) == -1){
		perror("mkstemp; ");
		exit(-1);
	}

	if((fpout = fdopen(fd,"w"))==NULL){
		perror("fdopen ");
		exit(-1);
	}
	
	/* read parameters for graphics */
	fscanf(fpin,"%d %d",&width, &height);
	fscanf(fpin, "%lf %lf", &xmin, &xmax);
	fscanf(fpin, "%lf %lf", &ymin, &ymax);
	fscanf(fpin, "%lf %lf", &x00, &y00);
	fscanf(fpin, "%lf %lf", &a, &b);
	fscanf(fpin, "%ld", &mapmax);

	if ((data = (unsigned short int *)
		calloc(width * height, sizeof (unsigned short int))) == NULL){
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}

	if ((fig = (cpHeader *) calloc(1, 
		sizeof (cpHeader))) == NULL){
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}
	scx = (xmax - xmin)/width;
	scy = (ymax - ymin)/height;

	count = 0;
	x0 = x00; y0 = y00;
	while(1){
		register int ite, ax, ay;
		x = f(x0, y0, a, b);
		y = g(x0, y0, a, b);
		x0 = x; y0 = y;
		/*
		printf("%lf %lf\n", x, y); fflush(stdout);
		*/
		if (fabs(x) + fabs(y) >300.0){ break; }
		if (x < xmax && x > xmin && y < ymax && y > ymin){
			ax = (int)((x - xmin) / scx);
			ay = (int)((y - ymin) / scy);
			/*
			ite = *(data + (height - ay) * width + ax);
			printf("%d %d\n", ax, ay); fflush(stdout);
			ite++;
			*(data + (height - ay) * width + ax) = ite;
			*/

			(*(data + (height - ay) * width + ax))++; 

			count++;
			if (count == mapmax){ break; }
		}
	}
	fig->id = CP_MAGIC;
	fig->width = width;
	fig->height = height;
	fig->iteration = mapmax;
	printf("putting histogram info into a file...");
	fflush(stdout);
	fwrite(fig, sizeof (cpHeader), 1, fpout);
	fwrite(data, sizeof (unsigned short int), width * height, fpout);
	printf("done.\n");
	fclose(fpout);

	if((fpin = fopen(basename, "r")) == NULL){
		perror("fopen for read ");
		exit(-1);
	}

	unlink(basename);

	sprintf(fname, "%s.cpz", argv[1]);
	if ((fpout = fopen(fname, "w")) == NULL){
		perror("fopen for writing ");
		exit(-1);
	}
	write_zfile(fpin, fpout);
	fclose(fpin);
	fclose(fpout);
}	
