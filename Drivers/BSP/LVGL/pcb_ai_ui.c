#include "./BSP/LVGL/pcb_ai_ui.h"

#include "lvgl.h"
#include <stdio.h>

typedef struct {
    lv_obj_t *root;
    lv_obj_t *label_status;
    lv_obj_t *label_sample;
    lv_obj_t *label_expected;
    lv_obj_t *label_score;
    lv_obj_t *label_threshold_low;
    lv_obj_t *label_threshold_high;
    lv_obj_t *label_result;
    lv_obj_t *label_latency;
    lv_obj_t *label_compare;
    lv_obj_t *btn_run;
    lv_obj_t *btn_next;
} pcb_ai_ui_t;

static pcb_ai_ui_t g_ui;
static void (*g_run_cb)(void) = NULL;
static void (*g_next_cb)(void) = NULL;

#define C_BG       lv_color_hex(0x101820)
#define C_PANEL    lv_color_hex(0x182430)
#define C_LINE     lv_color_hex(0x2E4050)
#define C_TEXT     lv_color_hex(0xF4F7FA)
#define C_MUTED    lv_color_hex(0x9DAAB5)
#define C_ACCENT   lv_color_hex(0x41B8D5)
#define C_NORMAL   lv_color_hex(0x2ECC71)
#define C_SUSPECT  lv_color_hex(0xF2A93B)
#define C_ANOMALY  lv_color_hex(0xE96666)

static lv_obj_t *make_panel(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, C_PANEL, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, C_LINE, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_radius(obj, 6, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static lv_obj_t *make_label(lv_obj_t *parent,
                            const char *text,
                            int32_t x,
                            int32_t y,
                            const lv_font_t *font,
                            lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, color, 0);
    if (font != NULL) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    return label;
}

static lv_obj_t *make_row(lv_obj_t *parent, int32_t y, const char *name, const char *value)
{
    make_label(parent, name, 16, y, &lv_font_montserrat_14, C_MUTED);
    lv_obj_t *label = make_label(parent, value, 170, y, &lv_font_montserrat_14, C_TEXT);
    lv_obj_set_width(label, 250);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    return label;
}

static lv_obj_t *make_button(lv_obj_t *parent,
                             const char *text,
                             int32_t x,
                             int32_t y,
                             int32_t w,
                             int32_t h,
                             lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_color(btn, C_ACCENT, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, C_TEXT, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);
    return btn;
}

static const char *result_name(int result_code)
{
    if (result_code == 0) {
        return "NORMAL";
    }
    if (result_code == 1) {
        return "SUSPECT";
    }
    return "ANOMALY";
}

static lv_color_t result_color(int result_code)
{
    if (result_code == 0) {
        return C_NORMAL;
    }
    if (result_code == 1) {
        return C_SUSPECT;
    }
    return C_ANOMALY;
}

static void run_event_cb(lv_event_t *e)
{
    (void)e;
    if (g_run_cb != NULL) {
        g_run_cb();
    }
}

static void next_event_cb(lv_event_t *e)
{
    (void)e;
    if (g_next_cb != NULL) {
        g_next_cb();
    }
}

void pcb_ai_ui_set_run_callback(void (*cb)(void))
{
    g_run_cb = cb;
}

void pcb_ai_ui_set_next_callback(void (*cb)(void))
{
    g_next_cb = cb;
}

void pcb_ai_ui_create(void)
{
    if (g_ui.root != NULL) {
        return;
    }

    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, C_BG, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    int32_t sw = lv_display_get_horizontal_resolution(NULL);
    int32_t sh = lv_display_get_vertical_resolution(NULL);

    g_ui.root = make_panel(screen, 10, 10, sw - 20, sh - 20);
    make_label(g_ui.root, "PCB Anomaly EdgeAI", 18, 14, &lv_font_montserrat_18, C_TEXT);
    g_ui.label_status = make_label(g_ui.root, "Starting", sw - 250, 18, &lv_font_montserrat_14, C_MUTED);
    lv_obj_set_width(g_ui.label_status, 220);
    lv_label_set_long_mode(g_ui.label_status, LV_LABEL_LONG_CLIP);

    lv_obj_t *main_panel = make_panel(g_ui.root, 18, 58, sw - 56, sh - 150);
    g_ui.label_sample = make_row(main_panel, 18, "Sample", "--/--");
    g_ui.label_expected = make_row(main_panel, 48, "Expected label", "--");
    g_ui.label_score = make_row(main_panel, 78, "Score", "--");
    g_ui.label_threshold_low = make_row(main_panel, 108, "Threshold low", "--");
    g_ui.label_threshold_high = make_row(main_panel, 138, "Threshold high", "--");
    g_ui.label_result = make_row(main_panel, 168, "Result", "--");
    g_ui.label_latency = make_row(main_panel, 198, "Latency", "--");
    g_ui.label_compare = make_row(main_panel, 228, "Compare", "--");

    int32_t btn_y = sh - 74;
    g_ui.btn_run = make_button(g_ui.root, "Run", 18, btn_y, 110, 42, run_event_cb);
    g_ui.btn_next = make_button(g_ui.root, "Next", 146, btn_y, 110, 42, next_event_cb);
}

void pcb_ai_ui_set_status(const char *text)
{
    if (g_ui.label_status != NULL) {
        lv_label_set_text(g_ui.label_status, (text != NULL) ? text : "");
    }
}

void pcb_ai_ui_update_result(float score,
                             float threshold_low,
                             float threshold_high,
                             int result_code,
                             uint32_t latency_ms,
                             int sample_index,
                             int sample_count,
                             int expected_label,
                             const char *compare_text)
{
    char buf[48];

    if (g_ui.root == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "%d / %d", sample_index, sample_count);
    lv_label_set_text(g_ui.label_sample, buf);

    snprintf(buf, sizeof(buf), "%d", expected_label);
    lv_label_set_text(g_ui.label_expected, buf);

    snprintf(buf, sizeof(buf), "%.9g", (double)score);
    lv_label_set_text(g_ui.label_score, buf);

    snprintf(buf, sizeof(buf), "%.9g", (double)threshold_low);
    lv_label_set_text(g_ui.label_threshold_low, buf);

    snprintf(buf, sizeof(buf), "%.9g", (double)threshold_high);
    lv_label_set_text(g_ui.label_threshold_high, buf);

    lv_label_set_text(g_ui.label_result, result_name(result_code));
    lv_obj_set_style_text_color(g_ui.label_result, result_color(result_code), 0);

    snprintf(buf, sizeof(buf), "%lums", (unsigned long)latency_ms);
    lv_label_set_text(g_ui.label_latency, buf);

    lv_label_set_text(g_ui.label_compare, (compare_text != NULL) ? compare_text : "--");
}
