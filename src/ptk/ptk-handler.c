/*
 * SpaceFM ptk-handler.c
 * 
 * Copyright (C) 2014 IgnorantGuru <ignorantguru@gmx.com>
 * Copyright (C) 2014 OmegaPhil <omegaphil@gmail.com>
 * Copyright (C) 2006 Hong Jen Yee (PCMan) <pcman.tw (AT) gmail.com>
 * 
 * License: See COPYING file
 * 
*/


#include <glib/gi18n.h>
#include <string.h>

#include "ptk-handler.h"
#include "settings.h"
#include "exo-tree-view.h"

#include "gtk2-compat.h"

// Archive handlers treeview model enum
enum {
    COL_XSET_NAME,
    COL_HANDLER_NAME
};

// Archive creation handlers combobox model enum
enum {
    // COL_XSET_NAME
    COL_HANDLER_EXTENSIONS = 1
};

/*
const ArchiveHandler handlers[]=
    {
        {
            "application/x-bzip-compressed-tar",
            "tar %o -cvjf",
            "tar -xvjf",
            "tar -tvf",
            ".tar.bz2", "arc_tar_bz2", TRUE
        },
        {
            "application/x-compressed-tar",
            "tar %o -cvzf",
            "tar -xvzf",
            "tar -tvf",
            ".tar.gz", "arc_tar_gz", TRUE
        },
        {
            "application/x-xz-compressed-tar",  //MOD added
            "tar %o -cvJf",
            "tar -xvJf",
            "tar -tvf",
            ".tar.xz", "arc_tar_xz", TRUE
        },
        {
            "application/zip",
            "zip %o -r",
            "unzip",
            "unzip -l",
            ".zip", "arc_zip", TRUE
        },
        {
            "application/x-7z-compressed",
            "7za %o a", // hack - also uses 7zr if available
            "7za x",
            "7za l",
            ".7z", "arc_7z", TRUE
        },
        {
            "application/x-tar",
            "tar %o -cvf",
            "tar -xvf",
            "tar -tvf",
            ".tar", "arc_tar", TRUE
        },
        {
            "application/x-rar",
            "rar a -r %o",
            "unrar -o- x",
            "unrar lt",
            ".rar", "arc_rar", TRUE
        },
        {
            "application/x-gzip",
            NULL,
            "gunzip",
            NULL,
            ".gz", NULL, TRUE
        }
    };
*/
// Function prototypes
static void on_configure_handler_enabled_check( GtkToggleButton *togglebutton,
                                                gpointer user_data );
static void restore_defaults( GtkWidget* dlg );
static gboolean validate_handler( GtkWidget* dlg );


static XSet* add_new_arctype()
{
    // creates a new xset for a custom archive type
    XSet* set;
    char* rand;
    char* name = NULL;

    // get a unique new xset name
    do
    {
        g_free( name );
        rand = randhex8();
        name = g_strdup_printf( "arccust_%s", rand );
        g_free( rand );
    }
    while ( xset_is( name ) );

    // create and return the xset
    set = xset_get( name );
    g_free( name );
    return set;
}

// handler_xset_name optional if handler_xset passed
static void config_load_handler_settings( XSet* handler_xset,
                                          gchar* handler_xset_name,
                                          GtkWidget* dlg )
{
    // Fetching actual xset if only the name has been passed
    if ( !handler_xset )
        handler_xset = xset_get( handler_xset_name );

/*igcr this code is repeated in several places in this file.  Would be more
 * efficient to create a struct and just pass that (or set it as a single
 * object) - see ptk-file-misc.c MoveSet typedef */
    // Fetching widget references
    GtkWidget* chkbtn_handler_enabled = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "chkbtn_handler_enabled" );
    GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_name" );
    GtkWidget* entry_handler_mime = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_mime" );
    GtkWidget* entry_handler_extension = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extension" );
    GtkWidget* entry_handler_compress = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_compress" );
    GtkWidget* entry_handler_extract = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extract" );
    GtkWidget* entry_handler_list = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_list" );
    GtkWidget* chkbtn_handler_compress_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_compress_term" );
    GtkWidget* chkbtn_handler_extract_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_extract_term" );
    GtkWidget* chkbtn_handler_list_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "chkbtn_handler_list_term" );
    GtkWidget* btn_remove = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "btn_remove" );
    GtkWidget* btn_apply = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "btn_apply" );

    /* At this point a handler exists, so making remove and apply buttons
     * sensitive as well as the enabled checkbutton */
    gtk_widget_set_sensitive( GTK_WIDGET( btn_remove ), TRUE );
    gtk_widget_set_sensitive( GTK_WIDGET( btn_apply ), TRUE );
    gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_enabled ), TRUE );

    // Configuring widgets with handler settings. Only name, MIME and
    // extension warrant a warning
    // Commands are prefixed with '+' when they are ran in a terminal
    gboolean check_value = handler_xset->b != XSET_B_TRUE ? FALSE : TRUE;
    int start;
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_enabled ),
                                    check_value );
    gtk_entry_set_text( GTK_ENTRY( entry_handler_name ),
                                    handler_xset->menu_label ?
                                    handler_xset->menu_label : "" );
    gtk_entry_set_text( GTK_ENTRY( entry_handler_mime ),
                                    handler_xset->s ?
                                    handler_xset->s : "" );
    gtk_entry_set_text( GTK_ENTRY( entry_handler_extension ),
                                    handler_xset->x ?
                                    handler_xset->x : "" );
/*igcr  all places below in this file where you use:
 *          gtk_entry_set_text( ..., g_strdup( ... ) )
 * and similar are memory leaks.  Don't use the g_strdup - set_text merely
 * copies the const string passed - see corrected example above.
 * Also, I didn't want the warnings above so removed them */
    if (!handler_xset->y)
    {
        gtk_entry_set_text( GTK_ENTRY( entry_handler_compress ),
                            g_strdup( "" ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_compress_term ),
                                        FALSE);
    }
    else
    {
        if ( handler_xset->y[0] == '+' )
        {
            gtk_entry_set_text( GTK_ENTRY( entry_handler_compress ),
                                g_strdup( (handler_xset->y) + 1 ) );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_compress_term ),
                                            TRUE);
        }
        else
        {
            gtk_entry_set_text( GTK_ENTRY( entry_handler_compress ),
                                g_strdup( handler_xset->y ) );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_compress_term ),
                                            FALSE);
        }
    }
    if (!handler_xset->z)
    {
        gtk_entry_set_text( GTK_ENTRY( entry_handler_extract ),
                            g_strdup( "" ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_extract_term ),
                                        FALSE);
    }
    else
    {
        if ( handler_xset->z[0] == '+' )
        {
            gtk_entry_set_text( GTK_ENTRY( entry_handler_extract ),
                                g_strdup( (handler_xset->z) + 1 ) );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_extract_term ),
                                            TRUE);
        }
        else
        {
            gtk_entry_set_text( GTK_ENTRY( entry_handler_extract ),
                                g_strdup( handler_xset->z ) );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_extract_term ),
                                            FALSE);
        }
    }
    if (!handler_xset->context)
    {
        gtk_entry_set_text( GTK_ENTRY( entry_handler_list ),
                            g_strdup( "" ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_list_term ),
                                        FALSE);
    }
    else
    {
        if ( handler_xset->context[0] == '+' )
        {
            gtk_entry_set_text( GTK_ENTRY( entry_handler_list ),
                                g_strdup( (handler_xset->context) + 1 ) );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_list_term ),
                                            TRUE);
        }
        else
        {
            gtk_entry_set_text( GTK_ENTRY( entry_handler_list ),
                                g_strdup( handler_xset->context ) );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_list_term ),
                                            FALSE);
        }
    }
}

