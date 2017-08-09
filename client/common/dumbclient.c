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

#include <freerdp/client/channels.h>
#include <freerdp/client/rdpei.h>
#include <freerdp/client/tsmf.h>
#include <freerdp/client/rail.h>
//#include <freerdp/client/cliprdr.h>
#include <freerdp/client/rdpgfx.h>
#include <freerdp/client/encomsp.h>


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
	RdpeiClientContext* rdpei;
	RdpgfxClientContext* gfx;
	EncomspClientContext* encomsp;
};
typedef struct dc_context dcContext;

dumbconfig dc;
freerdp* instance;



/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT wlf_encomsp_participant_created(EncomspClientContext* context,
	ENCOMSP_PARTICIPANT_CREATED_PDU* participantCreated)
{
	return CHANNEL_RC_OK;
}

static void wlf_encomsp_init(dcContext* wlf, EncomspClientContext* encomsp)
{
	if (!wlf)
		return;

	wlf->encomsp = encomsp;
	encomsp->custom = (void*)wlf;
	encomsp->ParticipantCreated = wlf_encomsp_participant_created;
}

static void wlf_encomsp_uninit(dcContext* wlf, EncomspClientContext* encomsp)
{
	if (!wlf)
		return;

	wlf->encomsp = NULL;
}


void wlf_OnChannelConnectedEventHandler(rdpContext* context,
	ChannelConnectedEventArgs* e)
{
	dcContext* wlf = (dcContext*)context;
	rdpSettings* settings = context->settings;

	if (strcmp(e->name, RDPEI_DVC_CHANNEL_NAME) == 0)
	{
		wlf->rdpei = (RdpeiClientContext*)e->pInterface;
	}
	else if (strcmp(e->name, TSMF_DVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, RDPGFX_DVC_CHANNEL_NAME) == 0)
	{
		if (settings->SoftwareGdi)
			gdi_graphics_pipeline_init(context->gdi, (RdpgfxClientContext*)e->pInterface);
	}
	else if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, ENCOMSP_SVC_CHANNEL_NAME) == 0)
	{
		wlf_encomsp_init(wlf, (EncomspClientContext*)e->pInterface);
	}
}

void wlf_OnChannelDisconnectedEventHandler(rdpContext* context,
	ChannelDisconnectedEventArgs* e)
{
	dcContext* wlf = (dcContext*)context;
	rdpSettings* settings = context->settings;

	if (strcmp(e->name, RDPEI_DVC_CHANNEL_NAME) == 0)
	{
		wlf->rdpei = NULL;
	}
	else if (strcmp(e->name, TSMF_DVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, RDPGFX_DVC_CHANNEL_NAME) == 0)
	{
		if (settings->SoftwareGdi)
			gdi_graphics_pipeline_uninit(context->gdi,
			(RdpgfxClientContext*)e->pInterface);
	}
	else if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, ENCOMSP_SVC_CHANNEL_NAME) == 0)
	{
		wlf_encomsp_uninit(wlf, (EncomspClientContext*)e->pInterface);
	}
}



static BOOL dc_begin_paint(rdpContext* context)
{

	OutputDebugString("dc_begin_paint\n");

	rdpGdi* gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = TRUE;
	return TRUE;
}

static BOOL dc_end_paint(dcContext* context)
{
	OutputDebugString("dc_end_paint\n");

	rdpGdi* gdi = context->context.gdi;

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
		OutputDebugString("dc_end_paint call drawFinish\n");

		dc.drawFinish(data, x, y, w, h);
	}

	return TRUE;
}

static BOOL dc_pre_connect(freerdp* instance)
{
	OutputDebugString("dc_pre_connect\n");

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
	OutputDebugString("dc_post_connect\n");

	if (!gdi_init(instance, PIXEL_FORMAT_BGRA32))
		return FALSE;

	instance->update->BeginPaint = dc_begin_paint;
	instance->update->EndPaint = dc_end_paint;
	return TRUE;
}

