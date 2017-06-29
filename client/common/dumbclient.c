/**
herman
 */


#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/client/file.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/client/cliprdr.h>
//#include <freerdp/client/channels.h>
//#include <freerdp/channels/channels.h>

#include <winpr/crt.h>
#include <winpr/synch.h>
#include <freerdp/log.h>


#include "dumbclient.h"

#define TAG "dumbclient"

struct dc_context
{
	rdpContext context;

	BOOL waitingFrameDone;
	BOOL haveDamage;

	/* Channels */
	//RdpeiClientContext* rdpei;
	//RdpgfxClientContext* gfx;
	//EncomspClientContext* encomsp;
};
typedef struct dc_context dcContext;

dumbconfig dc;
freerdp* instance;


static BOOL dc_context_new(freerdp* instance, rdpContext* context)
{
	return TRUE;
}

static void dc_context_free(freerdp* instance, rdpContext* context)
{
}

static BOOL dc_begin_paint(rdpContext* context)
{

	printf("dc_begin_paint\n");

	rdpGdi* gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = TRUE;
	return TRUE;
}

static BOOL dc_end_paint(rdpContext* context)
{
	printf("dc_end_paint\n");

	rdpGdi* gdi = context->gdi;

	INT32 x, y;
	UINT32 w, h;
	x = gdi->primary->hdc->hwnd->invalid->x;
	y = gdi->primary->hdc->hwnd->invalid->y;
	w = gdi->primary->hdc->hwnd->invalid->w;
	h = gdi->primary->hdc->hwnd->invalid->h;
	byte* data = gdi->primary_buffer;

	if (gdi->primary->hdc->hwnd->invalid->null)
		return TRUE;

	if (dc.drawFinish != NULL) {
		printf("dc_end_paint call drawFinish\n");

		dc.drawFinish(data, x, y, w, h);
	}

	return TRUE;
}

static BOOL dc_pre_connect(freerdp* instance)
{
	rdpSettings* settings;
	settings = instance->settings;
	settings->OrderSupport[NEG_DSTBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_PATBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_SCRBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_OPAQUE_RECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_DRAWNINEGRID_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTIDSTBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTIPATBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTISCRBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTIOPAQUERECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTI_DRAWNINEGRID_INDEX] = TRUE;
	settings->OrderSupport[NEG_LINETO_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYLINE_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEMBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEM3BLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_SAVEBITMAP_INDEX] = TRUE;
	settings->OrderSupport[NEG_GLYPH_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_GLYPH_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYGON_SC_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYGON_CB_INDEX] = TRUE;
	settings->OrderSupport[NEG_ELLIPSE_SC_INDEX] = TRUE;
	settings->OrderSupport[NEG_ELLIPSE_CB_INDEX] = TRUE;
	return TRUE;
}

static BOOL dc_post_connect(freerdp* instance)
{
	if (!gdi_init(instance, PIXEL_FORMAT_BGRA32))
		return FALSE;

	instance->update->BeginPaint = dc_begin_paint;
	instance->update->EndPaint = dc_end_paint;
	return TRUE;
}

static void* dc_client_thread_proc(freerdp* instance)
{
	DWORD nCount;
	DWORD status;
	HANDLE handles[64];

	if (!freerdp_connect(instance))
	{
		printf("connection failure\n");
		return NULL;
	}

	while (!freerdp_shall_disconnect(instance))
	{
		nCount = freerdp_get_event_handles(instance->context, &handles[0], 64);

		if (nCount == 0)
		{
			printf("%s: freerdp_get_event_handles failed\n", __FUNCTION__);
			break;
		}

		status = WaitForMultipleObjects(nCount, handles, FALSE, 100);

		if (status == WAIT_FAILED)
		{
			printf("%s: WaitForMultipleObjects failed with %d\n", __FUNCTION__,
			         status);
			break;
		}

		if (!freerdp_check_event_handles(instance->context))
		{
			printf("Failed to check FreeRDP event handles\n");
			break;
		}
	}

	freerdp_disconnect(instance);
	ExitThread(0);
	return NULL;
}


dumbconfig* dumb_config_new(void) {
	dumbconfig* p = malloc(sizeof(dumbconfig));
	ZeroMemory(p, sizeof(dumbconfig));
	return p;
}

