#include <stdio.h>
#include <unistd.h>
#include <png.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <cpDefs.h>
#define BUF 20
#define MAXTHRESHOLD 100
#define max(x, y) (x) < (y) ? (y) : (x)
#define min(x, y) (x) < (y) ? (x) : (y)


void read_zfile(FILE *, FILE *);
void write_zfile(FILE *, FILE *);

int save_png(char *fname, cpHeader *h, cpIRGBList *data);


int main(int argc, char **argv)
{
	int i, j, k, c;
	int count;
	void hsv2rgb(struct rgblist *, struct hsvlist *);
	void rgb2hsv(struct rgblist *, struct hsvlist *);
	FILE *fopen(), *fpin,  *fpchunk, *fpz; 
	char *tmpname;
	char pipe[BUFSIZ], fname[BUFSIZ];
	int fd;
	char basename[BUFSIZ] = "/tmp/xxx.XXXXXX";
	char *str;
	unsigned short int *data;
	cpIRGBList *colband, *colptr, *ptr, *dataRGB;
	struct rgblist RGB;
	struct hsvlist colnode[MAXTHRESHOLD], HSV;
	cpHeader *fig;
	int node, div, threshold[MAXTHRESHOLD];
	int ix, iy, maximum = 0;
	double dh, ds, dv;

	if (argc != 3) {
		fprintf(stderr, "usage: %s setting_file cpzfile\n", argv[0]);
		exit(0);
	}

	sprintf(pipe, CPP_COMMAND_LINE, argv[1]);

	if((fpin = popen(pipe, "r")) == NULL){
		perror("popen for reading ");
		exit(-1);
	}
	
	/* read parameters for graphics */
	fscanf(fpin, "%d", &node);
	fscanf(fpin, "%d", &div);
	for (i = 0; i < node; i++){
		fscanf(fpin, "%lf %lf %lf", 
			&colnode[i].h, &colnode[i].s, &colnode[i].v);
	}

	for (i = 0; i < node; i++){ fscanf(fpin, "%d", &threshold[i]); }
	fclose(fpin);


	strcpy(fname, argv[2]);
	str = rindex(fname, '.');
	if (str == NULL){
		strcat(fname, ".cpz");
	} else {
		str++;
		if (!strcmp(str, ".cpz")){
			fprintf(stderr, "%s is not cpz file\n", argv[2]);
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
		fprintf(stderr, "%s is not figdata\n", argv[2]);
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

	if ((colband = (cpIRGBList *)calloc((node + 1) * div,
		sizeof (cpIRGBList)))==NULL){
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}
	colptr = colband;
	for (i = 0; i < node - 1; i++){ 
		dh = (colnode[i + 1].h - colnode[i].h)/div;
		ds = (colnode[i + 1].s - colnode[i].s)/div;
		dv = (colnode[i + 1].v - colnode[i].v)/div;
		HSV.h = colnode[i].h; 
		HSV.s = colnode[i].s; 
		HSV.v = colnode[i].v;
		for (j = 0; j < div; j++){
			HSV.h += dh; HSV.s += ds; HSV.v += dv;
			hsv2rgb(&RGB, &HSV);
			colptr->r = (int)RGB.r;
			colptr->g = (int)RGB.g;
			colptr->b = (int)RGB.b;
			colptr++;
		}
	}
	colptr = colband;

	if ((dataRGB = (cpIRGBList *)calloc(ix * iy,
		sizeof (cpIRGBList))) == NULL){
		fprintf(stderr, "cannot allocate memory\n");
		exit(1);
	}

	count = 0;
	ptr = dataRGB;
	for (j = 0; j < iy; j++){
		for (i = 0; i < ix; i++){
			register int ite, now;
			ite = *(data + j * ix + i);
#ifdef DEBUG
			if (ite>0) printf("%d:%d:%d", i, j, count);fflush(stdout);
#endif
			{int m; m = max(maximum, ite); maximum = m;}
			for (k = 0; k < node; k++){
				if (ite <= threshold[k]){
					if (k == 0){ now = 0; break; } 
					else {
						now = k * div;
						now += (int)((double)div * 
						((double)(ite - threshold[k-1])/
						(double)(threshold[k] - threshold[k-1])));
						break;
					}
				}
			}
			if (now < 0){now = 0;}
			if (now >= div * (node-1)){ 
				now = div * (node - 1) - 1;
			}
			ptr->r = colband[now].r;
			ptr->g = colband[now].g;
			ptr->b = colband[now].b;
#ifdef DEBUG
			if (ptr->r == 0 && ptr->g ==0 && ptr->b ==0){
				printf("%d %d %d\n", 
					colband[now].r, colband[now].g, colband[now].b);
				printf("now = %d\n", now);
			}
			count++;
			printf("[%d] ",count);
#endif
			ptr++;
		}
	}
#ifdef COLBAND
	for (i = 0; i < div * (node - 1); i++){
		printf("%d %d %d\n", colband[i].r, colband[i].g, colband[i].b);
		fflush(stdout);
	}
#endif
	printf("maximum = %d, put image to png file...", maximum);
	fflush(stdout);
	save_png(fname, fig, dataRGB);
	printf("done.\n");
}	


void hsv2rgb(rgb, hsv)
	struct rgblist *rgb;
	struct hsvlist *hsv;
{
	int icol;
	double h1, f;
	double a[7];

	if (hsv->h > 360.0) hsv->h = 360 - hsv->h;
	h1 = hsv->h / 60.0;
	icol = (int)h1;
	f = h1 - (double)icol;

	a[1] = hsv->v;
	a[2] = hsv->v;
	a[3] = hsv->v * (1.0 - (hsv->s * f));
	a[4] = hsv->v * (1.0 - hsv->s);
	a[5] = a[4];
	a[6] = hsv->v * (1.0 - (hsv->s * (1.0 - f)));

	if (icol > 4) icol -= 4; else icol += 2;
	rgb->r = a[icol];
	if (icol > 4) icol -= 4; else icol += 2;
	rgb->b = a[icol];
	if (icol > 4) icol -= 4; else icol += 2;
	rgb->g = a[icol];
	rgb->r *= 255.0;
	rgb->g *= 255.0;
	rgb->b *= 255.0;
}

void rgb2hsv(rgb, hsv)
	struct rgblist *rgb;
	struct hsvlist *hsv;
{
	double h, s, v, x, r1, b1, g1;
	h = 0;
	v = max(rgb->r, max(rgb->g, rgb->b));
	x = min(rgb->r, max(rgb->b, rgb->b));
	if (fabs(v - x) < 1.0e-7 ) return;
	s = (v - x) / v;
	r1 = (v - rgb->r) / (v - x);
	g1 = (v - rgb->g) / (v - x);
	b1 = (v - rgb->b) / (v - x);
	if (rgb->r == v){
		if (rgb->g == x) h = 5.0 + b1;
		else h = 1.0 - g1;
	} else if (rgb->g == v){
		if (rgb->b == x) h = r1 + 1.0;
		else h = 3.0 - b1;
	} else {
		if (rgb->r == x) h = 3.0 + g1;
		else h = 5.0 - r1;
	}
	h /= 60.0;
	hsv->h = h;
	hsv->v = v;
	hsv->s = s;
}

int save_png(char *fname, cpHeader *h, cpIRGBList *data)
{
	FILE *fpout;
	char buf[BUFSIZ];
	char *sptr;
	unsigned char **image;
	png_structp	png_ptr;
	png_infop	info_ptr;

	int i, j;
	int s;

	strcpy(buf, fname);
	if ((sptr = rindex(buf, '.')) != NULL){ *sptr = '\0'; }
	strcat(buf, ".png");

	if ((fpout = fopen(buf,"w"))==NULL){
		fprintf(stderr, "cannot open %s\n", buf);
		exit(0);
	}

	image = (png_bytepp)malloc(h->height * sizeof (png_bytep)); 

	for (i = 0; i < h->height; i++){
		image[i] = (png_bytep)malloc(h->width * sizeof (png_byte) * 3);
		/* for RGB pixels */
	}

	for (i = 0; i < h->height; i++){
		for (j = 0; j < h->width; j++){
			image[i][3 * j]     = data->r;
			image[i][3 * j + 1] = data->g;
			image[i][3 * j + 2] = data->b;
			data++;
		}
	}

	png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fpout);
	png_set_IHDR(png_ptr, info_ptr, h->width, h->height, 
		8 * sizeof (unsigned char), PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, image);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fpout);
	return 0;
}