static void* dc_client_thread_proc(freerdp* instance)
{
	OutputDebugString("dc_client_thread_proc\n");

	DWORD nCount;
	DWORD status;
	HANDLE handles[64];

	if (!freerdp_connect(instance))
	{
		OutputDebugString("connection failure\n");
		return NULL;
	}

	while (!freerdp_shall_disconnect(instance))
	{
		nCount = freerdp_get_event_handles(instance->context, &handles[0], 64);

		if (nCount == 0)
		{
			OutputDebugString("%s: freerdp_get_event_handles failed\n", __FUNCTION__);
			break;
		}

		status = WaitForMultipleObjects(nCount, handles, FALSE, 100);

		if (status == WAIT_FAILED)
		{
			OutputDebugString("%s: WaitForMultipleObjects failed with %d\n", __FUNCTION__,
			         status);
			break;
		}

		if (!freerdp_check_event_handles(instance->context))
		{
			OutputDebugString("Failed to check FreeRDP event handles\n");
			break;
		}
	}

	freerdp_disconnect(instance);
	ExitThread(0);
	return NULL;
}

const char* defaultArgv[] = {
	"+sec-tls",
	"/v:192.168.178.65:3389",
	"/u:Administrator",
	"/p:Test123",
	"/gdi:sw",
	"/gfx:AVC444",
	"/cert-ignore"
};




dumbconfig* dumb_config_new(void) {
	OutputDebugString("dumb_config_new\n");
	dumbconfig* p = malloc(sizeof(dumbconfig));
	ZeroMemory(p, sizeof(dumbconfig));
	p->drawFinish = NULL;
	p->argv = defaultArgv;
	p->argc = sizeof(defaultArgv)/sizeof(defaultArgv[0]);
	return p;
}

void dumb_config_free(dumbconfig* dc) {
	free(dc);
}






