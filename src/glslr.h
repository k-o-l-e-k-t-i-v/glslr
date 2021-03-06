#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <pthread.h>

/* glslr includes */
#include "config.h"
#include "base.h"
#include "graphics.h"
#include "v4l2_controls.h"
#include "util_textfile.h"

/* pdreceive includes */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <curl/curl.h>

typedef struct JpegMemory_s {
    CURL *curl_handle;
    unsigned char *memory;
    char *size_string;
    size_t size;
    size_t jpeg_size;
    bool header_found;
    bool stop;
} JpegMemory_t;

struct Glslr_ {
	Graphics *graphics;
	Graphics_LAYOUT layout_backup;
#ifdef VIDEO
    Sourceparams_t sourceparams;
    Videocapabilities_t capabilities;
#endif
    JpegMemory_t mem;
    pthread_t thread;
    bool sony_thread_active;
	int is_fullscreen;
	int use_backbuffer;
    int use_video;
    int use_sony;
    int do_save;
	int use_tcp;
	int use_net;
	int port;
	struct {
		int x, y;
		int fd;
	} mouse;
	netin_val *net_input_val;
	int net_params;
    int video_dev_num;
    char video_device[12];
	double time_origin;
	unsigned int frame;
	struct {
		int debug;
		int render_time;
	} verbose;
	struct {
		int numer;
		int denom;
	} scaling;
    // TODO - could be a struct too
    int save_tga;
    char *save_dirpath;
    char *save_filename;
};

typedef struct _fdpoll {
	int fdp_fd;
	char *fdp_outbuf;/*output message buffer*/
	int fdp_outlen;     /*length of output message*/
	int fdp_discard;/*buffer overflow: output message is incomplete, discard it*/
	int fdp_gotsemi;/*last char from input was a semicolon*/
} t_fdpoll;

typedef struct {
	const char *path;
    time_t last_modify_time;
} SourceObject;

typedef struct Glslr_ Glslr;

void udpmakeoutput(char *buf, Glslr *gx);
void sockerror(const char *s);
void x_closesocket(int fd);
void dopoll(Glslr *gx);
int Glslr_HostInitialize(void);
void Glslr_HostDeinitialize(void);
size_t Glslr_InstanceSize(void);
int Glslr_Construct(Glslr *gx);
void Glslr_Destruct(Glslr *gx);
int Glslr_ParseArgs(Glslr *gx, int argc, const char *argv[]);
int Glslr_Main(Glslr *gx);
void Glslr_Usage(void);
int Glslr_GetLineCount(char *code, size_t size);
void Glslr_IncludeAdditionalCode(char *code, int *len, int *lines_before, int *lines_included);

// TODO define in only one headerfile
void *getJpegData(void *memory);
