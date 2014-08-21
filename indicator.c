/*
 * indicator.c - hexchat plugin to support the Messaging Indicator
 *
 * Copyright (C) 2009 Scott Parkerson <scott.parkerson@gmail.com>
 * Copyright (C) 2009-2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * modified by cheshire-mouse
 */

#include <config.h>
#include <messaging-menu.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "hexchat-plugin.h"
#include <unity.h>

#define MESSAGING_INDICATOR_PLUGIN_NAME     _("Messaging Indicator")
#define MESSAGING_INDICATOR_PLUGIN_VERSION  VERSION
#define MESSAGING_INDICATOR_PLUGIN_DESC     _("Notify the user on Xchat events via the Messaging Indicator")

void hexchat_plugin_get_info   (char **plugin_name, char **plugin_desc, char **plugin_version, void **reserved);
int  hexchat_plugin_init       (hexchat_plugin *plugin_handle, char **plugin_name, char **plugin_desc, char **plugin_version, char *arg);
int  hexchat_plugin_deinit     (void);

static hexchat_plugin	*ph;
static MessagingMenuApp *mmapp;
#ifdef HAVE_UNITY
static UnityLauncherEntry	*launcher = NULL;
#endif
static hexchat_hook	*msg_hilight;
static hexchat_hook	*action_hilight;
static hexchat_hook	*pm;
static hexchat_hook	*pm_dialog;
static hexchat_hook	*ch_nick;
static hexchat_hook	*tab_focus;
static GtkWindow	*win;
static gchar		*focused;
static gboolean		run = FALSE;
static gint count = 0;
static void remove_indsource (gchar *sourceid);
static void add_indsource (gchar *sourceid);
static void really_activate_window (GtkWindow *win);
gboolean focus_win_cb (GtkWindow *win,  GParamSpec *pspec, gpointer data);
static hexchat_context *stored_ctx = NULL;

static void
source_display (MessagingMenuApp *mmapp,
				const gchar *channel,
				gpointer user_data)
{
	GtkWindow *win;

		if (channel) {
			hexchat_context *current_ctx;
			current_ctx = hexchat_get_context(ph);
			hexchat_context *ctx;
			ctx = hexchat_find_context (ph, NULL, channel);
			/*
			g_debug ("INDICATOR: Changing to channel %s", channel);
			*/
			win = (GtkWindow *)hexchat_get_info (ph, "win_ptr");
			if (hexchat_set_context (ph, ctx)) {
				/*
				g_debug ("hexchat-indicator: Showing window");
				*/
				hexchat_command (ph, "GUI SHOW");
				hexchat_command (ph, "GUI FOCUS");
				gtk_widget_show (GTK_WIDGET (win));
				really_activate_window (GTK_WINDOW (win));
				/* gtk_window_present_with_time(GTK_WINDOW (win), timestamp); */
		} else {
			g_warning ("hexchat-indicator: context change fail");
		}
		if (channel != NULL) {
			remove_indsource (channel);
		}
		hexchat_set_context (ph, current_ctx);
	}
	/*
	g_debug ("hexchat-indicator: Indicator displayed");
	*/
}

static void
remove_indsource (gchar *channel)
{
	count = count - 1;
	messaging_menu_app_remove_source (MESSAGING_MENU_APP (mmapp), channel);

#ifdef HAVE_UNITY
	if (launcher == NULL) 
	{
		return;
	}

	/*
	g_debug ("LAUNCHER: count is %d", count);
	*/
	if (count > 0)
	{
		/*
		g_debug ("LAUNCHER: setting count to %d", count);
		*/
		unity_launcher_entry_set_count (launcher, count);
		unity_launcher_entry_set_count_visible (launcher, TRUE);
	} else {
		unity_launcher_entry_set_count (launcher, count);
		/*
		g_debug ("LAUNCHER: hiding count");
		*/
		unity_launcher_entry_set_count_visible (launcher, FALSE);
	}
#endif
}

static void
update_indicator (gchar *sourceid)
{
	if (sourceid != NULL)
	{
		/*
		g_debug ("hexchat-indicator: found existing indicator");
		*/
		messaging_menu_app_set_source_time (MESSAGING_MENU_APP (mmapp),
			sourceid,  g_get_real_time ());
	}

}