static void config_unload_handler_settings( GtkWidget* dlg )
{
    // Fetching widget references
    GtkWidget* chkbtn_handler_enabled = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "chkbtn_handler_enabled" );
    GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_name" );
    GtkWidget* entry_handler_mime = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_mime" );
    GtkWidget* entry_handler_extension = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extension" );
    GtkWidget* entry_handler_compress = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_compress" );
    GtkWidget* entry_handler_extract = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extract" );
    GtkWidget* entry_handler_list = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_list" );
    GtkWidget* chkbtn_handler_compress_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_compress_term" );
    GtkWidget* chkbtn_handler_extract_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_extract_term" );
    GtkWidget* chkbtn_handler_list_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "chkbtn_handler_list_term" );
    GtkWidget* btn_remove = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "btn_remove" );
    GtkWidget* btn_apply = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "btn_apply" );

    // Disabling main change buttons
    gtk_widget_set_sensitive( GTK_WIDGET( btn_remove ), FALSE );
    gtk_widget_set_sensitive( GTK_WIDGET( btn_apply ), FALSE );

    // Unchecking handler
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_enabled ),
                                  FALSE);

    // Resetting all widgets
    gtk_entry_set_text( GTK_ENTRY( entry_handler_name ), g_strdup( "" ) );
    gtk_entry_set_text( GTK_ENTRY( entry_handler_mime ), g_strdup( "" ) );
    gtk_entry_set_text( GTK_ENTRY( entry_handler_extension ), g_strdup( "" ) );
    gtk_entry_set_text( GTK_ENTRY( entry_handler_compress ), g_strdup( "" ) );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_compress_term ),
                                  FALSE);
    gtk_entry_set_text( GTK_ENTRY( entry_handler_extract ), g_strdup( "" ) );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_extract_term ),
                                  FALSE);
    gtk_entry_set_text( GTK_ENTRY( entry_handler_list ), g_strdup( "" ) );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( chkbtn_handler_list_term ),
                                  FALSE);
}

static void populate_archive_handlers( GtkListStore* list, GtkWidget* dlg )
{
    // Fetching available archive handlers (literally gets member s from
    // the xset) - user-defined order has already been set
    char* archive_handlers_s = xset_get_s( "arc_conf2" );
/*igcr copying all these strings is inefficient, just need to parse */
    gchar** archive_handlers = g_strsplit( archive_handlers_s, " ", -1 );

    // Debug code
    //g_message("archive_handlers_s: %s", archive_handlers_s);

    // Looping for handlers (NULL-terminated list)
    GtkTreeIter iter;
    int i;
    for (i = 0; archive_handlers[i] != NULL; ++i)
    {
        // Obtaining appending iterator for treeview model
        gtk_list_store_append( GTK_LIST_STORE( list ), &iter );

        // Fetching handler
        XSet* handler_xset = xset_get( archive_handlers[i] );
/*igcr should verify arctype_ prefix in xset name before loading? */
        // Adding handler to model
/*igcr memory leak - don't copy these strings, just pass them */
        gchar* handler_name = g_strdup( handler_xset->menu_label );
        gchar* xset_name = g_strdup( archive_handlers[i] );
        gtk_list_store_set( GTK_LIST_STORE( list ), &iter,
                            COL_XSET_NAME, xset_name,
                            COL_HANDLER_NAME, handler_name,
                            -1 );

        // Populating widgets if this is the first handler
        if ( i == 0 )
            config_load_handler_settings( handler_xset, NULL,
                                          GTK_WIDGET( dlg ) );
    }

    // Clearing up archive_handlers
    g_strfreev( archive_handlers );
}


static void on_configure_button_press( GtkButton* widget, GtkWidget* dlg )
{
    const char* dialog_title = _("Archive Handlers");

    // Fetching widgets and handler details
    GtkButton* btn_add = (GtkButton*)g_object_get_data( G_OBJECT( dlg ),
                                            "btn_add" );
    GtkButton* btn_apply = (GtkButton*)g_object_get_data( G_OBJECT( dlg ),
                                            "btn_apply" );
    GtkButton* btn_remove = (GtkButton*)g_object_get_data( G_OBJECT( dlg ),
                                            "btn_remove" );
    GtkTreeView* view_handlers = (GtkTreeView*)g_object_get_data( G_OBJECT( dlg ),
                                            "view_handlers" );
    GtkWidget* chkbtn_handler_enabled = (GtkWidget*)g_object_get_data(
                                            G_OBJECT( dlg ),
                                            "chkbtn_handler_enabled" );
    GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data(
                                            G_OBJECT( dlg ),
                                                "entry_handler_name" );
    GtkWidget* entry_handler_mime = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_mime" );
    GtkWidget* entry_handler_extension = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extension" );
    GtkWidget* entry_handler_compress = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_compress" );
    GtkWidget* entry_handler_extract = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extract" );
    GtkWidget* entry_handler_list = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_list" );
    GtkWidget* chkbtn_handler_compress_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_compress_term" );
    GtkWidget* chkbtn_handler_extract_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_extract_term" );
    GtkWidget* chkbtn_handler_list_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "chkbtn_handler_list_term" );
    const gchar* handler_name = gtk_entry_get_text( GTK_ENTRY ( entry_handler_name ) );
    const gchar* handler_mime = gtk_entry_get_text( GTK_ENTRY ( entry_handler_mime ) );
    const gchar* handler_extension = gtk_entry_get_text(
                        GTK_ENTRY ( entry_handler_extension ) );
    const gboolean handler_compress_term = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON ( chkbtn_handler_compress_term ) );
    const gboolean handler_extract_term = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON ( chkbtn_handler_extract_term ) );
    const gboolean handler_list_term = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON ( chkbtn_handler_list_term ) );
    gchar* handler_compress, *handler_extract, *handler_list;

    // Commands are prefixed with '+' when they are to be ran in a
    // terminal
    // g_strdup'd to avoid anal const compiler warning...
    if (handler_compress_term)
    {
        handler_compress = g_strconcat( "+",
            gtk_entry_get_text( GTK_ENTRY ( entry_handler_compress ) ),
            NULL );
    }
    else
    {
        handler_compress = g_strdup( gtk_entry_get_text(
            GTK_ENTRY ( entry_handler_compress ) ) );
    }

    if (handler_extract_term)
    {
        handler_extract = g_strconcat( "+",
            gtk_entry_get_text( GTK_ENTRY ( entry_handler_extract ) ),
            NULL );
    }
    else
    {
        handler_extract = g_strdup( gtk_entry_get_text(
            GTK_ENTRY ( entry_handler_extract ) ) );
    }

    if (handler_list_term)
    {
        handler_list = g_strconcat( "+",
            gtk_entry_get_text( GTK_ENTRY ( entry_handler_list ) ),
            NULL );
    }
    else
    {
        handler_list = g_strdup( gtk_entry_get_text(
            GTK_ENTRY ( entry_handler_list ) ) );
    }

    // Fetching selection from treeview
    GtkTreeSelection* selection;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view_handlers ) );

    // Fetching the model and iter from the selection
    GtkTreeIter it;
    GtkTreeModel* model;

    // Checking if no selection is present (happens when the dialog first
    // loads, and when a handler is deleted)
    if ( !gtk_tree_selection_get_selected( GTK_TREE_SELECTION( selection ),
         NULL, NULL ) )
    {
        // There isnt selection - selecting the first item in the list
        GtkTreePath* new_path = gtk_tree_path_new_first();
        gtk_tree_selection_select_path( GTK_TREE_SELECTION( selection ),
                                new_path );
        gtk_tree_path_free( new_path );
        new_path = NULL;
    }

    // Obtaining iter for the current selection
    gchar* handler_name_from_model = NULL;  // Used to detect renames
    gchar* xset_name = NULL;
    XSet* handler_xset = NULL;

    // Getting selection fails if there are no handlers
    if ( gtk_tree_selection_get_selected( GTK_TREE_SELECTION( selection ),
         &model, &it ) )
    {
        // Succeeded - handlers present
        // Fetching data from the model based on the iterator. Note that
        // this variable used for the G_STRING is defined on the stack,
        // so should be freed for me
/*igcr  memory leak - xset_name and handler_name_from_model must be freed
 * by you.  See https://developer.gnome.org/gtk3/stable/GtkTreeModel.html#gtk-tree-model-get */
        gtk_tree_model_get( model, &it,
                            COL_XSET_NAME, &xset_name,
                            COL_HANDLER_NAME, &handler_name_from_model,
                            -1 );

        // Fetching the xset now I have the xset name
/*igcr xset_get always returns a valid xset as it will create it if it doesn't
 * exist, so it looks like you want to use xset_is here instead to just fetch
 * an xset only if it exists. */
        handler_xset = xset_get(xset_name);

        // Making sure it has been fetched
        if (!handler_xset)
        {
            g_warning("Unable to fetch the xset for the archive handler '%s'",
                                                            handler_name);
            goto _clean_exit;
        }
    }

    if ( widget == btn_add )
    {
        // Exiting if there is no handler to add
        if ( !( handler_name && handler_name[0] ) )
            goto _clean_exit;

        // Adding new handler as a copy of the current active handler
        XSet* new_handler_xset = add_new_arctype();
        new_handler_xset->b = gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON( chkbtn_handler_enabled )
                              ) ? XSET_B_TRUE : XSET_B_FALSE;

