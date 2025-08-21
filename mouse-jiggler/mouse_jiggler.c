#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <mouse_jiggler_icons.h>

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
    canvas_draw_str(canvas, 14, 10, "USB Mouse Jiggler");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

static void mouse_jiggler_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;

    UsbMouseEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}
static void mouse_jiggler_jiggle(void* ctx) {
	UNUSED(ctx);
	
    static short horizontal_travel_dist = 0;
	static short horizontal_movement_cycles = 0;
	static short horizontal_current_cycle = 0;
    static short vertical_travel_dist = 0;
	static short vertical_movement_cycles = 0;
	static short vertical_current_cycle = 0;

    if(horizontal_current_cycle >= horizontal_movement_cycles) {
        horizontal_travel_dist = furi_hal_random_get() % 3 - 1;
        horizontal_movement_cycles = furi_hal_random_get() % 1000 + 1;
        horizontal_current_cycle = 0;
    }
	
	if(vertical_current_cycle >= vertical_movement_cycles) {
        vertical_travel_dist = furi_hal_random_get() % 3 - 1;
        vertical_movement_cycles = furi_hal_random_get() % 1000 + 1;
        vertical_current_cycle = 0;
    }

    furi_hal_hid_mouse_move(horizontal_travel_dist, vertical_travel_dist);
    horizontal_current_cycle++;
    vertical_current_cycle++;
}
int32_t mouse_jiggler_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(UsbMouseEvent));
    furi_check(event_queue);

    ViewPort* view_port = view_port_alloc();

    FuriTimer* timer = furi_timer_alloc(mouse_jiggler_jiggle, FuriTimerTypePeriodic, event_queue);

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);

    view_port_draw_callback_set(view_port, mouse_jiggler_render_callback, NULL);
    view_port_input_callback_set(view_port, mouse_jiggler_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_timer_start(timer, 3);

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

    furi_hal_usb_set_config(usb_mode_prev, NULL);

    // remove & free all stuff created by app
    furi_timer_free(timer);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    return 0;
}