static BOOL wl_context_new(freerdp* instance, rdpContext* context)
{
	if (!(context->channels = freerdp_channels_new()))
		return FALSE;

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


//static BOOL wl_pre_connect(freerdp* instance)
//{
//
//	//if (freerdp_channels_pre_connect(instance->context->channels, instance) != CHANNEL_RC_OK)
//	//	return FALSE;
//	return TRUE;
//}

//static BOOL wl_post_connect(freerdp* instance)
//{
//	rdpGdi* gdi;
//
//
//	if (!gdi_init(instance, PIXEL_FORMAT_BGRA32, NULL))
//		return FALSE;
//
//	gdi = instance->context->gdi;
//	if (!gdi)
//		return FALSE;
//
//	instance->update->BeginPaint = wl_begin_paint;
//	instance->update->EndPaint = wl_end_paint;
//
//	/*if (freerdp_channels_post_connect(instance->context->channels, instance) != CHANNEL_RC_OK)
//		return FALSE;
//*/
//	printf("drawFinish?\n");
//
//	if (dc.drawFinish != NULL) {
//		printf("drawFinish+\n");
//		dc.drawFinish(gdi->primary_buffer, 0, 0, gdi->width, gdi->height);
//	}
//	else {
//		printf("drawFinish-\n");
//	}
//	return TRUE;
//}

//static void wl_post_disconnect(freerdp* instance)
//{
//	if (!instance)
//		return;
//	if (!instance->context)
//		return;
//	gdi_free(instance);
//
//}

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

static int wlfreerdp_run(freerdp* instance) // new one
{
	DWORD count;
	HANDLE handles[64];
	DWORD status;

	OutputDebugString("freerdp_connect\n");

	if (!freerdp_connect(instance))
	{
		OutputDebugString("Failed to connect\n");
		return -1;
	
	}
	OutputDebugString("freerdp_connect-\n");



	//handle_uwac_events(instance, g_display);

	while (!freerdp_shall_disconnect(instance))
	{
		//handles[0] = g_displayHandle;

		count = freerdp_get_event_handles(instance->context, &handles[0], 64);
		if (!count)
		{
			OutputDebugString("Failed to get FreeRDP file descriptor\n");
			break;
		}

		status = WaitForMultipleObjects(count, handles, FALSE, INFINITE);
		if (WAIT_FAILED == status)
		{
			OutputDebugString("%s: WaitForMultipleObjects failed\n", __FUNCTION__);
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



static BOOL wl_update_content(dcContext* context_w)
{
	if (!context_w->waitingFrameDone && context_w->haveDamage)
	{
		//UwacWindowSubmitBuffer(context_w->window, true);
		context_w->waitingFrameDone = TRUE;
		context_w->haveDamage = FALSE;
	}

	return TRUE;
}

//static BOOL wl_begin_paint(rdpContext* context)
//{
//	rdpGdi* gdi;
//	gdi = context->gdi;
//	gdi->primary->hdc->hwnd->invalid->null = TRUE;
//	return TRUE;
//}


//static BOOL wl_end_paint(rdpContext* context)
//{
//	rdpGdi* gdi;
//	char* data;
//	dcContext* context_w;
//	INT32 x, y;
//	UINT32 w, h;
//	int i;
//	gdi = context->gdi;
//
//	if (gdi->primary->hdc->hwnd->invalid->null)
//		return TRUE;
//
//	x = gdi->primary->hdc->hwnd->invalid->x;
//	y = gdi->primary->hdc->hwnd->invalid->y;
//	w = gdi->primary->hdc->hwnd->invalid->w;
//	h = gdi->primary->hdc->hwnd->invalid->h;
//	context_w = (dcContext*)context;
//	//data = UwacWindowGetDrawingBuffer(context_w->window);
//
//	if (!data)
//		return FALSE;
//
//	for (i = 0; i < h; i++)
//	{
//		memcpy(data + ((i + y) * (gdi->width * GetBytesPerPixel(
//			gdi->dstFormat))) + x * GetBytesPerPixel(gdi->dstFormat),
//			gdi->primary_buffer + ((i + y) * (gdi->width * GetBytesPerPixel(
//				gdi->dstFormat))) + x * GetBytesPerPixel(gdi->dstFormat),
//			w * GetBytesPerPixel(gdi->dstFormat));
//	}
//
//	/*if (UwacWindowAddDamage(context_w->window, x, y, w, h) != UWAC_SUCCESS)
//		return FALSE;*/
//
//	context_w->haveDamage = TRUE;
//	return wl_update_content(context_w);
//}


static BOOL wl_pre_connect(freerdp* instance)
{
	OutputDebugString(L"wl_pre_connect\n");

	rdpSettings* settings;
	dcContext* context;

	if (!instance)
		return FALSE;

	context = (dcContext*)instance->context;
	settings = instance->settings;

	if (!context || !settings)
		return FALSE;

	settings->OsMajorType = OSMAJORTYPE_UNIX;
	settings->OsMinorType = OSMINORTYPE_NATIVE_WAYLAND;
	settings->SoftwareGdi = TRUE;
	ZeroMemory(settings->OrderSupport, 32);
	settings->OrderSupport[NEG_DSTBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_PATBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_SCRBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_OPAQUE_RECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_DRAWNINEGRID_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIDSTBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIPATBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTISCRBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIOPAQUERECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTI_DRAWNINEGRID_INDEX] = FALSE;
	settings->OrderSupport[NEG_LINETO_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYLINE_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEMBLT_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEM3BLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEMBLT_V2_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEM3BLT_V2_INDEX] = FALSE;
	settings->OrderSupport[NEG_SAVEBITMAP_INDEX] = FALSE;
	settings->OrderSupport[NEG_GLYPH_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_GLYPH_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYGON_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_POLYGON_CB_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_CB_INDEX] = FALSE;
	PubSub_SubscribeChannelConnected(instance->context->pubSub,
		(pChannelConnectedEventHandler)wlf_OnChannelConnectedEventHandler);
	PubSub_SubscribeChannelDisconnected(instance->context->pubSub,
		(pChannelDisconnectedEventHandler)wlf_OnChannelDisconnectedEventHandler);

	if (!freerdp_client_load_addins(instance->context->channels,
		instance->settings))
		return FALSE;

	return TRUE;
}

static BOOL wl_post_connect(freerdp* instance)
{
	OutputDebugString(L"wl_post_connect\n");

	rdpGdi* gdi;
	//UwacWindow* window;
	dcContext* context;

	if (!gdi_init(instance, PIXEL_FORMAT_BGRA32))
		return FALSE;

	gdi = instance->context->gdi;

	if (!gdi)
		return FALSE;

	context = (dcContext*)instance->context;
	/*context->window = window = UwacCreateWindowShm(context->display, gdi->width,
		gdi->height, WL_SHM_FORMAT_XRGB8888);*/

	/*if (!window)
		return FALSE;*/

	//UwacWindowSetFullscreenState(window, NULL, instance->context->settings->Fullscreen);
	//UwacWindowSetTitle(window, "FreeRDP");
	instance->update->BeginPaint = wl_begin_paint;
	instance->update->EndPaint = wl_end_paint;
	//memcpy(UwacWindowGetDrawingBuffer(context->window), gdi->primary_buffer,
	//	gdi->width * gdi->height * 4);
	//UwacWindowAddDamage(context->window, 0, 0, gdi->width, gdi->height);
	context->haveDamage = TRUE;

	freerdp_keyboard_init(instance->context->settings->KeyboardLayout);

	return wl_update_content(context);
}

static void wl_post_disconnect(freerdp* instance)
{
	OutputDebugString(L"wl_post_disconnect\n");

	dcContext* context;

	if (!instance)
		return;

	if (!instance->context)
		return;

	context = (dcContext*)instance->context;
	gdi_free(instance);

	//if (context->window)
	//	UwacDestroyWindow(&context->window);
}

//static BOOL handle_uwac_events(freerdp* instance, UwacDisplay* display)
//{
//	UwacEvent event;
//	dcContext* context;
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
//		switch (event.type)
//		{
//		case UWAC_EVENT_FRAME_DONE:
//			if (!instance)
//				continue;
//
//			context = (wlfContext*)instance->context;
//			context->waitingFrameDone = FALSE;
//
//			if (context->haveDamage && !wl_end_paint(instance->context))
//				return FALSE;
//
//			break;
//
//		case UWAC_EVENT_POINTER_ENTER:
//			if (!wlf_handle_pointer_enter(instance, &event.mouse_enter_leave))
//				return FALSE;
//
//			break;
//
//		case UWAC_EVENT_POINTER_MOTION:
//			if (!wlf_handle_pointer_motion(instance, &event.mouse_motion))
//				return FALSE;
//
//			break;
//
//		case UWAC_EVENT_POINTER_BUTTONS:
//			if (!wlf_handle_pointer_buttons(instance, &event.mouse_button))
//				return FALSE;
//
//			break;
//
//		case UWAC_EVENT_POINTER_AXIS:
//			if (!wlf_handle_pointer_axis(instance, &event.mouse_axis))
//				return FALSE;
//
//			break;
//
//		case UWAC_EVENT_KEY:
//			if (!wlf_handle_key(instance, &event.key))
//				return FALSE;
//
//			break;
//
//		case UWAC_EVENT_KEYBOARD_ENTER:
//			if (!wlf_keyboard_enter(instance, &event.keyboard_enter_leave))
//				return FALSE;
//
//			break;
//
//		default:
//			break;
//		}
//	}
//
//	return TRUE;
//}

//static int wlfreerdp_run(freerdp* instance) // new
//{
//	dcContext* context;
//	DWORD count;
//	HANDLE handles[64];
//	DWORD status;
//
//	if (!instance)
//		return -1;
//
//	context = (dcContext*)instance->context;
//
//	if (!context)
//		return -1;
//
//	if (!freerdp_connect(instance))
//	{
//		printf("Failed to connect\n");
//		return -1;
//	}
//
//	//handle_uwac_events(instance, context->display);
//
//	while (!freerdp_shall_disconnect(instance))
//	{
//		//handles[0] = context->displayHandle;
//		nCount = freerdp_get_event_handles(instance->context, &handles[0], 64);
//
//		if (!count)
//		{
//			printf("Failed to get FreeRDP file descriptor\n");
//			break;
//		}
//
//		status = WaitForMultipleObjects(count + 1, handles, FALSE, INFINITE);
//
//		if (WAIT_FAILED == status)
//		{
//			printf("%s: WaitForMultipleObjects failed\n", __FUNCTION__);
//			break;
//		}
//
//		if (!handle_uwac_events(instance, context->display))
//		{
//			printf("error handling UWAC events\n");
//			break;
//		}
//
//		//if (WaitForMultipleObjects(count, &handles[1], FALSE, INFINITE)) {
//		if (freerdp_check_event_handles(instance->context) != TRUE)
//		{
//			printf("Failed to check FreeRDP file descriptor\n");
//			break;
//		}
//
//		//}
//	}
//
//	freerdp_disconnect(instance);
//	return 0;
//}

static BOOL wlf_client_global_init()
{
	OutputDebugString(L"wlf_client_global_init\n");

	// TODO what is it used for?
	/*if (freerdp_handle_signals() != 0) {
		OutputDebugString(L"wlf_client_global_init freerdp_handle_signals() != 0\n");

		return FALSE;

	}*/

	return TRUE;
}

static void wlf_client_global_uninit()
{
}

static BOOL wlf_client_new(freerdp* instance, rdpContext* context)
{
	OutputDebugString(L"wlf_client_new\n");

	//UwacReturnCode status;
	dcContext* wfl = (dcContext*)context;

	if (!instance || !context) {
		OutputDebugString(L"wlf_client_new !instance || !contex\n");
		return FALSE;
	}

	instance->PreConnect = wl_pre_connect;
	instance->PostConnect = wl_post_connect;
	instance->PostDisconnect = wl_post_disconnect;
	instance->Authenticate = client_cli_authenticate;
	instance->GatewayAuthenticate = client_cli_gw_authenticate;
	instance->VerifyCertificate = client_cli_verify_certificate;
	instance->VerifyChangedCertificate = client_cli_verify_changed_certificate;
	instance->LogonErrorInfo = NULL;
	//wfl->display = UwacOpenDisplay(NULL, &status);

	/*if (!wfl->display || (status != UWAC_SUCCESS))
		return FALSE;*/

	//wfl->displayHandle = CreateFileDescriptorEvent(NULL, FALSE, FALSE,
	//	UwacDisplayGetFd(wfl->display), WINPR_FD_READ);

	//if (!wfl->displayHandle)
	//	return FALSE;

	return TRUE;
}


static void wlf_client_free(freerdp* instance, rdpContext* context)
{
	dcContext* wlf = (dcContext*)instance->context;

	if (!context)
		return;

	//if (wlf->display)
	//	UwacCloseDisplay(&wlf->display);

	//if (wlf->displayHandle)
	//	CloseHandle(wlf->displayHandle);
}

static int wfl_client_start(rdpContext* context)
{
	OutputDebugString(L"wfl_client_start\n");

	return 0;
}

static int wfl_client_stop(rdpContext* context)
{
	return 0;
}

static int RdpClientEntry(RDP_CLIENT_ENTRY_POINTS* pEntryPoints)
{
	OutputDebugString(L"RdpClientEntry\n");


	ZeroMemory(pEntryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
	pEntryPoints->Version = RDP_CLIENT_INTERFACE_VERSION;
	pEntryPoints->Size = sizeof(RDP_CLIENT_ENTRY_POINTS_V1);
	pEntryPoints->GlobalInit = wlf_client_global_init;
	pEntryPoints->GlobalUninit = wlf_client_global_uninit;
	pEntryPoints->ContextSize = sizeof(dcContext);
	pEntryPoints->ClientNew = wlf_client_new;
	pEntryPoints->ClientFree = wlf_client_free;
	pEntryPoints->ClientStart = wfl_client_start;
	pEntryPoints->ClientStop = wfl_client_stop;
	return 0;
}

WSADATA wsaData;


int dumb_start(dumbconfig* pConfig)
{
	OutputDebugString(L"initialize winsocks");

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}



	OutputDebugString(L"copy dc\n");

	// flat copy config
	dc = *pConfig;



	OutputDebugString(L"assign argc, argv\n");
	int argc = pConfig->argc;
	char** argv = pConfig->argv;




	int rc = -1;
	DWORD status;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
	rdpContext* context;



	//if (!handle_uwac_events(NULL, g_display))
	//	exit(1);
	RdpClientEntry(&clientEntryPoints);

	OutputDebugString(L"creating client context..\n");

	context = freerdp_client_context_new(&clientEntryPoints);

	if (!context)
		goto fail;

	OutputDebugString(L"parsing cmdline..\n");

	status = freerdp_client_settings_parse_command_line(context->settings, argc,
		argv, FALSE);
	status = freerdp_client_settings_command_line_status_print(context->settings,
		status, argc, argv);

	OutputDebugString(L"parsed cmdline");


	if (status)
		return 0;

	if (freerdp_client_start(context) != 0)
		goto fail;


	OutputDebugString(L"run...");

	rc = wlfreerdp_run(context->instance);
	OutputDebugString(L"run... done!");

	if (freerdp_client_stop(context) != 0)
		rc = -1;

fail:
	freerdp_client_context_free(context);
	return rc;
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