/*igcr memory leaks - don't pass g_strdup, just the const str */
        xset_set_set( new_handler_xset, "label", g_strdup( handler_name ) );
        xset_set_set( new_handler_xset, "s", g_strdup( handler_mime ) );  // Mime Type(s)
        xset_set_set( new_handler_xset, "x", g_strdup( handler_extension ) );  // Extension(s)
        xset_set_set( new_handler_xset, "y", g_strdup( handler_compress ) );  // Compress command
        xset_set_set( new_handler_xset, "z", g_strdup( handler_extract ) );  // Extract command
        xset_set_set( new_handler_xset, "cxt", g_strdup( handler_list ) );  // List command

        // Fetching list store
/*igcr jfyi shouldn't need an object for this - can get list store from list */
        GtkListStore* list = (GtkListStore*)g_object_get_data( G_OBJECT( dlg ), "list" );

        // Obtaining appending iterator for treeview model
        GtkTreeIter iter;
        gtk_list_store_append( GTK_LIST_STORE( list ), &iter );

        // Adding handler to model
/*igcr you don't need to copy these two strings, just pass them */
        gchar* new_handler_name = g_strdup( handler_name );
        gchar* new_xset_name = g_strdup( new_handler_xset->name );
        gtk_list_store_set( GTK_LIST_STORE( list ), &iter,
                            COL_XSET_NAME, new_xset_name,
                            COL_HANDLER_NAME, new_handler_name,
                            -1 );

        // Updating available archive handlers list
        if (g_strcmp0( xset_get_s( "arc_conf2" ), "" ) <= 0)
        {
            // No handlers present - adding new handler
            xset_set( "arc_conf2", "s", new_xset_name );
        }
        else
        {
            // Adding new handler to handlers
            gchar* new_handlers_list = g_strdup_printf( "%s %s",
                                                    xset_get_s( "arc_conf2" ),
                                                    new_xset_name );
            xset_set( "arc_conf2", "s", new_handlers_list );

            // Clearing up
            g_free(new_handlers_list);
        }

        // Clearing up
        g_free(new_handler_name);
        g_free(new_xset_name);

        // Activating the new handler - the normal loading code
        // automatically kicks in
        GtkTreePath* new_handler_path = gtk_tree_model_get_path( GTK_TREE_MODEL( model ),
                                                                &iter );
        gtk_tree_view_set_cursor( GTK_TREE_VIEW( view_handlers ),
                                        new_handler_path, NULL, FALSE );
        gtk_tree_path_free( new_handler_path );

        // Making sure the remove and apply buttons are sensitive
        gtk_widget_set_sensitive( GTK_WIDGET( btn_remove ), TRUE );
        gtk_widget_set_sensitive( GTK_WIDGET( btn_apply ), TRUE );

        /* Validating - remember that IG wants the handler to be saved
         * even with invalid commands */
        validate_handler( dlg );
    }
    else if ( widget == btn_apply )
    {
        // Exiting if apply has been pressed when no handlers are present
        if (xset_name == NULL) goto _clean_exit;

        /* Validating - remember that IG wants the handler to be saved
         * even with invalid commands */
        validate_handler( dlg );

        // Determining current handler enabled state
        gboolean handler_enabled = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON( chkbtn_handler_enabled ) ) ?
            XSET_B_TRUE : XSET_B_FALSE;

        // Checking if the handler has been renamed
        if (g_strcmp0( handler_name_from_model, handler_name ) != 0)
        {
            // It has - updating model
            gtk_list_store_set( GTK_LIST_STORE( model ), &it,
                        COL_XSET_NAME, xset_name,
                        COL_HANDLER_NAME, handler_name,
                        -1 );
        }

        // Saving archive handler
        handler_xset->b = handler_enabled;
        xset_set_set( handler_xset, "label", handler_name );
        xset_set_set( handler_xset, "s", handler_mime );
        xset_set_set( handler_xset, "x", handler_extension );
        xset_set_set( handler_xset, "y", handler_compress );
        xset_set_set( handler_xset, "z", handler_extract );
        xset_set_set( handler_xset, "cxt", handler_list );
    }
    else if ( widget == btn_remove )
    {
        // Exiting if remove has been pressed when no handlers are present
        if (xset_name == NULL) goto _clean_exit;

        // Updating available archive handlers list - fetching current
        // handlers
        const char* archive_handlers_s = xset_get_s( "arc_conf2" );
/*igcr considered that archive_handlers_s may == NULL ? thus archive_handlers
 * may == NULL   should confirm !NULL before access  - potential segfault on for loop */
/*igcr also inefficient to copy all these strings  - although may be fast
 * enough for this function - could use strstr to find deleted handler */
        gchar** archive_handlers = g_strsplit( archive_handlers_s, " ", -1 );
        gchar* new_archive_handlers_s = g_strdup( "" );
        gchar* new_archive_handlers_s_temp;

        // Looping for handlers (NULL-terminated list)
        int i;
        for (i = 0; archive_handlers[i] != NULL; ++i)
        {
            // Appending to new archive handlers list when it isnt the
            // deleted handler - remember that archive handlers are
            // referred to by their xset names, not handler names!!
            if (g_strcmp0( archive_handlers[i], xset_name ) != 0)
            {
                // Debug code
                //g_message("archive_handlers[i]: %s\nxset_name: %s",
                //                        archive_handlers[i], xset_name);

                new_archive_handlers_s_temp = new_archive_handlers_s;
                if (g_strcmp0( new_archive_handlers_s, "" ) == 0)
                {
                    new_archive_handlers_s = g_strdup( archive_handlers[i] );
                }
                else
                {
                    new_archive_handlers_s = g_strdup_printf( "%s %s",
                                new_archive_handlers_s, archive_handlers[i] );
                }
                g_free(new_archive_handlers_s_temp);
            }
        }

        // Finally updating handlers
        xset_set( "arc_conf2", "s", new_archive_handlers_s );

        // Deleting xset
        xset_custom_delete( handler_xset, FALSE );
        handler_xset = NULL;

        // Removing handler from the list
        gtk_list_store_remove( GTK_LIST_STORE( model ), &it );

        if (g_strcmp0( new_archive_handlers_s, "" ) == 0)
        {
            /* Making remove and apply buttons insensitive if the last
             * handler has been removed */
            gtk_widget_set_sensitive( GTK_WIDGET( btn_remove ), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET( btn_apply ), FALSE );

            /* Now that all are removed, the user needs sensitive widgets
             * to be able to add a further handler */
            gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_enabled ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_name ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_mime ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_extension ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_compress ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_extract ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_list ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_compress_term ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_extract_term ),
                                      TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_list_term ),
                                      TRUE );
        }
        else
        {
            /* Still remaining handlers - selecting the first one,
             * otherwise nothing is now selected */
            GtkTreePath *new_path = gtk_tree_path_new_first();
            gtk_tree_selection_select_path( GTK_TREE_SELECTION( selection ),
                                    new_path );
            gtk_tree_path_free( new_path );
        }

        // Clearing up
        g_strfreev( archive_handlers );
        g_free( new_archive_handlers_s );
    }

_clean_exit:

    // Saving settings
    xset_autosave( FALSE, FALSE );

    // Freeing strings
    g_free( handler_compress );
    g_free( handler_extract );
    g_free( handler_list );
}

static void on_configure_changed( GtkTreeSelection* selection,
                                  GtkWidget* dlg )
{
    /* This event is triggered when the selected row is changed through
     * the keyboard, or no row is selected at all */

    // Fetching the model and iter from the selection
    GtkTreeIter it;
    GtkTreeModel* model;
    if ( !gtk_tree_selection_get_selected( selection, &model, &it ) )
    {
        // User has unselected all rows - removing loaded handler
        config_unload_handler_settings( dlg );
        return;
    }

    /* Fetching data from the model based on the iterator. Note that this
     * variable used for the G_STRING is defined on the stack, so should
     * be freed for me */
/*igcr memory leak - free these */
    gchar* handler_name;  // Not actually used...
    gchar* xset_name;
    gtk_tree_model_get( model, &it,
                        COL_XSET_NAME, &xset_name,
                        COL_HANDLER_NAME, &handler_name,
                        -1 );

    // Loading new archive handler values
    config_load_handler_settings( NULL, xset_name, dlg );

    /* Focussing archive handler name
     * Selects the text rather than just placing the cursor at the start
     * of the text... */
    /*GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_name" );
    gtk_widget_grab_focus( entry_handler_name );*/
}

