#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <mouse_jiggler_icons.h>

const char VERSION[] = "1.0";

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    union {
        InputEvent input;
    };
    EventType type;
} UsbMouseEvent;

static void mouse_jiggler_render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);

    canvas_draw_icon(canvas, 0, 0, &I_mouse_jiggler);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 14, 9, "Mouse Jiggler v");
    canvas_draw_str(canvas, 94, 9, VERSION);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 33, "GitHub.com/DavidBerdik/");
    canvas_draw_str(canvas, 0, 43, "flipper-mouse-jiggler");
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

int32_t mouse_jiggler_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(UsbMouseEvent));
    furi_check(event_queue);

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, mouse_jiggler_render_callback, NULL);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    UsbMouseEvent event;

    while(1) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, FuriWaitForever);
        
        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeInput) {
                if((event.input.type == InputTypeLong) && (event.input.key == InputKeyBack)) {
                    break;
                }
            }
        }

        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    return 0;
}