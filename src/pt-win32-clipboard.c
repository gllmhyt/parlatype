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

static void
pt_win32clibboard_copydata_cb (HWND main_window_handle,
			       WPARAM wParam,
			       LPARAM lparam,
			       PtWin32clipboard *self)
{
	PtPlayer *player = pt_controller_get_player (PT_CONTROLLER (self));
	gchar *timestamp;
	gdouble value;

	//Copy the information sent via lparam param into a structure
	COPYDATASTRUCT* copy_data_structure = {0};
	copy_data_structure = (COPYDATASTRUCT*)lparam;

	switch (copy_data_structure->dwData) {
		case PLAY_PAUSE:
			pt_player_play_pause (player);
			break;
		case GOTO_TIMESTAMP:
			timestamp = (gchar*)copy_data_structure->lpData;
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
			timestamp = pt_player_get_timestamp (player);
			if (!timestamp)
				timestamp = "";
			COPYDATASTRUCT cds;
			cds.dwData = RESPONSE_TIMESTAMP;
			cds.cbData = sizeof(TCHAR) * (strlen(timestamp) + 1);
			cds.lpData = timestamp;
			SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)(HWND)main_window_handle, (LPARAM)(LPVOID)&cds);
			break;
		case GET_URI:
			uri = pt_player_get_uri (player);
			if (!uri)
				uri = "";
			COPYDATASTRUCT cds;
			cds.dwData = RESPONSE_URI;
			cds.cbData = sizeof(TCHAR) * (strlen(uri) + 1);
			cds.lpData = uri;
			SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)(HWND)main_window_handle, (LPARAM)(LPVOID)&cds);
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

	g_print ("message handler: WM_COPYDATA\n");
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

	//self->priv->class_name = TEXT("ParlatypeWND");
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.lpfnWndProc	= message_handler;
	wc.hInstance	= GetModuleHandle(NULL);
	wc.lpszClassName = TEXT(WND_CLASS_NAME);

	if (!RegisterClass(&wc)) {
		g_print ("Window class registration failed\n");
		return;
	}

	self->priv->hwnd = CreateWindow("ParlatypeWND",
	                                TEXT("ParlatypeWIN32"),	/* title */
	                                0,			/* window style */
	                                0, 0,			/* x and y coordinates */
	                                0, 0,			/* with and height */
	                                NULL,			/* parent window */
	                                NULL,			/* menu */
	                                GetModuleHandle(NULL),
	                                NULL);
	if(!self->priv->hwnd) {
		g_print ("Can't create hidden window\n");
		UnregisterClass(WND_CLASS_NAME, NULL);
		return;
	}
	SetWindowLongPtr(self->priv->hwnd, GWLP_USERDATA, (uintptr_t)self);
	g_print ("Created hidden window\n");
}

static void
pt_win32clipboard_dispose (GObject *object)
{
	PtWin32clipboard *self = PT_WIN32CLIPBOARD (object);

	if (self->priv->hwnd)
		DestroyWindow (self->priv->hwnd);
	//UnregisterClass (WND_CLASS_NAME, NULL);

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