static void on_configure_drag_end( GtkWidget* widget,
                                   GdkDragContext* drag_context,
                                   GtkListStore* list )
{
    // Regenerating archive handlers list xset
    // Obtaining iterator pointing at first handler
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter_first( GTK_TREE_MODEL( list ), &iter ))
    {
        // Failed to get iterator - warning user and exiting
        g_warning("Drag'n'drop end event detected, but unable to get an"
        " iterator to the start of the model!");
        return;
    }

    // Looping for all handlers
    gchar* handler_name_unused;  // Not actually used...
    gchar* xset_name;
    gchar* archive_handlers = g_strdup( "" );
    gchar* archive_handlers_temp;
    do
    {
        // Fetching data from the model based on the iterator. Note that
        // this variable used for the G_STRING is defined on the stack,
        // so should be freed for me
/*igcr memory leak - free these */
        gtk_tree_model_get( GTK_TREE_MODEL( list ), &iter,
                            COL_XSET_NAME, &xset_name,
                            COL_HANDLER_NAME, &handler_name_unused,
                            -1 );

        archive_handlers_temp = archive_handlers;
        if (g_strcmp0( archive_handlers, "" ) == 0)
        {
            archive_handlers = g_strdup( xset_name );
        }
        else
        {
            archive_handlers = g_strdup_printf( "%s %s",
                archive_handlers, xset_name );
        }
        g_free(archive_handlers_temp);
    }
    while(gtk_tree_model_iter_next( GTK_TREE_MODEL( list ), &iter ));

/*igcr how can you be saving the list - what if the user presses cancel?
 * also, is it necessary to update the handlers list on reorder?  or can
 * it wait until dialog closes? */
    // Saving the new archive handlers list
    xset_set( "arc_conf2", "s", archive_handlers );
    g_free(archive_handlers);

    // Saving settings
    xset_autosave( FALSE, FALSE );

    // Ensuring first handler is selected (otherwise none are)
    GtkTreeSelection *selection = gtk_tree_view_get_selection(
                                        GTK_TREE_VIEW( widget ) );
    GtkTreePath *new_path = gtk_tree_path_new_first();
    gtk_tree_selection_select_path( GTK_TREE_SELECTION( selection ),
                            new_path );
    gtk_tree_path_free( new_path );
}

static void on_configure_handler_enabled_check( GtkToggleButton *togglebutton,
                                                gpointer user_data )
{
    /* When no handler is selected (i.e. the user selects outside of the
     * populated handlers list), the enabled checkbox might be checked
     * off - however the widgets must not be set insensitive when this
     * happens */
    GtkTreeView* view_handlers = (GtkTreeView*)g_object_get_data( G_OBJECT( user_data ),
                                            "view_handlers" );

    // Fetching selection from treeview
    GtkTreeSelection* selection;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view_handlers ) );
    
    // Exiting if no handler is selected
    GtkTreeIter it;
    GtkTreeModel* model;
    if ( !gtk_tree_selection_get_selected( selection, &model, &it ) )
        return;

    // Fetching current status
    gboolean enabled = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON ( togglebutton ) );

    // Getting at widgets
    GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data(
                                            G_OBJECT( user_data ),
                                                "entry_handler_name" );
    GtkWidget* entry_handler_mime = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                                "entry_handler_mime" );
    GtkWidget* entry_handler_extension = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                            "entry_handler_extension" );
    GtkWidget* entry_handler_compress = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                            "entry_handler_compress" );
    GtkWidget* entry_handler_extract = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                            "entry_handler_extract" );
    GtkWidget* entry_handler_list = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                                "entry_handler_list" );
    GtkWidget* chkbtn_handler_compress_term = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                        "chkbtn_handler_compress_term" );
    GtkWidget* chkbtn_handler_extract_term = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                        "chkbtn_handler_extract_term" );
    GtkWidget* chkbtn_handler_list_term = (GtkWidget*)g_object_get_data( G_OBJECT( user_data ),
                                            "chkbtn_handler_list_term" );

    // Setting sensitive/insensitive various widgets as appropriate
    gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_name ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_mime ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_extension ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_compress ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_extract ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( entry_handler_list ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_compress_term ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_extract_term ), enabled );
    gtk_widget_set_sensitive( GTK_WIDGET( chkbtn_handler_list_term ), enabled );
}

/*igcr some duplication here with on_configure_changed() - can you call
 * on_configure_changed(), or can a single event be used for selection
 * changed?  row-activated is when a row is activated by clicking
 * or double-clicking, or via keypress space/enter, not merely selected.  */
static void on_configure_row_activated( GtkTreeView* view,
                                        GtkTreePath* tree_path,
                                        GtkTreeViewColumn* col,
                                        GtkWidget* dlg )
{
    // This event is triggered when the selected row is changed by the
    // mouse

    // Fetching the model from the view
    GtkTreeModel* model = gtk_tree_view_get_model( GTK_TREE_VIEW( view ) );

    // Obtaining an iterator based on the view position
    GtkTreeIter it;
    if ( !gtk_tree_model_get_iter( model, &it, tree_path ) )
        return;

    // Fetching data from the model based on the iterator. Note that this
    // variable used for the G_STRING is defined on the stack, so should
    // be freed for me
/*igcr memory leaks - free these */
    gchar* handler_name;  // Not actually used...
    gchar* xset_name;
    gtk_tree_model_get( model, &it,
                        COL_XSET_NAME, &xset_name,
                        COL_HANDLER_NAME, &handler_name,
                        -1 );

    // Loading new archive handler values
    config_load_handler_settings( NULL, xset_name, dlg );

    // Focussing archive handler name
    // Selects the text rather than just placing the cursor at the start
    // of the text...
    /*GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_name" );
    gtk_widget_grab_focus( entry_handler_name );*/
}

