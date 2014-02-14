#ifndef _ITEM_PROP_H_
#define _ITEM_PROP_H_

#include <settings.h>

G_BEGIN_DECLS

enum {
    CONTEXT_SHOW,
    CONTEXT_ENABLE,
    CONTEXT_HIDE,
    CONTEXT_DISABLE
};

enum {
    CONTEXT_MIME,
    CONTEXT_NAME,
    CONTEXT_DIR,
    CONTEXT_WRITE_ACCESS,
    CONTEXT_IS_TEXT,
    CONTEXT_IS_DIR,
    CONTEXT_IS_LINK,
    CONTEXT_IS_ROOT,
    CONTEXT_MUL_SEL,
    CONTEXT_CLIP_FILES,
    CONTEXT_CLIP_TEXT,
    CONTEXT_PANEL,
    CONTEXT_PANEL_COUNT,
    CONTEXT_TAB,
    CONTEXT_TAB_COUNT,
    CONTEXT_BOOKMARK,
    CONTEXT_DEVICE,
    CONTEXT_DEVICE_MOUNT_POINT,
    CONTEXT_DEVICE_LABEL,
    CONTEXT_DEVICE_FSTYPE,
    CONTEXT_DEVICE_UDI,
    CONTEXT_DEVICE_PROP,
    CONTEXT_TASK_COUNT,
    CONTEXT_TASK_DIR,
    CONTEXT_TASK_TYPE,
    CONTEXT_TASK_NAME,
    CONTEXT_PANEL1_DIR,
    CONTEXT_PANEL2_DIR,
    CONTEXT_PANEL3_DIR,
    CONTEXT_PANEL4_DIR,
    CONTEXT_PANEL1_SEL,
    CONTEXT_PANEL2_SEL,
    CONTEXT_PANEL3_SEL,
    CONTEXT_PANEL4_SEL,
    CONTEXT_PANEL1_DEVICE,
    CONTEXT_PANEL2_DEVICE,
    CONTEXT_PANEL3_DEVICE,
    CONTEXT_PANEL4_DEVICE,
    CONTEXT_END
};

char* get_text_view( GtkTextView* view );  // Exposed for use in ptk-handler.c
void load_text_view( GtkTextView* view, const char* line );  // Exposed for use in ptk-handler.c
void xset_item_prop_dlg( XSetContext* context, XSet* set, int page );
int xset_context_test( XSetContext* context, char* rules, gboolean def_disable );

G_END_DECLS

#endif

