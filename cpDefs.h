typedef struct rgblist { 
	double b; 
	double g; 
	double r; 
} cpRGBList;

typedef struct hsvlist { 
	double h; 
	double s; 
	double v;
} cpHSVList;

typedef struct irgblist { 
	unsigned char b;
	unsigned char g;
	unsigned char r;
} cpIRGBList;

typedef struct fighead { 
	int id; 
	int width; 
	int height; 
	int iteration;
	char comment[20];
} cpHeader;

#define CP_MAGIC 0x03011967

#define CPP_COMMAND_LINE "/usr/bin/cpp -P %s"