static void restore_defaults( GtkWidget* dlg )
{
    // Note that defaults are also maintained in settings.c:xset_defaults

/*igcr also add a cancel button? */
    // Exiting if the user doesn't really want to restore defaults
    gboolean overwrite_handlers;
    if (xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                        _("Archive Handlers - Restore Defaults"), NULL,
                        GTK_BUTTONS_YES_NO, _("Do you want to overwrite"
                        " any existing default handlers? Missing handlers"
                        " will be added regardless."),
                        NULL, NULL) == GTK_RESPONSE_YES)
        overwrite_handlers = TRUE;
    else overwrite_handlers = FALSE;

    char *handlers_to_add = NULL, *str = NULL;

    /* Fetching current list of archive handlers, and constructing a
     * searchable version ensuring that a unique handler can be specified */
    char *handlers = xset_get_s( "arc_conf2" );
    char *handlers_search = NULL;
    if (g_strcmp0( handlers, "" ) <= 0)
        handlers_search = g_strdup( "" );
    else
        handlers_search = g_strconcat( " ", handlers, " ", NULL );

    gboolean handler_present = TRUE;
    XSet* set = xset_is( "arctype_7z" );
    if (!set)
    {
        set = xset_set( "arctype_7z", "label", "7-Zip" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "7-Zip" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-7z-compressed" );
        set->x = g_strdup( ".7z" );
        set->y = g_strdup( "+\"$(which 7za || echo 7zr)\" a %o %N" );  // compress command
        set->z = g_strdup( "+\"$(which 7za || echo 7zr)\" x %x" );     // extract command
        set->context = g_strdup( "+\"$(which 7za || echo 7zr)\" l %x" );  // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_7z " ))
        handlers_to_add = g_strdup( "arctype_7z" );

    handler_present = TRUE;
    set = xset_is( "arctype_gz" );
    if (!set)
    {
        set = xset_set( "arctype_gz", "label", "Gzip" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "Gzip" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-gzip:application/x-gzpdf" );
        set->x = g_strdup( ".gz" );
        set->y = g_strdup( "gzip -c %N > %O" );     // compress command
        set->z = g_strdup( "gzip -cd %x > %G" );    // extract command
        set->context = g_strdup( "+gunzip -l %x" );  // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_gz " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_gz", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_gz" );
    }

    handler_present = TRUE;
    set = xset_is( "arctype_rar" );
    if (!set)
    {
        set = xset_set( "arctype_rar", "label", "RAR" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "RAR" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-rar" );
        set->x = g_strdup( ".rar" );
        set->y = g_strdup( "+rar a -r %o %N" );     // compress command
        set->z = g_strdup( "+unrar -o- x %x" );     // extract command
        set->context = g_strdup( "+unrar lt %x" );  // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_rar " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_rar", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_rar" );
    }

    handler_present = TRUE;
    set = xset_is( "arctype_tar" );
    if (!set)
    {
        set = xset_set( "arctype_tar", "label", "Tar" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "Tar" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-tar" );
        set->x = g_strdup( ".tar" );
        set->y = g_strdup( "tar -cvf %o %N" );       // compress command
        set->z = g_strdup( "tar -xvf %x" );          // extract command
        set->context = g_strdup( "+tar -tvf %x" );   // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_tar " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_tar", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_tar" );
    }

    handler_present = TRUE;
    set = xset_is( "arctype_tar_bz2" );
    if (!set)
    {
        set = xset_set( "arctype_tar_bz2", "label", "Tar (bzip2)" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "Tar bzip2" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-bzip-compressed-tar" );
        set->x = g_strdup( ".tar.bz2" );
        set->y = g_strdup( "tar -cvjf %o %N" );       // compress command
        set->z = g_strdup( "tar -xvjf %x" );          // extract command
        set->context = g_strdup( "+tar -tvf %x" );    // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_tar_bz2 " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_tar_bz2", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_tar_bz2" );
    }

    handler_present = TRUE;
    set = xset_is( "arctype_tar_gz" );
    if (!set)
    {
        set = xset_set( "arctype_tar_gz", "label", "Tar (gzip)" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "Tar Gzip" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-compressed-tar" );
        set->x = g_strdup( ".tar.gz" );
        set->y = g_strdup( "tar -cvzf %o %N" );       // compress command
        set->z = g_strdup( "tar -xvzf %x" );          // extract command
        set->context = g_strdup( "+tar -tvf %x" );    // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_tar_gz " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_tar_gz", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_tar_gz" );
    }

    handler_present = TRUE;
    set = xset_is( "arctype_tar_xz" );
    if (!set)
    {
        set = xset_set( "arctype_tar_xz", "label", "Tar (xz)" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "Tar xz" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-xz-compressed-tar" );
        set->x = g_strdup( ".tar.xz" );
        set->y = g_strdup( "tar -cvJf %o %N" );       // compress command
        set->z = g_strdup( "tar -xvJf %x" );          // extract command
        set->context = g_strdup( "+tar -tvf %x" );    // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_tar_xz " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_tar_xz", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_tar_xz" );
    }

    handler_present = TRUE;
    set = xset_is( "arctype_zip" );
    if (!set)
    {
        set = xset_set( "arctype_zip", "label", "Zip" );
        handler_present = FALSE;
    }
    if (overwrite_handlers || !handler_present)
    {
        set->menu_label = g_strdup( "Zip" );
        set->b = XSET_B_TRUE;
        set->s = g_strdup( "application/x-zip:application/zip" );
        set->x = g_strdup( ".zip" );
        set->y = g_strdup( "+zip -r %o %N" );       // compress command
        set->z = g_strdup( "+unzip %x" );           // extract command
        set->context = g_strdup( "+unzip -l %x" );  // list command
    }

    /* Archive handler list check is separate as the XSet may exist but
     * the handler may not be in the list */
    if (!g_strstr_len( handlers_search, -1, " arctype_zip " ))
    {
        if (handlers_to_add)
        {
            str = handlers_to_add;
            handlers_to_add = g_strconcat( handlers_to_add,
                                           " arctype_zip", NULL );
            g_free( str );
        }
        else handlers_to_add = g_strdup( "arctype_zip" );
    }

    // Clearing up
    g_free( handlers_search );

    // Updating list of archive handlers
    if (handlers_to_add)
    {
        if (g_strcmp0( handlers, "" ) <= 0)
            handlers = handlers_to_add;
        else
        {
            handlers = g_strconcat( handlers, " ", handlers_to_add, NULL );
            g_free( handlers_to_add );
        }
        xset_set( "arc_conf2", "s", handlers );
        g_free( handlers );
    }

    /* Clearing and adding archive handlers to list (this also selects
     * the first handler and therefore populates the handler widgets) */
    GtkListStore* list = (GtkListStore*)g_object_get_data( G_OBJECT( dlg ), "list" );
    gtk_list_store_clear( GTK_LIST_STORE( list ) );
    populate_archive_handlers( GTK_LIST_STORE( list ), GTK_WIDGET( dlg ) );
}

static gboolean validate_handler( GtkWidget* dlg )
{
    const char* dialog_title = _("Archive Handlers");

    // Fetching widgets and handler details
    GtkWidget* entry_handler_name = (GtkWidget*)g_object_get_data(
                                            G_OBJECT( dlg ),
                                                "entry_handler_name" );
    GtkWidget* entry_handler_mime = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_mime" );
    GtkWidget* entry_handler_extension = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extension" );
    GtkWidget* entry_handler_compress = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_compress" );
    GtkWidget* entry_handler_extract = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "entry_handler_extract" );
    GtkWidget* entry_handler_list = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                                "entry_handler_list" );
    GtkWidget* chkbtn_handler_compress_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_compress_term" );
    GtkWidget* chkbtn_handler_extract_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                        "chkbtn_handler_extract_term" );
    GtkWidget* chkbtn_handler_list_term = (GtkWidget*)g_object_get_data( G_OBJECT( dlg ),
                                            "chkbtn_handler_list_term" );
    const gchar* handler_name = gtk_entry_get_text( GTK_ENTRY ( entry_handler_name ) );
    const gchar* handler_mime = gtk_entry_get_text( GTK_ENTRY ( entry_handler_mime ) );
    const gchar* handler_extension = gtk_entry_get_text( GTK_ENTRY ( entry_handler_extension ) );
    const gboolean handler_compress_term = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON ( chkbtn_handler_compress_term ) );
    const gboolean handler_extract_term = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON ( chkbtn_handler_extract_term ) );
    const gboolean handler_list_term = gtk_toggle_button_get_active(
                        GTK_TOGGLE_BUTTON ( chkbtn_handler_list_term ) );
    gchar* handler_compress, *handler_extract, *handler_list;

    /* Commands are prefixed with '+' when they are to be ran in a
     * terminal
     * g_strdup'd to avoid anal const compiler warning... */
    if (handler_compress_term)
    {
        handler_compress = g_strconcat( "+",
            gtk_entry_get_text( GTK_ENTRY ( entry_handler_compress ) ),
            NULL );
    }
    else
    {
        handler_compress = g_strdup( gtk_entry_get_text(
            GTK_ENTRY ( entry_handler_compress ) ) );
    }

    if (handler_extract_term)
    {
        handler_extract = g_strconcat( "+",
            gtk_entry_get_text( GTK_ENTRY ( entry_handler_extract ) ),
            NULL );
    }
    else
    {
        handler_extract = g_strdup( gtk_entry_get_text(
            GTK_ENTRY ( entry_handler_extract ) ) );
    }

    if (handler_list_term)
    {
        handler_list = g_strconcat( "+",
            gtk_entry_get_text( GTK_ENTRY ( entry_handler_list ) ),
            NULL );
    }
    else
    {
        handler_list = g_strdup( gtk_entry_get_text(
            GTK_ENTRY ( entry_handler_list ) ) );
    }

    /* Validating data. Note that data straight from widgets shouldnt
     * be modified or stored
     * Note that archive creation also allows for a command to be
     * saved */
    if (g_strcmp0( handler_name, "" ) <= 0)
    {
        /* Handler name not set - warning user and exiting. Note
         * that the created dialog does not have an icon set */
        xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            _("Please enter a valid handler name "
                            "before saving."), NULL, NULL );
        gtk_widget_grab_focus( entry_handler_name );
        return FALSE;
    }

    // Empty MIME is allowed if extension is filled
    if (g_strcmp0( handler_mime, "" ) <= 0 &&
        g_strcmp0( handler_extension, "" ) <= 0)
    {
        /* Handler MIME not set - warning user and exiting. Note
         * that the created dialog does not have an icon set */
/*igcr memory leak - passing g_strdup_printf */
        xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            g_strdup_printf(_("Please enter a valid "
                            "MIME content type OR extension for the "
                            "'%s' handler before saving."),
                            handler_name), NULL, NULL );
        gtk_widget_grab_focus( entry_handler_mime );
        return FALSE;
    }
    if (g_strstr_len( handler_mime, -1, " " ) &&
        g_strcmp0( handler_extension, "" ) <= 0)
    {
        /* Handler MIME contains a space - warning user and exiting.
         * Note that the created dialog does not have an icon set */
/*igcr memory leak - passing g_strdup_printf */
        xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            g_strdup_printf(_("Please ensure the MIME"
                            " content type for the '%s' handler "
                            "contains no spaces before saving."),
                            handler_name), NULL, NULL );
        gtk_widget_grab_focus( entry_handler_mime );
        return FALSE;
    }

    /* Empty extension is allowed if MIME type has been given, but if
     * anything has been entered it must be valid */
    if (
        (
            g_strcmp0( handler_extension, "" ) <= 0 &&
            g_strcmp0( handler_mime, "" ) <= 0
        )
        ||
        (
            g_strcmp0( handler_extension, "" ) > 0 &&
            handler_extension && *handler_extension != '.'
        )
    )
    {
        /* Handler extension is either not set or does not start with
         * a full stop - warning user and exiting. Note that the created
         * dialog does not have an icon set */
/*igcr memory leak - passing g_strdup_printf */
        xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            g_strdup_printf(_("Please enter a valid "
                            "file extension OR MIME content type for"
                            " the '%s' handler before saving."),
                            handler_name), NULL, NULL );
        gtk_widget_grab_focus( entry_handler_extension );
        return FALSE;
    }

    /* Other settings are commands to run in different situations -
     * since different handlers may or may not need different
     * commands, empty commands are allowed but if something is given,
     * relevant substitution characters should be in place */

    /* Compression handler validation - remember to maintain this code
     * in ptk_file_archiver_create too
     * Checking if a compression command has been entered */
    if (g_strcmp0( handler_compress, "" ) != 0 &&
        g_strcmp0( handler_compress, "+" ) != 0)
    {
        /* It has - making sure all substitution characters are in
         * place - not mandatory to only have one of the particular
         * type */
        if (
            (
                !g_strstr_len( handler_compress, -1, "%o" ) &&
                !g_strstr_len( handler_compress, -1, "%O" )
            )
            ||
            (
                !g_strstr_len( handler_compress, -1, "%n" ) &&
                !g_strstr_len( handler_compress, -1, "%N" )
            )
        )
        {
/*igcr memory leak - passing g_strdup_printf - also fits on small screen? */
            xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            g_strdup_printf(_("The following "
                            "substitution variables should be in the"
                            " '%s' compression command:\n\n"
                            "One of the following:\n\n"
                            "%%%%n: First selected file/directory to"
                            " archive\n"
                            "%%%%N: All selected files/directories to"
                            " archive\n\n"
                            "and one of the following:\n\n"
                            "%%%%o: Resulting single archive\n"
                            "%%%%O: Resulting archive per source "
                            "file/directory (see %%%%n/%%%%N)"),
                            handler_name), NULL, NULL );
            gtk_widget_grab_focus( entry_handler_compress );
            return FALSE;
        }
    }

    if (g_strcmp0( handler_extract, "" ) != 0 &&
        g_strcmp0( handler_extract, "+" ) != 0 &&
        (
            !g_strstr_len( handler_extract, -1, "%x" )
        ))
    {
        /* Not all substitution characters are in place - warning
         * user and exiting. Note that the created dialog does not
         * have an icon set
         * TODO: IG problem */
/*igcr memory leak - passing g_strdup_printf */
        xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            g_strdup_printf(_("The following "
                            "placeholders should be in the '%s' extraction"
                            " command:\n\n%%%%x: "
                            "Archive to extract"),
                            handler_name), NULL, NULL );
        gtk_widget_grab_focus( entry_handler_extract );
        return FALSE;
    }

    if (g_strcmp0( handler_list, "" ) != 0 &&
        g_strcmp0( handler_list, "+" ) != 0 &&
        (
            !g_strstr_len( handler_list, -1, "%x" )
        ))
    {
        /* Not all substitution characters are in place  - warning
         * user and exiting. Note that the created dialog does not
         * have an icon set
         * TODO: Confirm if IG still has this problem */
/*igcr memory leak - passing g_strdup_printf */
        xset_msg_dialog( GTK_WIDGET( dlg ), GTK_MESSAGE_WARNING,
                            dialog_title, NULL, FALSE,
                            g_strdup_printf(_("The following "
                            "placeholders should be in the '%s' list"
                            " command:\n\n%%%%x: "
                            "Archive to list"),
                            handler_name), NULL, NULL );
        gtk_widget_grab_focus( entry_handler_list );
        return FALSE;
    }

    // Validation passed
    return TRUE;
}

