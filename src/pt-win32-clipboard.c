/* Copyright (C) Gabor Karsay 2020 <gabor.karsay@gmx.at>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "config.h"
#include <windows.h>
#include <gtk/gtk.h>
#include <pt-player.h>
#include "pt-window.h"
#include "pt-win32-clipboard.h"

struct _PtWin32clipboardPrivate
{
	HWND hwnd;	/* Handle of hidden window */
};

enum
{
	/* Methods without response */

	PLAY_PAUSE,
	GOTO_TIMESTAMP,
	JUMP_BACK,
	JUMP_FORWARD,
	DECREASE_SPEED,
	INCREASE_SPEED,

	/* Methods with response */

	GET_TIMESTAMP,
	GET_URI,

	/* Responses */
	RESPONSE_TIMESTAMP,
	RESPONSE_URI
};

#define WND_CLASS_NAME "ParlatypeWND"

G_DEFINE_TYPE_WITH_PRIVATE (PtWin32clipboard, pt_win32clipboard, PT_CONTROLLER_TYPE)

/*
 * LogLastWinError
 *
 * Log the last Windows error as returned by
 * GetLastError.
 *
 * Taken from gimp-2.10.8/plug-ins/twain/tw_win.c
 * Copyright (C) 1999 Craig Setera, GPL-3+
 */
void
LogLastWinError (void)
{
	LPVOID lpMsgBuf;

	FormatMessage (
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError (),
		MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);

	LogMessage ("%s\n", lpMsgBuf);

	/* Free the buffer. */
	LocalFree (lpMsgBuf);
}


static void
pt_win32clibboard_copydata_cb (HWND              window,
			       WPARAM            sender,
			       LPARAM            data,
			       PtWin32clipboard *self)
{
	PtPlayer  *player = pt_controller_get_player (PT_CONTROLLER (self));
	gchar     *senddata;
	gunichar2 *senddata_utf16;
	gdouble    value;

	COPYDATASTRUCT *copydata = (COPYDATASTRUCT*) data;
	COPYDATASTRUCT  response;

	/*  TODO what about this?
	if (copydata->cbData == 0)
		return;*/

	switch (copydata->dwData) {
		case PLAY_PAUSE:
			pt_player_play_pause (player);
			break;
		case GOTO_TIMESTAMP:
			timestamp = (gchar*) copydata->lpData;
			pt_player_goto_timestamp (player, timestamp);
			break;
		case JUMP_BACK:
			pt_player_jump_back (player);
			break;
		case JUMP_FORWARD:
			pt_player_jump_forward (player);
			break;
		case DECREASE_SPEED:
			g_object_get (player, "speed", &value, NULL);
			pt_player_set_speed (player, value - 0.1);
			break;
		case INCREASE_SPEED:
			g_object_get (player, "speed", &value, NULL);
			pt_player_set_speed (player, value + 0.1);
			break;
		case GET_TIMESTAMP:
			/* TODO can we send NULL? */
			senddata = pt_player_get_timestamp (player);
			if (!senddata)
				senddata = "";
			response.dwData = RESPONSE_TIMESTAMP;
			response.cbData = sizeof (TCHAR) * (strlen (senddata) + 1); /* TODO check strlen vs g_utf8_strlen */
			response.lpData = senddata;
			SendMessage ((HWND) sender, WM_COPYDATA,
				     (WPARAM) (HWND) window,
				     (LPARAM) (LPVOID) &response);
			if (senddata[0] != '\0')
				g_free (senddata);
			break;
		case GET_URI:
			senddata = pt_player_get_uri (player);
			if (!senddata)
				senddata = "";
			senddata_utf16 = g_utf8_to_utf16 (uri, -1, NULL, NULL, NULL);
			response.dwData = RESPONSE_URI;
			response.cbData = sizeof (WCHAR) * (g_utf8_strlen (senddata, -1) + 1);
			response.lpData = senddata_utf16;
			SendMessage ((HWND) sender, WM_COPYDATA,
				     (WPARAM) (HWND) window,
				     (LPARAM) (LPVOID) &response);
			if (senddata[0] != '\0')
				g_free (senddata);
			g_free (senddata_utf16);
			break;
		default:
			break;
	}

	/* Windows' pchar (ANSI) = simply cast to gchar
	 * Windows' wchar (Unicode) has to be converted to gchar:
	 * gchar *res = g_utf16_to_utf8 (copy_data_structure->lpData, -1, NULL, NULL, NULL); */
}

static LRESULT CALLBACK
message_handler (HWND hwnd,
                 UINT Message,
                 WPARAM wParam,
                 LPARAM lParam)
{
	if (Message != WM_COPYDATA)
		return DefWindowProc (hwnd, Message, wParam, lParam);

	g_log_structured (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
		          "MESSAGE", "message handler: WM_COPYDATA");
	PtWin32clipboard *self = (PtWin32clipboard*)
			(uintptr_t) GetWindowLongPtr (hwnd, GWLP_USERDATA);
	pt_win32clibboard_copydata_cb (hwnd, wParam, lParam, self);

	return TRUE;
}

void
pt_win32clipboard_start (PtWin32clipboard *self)
{
}

static void
pt_win32clipboard_init (PtWin32clipboard *self)
{
	self->priv = pt_win32clipboard_get_instance_private (self);
	WNDCLASS wc;

	memset (&wc, 0, sizeof (wc));
	wc.lpfnWndProc	 = message_handler;
	wc.hInstance	 = GetModuleHandle (NULL);
	wc.lpszClassName = WND_CLASS_NAME;

	if (!RegisterClass(&wc)) {
		LogLastWinError ();
		return;
	}

	/* TODO Try L"ParlatypeWIN32" and TEXT ("ParlatypeWIN32") */
	self->priv->hwnd = CreateWindow("ParlatypeWND",
	                                "ParlatypeWIN32",	/* title */
	                                0,			/* window style */
	                                0, 0,			/* x and y coordinates */
	                                0, 0,			/* with and height */
	                                NULL,			/* parent window */
	                                NULL,			/* menu */
	                                wc.hInstance,
	                                NULL);

	if (!self->priv->hwnd) {
		g_log_structured (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			          "MESSAGE", "Can't create hidden window");
		UnregisterClass(WND_CLASS_NAME, NULL);
		return;
	}

	SetWindowLongPtr (self->priv->hwnd, GWLP_USERDATA, (uintptr_t) self);
	g_log_structured (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
		          "MESSAGE", "Created hidden window");
}

static void
pt_win32clipboard_dispose (GObject *object)
{
	PtWin32clipboard *self = PT_WIN32CLIPBOARD (object);

	if (self->priv->hwnd)
		DestroyWindow (self->priv->hwnd);

	G_OBJECT_CLASS (pt_win32clipboard_parent_class)->dispose (object);
}

static void
pt_win32clipboard_class_init (PtWin32clipboardClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = pt_win32clipboard_dispose;
}

PtWin32clipboard *
pt_win32clipboard_new (PtWindow *win)
{
	return g_object_new (PT_WIN32CLIPBOARD_TYPE, "win", win, NULL);
}
