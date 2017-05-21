#ifdef __GNUC__
#define DUMBCLIENT_API __attribute__((dllexport))
#else
#define DUMBCLIENT_API __declspec(dllexport)
//#define FREERDP_API __declspec(dllimport)
#endif

typedef int(*draw_finish)(unsigned char* gdi_bitmap, int x, int y, int width, int height);
typedef struct dumbconfig {
	draw_finish drawFinish;
	char** argv;
	int argc;
} dumbconfig;



DUMBCLIENT_API int dumb_start(dumbconfig* pConfig);
DUMBCLIENT_API dumbconfig* dumb_config_new(void);
DUMBCLIENT_API void dumb_config_free(dumbconfig* dc);