void ptk_handler_show_config( PtkFileBrowser* file_browser )
{
    /*
    Archives Types - 1 per xset as:
        set->name       xset name
        set->b          enabled  (XSET_UNSET|XSET_B_FALSE|XSET_B_TRUE)
        set->menu_label Display Name
        set->s          Mime Type(s)
        set->x          Extension(s)
        set->y          Compress Command
        set->z          Extract Command
        set->context    List Command

    Configure menu item is used to store some dialog data:
        get this set with:
            set = xset_get( "arc_conf2" );
        set->x          dialog width  (string)
        set->y          dialog height (string)
        set->s          space separated list of xset names (archive types)

    Example to add a new custom archive type:
        XSet* newset = add_new_arctype();
        newset->b = XSET_B_TRUE;                              // enable
        xset_set_set( newset, "label", "Windows CAB" );        // set archive Name - this internally sets menu_label
        xset_set_set( newset, "s", "application/winjunk" );    // set Mime Type(s)
        xset_set_set( newset, "x", ".cab" );                   // set Extension(s)
        xset_set_set( newset, "y", "createcab" );              // set Compress cmd
        xset_set_set( newset, "z", "excab" );                  // set Extract cmd
        xset_set_set( newset, "cxt", "listcab" );              // set List cmd - This really is cxt and not ctxt - xset_set_set bug thats already worked around

    Example to retrieve an xset for an archive type:
        XSet* set = xset_is( "arctype_rar" );
        if ( !set )
            // there is no set named "arctype_rar" (remove it from the list)
        else
        {
            const char* display_name = set->menu_label;
            const char* compress_cmd = set->y;
            gboolean enabled = xset_get_b_set( set );
            // etc
        }
    */

    // TODO: <IgnorantGuru> Also, you might have a look at how your config dialog behaves from the desktop menu.  Specifically, you may want to pass your function (DesktopWindow* desktop) in lieu of file_browser in that case.  So the prototype will be:
    // nm don't have that branch handy.  But you function can accept both file_browser and desktop and use whichever is non-NULL for the parent
    // If that doesn't make sense now, ask me later or I can hack it in.  That archive menu appears when right-clicking a desktop item

    /* Archive handlers dialog, attaching to top-level window (in GTK,
     * everything is a 'widget') - no buttons etc added as everything is
     * custom...
     * Extra NULL on the NULL-terminated list to placate an irrelevant
     * compilation warning
     * TODO: The below fails - file_browser->main_window is not a valid
     * widget when this function is called in a normal fashion, it is
     * when called via a keyboard shortcut. file_browser on its own
     * reports as a valid widget, but only broken modal behaviour results
     */
/*igcr file_browser may be null if desktop use later accomodated */
    GtkWidget *top_level = file_browser ? gtk_widget_get_toplevel(
                                GTK_WIDGET( file_browser->main_window ) ) :
                                NULL;
    GtkWidget *dlg = gtk_dialog_new_with_buttons( _("Archive Handlers"),
                    top_level ? GTK_WINDOW( top_level ) : NULL,
                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                    NULL, NULL );
    gtk_container_set_border_width( GTK_CONTAINER ( dlg ), 5 );

    // Debug code
    //g_message( "Parent window title: %s", gtk_window_get_title( GTK_WINDOW( top_level ) ) );

    // Forcing dialog icon
    xset_set_window_icon( GTK_WINDOW( dlg ) );

    // Setting saved dialog size
    int width = xset_get_int( "arc_conf2", "x" );
    int height = xset_get_int( "arc_conf2", "y" );
    if ( width && height )
        gtk_window_set_default_size( GTK_WINDOW( dlg ), width, height );

    // Adding the help button but preventing it from taking the focus on click
    gtk_button_set_focus_on_click(
                                    GTK_BUTTON(
                                        gtk_dialog_add_button(
                                            GTK_DIALOG( dlg ),
                                            GTK_STOCK_HELP,
                                            GTK_RESPONSE_HELP
                                        )
                                    ),
                                    FALSE );

    // Adding standard buttons and saving references in the dialog
    // 'Restore defaults' button has custom text but a stock image
    GtkButton* btn_defaults = GTK_BUTTON( gtk_dialog_add_button( GTK_DIALOG( dlg ),
                                                _("Re_store Defaults"),
                                                GTK_RESPONSE_NONE ) );
    GtkWidget* btn_defaults_image = xset_get_image( "GTK_STOCK_CLEAR",
                                                GTK_ICON_SIZE_BUTTON );
    gtk_button_set_image( GTK_BUTTON( btn_defaults ), GTK_WIDGET ( btn_defaults_image ) );
    g_object_set_data( G_OBJECT( dlg ), "btn_defaults", GTK_BUTTON( btn_defaults ) );
    g_object_set_data( G_OBJECT( dlg ), "btn_ok",
                        gtk_dialog_add_button( GTK_DIALOG( dlg ),
                                                GTK_STOCK_OK,
                                                GTK_RESPONSE_OK ) );

    // Generating left-hand side of dialog
    GtkWidget* lbl_handlers = gtk_label_new( NULL );
    gtk_label_set_markup( GTK_LABEL( lbl_handlers ), _("<b>Handlers</b>") );
    gtk_misc_set_alignment( GTK_MISC( lbl_handlers ), 0, 0 );

    // Generating the main manager list
    // Creating model - xset name then handler name
    GtkListStore* list = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );
    g_object_set_data( G_OBJECT( dlg ), "list", list );

    // Creating treeview - setting single-click mode (normally this
    // widget is used for file lists, where double-clicking is the norm
    // for doing an action)
    GtkWidget* view_handlers = exo_tree_view_new();
    gtk_tree_view_set_model( GTK_TREE_VIEW( view_handlers ), GTK_TREE_MODEL( list ) );