void dumb_config_free(dumbconfig* dc) {
	free(dc);
}






static BOOL wl_context_new(freerdp* instance, rdpContext* context)
{
	/*if (!(context->channels = freerdp_channels_new()))
		return FALSE;*/

	return TRUE;
}

static void wl_context_free(freerdp* instance, rdpContext* context)
{

}

static BOOL wl_begin_paint(rdpContext* context)
{
	rdpGdi* gdi;

	gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = 1;
	return TRUE;
}


static BOOL wl_end_paint(rdpContext* context)
{
	rdpGdi* gdi;
	INT32 x, y;
	UINT32 w, h;
	int i;

	gdi = context->gdi;
	if (gdi->primary->hdc->hwnd->invalid->null)
		return TRUE;

	x = gdi->primary->hdc->hwnd->invalid->x;
	y = gdi->primary->hdc->hwnd->invalid->y;
	w = gdi->primary->hdc->hwnd->invalid->w;
	h = gdi->primary->hdc->hwnd->invalid->h;



	// gdi->primary_buffer
	if (dc.drawFinish != NULL)
		dc.drawFinish(gdi->primary_buffer, x, y, w, h);


	return TRUE;
}


static BOOL wl_pre_connect(freerdp* instance)
{

	//if (freerdp_channels_pre_connect(instance->context->channels, instance) != CHANNEL_RC_OK)
	//	return FALSE;
	return TRUE;
}

static BOOL wl_post_connect(freerdp* instance)
{
	rdpGdi* gdi;


	if (!gdi_init(instance, PIXEL_FORMAT_BGRA32, NULL))
		return FALSE;

	gdi = instance->context->gdi;
	if (!gdi)
		return FALSE;

	instance->update->BeginPaint = wl_begin_paint;
	instance->update->EndPaint = wl_end_paint;

	/*if (freerdp_channels_post_connect(instance->context->channels, instance) != CHANNEL_RC_OK)
		return FALSE;
*/
	printf("drawFinish?\n");

	if (dc.drawFinish != NULL) {
		printf("drawFinish+\n");
		dc.drawFinish(gdi->primary_buffer, 0, 0, gdi->width, gdi->height);
	}
	else {
		printf("drawFinish-\n");
	}
	return TRUE;
}

static void wl_post_disconnect(freerdp* instance)
{
	if (!instance)
		return;
	if (!instance->context)
		return;
	gdi_free(instance);

}

//static BOOL handle_uwac_events(freerdp* instance, UwacDisplay *display) {
//	UwacEvent event;
//	wlfContext *context;
//
//	if (UwacDisplayDispatch(display, 1) < 0)
//		return FALSE;
//
//	while (UwacHasEvent(display))
//	{
//		if (UwacNextEvent(display, &event) != UWAC_SUCCESS)
//			return FALSE;
//
//		/*printf("UWAC event type %d\n", event.type);*/
//		switch (event.type) {
//		case UWAC_EVENT_FRAME_DONE:
//			if (!instance)
//				continue;
//
//			context = (wlfContext *)instance->context;
//			context->waitingFrameDone = FALSE;
//			if (context->haveDamage && !wl_end_paint(instance->context))
//				return FALSE;
//			break;
//		case UWAC_EVENT_POINTER_ENTER:
//			if (!wlf_handle_pointer_enter(instance, &event.mouse_enter_leave))
//				return FALSE;
//			break;
//		case UWAC_EVENT_POINTER_MOTION:
//			if (!wlf_handle_pointer_motion(instance, &event.mouse_motion))
//				return FALSE;
//			break;
//		case UWAC_EVENT_POINTER_BUTTONS:
//			if (!wlf_handle_pointer_buttons(instance, &event.mouse_button))
//				return FALSE;
//			break;
//		case UWAC_EVENT_POINTER_AXIS:
//			if (!wlf_handle_pointer_axis(instance, &event.mouse_axis))
//				return FALSE;
//			break;
//		case UWAC_EVENT_KEY:
//			if (!wlf_handle_key(instance, &event.key))
//				return FALSE;
//			break;
//		case UWAC_EVENT_KEYBOARD_ENTER:
//			if (!wlf_keyboard_enter(instance, &event.keyboard_enter_leave))
//				return FALSE;
//			break;
//		default:
//			break;
//		}
//	}
//	return TRUE;
//}

