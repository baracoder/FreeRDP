#ifdef __GNUC__
#define DUMBCLIENT_API __attribute__((dllexport))
#else
#define DUMBCLIENT_API __declspec(dllexport)
//#define FREERDP_API __declspec(dllimport)
#endif

typedef unsigned int UINT32;
typedef int BOOL;

typedef int(*draw_finish)(unsigned char* gdi_bitmap, int x, int y, int width, int height, int gdiW, int gdiH);
typedef int(*new_window)(UINT32 windowId, UINT32 style, UINT32 extendeStyle, int x, int y, int w, int h);
typedef int(*window_moved_resized)(UINT32 windowId, int x, int y, int w, int h);
typedef int(*window_destroyed)(UINT32 windowId);
typedef int(*update_rect)(UINT32 windowId, int localX, int localY, int w, int h);


typedef struct dumbconfig {
	draw_finish drawFinish;
	new_window newWindow;
	window_moved_resized windowMovedResized;
	window_destroyed windowDestroyed;
	char** argv;
	int argc;

} dumbconfig;


DUMBCLIENT_API void* dumb_prepare(dumbconfig* pConfig);
DUMBCLIENT_API int dumb_start(void* context);
DUMBCLIENT_API dumbconfig* dumb_config_new(void);
DUMBCLIENT_API void dumb_config_free(dumbconfig* dc);

DUMBCLIENT_API void dumb_key_event(void* context, int pressed, int scancode);
DUMBCLIENT_API void dumb_mouse_buttons_event(void* context, int pressed, int btn, int x, int y);
DUMBCLIENT_API void dumb_mouse_move_event(void* context, int x, int y);
DUMBCLIENT_API void dc_rail_send_activate(void* context, UINT32 windowId, BOOL enabled);
DUMBCLIENT_API void xf_rail_send_client_system_command_close(void* context, UINT32 windowId);
DUMBCLIENT_API void xf_rail_adjust_position(void* context, UINT32 windowId, int x, int y, int w, int h);

