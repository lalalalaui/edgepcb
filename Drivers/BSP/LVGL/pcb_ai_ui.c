#include "./BSP/LVGL/pcb_ai_ui.h"

#include "lvgl.h"
#include <stdio.h>
#include <string.h>

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
    lv_obj_t *canvas_input;
    lv_obj_t *canvas_heatmap;
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

#define PATCH_SIZE       96
#define PATCH_CHANNELS   3
#define CANVAS_SCALE     2
#define CANVAS_SIZE      (PATCH_SIZE * CANVAS_SCALE)
#define CANVAS_PIXELS    (CANVAS_SIZE * CANVAS_SIZE)

#if defined(__GNUC__)
#define SDRAM_AI_ATTR __attribute__((section(".sdram_ai"), used, aligned(32)))
#else
#define SDRAM_AI_ATTR
#endif

static uint16_t s_input_canvas_buf[CANVAS_PIXELS] SDRAM_AI_ATTR;
static uint16_t s_heatmap_canvas_buf[CANVAS_PIXELS] SDRAM_AI_ATTR;

static float clamp01(float v)
{
    if (v < 0.0f) {
        return 0.0f;
    }
    if (v > 1.0f) {
        return 1.0f;
    }
    return v;
}

static uint8_t float_to_u8(float v)
{
    v = clamp01(v);
    return (uint8_t)(v * 255.0f + 0.5f);
}

static uint16_t rgb565_from_u8(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)((uint16_t)(r & 0xF8U) << 8)
         | (uint16_t)((uint16_t)(g & 0xFCU) << 3)
         | (uint16_t)(b >> 3);
}

static void put_scaled_pixel(uint16_t *buf, int32_t x, int32_t y, uint16_t color)
{
    const int32_t dx = x * CANVAS_SCALE;
    const int32_t dy = y * CANVAS_SCALE;
    const int32_t row0 = dy * CANVAS_SIZE + dx;
    const int32_t row1 = row0 + CANVAS_SIZE;

    buf[row0] = color;
    buf[row0 + 1] = color;
    buf[row1] = color;
    buf[row1 + 1] = color;
}

static uint16_t heatmap_color(float t)
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    t = clamp01(t);
    if (t < 0.5f) {
        const float k = t * 2.0f;
        r = (uint8_t)(255.0f * k + 0.5f);
        g = (uint8_t)(210.0f * k + 0.5f);
        b = (uint8_t)(90.0f * (1.0f - k) + 0.5f);
    } else {
        const float k = (t - 0.5f) * 2.0f;
        r = 255U;
        g = (uint8_t)(210.0f * (1.0f - k) + 0.5f);
        b = 0U;
    }

    return rgb565_from_u8(r, g, b);
}

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
    make_label(parent, name, 12, y, &lv_font_montserrat_14, C_MUTED);
    lv_obj_t *label = make_label(parent, value, 136, y, &lv_font_montserrat_14, C_TEXT);
    lv_obj_set_width(label, 140);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    return label;
}

static lv_obj_t *make_canvas_panel(lv_obj_t *parent,
                                   int32_t x,
                                   int32_t y,
                                   const char *title,
                                   uint16_t *buf)
{
    lv_obj_t *panel = make_panel(parent, x, y, 214, 232);
    make_label(panel, title, 11, 10, &lv_font_montserrat_14, C_MUTED);

    lv_obj_t *canvas = lv_canvas_create(panel);
    lv_obj_set_pos(canvas, 11, 34);
    lv_obj_set_size(canvas, CANVAS_SIZE, CANVAS_SIZE);
    lv_canvas_set_buffer(canvas, buf, CANVAS_SIZE, CANVAS_SIZE, LV_COLOR_FORMAT_RGB565);
    memset(buf, 0, sizeof(uint16_t) * CANVAS_PIXELS);
    lv_obj_invalidate(canvas);
    return canvas;
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

    g_ui.canvas_input = make_canvas_panel(g_ui.root, 18, 58, "Input patch", s_input_canvas_buf);
    g_ui.canvas_heatmap = make_canvas_panel(g_ui.root, sw - 248, 58, "Reconstruction error", s_heatmap_canvas_buf);

    lv_obj_t *main_panel = make_panel(g_ui.root, 244, 58, sw - 510, 292);
    g_ui.label_sample = make_row(main_panel, 18, "Sample", "--/--");
    g_ui.label_expected = make_row(main_panel, 48, "Expected label", "--");
    g_ui.label_score = make_row(main_panel, 78, "Score", "--");
    g_ui.label_threshold_low = make_row(main_panel, 108, "Threshold low", "--");
    g_ui.label_threshold_high = make_row(main_panel, 138, "Threshold high", "--");
    g_ui.label_result = make_row(main_panel, 168, "Result", "--");
    g_ui.label_latency = make_row(main_panel, 198, "Latency", "--");
    g_ui.label_compare = make_row(main_panel, 228, "Compare", "--");

    int32_t btn_y = sh - 88;
    g_ui.btn_run = make_button(g_ui.root, "Run", 244, btn_y, 112, 42, run_event_cb);
    g_ui.btn_next = make_button(g_ui.root, "Next", 374, btn_y, 112, 42, next_event_cb);
}

void pcb_ai_ui_set_status(const char *text)
{
    if (g_ui.label_status != NULL) {
        lv_label_set_text(g_ui.label_status, (text != NULL) ? text : "");
    }
}

void pcb_ai_ui_show_input_patch(const float *input)
{
    const int32_t plane = PATCH_SIZE * PATCH_SIZE;

    if ((g_ui.canvas_input == NULL) || (input == NULL)) {
        return;
    }

    for (int32_t y = 0; y < PATCH_SIZE; y++) {
        for (int32_t x = 0; x < PATCH_SIZE; x++) {
            const int32_t idx = y * PATCH_SIZE + x;
            const uint8_t r = float_to_u8(input[idx]);
            const uint8_t g = float_to_u8(input[plane + idx]);
            const uint8_t b = float_to_u8(input[(2 * plane) + idx]);
            put_scaled_pixel(s_input_canvas_buf, x, y, rgb565_from_u8(r, g, b));
        }
    }

    lv_obj_invalidate(g_ui.canvas_input);
}

void pcb_ai_ui_show_heatmap(const float *input, const float *output)
{
    const int32_t plane = PATCH_SIZE * PATCH_SIZE;
    float max_err = 0.0f;

    if ((g_ui.canvas_heatmap == NULL) || (input == NULL) || (output == NULL)) {
        return;
    }

    for (int32_t i = 0; i < plane; i++) {
        const float dr = input[i] - output[i];
        const float dg = input[plane + i] - output[plane + i];
        const float db = input[(2 * plane) + i] - output[(2 * plane) + i];
        const float err = ((dr * dr) + (dg * dg) + (db * db)) / 3.0f;
        if (err > max_err) {
            max_err = err;
        }
    }

    if (max_err < 1.0e-12f) {
        max_err = 1.0e-12f;
    }

    for (int32_t y = 0; y < PATCH_SIZE; y++) {
        for (int32_t x = 0; x < PATCH_SIZE; x++) {
            const int32_t idx = y * PATCH_SIZE + x;
            const float dr = input[idx] - output[idx];
            const float dg = input[plane + idx] - output[plane + idx];
            const float db = input[(2 * plane) + idx] - output[(2 * plane) + idx];
            const float err = ((dr * dr) + (dg * dg) + (db * db)) / 3.0f;
            put_scaled_pixel(s_heatmap_canvas_buf, x, y, heatmap_color(err / max_err));
        }
    }

    lv_obj_invalidate(g_ui.canvas_heatmap);
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