static int wlfreerdp_run(freerdp* instance)
{
	DWORD count;
	HANDLE handles[64];
	DWORD status;

	printf("freerdp_connect\n");

	if (!freerdp_connect(instance))
	{
		printf("Failed to connect\n");
		return -1;
	
	}
	printf("freerdp_connect-\n");



	//handle_uwac_events(instance, g_display);

	while (!freerdp_shall_disconnect(instance))
	{
		//handles[0] = g_displayHandle;

		count = freerdp_get_event_handles(instance->context, &handles[0], 64);
		if (!count)
		{
			printf("Failed to get FreeRDP file descriptor\n");
			break;
		}

		status = WaitForMultipleObjects(count, handles, FALSE, INFINITE);
		if (WAIT_FAILED == status)
		{
			printf("%s: WaitForMultipleObjects failed\n", __FUNCTION__);
			break;
		}

		//if (!handle_uwac_events(instance, g_display)) {
		//	printf("error handling UWAC events\n");
		//	break;
		//}

		//if (WaitForMultipleObjects(count, &handles[1], FALSE, INFINITE)) {
		if (freerdp_check_event_handles(instance->context) != TRUE)
		{
			printf("Failed to check FreeRDP file descriptor\n");
			break;
		}
		//}

	}

	//freerdp_channels_disconnect(instance->context->channels, instance);
	freerdp_disconnect(instance);

	return 0;
}

int dumb_start(dumbconfig* pConfig)
{

	printf("copy dc\n");

	// flat copy config
	dc = *pConfig;

	if (dc.drawFinish != NULL) {
		printf("drawFinish+\n");
		dc.drawFinish(NULL, 0, 0, 1, 2);
	}
	else {
		printf("drawFinish-\n");
	}

	printf("assign argc, argv\n");

	int status;
	int argc = pConfig->argc;
	char** argv = pConfig->argv;



	instance = freerdp_new();
	instance->PreConnect = wl_pre_connect;
	instance->PostConnect = wl_post_connect;
	instance->PostDisconnect = wl_post_disconnect;
	//instance->Authenticate = client_cli_authenticate;
	//instance->GatewayAuthenticate = client_cli_gw_authenticate;
	//instance->VerifyCertificate = client_cli_verify_certificate;
	//instance->VerifyChangedCertificate = client_cli_verify_changed_certificate;

	instance->ContextSize = sizeof(struct dc_context)*2;
	instance->ContextNew = wl_context_new;
	instance->ContextFree = wl_context_free;

	freerdp_context_new(instance);

	status = freerdp_client_settings_parse_command_line(instance->settings, argc, argv, FALSE);

	status = freerdp_client_settings_command_line_status_print(instance->settings, status, argc, argv);

	if (status)
		return 0;

	//if (!freerdp_client_load_addins(instance->context->channels, instance->settings))
	//	exit(-1);

	wlfreerdp_run(instance);

	freerdp_context_free(instance);

	freerdp_free(instance);

	return 0;
}


void dumb_key_event(int pressed, int scancode) {
	if (instance)
		freerdp_input_send_keyboard_event_ex(instance->context->input, pressed, scancode);
}

void dumb_mouse_buttons_event(int pressed, enum DumbMouseButtons btn, int x, int y) {
	UINT16 flags;
	if (pressed) {
		flags = PTR_FLAGS_DOWN;
	}
	else {
		flags = 0;
	}
	switch (btn) {
	case LEFT:
		flags |= PTR_FLAGS_BUTTON1;
		break;
	case RIGHT:
		flags |= PTR_FLAGS_BUTTON2;
		break;
	case MIDDLE:
		flags |= PTR_FLAGS_BUTTON3;
		break;
	}
	if (instance)
		instance->context->input->MouseEvent(instance->context->input, flags, x, y);
}

void dumb_mouse_move_event(int x, int y) {
	if (instance)
		instance->context->input->MouseEvent(instance->context->input, PTR_FLAGS_MOVE, x, y);
}