/**
herman

most parts are taken from the wayland client, some from x11 client
 */


#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/gdi/gfx.h>
#include <freerdp/client.h>
#include <libfreerdp/core/client.h>
#include <freerdp/client/file.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/client/cliprdr.h>

#include <freerdp/client/channels.h>
#include <freerdp/client/rdpei.h>
#include <freerdp/client/tsmf.h>
#include <freerdp/client/rail.h>
#include <freerdp/client/rdpgfx.h>
#include <freerdp/client/encomsp.h>
#include <freerdp/locale/keyboard.h>


#include <winpr/crt.h>
#include <winpr/synch.h>
#include <freerdp/log.h>


#include "dumbclient.h"

#define TAG "dumbclient"


struct dc_context
{
	rdpContext context;

	/* Channels */
	RdpeiClientContext* rdpei;
	RdpgfxClientContext* gfx;
	EncomspClientContext* encomsp;
	RailClientContext* rail;

	dumbconfig* dConfig;
};
typedef struct dc_context dcContext;

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

int dumb_rail_init(dcContext* xfc, RailClientContext* rail);

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
		dumb_rail_init(wlf, (RailClientContext*)e->pInterface);
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


const char* defaultArgv[] = {
	"/v:192.168.178.65:3389",
	"/u:Administrator",
	"/p:Test123",
	"/gdi:sw",
	"/gfx:AVC444",
	"/cert-ignore"
};




dumbconfig* dumb_config_new(void) {
	OutputDebugString(L"dumb_config_new\n");
	dumbconfig* p = malloc(sizeof(dumbconfig));
	ZeroMemory(p, sizeof(dumbconfig));
	p->argv = defaultArgv;
	p->argc = sizeof(defaultArgv)/sizeof(defaultArgv[0]);
	return p;
}

void dumb_config_free(dumbconfig* dc) {
	free(dc);
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
	dcContext* dc = context;
	rdpGdi* gdi;
	INT32 x, y;
	UINT32 w, h;

	gdi = context->gdi;
	if (gdi->primary->hdc->hwnd->invalid->null)
		return TRUE;

	x = gdi->primary->hdc->hwnd->invalid->x;
	y = gdi->primary->hdc->hwnd->invalid->y;
	w = gdi->primary->hdc->hwnd->invalid->w;
	h = gdi->primary->hdc->hwnd->invalid->h;

	if (dc->dConfig->drawFinish != NULL)
		dc->dConfig->drawFinish(gdi->primary_buffer, x, y, w, h, gdi->width, gdi->height);


	return TRUE;
}


static int wlfreerdp_run(freerdp* instance) // new one
{
	DWORD count;
	HANDLE handles[64];
	DWORD status;

	OutputDebugString(L"freerdp_connect\n");

	if (!freerdp_connect(instance))
	{
		OutputDebugString(L"Failed to connect\n");
		return -1;
	
	}
	OutputDebugString(L"freerdp_connect-\n");



	//handle_uwac_events(instance, g_display);

	while (!freerdp_shall_disconnect(instance))
	{
		//handles[0] = g_displayHandle;

		count = freerdp_get_event_handles(instance->context, &handles[0], 64);
		if (!count)
		{
			OutputDebugString(L"Failed to get FreeRDP file descriptor\n");
			break;
		}

		status = WaitForMultipleObjects(count, handles, FALSE, INFINITE);
		if (WAIT_FAILED == status)
		{
			OutputDebugString(L": WaitForMultipleObjects failed\n");
			break;
		}

		if (freerdp_check_event_handles(instance->context) != TRUE)
		{
			printf("Failed to check FreeRDP file descriptor\n");
			break;
		}

	}

	// unresolved symbol, even though header is presend and body exists
	//freerdp_channels_disconnect(instance->context->channels, instance);
	freerdp_disconnect(instance);

	return 0;
}



static BOOL wl_update_content(dcContext* context_w)
{
	return TRUE;
}


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

	// Set instance

	if (!gdi)
		return FALSE;

	context = (dcContext*)instance->context;

	instance->update->BeginPaint = wl_begin_paint;
	instance->update->EndPaint = wl_end_paint;

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
}



BOOL wlf_client_global_init(void)
{
	OutputDebugString(L"wlf_client_global_init\n");

	// TODO what is it used for?
	/*if (freerdp_handle_signals() != 0) {
		OutputDebugString(L"wlf_client_global_init freerdp_handle_signals() != 0\n");

		return FALSE;

	}*/

	return TRUE;
}