static void
add_indsource (gchar *sourceid)
{
	if (messaging_menu_app_has_source(MESSAGING_MENU_APP (mmapp), sourceid)) {
		update_indicator (sourceid);
		return;
	}

	if (!run)
	{
		GtkWindow *win = (GtkWindow *)hexchat_get_info (ph, "win_ptr");
		if (GTK_IS_WINDOW (win))
		{
			g_signal_connect(G_OBJECT(win), "notify::has-toplevel-focus", G_CALLBACK (focus_win_cb), NULL);
		run = TRUE;
		}
	}

	count = count + 1;
	messaging_menu_app_append_source (MESSAGING_MENU_APP (mmapp), sourceid, NULL, sourceid);
	messaging_menu_app_draw_attention (MESSAGING_MENU_APP (mmapp), sourceid);

#ifdef HAVE_UNITY
	if (launcher == NULL)
	{
		return;
	}

	/*
	g_debug ("LAUNCHER: count is %d", count);
	*/
	if (count > 0)
	{
		unity_launcher_entry_set_count (launcher, count);
		unity_launcher_entry_set_count_visible (launcher, TRUE);
	}
#endif
}

static int
nick_change_cb (char *word[], void *data)
{
	char *old_nick, *new_nick;

	old_nick = word[1];
	new_nick = word[2];

	/* TODO: libmessaging doesn't allow renaming a source, so removing
	   the old nick one and adding a new entry, that reset the time though */
	if (messaging_menu_app_has_source (MESSAGING_MENU_APP (mmapp), old_nick)) {
		messaging_menu_app_remove_source (MESSAGING_MENU_APP (mmapp), old_nick);
		messaging_menu_app_append_source (MESSAGING_MENU_APP (mmapp), new_nick, NULL, new_nick);
	}

	return HEXCHAT_EAT_NONE;
}

static int
indicate_msg_cb (char *word[], gpointer data)
{
	GtkWindow *win;
	hexchat_context *ctx;
	ctx = hexchat_find_context (ph, NULL, word[2]);
	hexchat_set_context (ph, ctx);
	gchar *channel = hexchat_get_info (ph, "channel");
	win = (GtkWindow *)hexchat_get_info (ph, "win_ptr");
	
	if (focused == channel && GTK_WINDOW (win)->has_toplevel_focus)
		return HEXCHAT_EAT_NONE;

	add_indsource (channel);
	return HEXCHAT_EAT_NONE;
}

static int
focus_tab_cb (char *word[], gpointer data)
{
	gchar *channel;
	channel = hexchat_get_info (ph, "channel");
	/*
	g_debug ("hexchat-indicator: tab focused for channel %s", channel);
	*/
	if (messaging_menu_app_has_source(MESSAGING_MENU_APP (mmapp), channel)) {
		/*
		g_debug ("hexchat-indicator: found indicator for %s", channel);
		*/
		remove_indsource (channel);
	}
	focused = channel;
	/* store context to workaround focus_win_cb issues */
	stored_ctx =  hexchat_get_context (ph);
	return HEXCHAT_EAT_NONE;
}

gboolean
focus_win_cb (GtkWindow *win,  GParamSpec *pspec, gpointer data)
{
	gchar *channel;

	if (!GTK_WINDOW (win)->has_toplevel_focus)
	{
		return FALSE;
	}
	/*
	g_debug ("hexchat-indicator: window focused");
	*/
	
	/* restore the context of the previously focussed tab, not sure if xchat
	   is buggy or doing it on purpose but it sets the current context, 
	   when receiving a message, to the channel which received the event and
	   doesn't change it back to the selected tab on focus events. 
	   The focus_tab stored value should give us what we want though */
	if (stored_ctx)
		hexchat_set_context (ph, stored_ctx);
	
	channel = hexchat_get_info (ph, "channel");
	/*
	g_debug ("hexchat-indicator: tab focused for channel %s", channel);
	*/
	if (messaging_menu_app_has_source(MESSAGING_MENU_APP (mmapp), channel)) {
		/*
		g_debug ("hexchat-indicator: found indicator for %s", channel);
		*/
		remove_indsource (channel);
	}
	focused = channel;
	return FALSE;
}