/*igcr probably doesn't need to be single click, as you're not using row
 * activation, only selection changed? */
    exo_tree_view_set_single_click( (ExoTreeView*)view_handlers, TRUE );
    gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view_handlers ), FALSE );
    g_object_set_data( G_OBJECT( dlg ), "view_handlers", view_handlers );

    // Turning the treeview into a scrollable widget
    GtkWidget* view_scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( view_scroll ),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC );
    gtk_container_add( GTK_CONTAINER( view_scroll ), view_handlers );

    // Enabling item reordering (GTK-handled drag'n'drop)
    gtk_tree_view_set_reorderable( GTK_TREE_VIEW( view_handlers ), TRUE );

    // Connecting treeview callbacks
    g_signal_connect( G_OBJECT( view_handlers ), "drag-end",
                        G_CALLBACK( on_configure_drag_end ),
                        GTK_LIST_STORE( list ) );
    g_signal_connect( G_OBJECT( view_handlers ), "row-activated",
                        G_CALLBACK( on_configure_row_activated ),
                        GTK_WIDGET( dlg ) );
    g_signal_connect( G_OBJECT( gtk_tree_view_get_selection(
                                    GTK_TREE_VIEW( view_handlers ) ) ),
                        "changed",
                        G_CALLBACK( on_configure_changed ),
                        GTK_WIDGET( dlg ) );

    // Adding column to the treeview
    GtkTreeViewColumn* col = gtk_tree_view_column_new();

    // Change columns to optimal size whenever the model changes
    gtk_tree_view_column_set_sizing( col, GTK_TREE_VIEW_COLUMN_AUTOSIZE );
    
    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start( col, renderer, TRUE );

    // Tie model data to the column
    gtk_tree_view_column_add_attribute( col, renderer,
                                         "text", COL_HANDLER_NAME);

    gtk_tree_view_append_column ( GTK_TREE_VIEW( view_handlers ), col );

    // Set column to take all available space - false by default
    gtk_tree_view_column_set_expand ( col, TRUE );

    // Treeview widgets
    GtkButton* btn_remove = GTK_BUTTON( gtk_button_new_with_mnemonic( _("_Remove") ) );
    gtk_button_set_image( btn_remove, xset_get_image( "GTK_STOCK_REMOVE",
                                                    GTK_ICON_SIZE_BUTTON ) );
    gtk_button_set_focus_on_click( btn_remove, FALSE );
    gtk_widget_set_sensitive( GTK_WIDGET( btn_remove ), FALSE );
    g_signal_connect( G_OBJECT( btn_remove ), "clicked",
                        G_CALLBACK( on_configure_button_press ), dlg );
    g_object_set_data( G_OBJECT( dlg ), "btn_remove", GTK_BUTTON( btn_remove ) );

    GtkButton* btn_add = GTK_BUTTON( gtk_button_new_with_mnemonic( _("_Add") ) );
    gtk_button_set_image( btn_add, xset_get_image( "GTK_STOCK_ADD",
                                                GTK_ICON_SIZE_BUTTON ) );
    gtk_button_set_focus_on_click( btn_add, FALSE );
    g_signal_connect( G_OBJECT( btn_add ), "clicked",
                        G_CALLBACK( on_configure_button_press ), dlg );
    g_object_set_data( G_OBJECT( dlg ), "btn_add", GTK_BUTTON( btn_add ) );

    GtkButton* btn_apply = GTK_BUTTON( gtk_button_new_with_mnemonic( _("A_pply") ) );
    gtk_button_set_image( btn_apply, xset_get_image( "GTK_STOCK_APPLY",
                                                GTK_ICON_SIZE_BUTTON ) );
    gtk_button_set_focus_on_click( btn_apply, FALSE );
    gtk_widget_set_sensitive( GTK_WIDGET( btn_apply ), FALSE );
    g_signal_connect( G_OBJECT( btn_apply ), "clicked",
                        G_CALLBACK( on_configure_button_press ), dlg );
    g_object_set_data( G_OBJECT( dlg ), "btn_apply", GTK_BUTTON( btn_apply ) );

    // Generating right-hand side of dialog
    GtkWidget* chkbtn_handler_enabled = gtk_check_button_new_with_label( _("Handler Enabled") );
    g_signal_connect( G_OBJECT( chkbtn_handler_enabled ), "toggled",
                G_CALLBACK ( on_configure_handler_enabled_check ), dlg );
    g_object_set_data( G_OBJECT( dlg ), "chkbtn_handler_enabled",
                        GTK_CHECK_BUTTON( chkbtn_handler_enabled ) );
    GtkWidget* lbl_handler_name = gtk_label_new( NULL );
    gtk_label_set_markup_with_mnemonic( GTK_LABEL( lbl_handler_name ),
                                        _("_Name:") ),
    gtk_misc_set_alignment( GTK_MISC( lbl_handler_name ), 0, 0.5 );
    GtkWidget* lbl_handler_mime = gtk_label_new( NULL );
    gtk_label_set_markup_with_mnemonic( GTK_LABEL( lbl_handler_mime ),
                                        _("MIME _Type:") );
    gtk_misc_set_alignment( GTK_MISC( lbl_handler_mime ), 0, 0.5 );
    GtkWidget* lbl_handler_extension = gtk_label_new( NULL );
    gtk_label_set_markup_with_mnemonic( GTK_LABEL( lbl_handler_extension ),
                                        _("E_xtension:") );
    gtk_misc_set_alignment( GTK_MISC( lbl_handler_extension ), 0, 0.5 );
    GtkWidget* lbl_handler_compress = gtk_label_new( NULL );
    gtk_label_set_markup_with_mnemonic( GTK_LABEL( lbl_handler_compress ),
                                        _("<b>Co_mpress:</b>") );
    gtk_misc_set_alignment( GTK_MISC( lbl_handler_compress ), 0, 0.5 );
    GtkWidget* lbl_handler_extract = gtk_label_new( NULL );
    gtk_label_set_markup_with_mnemonic( GTK_LABEL( lbl_handler_extract ),
                                        _("<b>_Extract:</b>") );
    gtk_misc_set_alignment( GTK_MISC( lbl_handler_extract ), 0, 0.5 );
    GtkWidget* lbl_handler_list = gtk_label_new( NULL );
    gtk_label_set_markup_with_mnemonic( GTK_LABEL( lbl_handler_list ),
                                        _("<b>_List:</b>") );
    gtk_misc_set_alignment( GTK_MISC( lbl_handler_list ), 0, 0.5 );
    GtkWidget* entry_handler_name = gtk_entry_new();
    g_object_set_data( G_OBJECT( dlg ), "entry_handler_name",
                        GTK_ENTRY( entry_handler_name ) );
    GtkWidget* entry_handler_mime = gtk_entry_new();
    g_object_set_data( G_OBJECT( dlg ), "entry_handler_mime",
                        GTK_ENTRY( entry_handler_mime ) );
    GtkWidget* entry_handler_extension = gtk_entry_new();
    g_object_set_data( G_OBJECT( dlg ), "entry_handler_extension",
                        GTK_ENTRY( entry_handler_extension ) );
    GtkWidget* entry_handler_compress = gtk_entry_new();
    g_object_set_data( G_OBJECT( dlg ), "entry_handler_compress",
                        GTK_ENTRY( entry_handler_compress ) );
    GtkWidget* entry_handler_extract = gtk_entry_new();
    g_object_set_data( G_OBJECT( dlg ), "entry_handler_extract",
                        GTK_ENTRY( entry_handler_extract ) );
    GtkWidget* entry_handler_list = gtk_entry_new();
    g_object_set_data( G_OBJECT( dlg ), "entry_handler_list",
                        GTK_ENTRY( entry_handler_list ) );

    // Setting widgets to be activated associated with label mnemonics
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl_handler_name ),
                                   entry_handler_name );
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl_handler_mime ),
                                   entry_handler_mime );
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl_handler_extension ),
                                   entry_handler_extension );
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl_handler_compress ),
                                   entry_handler_compress );
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl_handler_extract ),
                                   entry_handler_extract );
    gtk_label_set_mnemonic_widget( GTK_LABEL( lbl_handler_list ),
                                   entry_handler_list );

    GtkWidget* chkbtn_handler_compress_term =
                gtk_check_button_new_with_label( _("Run In Terminal") );
    g_object_set_data( G_OBJECT( dlg ), "chkbtn_handler_compress_term",
                        GTK_CHECK_BUTTON( chkbtn_handler_compress_term ) );
    GtkWidget* chkbtn_handler_extract_term =
                gtk_check_button_new_with_label( _("Run In Terminal") );
    g_object_set_data( G_OBJECT( dlg ), "chkbtn_handler_extract_term",
                        GTK_CHECK_BUTTON( chkbtn_handler_extract_term ) );
    GtkWidget* chkbtn_handler_list_term =
                gtk_check_button_new_with_label( _("Run In Terminal") );
    g_object_set_data( G_OBJECT( dlg ), "chkbtn_handler_list_term",
                        GTK_CHECK_BUTTON( chkbtn_handler_list_term ) );

    // Creating container boxes - at this point the dialog already comes
    // with one GtkVBox then inside that a GtkHButtonBox
    // For the right side of the dialog, standard GtkBox approach fails
    // to allow precise padding of labels to allow all entries to line up
    // - so reimplementing with GtkTable. Would many GtkAlignments have
    // worked?
    GtkWidget* hbox_main = gtk_hbox_new( FALSE, 4 );
    GtkWidget* vbox_handlers = gtk_vbox_new( FALSE, 4 );
    GtkWidget* hbox_view_buttons = gtk_hbox_new( FALSE, 4 );
    GtkWidget* tbl_settings = gtk_table_new( 11, 3 , FALSE );

    // Packing widgets into boxes
    // Remember, start and end-ness is broken
    // vbox_handlers packing must not expand so that the right side can
    // take the space
    gtk_box_pack_start( GTK_BOX( hbox_main ),
                        GTK_WIDGET( vbox_handlers ), FALSE, FALSE, 4 );
    gtk_box_pack_start( GTK_BOX( hbox_main ),
                       GTK_WIDGET( tbl_settings ), TRUE, TRUE, 4 );
    gtk_box_pack_start( GTK_BOX( vbox_handlers ),
                        GTK_WIDGET( lbl_handlers ), FALSE, FALSE, 4 );

    // view_handlers isn't added but view_scroll is - view_handlers is
    // inside view_scroll. No padding added to get it to align with the
    // enabled widget on the right side
    gtk_box_pack_start( GTK_BOX( vbox_handlers ),
                        GTK_WIDGET( view_scroll ), TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox_handlers ),
                        GTK_WIDGET( hbox_view_buttons ), FALSE, FALSE, 4 );
    gtk_box_pack_start( GTK_BOX( hbox_view_buttons ),
                        GTK_WIDGET( btn_remove ), TRUE, TRUE, 4 );
    gtk_box_pack_start( GTK_BOX( hbox_view_buttons ),
                        GTK_WIDGET( gtk_vseparator_new() ), TRUE, TRUE, 4 );
    gtk_box_pack_start( GTK_BOX( hbox_view_buttons ),
                        GTK_WIDGET( btn_add ), TRUE, TRUE, 4 );
    gtk_box_pack_start( GTK_BOX( hbox_view_buttons ),
                        GTK_WIDGET( btn_apply ), TRUE, TRUE, 4 );

    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( chkbtn_handler_enabled ), 0, 1, 1, 2,
                        GTK_FILL, GTK_FILL, 0, 0 );

    gtk_table_set_row_spacing( GTK_TABLE( tbl_settings ), 1, 5 );

    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( lbl_handler_name ), 0, 1, 2, 3,
                        GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( entry_handler_name ), 1, 4, 2, 3,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( lbl_handler_mime ), 0, 1, 3, 4,
                        GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( entry_handler_mime ), 1, 4, 3, 4,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( lbl_handler_extension ), 0, 1, 4, 5,
                        GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( entry_handler_extension ), 1, 4, 4, 5,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );

    gtk_table_set_row_spacing( GTK_TABLE( tbl_settings ), 5, 10 );

    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( lbl_handler_compress ), 0, 1, 6, 7,
                        GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( chkbtn_handler_compress_term ), 1, 4, 6, 7,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( entry_handler_compress ), 0, 4, 7, 8,
                        GTK_FILL, GTK_FILL, 0, 0 );

    gtk_table_set_row_spacing( GTK_TABLE( tbl_settings ), 7, 5 );

    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( lbl_handler_extract ), 0, 1, 8, 9,
                        GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( chkbtn_handler_extract_term ), 1, 4, 8, 9,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( entry_handler_extract ), 0, 4, 9, 10,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );

    gtk_table_set_row_spacing( GTK_TABLE( tbl_settings ), 9, 5 );

    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( lbl_handler_list ), 0, 1, 10, 11,
                        GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( chkbtn_handler_list_term ), 1, 4, 10, 11,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );
    gtk_table_attach( GTK_TABLE( tbl_settings ),
                        GTK_WIDGET( entry_handler_list ), 0, 4, 11, 12,
                        GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0 );

    // Packing boxes into dialog with padding to separate from dialog's
    // standard buttons at the bottom
    gtk_box_pack_start(
                GTK_BOX(
                    gtk_dialog_get_content_area( GTK_DIALOG( dlg ) )
                ),
                GTK_WIDGET( hbox_main ), TRUE, TRUE, 4 );

    // Adding archive handlers to list
    populate_archive_handlers( GTK_LIST_STORE( list ), GTK_WIDGET( dlg ) );

    // Rendering dialog - while loop is used to deal with standard
    // buttons that should not cause the dialog to exit
    gtk_widget_show_all( GTK_WIDGET( dlg ) );
    int response;
    while ( response = gtk_dialog_run( GTK_DIALOG( dlg ) ) )
    {
        if ( response == GTK_RESPONSE_OK )
        {
            break;
        }
        else if ( response == GTK_RESPONSE_HELP )
        {
            // TODO: Sort out proper help
            xset_show_help( dlg, NULL, "#designmode-style-context" );
        }
        else if ( response == GTK_RESPONSE_NONE )
        {
            // Restore defaults requested
            restore_defaults( GTK_WIDGET( dlg ) );
        }
        else
            break;
    }

    // Fetching dialog dimensions
    GtkAllocation allocation;
    gtk_widget_get_allocation ( GTK_WIDGET( dlg ), &allocation );
    width = allocation.width;
    height = allocation.height;

    // Checking if they are valid
    if ( width && height )
    {
        // They are - saving
        char* str = g_strdup_printf( "%d", width );
        xset_set( "arc_conf2", "x", str );
        g_free( str );
        str = g_strdup_printf( "%d", height );
        xset_set( "arc_conf2", "y", str );
        g_free( str );
    }

    // Clearing up dialog
    gtk_widget_destroy( dlg );
}