void wlf_client_global_uninit(void)
{
}

static BOOL wlf_client_new(freerdp* instance, rdpContext* context)
{
	OutputDebugString(L"wlf_client_new\n");

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
	return TRUE;
}


static void wlf_client_free(freerdp* instance, rdpContext* context)
{
	dcContext* wlf = (dcContext*)instance->context;

	if (!context)
		return;
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

// winsock structure
WSADATA wsaData;

void dumb_init_winsocks() {
	OutputDebugString(L"initialize winsocks");

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		printf("WSAStartup failed: %d\n", iResult);
}

void* dumb_prepare(dumbconfig* pConfig) {

	OutputDebugString(L"assign argc, argv\n");
	int argc = pConfig->argc;
	char** argv = pConfig->argv;
	DWORD status;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
	dcContext* context;

	RdpClientEntry(&clientEntryPoints);

	OutputDebugString(L"creating client context..\n");

	context = freerdp_client_context_new(&clientEntryPoints);

	if (!context)
		return NULL;

	context->dConfig = pConfig;


	OutputDebugString(L"parsing cmdline..\n");

	status = freerdp_client_settings_parse_command_line(context->context.settings, argc,
		argv, FALSE);
	status = freerdp_client_settings_command_line_status_print(context->context.settings,
		status, argc, argv);

	OutputDebugString(L"parsed cmdline");

	if (status)
		return NULL;
	
	return context;
}


int dumb_start(void* cPtr)
{
	rdpContext* context = cPtr;

	int rc = -1;

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


#pragma region input events

void dumb_key_event(void* context, int pressed, int scancode) {
	dcContext* c = context;
	if (c != NULL && c->context.input != NULL)
		freerdp_input_send_keyboard_event_ex(c->context.input, pressed, scancode);
}

void dumb_mouse_buttons_event(void* context, int pressed, int btn, int x, int y) {
	dcContext* c = context;
	if (c == NULL) return;

	UINT16 flags = 0;
	BOOL extended = FALSE;
	
	switch (btn) {
	case 0:
		flags |= PTR_FLAGS_BUTTON1;
		if (pressed)  flags |= PTR_FLAGS_DOWN;
		break;
	case 1:
		flags |= PTR_FLAGS_BUTTON2;
		if (pressed)  flags |= PTR_FLAGS_DOWN;
		break;
	case 2:
		flags |= PTR_FLAGS_BUTTON3;
		if (pressed)  flags |= PTR_FLAGS_DOWN;
		break;
	case 3:
		flags |= PTR_FLAGS_WHEEL | 0x0078;
		break;
	case 4:
		flags |= PTR_FLAGS_WHEEL | PTR_FLAGS_WHEEL_NEGATIVE | 0x0088;
		break;
	case 5:
		flags |= PTR_XFLAGS_BUTTON1;
		if (pressed)  flags |= PTR_XFLAGS_DOWN;
		extended = TRUE;
		break;
	case 6:
		flags |= PTR_XFLAGS_BUTTON2;
		if (pressed)  flags |= PTR_XFLAGS_DOWN;
		extended = TRUE;
		break;
	default:
		OutputDebugString(L"unknown mouse button");
		return;
	}
	if (flags) {
		if (extended == TRUE) {
			c->context.input->ExtendedMouseEvent(c->context.input, flags, x, y);
		}
		else {
			c->context.input->MouseEvent(c->context.input, flags, x, y);
		}

	}
}

void dumb_mouse_move_event(void* context, int x, int y) {
	dcContext* dc = context;
	if (dc->context.input != NULL)
		dc->context.input->MouseEvent(dc->context.input, PTR_FLAGS_MOVE, x, y);
}

#pragma endregion



#pragma region rail

static const char* error_code_names[] =
{
	"RAIL_EXEC_S_OK",
	"RAIL_EXEC_E_HOOK_NOT_LOADED",
	"RAIL_EXEC_E_DECODE_FAILED",
	"RAIL_EXEC_E_NOT_IN_ALLOWLIST",
	"RAIL_EXEC_E_FILE_NOT_FOUND",
	"RAIL_EXEC_E_FAIL",
	"RAIL_EXEC_E_SESSION_LOCKED"
};


void dc_rail_send_activate(void* context, UINT32 windowId, BOOL enabled)
{
	dcContext* xfc = context;
	RAIL_ACTIVATE_ORDER activate;
	activate.windowId = windowId;
	activate.enabled = enabled;
	xfc->rail->ClientActivate(xfc->rail, &activate);
}

void xf_rail_send_client_system_command_close(void* context, UINT32 windowId)
{
	dcContext* xfc = context;
	RAIL_SYSCOMMAND_ORDER syscommand;
	syscommand.windowId = windowId;
	syscommand.command = SC_CLOSE;
	xfc->rail->ClientSystemCommand(xfc->rail, &syscommand);
}

void xf_rail_adjust_position(void* context, UINT32 windowId, int x, int y, int w, int h)
{
	dcContext* xfc = context;
	RAIL_WINDOW_MOVE_ORDER windowMove;

	windowMove.windowId = windowId;
	/*
	* Calculate new size/position for the rail window(new values for windowOffsetX/windowOffsetY/windowWidth/windowHeight) on the server
	*/
	windowMove.left = x;
	windowMove.top = y;
	windowMove.right = windowMove.left + w;
	windowMove.bottom = windowMove.top + h;
	xfc->rail->ClientWindowMove(xfc->rail, &windowMove);
}


/* RemoteApp Core Protocol Extension */

static BOOL xf_rail_window_common(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, WINDOW_STATE_ORDER* windowState)
{
	dcContext* xfc = (dcContext*)context;
	UINT32 fieldFlags = orderInfo->fieldFlags;

	BOOL position_or_size_updated = FALSE;
	int winX = -1, winY = -1, winW = -1, winH = -1;

	UINT32 windowId = orderInfo->windowId;
	if (fieldFlags & WINDOW_ORDER_STATE_NEW)
	{
		if (xfc->dConfig->newWindow)
			return xfc->dConfig->newWindow(orderInfo->windowId, windowState->style, windowState->extendedStyle, windowState->windowOffsetX, windowState->windowOffsetY, windowState->windowWidth, windowState->windowHeight);
		return TRUE;
		// TODO callback set title (windowState->titleInfo.string, windowState->titleInfo.length)
	}

	/* Keep track of any position/size update so that we can force a refresh of the window */
	if ((fieldFlags & WINDOW_ORDER_FIELD_WND_OFFSET) ||
		(fieldFlags & WINDOW_ORDER_FIELD_WND_SIZE) ||
		(fieldFlags & WINDOW_ORDER_FIELD_CLIENT_AREA_OFFSET) ||
		(fieldFlags & WINDOW_ORDER_FIELD_CLIENT_AREA_SIZE) ||
		(fieldFlags & WINDOW_ORDER_FIELD_WND_CLIENT_DELTA) ||
		(fieldFlags & WINDOW_ORDER_FIELD_VIS_OFFSET) ||
		(fieldFlags & WINDOW_ORDER_FIELD_VISIBILITY))
	{
		position_or_size_updated = TRUE;
	}

	/* Update Parameters */

	if (fieldFlags & WINDOW_ORDER_FIELD_WND_OFFSET)
	{
		winX = windowState->windowOffsetX;
		winY = windowState->windowOffsetY;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_WND_SIZE)
	{
		winW = windowState->windowWidth;
		winH = windowState->windowHeight;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_OWNER)
	{
		//appWindow->ownerWindowId = windowState->ownerWindowId;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_STYLE)
	{
		//appWindow->dwStyle = windowState->style;
		//appWindow->dwExStyle = windowState->extendedStyle;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_SHOW)
	{
		//appWindow->showState = windowState->showState;
		// TODO do something about minimized windows
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_TITLE)
	{
		// TODO set title somehow?
		//appWindow->title = title;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_CLIENT_AREA_OFFSET)
	{
		//winX = windowState->clientOffsetX;
		//winY = windowState->clientOffsetY;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_CLIENT_AREA_SIZE)
	{
		//winW = windowState->clientAreaWidth;
		//winH = windowState->clientAreaHeight;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_WND_CLIENT_DELTA)
	{
		//winX = windowState->windowClientDeltaX;
		//winY = windowState->windowClientDeltaY;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_WND_RECTS)
	{
		// TODO do i need to handle this?
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_VIS_OFFSET)
	{
		//winX = windowState->visibleOffsetX;
		//winY = windowState->visibleOffsetY;
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_VISIBILITY)
	{
		// TODO do i need this?
	}

	/* Update Window */

	if (fieldFlags & WINDOW_ORDER_FIELD_STYLE)
	{
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_SHOW)
	{
		//xf_ShowWindow(xfc, appWindow, appWindow->showState);
	}

	if (fieldFlags & WINDOW_ORDER_FIELD_TITLE)
	{
		// TODO maybe add title
		
			//xf_SetWindowText(xfc, appWindow, appWindow->title);
	}

	if (position_or_size_updated)
	{
		if (xfc->dConfig->windowMovedResized)
			xfc->dConfig->windowMovedResized(windowId, winX, winY, winW, winH);
	}
	

	return TRUE;
}

static BOOL xf_rail_window_delete(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo)
{
	dcContext* xfc = (dcContext*)context;

	if (xfc->dConfig->windowDestroyed)
		xfc->dConfig->windowDestroyed(orderInfo->windowId);
	return TRUE;
}

static BOOL xf_rail_window_icon(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, WINDOW_ICON_ORDER* windowIcon)
{
	return TRUE;
}

static BOOL xf_rail_window_cached_icon(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, WINDOW_CACHED_ICON_ORDER* windowCachedIcon)
{
	return TRUE;
}

static BOOL xf_rail_notify_icon_common(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, NOTIFY_ICON_STATE_ORDER* notifyIconState)
{
	return TRUE;
}

static BOOL xf_rail_notify_icon_create(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, NOTIFY_ICON_STATE_ORDER* notifyIconState)
{
	return xf_rail_notify_icon_common(context, orderInfo, notifyIconState);
}

static BOOL xf_rail_notify_icon_update(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, NOTIFY_ICON_STATE_ORDER* notifyIconState)
{
	return xf_rail_notify_icon_common(context, orderInfo, notifyIconState);
}

static BOOL xf_rail_notify_icon_delete(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo)
{
	return TRUE;
}

static BOOL xf_rail_monitored_desktop(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo, MONITORED_DESKTOP_ORDER* monitoredDesktop)
{
	return TRUE;
}

static BOOL xf_rail_non_monitored_desktop(rdpContext* context,
	WINDOW_ORDER_INFO* orderInfo)
{
	dcContext* xfc = (dcContext*)context;
	return TRUE;
}

void xf_rail_register_update_callbacks(rdpUpdate* update)
{
	rdpWindowUpdate* window = update->window;
	window->WindowCreate = xf_rail_window_common;
	window->WindowUpdate = xf_rail_window_common;
	window->WindowDelete = xf_rail_window_delete;
	window->WindowIcon = xf_rail_window_icon;
	window->WindowCachedIcon = xf_rail_window_cached_icon;
	window->NotifyIconCreate = xf_rail_notify_icon_create;
	window->NotifyIconUpdate = xf_rail_notify_icon_update;
	window->NotifyIconDelete = xf_rail_notify_icon_delete;
	window->MonitoredDesktop = xf_rail_monitored_desktop;
	window->NonMonitoredDesktop = xf_rail_non_monitored_desktop;
}

/* RemoteApp Virtual Channel Extension */

/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_execute_result(RailClientContext* context,
	RAIL_EXEC_RESULT_ORDER* execResult)
{
	dcContext* xfc = (dcContext*)context->custom;

	if (execResult->execResult != RAIL_EXEC_S_OK)
	{
		WLog_ERR(TAG, "RAIL exec error: execResult=%s NtError=0x%X\n",
			error_code_names[execResult->execResult], execResult->rawResult);
		freerdp_abort_connect(xfc->context.instance);
	}
	
	return CHANNEL_RC_OK;
}

/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_system_param(RailClientContext* context,
	RAIL_SYSPARAM_ORDER* sysparam)
{
	return CHANNEL_RC_OK;
}

/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_handshake(RailClientContext* context,
	RAIL_HANDSHAKE_ORDER* handshake)
{
	RAIL_EXEC_ORDER exec;
	RAIL_SYSPARAM_ORDER sysparam;
	RAIL_HANDSHAKE_ORDER clientHandshake;
	RAIL_CLIENT_STATUS_ORDER clientStatus;
	dcContext* xfc = (dcContext*)context->custom;
	rdpSettings* settings = xfc->context.settings;
	clientHandshake.buildNumber = 0x00001DB0;
	context->ClientHandshake(context, &clientHandshake);
	ZeroMemory(&clientStatus, sizeof(RAIL_CLIENT_STATUS_ORDER));
	// local resize not implemeted
	//clientStatus.flags = RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE;
	clientStatus.flags = 0;
	context->ClientInformation(context, &clientStatus);

	if (settings->RemoteAppLanguageBarSupported)
	{
		RAIL_LANGBAR_INFO_ORDER langBarInfo;
		langBarInfo.languageBarStatus = 0x00000008; /* TF_SFT_HIDDEN */
		context->ClientLanguageBarInfo(context, &langBarInfo);
	}

	ZeroMemory(&sysparam, sizeof(RAIL_SYSPARAM_ORDER));
	sysparam.params = 0;
	sysparam.params |= SPI_MASK_SET_HIGH_CONTRAST;
	sysparam.highContrast.colorScheme.string = NULL;
	sysparam.highContrast.colorScheme.length = 0;
	sysparam.highContrast.flags = 0x7E;
	sysparam.params |= SPI_MASK_SET_MOUSE_BUTTON_SWAP;
	sysparam.mouseButtonSwap = FALSE;
	sysparam.params |= SPI_MASK_SET_KEYBOARD_PREF;
	sysparam.keyboardPref = FALSE;
	sysparam.params |= SPI_MASK_SET_DRAG_FULL_WINDOWS;
	sysparam.dragFullWindows = FALSE;
	sysparam.params |= SPI_MASK_SET_KEYBOARD_CUES;
	sysparam.keyboardCues = FALSE;
	sysparam.params |= SPI_MASK_SET_WORK_AREA;
	sysparam.workArea.left = 0;
	sysparam.workArea.top = 0;
	sysparam.workArea.right = settings->DesktopWidth;
	sysparam.workArea.bottom = settings->DesktopHeight;
	sysparam.dragFullWindows = FALSE;
	context->ClientSystemParam(context, &sysparam);
	ZeroMemory(&exec, sizeof(RAIL_EXEC_ORDER));
	exec.RemoteApplicationProgram = settings->RemoteApplicationProgram;
	exec.RemoteApplicationWorkingDir = settings->ShellWorkingDirectory;
	exec.RemoteApplicationArguments = settings->RemoteApplicationCmdLine;
	context->ClientExecute(context, &exec);
	return CHANNEL_RC_OK;
}

/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_handshake_ex(RailClientContext* context,
	RAIL_HANDSHAKE_EX_ORDER* handshakeEx)
{
	return CHANNEL_RC_OK;
}

/**
* The app was resized on the server by clicking on gui element or something
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_local_move_size(RailClientContext* context,
	RAIL_LOCALMOVESIZE_ORDER* localMoveSize)
{
	// not implemented, unused
	return CHANNEL_RC_OK;
}

/**
* local resize, set min max information on the client
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_min_max_info(RailClientContext* context,
	RAIL_MINMAXINFO_ORDER* minMaxInfo)
{
	// not implemented, unused
	return CHANNEL_RC_OK;
}

/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_language_bar_info(RailClientContext* context,
	RAIL_LANGBAR_INFO_ORDER* langBarInfo)
{
	return CHANNEL_RC_OK;
}

/**
* Function description
*
* @return 0 on success, otherwise a Win32 error code
*/
static UINT xf_rail_server_get_appid_response(RailClientContext* context,
	RAIL_GET_APPID_RESP_ORDER* getAppIdResp)
{
	return CHANNEL_RC_OK;
}

int dumb_rail_init(dcContext* xfc, RailClientContext* rail)
{
	rdpContext* context = (rdpContext*)xfc;
	xfc->rail = rail;
	xf_rail_register_update_callbacks(context->update);
	rail->custom = (void*)xfc;
	rail->ServerExecuteResult = xf_rail_server_execute_result;
	rail->ServerSystemParam = xf_rail_server_system_param;
	rail->ServerHandshake = xf_rail_server_handshake;
	rail->ServerHandshakeEx = xf_rail_server_handshake_ex;
	rail->ServerLocalMoveSize = xf_rail_server_local_move_size;
	rail->ServerMinMaxInfo = xf_rail_server_min_max_info;
	rail->ServerLanguageBarInfo = xf_rail_server_language_bar_info;
	rail->ServerGetAppIdResponse = xf_rail_server_get_appid_response;

	return 1;
}

int xf_rail_uninit(dcContext* xfc, RailClientContext* rail)
{
	if (xfc->rail)
	{
		xfc->rail->custom = NULL;
		xfc->rail = NULL;
	}

	return 1;
}


#pragma endregion