/* Really raise the window, even if the window manager doesn't agree */
static void
really_activate_window (GtkWindow *window)
{
	Screen *screen;
	Time    timestamp;
	XEvent  xev;

	g_return_if_fail (GTK_IS_WINDOW (window));

	screen = GDK_SCREEN_XSCREEN (gtk_widget_get_screen (GTK_WIDGET (window)));
	timestamp = GDK_CURRENT_TIME;

	xev.xclient.type = ClientMessage;
	xev.xclient.serial = 0;
	xev.xclient.send_event = True;
	xev.xclient.display = GDK_DISPLAY ();
	xev.xclient.window = GDK_WINDOW_XWINDOW (GTK_WIDGET (window)->window);
	xev.xclient.message_type = gdk_x11_get_xatom_by_name ("_NET_ACTIVE_WINDOW");
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 2; /* Pager client type */
	xev.xclient.data.l[1] = timestamp;
	xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[3] = 0;
	xev.xclient.data.l[4] = 0;

	gdk_error_trap_push ();
	XSendEvent (GDK_DISPLAY (),
		RootWindowOfScreen (screen),
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xev);
	gdk_error_trap_pop ();
}

void
hexchat_plugin_get_info (char **plugin_name, char **plugin_desc, char **plugin_version, void **reserved)
{
	*plugin_name    = MESSAGING_INDICATOR_PLUGIN_NAME;
	*plugin_desc    = MESSAGING_INDICATOR_PLUGIN_DESC;
	*plugin_version = MESSAGING_INDICATOR_PLUGIN_VERSION;
}

int
hexchat_plugin_init (hexchat_plugin *plugin_handle, char **plugin_name, char **plugin_desc, char **plugin_version, char *arg)
{
	hexchat_plugin_get_info (plugin_name, plugin_desc, plugin_version, NULL);

	ph = plugin_handle;
	const gchar *desktop_id = g_strconcat(g_get_prgname(), ".desktop", NULL);

	mmapp = messaging_menu_app_new (desktop_id);
	messaging_menu_app_register (MESSAGING_MENU_APP (mmapp));
	g_signal_connect (mmapp, "activate-source", G_CALLBACK (source_display), NULL);

#ifdef HAVE_UNITY
	launcher = unity_launcher_entry_get_for_desktop_id (desktop_id);
#endif

	msg_hilight = hexchat_hook_print (ph, "Channel Msg Hilight",
		HEXCHAT_PRI_NORM, indicate_msg_cb, NULL);
	action_hilight = hexchat_hook_print (ph, "Channel Action Hilight",
		HEXCHAT_PRI_NORM, indicate_msg_cb, NULL);
	pm = hexchat_hook_print (ph, "Private Message",
		HEXCHAT_PRI_NORM, indicate_msg_cb, NULL);
	pm_dialog = hexchat_hook_print (ph, "Private Message to Dialog",
		HEXCHAT_PRI_NORM, indicate_msg_cb, NULL);
	tab_focus = hexchat_hook_print (ph, "Focus Tab",
		HEXCHAT_PRI_NORM, focus_tab_cb, NULL);
	ch_nick = hexchat_hook_print (ph, "Change Nick",
		HEXCHAT_PRI_NORM, nick_change_cb, NULL);

	hexchat_print (ph,  (g_strjoin (" ", MESSAGING_INDICATOR_PLUGIN_NAME, MESSAGING_INDICATOR_PLUGIN_VERSION, _("plugin loaded."), NULL)));

	return TRUE;
}

int
hexchat_plugin_deinit (void)
{
	hexchat_unhook (ph, msg_hilight);
	hexchat_unhook (ph, action_hilight);
	hexchat_unhook (ph, pm);
	hexchat_unhook (ph, pm_dialog);
	hexchat_unhook (ph, tab_focus);

	g_object_unref (mmapp);

	hexchat_print (ph,  (g_strjoin (" ", MESSAGING_INDICATOR_PLUGIN_NAME, MESSAGING_INDICATOR_PLUGIN_VERSION, _("plugin unloaded."), NULL)));

	return TRUE;
}
