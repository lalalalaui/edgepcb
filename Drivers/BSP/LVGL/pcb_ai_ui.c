#include "./BSP/LVGL/pcb_ai_ui.h"

#include "lvgl.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    lv_obj_t *root;
    lv_obj_t *label_title;
    lv_obj_t *label_status;
    lv_obj_t *panel_input;
    lv_obj_t *panel_heatmap;
    lv_obj_t *panel_main;
    lv_obj_t *label_input_title;
    lv_obj_t *label_heatmap_title;
    lv_obj_t *label_name_sample;
    lv_obj_t *label_name_expected;
    lv_obj_t *label_name_score;
    lv_obj_t *label_name_threshold_low;
    lv_obj_t *label_name_threshold_high;
    lv_obj_t *label_name_result;
    lv_obj_t *label_name_latency;
    lv_obj_t *label_name_compare;
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
    lv_obj_t *btn_anomaly;
    lv_obj_t *btn_classifier;
    lv_obj_t *btn_run;
    lv_obj_t *btn_next;
    int32_t canvas_scale;
    int32_t canvas_size;
    int mode;
} pcb_ai_ui_t;

static pcb_ai_ui_t g_ui;
static void (*g_run_cb)(void) = NULL;
static void (*g_next_cb)(void) = NULL;
static void (*g_anomaly_cb)(void) = NULL;
static void (*g_classifier_cb)(void) = NULL;

#define C_BG       lv_color_hex(0x101820)
#define C_PANEL    lv_color_hex(0x182430)
#define C_LINE     lv_color_hex(0x2E4050)
#define C_TEXT     lv_color_hex(0xF4F7FA)
#define C_MUTED    lv_color_hex(0x9DAAB5)
#define C_ACCENT   lv_color_hex(0x41B8D5)
#define C_INACTIVE lv_color_hex(0x2E4050)
#define C_NORMAL   lv_color_hex(0x2ECC71)
#define C_SUSPECT  lv_color_hex(0xF2A93B)
#define C_ANOMALY  lv_color_hex(0xE96666)

#define PATCH_SIZE       96
#define PATCH_CHANNELS   3
#define CANVAS_SCALE_MAX 2
#define CANVAS_SIZE_MAX  (PATCH_SIZE * CANVAS_SCALE_MAX)
#define CANVAS_PIXELS    (CANVAS_SIZE_MAX * CANVAS_SIZE_MAX)

#if defined(__GNUC__)
#define SDRAM_AI_ATTR __attribute__((section(".sdram_ai"), used, aligned(32)))
#else
#define SDRAM_AI_ATTR
#endif

static uint16_t s_input_canvas_buf[CANVAS_PIXELS] SDRAM_AI_ATTR;
static uint16_t s_heatmap_canvas_buf[CANVAS_PIXELS] SDRAM_AI_ATTR;

static const float s_imagenet_mean[3] = {0.485f, 0.456f, 0.406f};
static const float s_imagenet_std[3] = {0.229f, 0.224f, 0.225f};

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
    const int32_t scale = g_ui.canvas_scale;
    const int32_t size = g_ui.canvas_size;
    const int32_t dx = x * scale;
    const int32_t dy = y * scale;

    if (scale <= 1) {
        buf[dy * size + dx] = color;
    } else {
        const int32_t row0 = dy * size + dx;
        const int32_t row1 = row0 + size;
        buf[row0] = color;
        buf[row0 + 1] = color;
        buf[row1] = color;
        buf[row1 + 1] = color;
    }
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

static lv_obj_t *make_row_ex(lv_obj_t *parent,
                             int32_t y,
                             const char *name,
                             const char *value,
                             lv_obj_t **name_out)
{
    lv_obj_t *name_label = make_label(parent, name, 12, y, &lv_font_montserrat_14, C_MUTED);
    lv_obj_t *label = make_label(parent, value, 136, y, &lv_font_montserrat_14, C_TEXT);
    lv_obj_set_width(label, 140);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    if (name_out != NULL) {
        *name_out = name_label;
    }
    return label;
}

static lv_obj_t *make_canvas_panel_ex(lv_obj_t *parent,
                                      int32_t x,
                                      int32_t y,
                                      const char *title,
                                      uint16_t *buf,
                                      lv_obj_t **panel_out,
                                      lv_obj_t **title_out)
{
    const int32_t panel_w = g_ui.canvas_size + 22;
    const int32_t panel_h = g_ui.canvas_size + 40;
    printf("UI canvas %s panel\r\n", title);
    lv_obj_t *panel = make_panel(parent, x, y, panel_w, panel_h);
    printf("UI canvas %s title\r\n", title);
    lv_obj_t *title_label = make_label(panel, title, 11, 10, &lv_font_montserrat_14, C_MUTED);
    printf("UI canvas %s create\r\n", title);
    lv_obj_t *canvas = lv_canvas_create(panel);
    printf("UI canvas %s set pos\r\n", title);

    lv_obj_set_pos(canvas, 11, 34);
    lv_obj_set_size(canvas, g_ui.canvas_size, g_ui.canvas_size);
    printf("UI canvas %s set buffer\r\n", title);
    lv_canvas_set_buffer(canvas, buf, g_ui.canvas_size, g_ui.canvas_size, LV_COLOR_FORMAT_RGB565);
    printf("UI canvas %s clear\r\n", title);
    memset(buf, 0, sizeof(uint16_t) * CANVAS_PIXELS);
    lv_obj_invalidate(canvas);
    printf("UI canvas %s done\r\n", title);

    if (panel_out != NULL) {
        *panel_out = panel;
    }
    if (title_out != NULL) {
        *title_out = title_label;
    }
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

static void anomaly_event_cb(lv_event_t *e)
{
    (void)e;
    if (g_anomaly_cb != NULL) {
        g_anomaly_cb();
    }
}

static void classifier_event_cb(lv_event_t *e)
{
    (void)e;
    if (g_classifier_cb != NULL) {
        g_classifier_cb();
    }
}

static void clear_value_labels(void)
{
    if (g_ui.label_sample != NULL) {
        lv_label_set_text(g_ui.label_sample, "--/--");
        lv_label_set_text(g_ui.label_expected, "--");
        lv_label_set_text(g_ui.label_score, "--");
        lv_label_set_text(g_ui.label_threshold_low, "--");
        lv_label_set_text(g_ui.label_threshold_high, "--");
        lv_label_set_text(g_ui.label_result, "--");
        lv_label_set_text(g_ui.label_latency, "--");
        lv_label_set_text(g_ui.label_compare, "--");
        lv_obj_set_style_text_color(g_ui.label_result, C_TEXT, 0);
    }
}

static void update_mode_button_styles(void)
{
    if (g_ui.btn_anomaly != NULL) {
        lv_obj_set_style_bg_color(g_ui.btn_anomaly,
                                  (g_ui.mode == PCB_AI_UI_MODE_ANOMALY) ? C_ACCENT : C_INACTIVE,
                                  0);
    }
    if (g_ui.btn_classifier != NULL) {
        lv_obj_set_style_bg_color(g_ui.btn_classifier,
                                  (g_ui.mode == PCB_AI_UI_MODE_CLASSIFIER) ? C_ACCENT : C_INACTIVE,
                                  0);
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

void pcb_ai_ui_set_anomaly_callback(void (*cb)(void))
{
    g_anomaly_cb = cb;
}

void pcb_ai_ui_set_classifier_callback(void (*cb)(void))
{
    g_classifier_cb = cb;
}

void pcb_ai_ui_create(void)
{
    if (g_ui.root != NULL) {
        return;
    }

    lv_obj_t *screen = lv_screen_active();
    if (screen == NULL) {
        printf("UI error: no active LVGL screen\r\n");
        return;
    }

    int32_t sw = lv_display_get_horizontal_resolution(NULL);
    int32_t sh = lv_display_get_vertical_resolution(NULL);
    if ((sw <= 0) || (sh <= 0)) {
        printf("UI error: invalid LVGL res %ldx%ld\r\n", (long)sw, (long)sh);
        return;
    }

    const int compact = ((sw < 700) || (sh < 430)) ? 1 : 0;
    g_ui.canvas_scale = compact ? 1 : 2;
    g_ui.canvas_size = PATCH_SIZE * g_ui.canvas_scale;

    const int32_t root_margin = compact ? 4 : 10;
    const int32_t header_y = compact ? 8 : 12;
    const int32_t panel_y = compact ? 50 : 58;
    const int32_t panel_w = g_ui.canvas_size + 22;
    const int32_t input_x = compact ? 8 : 18;
    const int32_t heatmap_x = sw - panel_w - (compact ? 8 : 28);
    const int32_t main_x = input_x + panel_w + (compact ? 10 : 12);
    int32_t main_w = heatmap_x - main_x - (compact ? 8 : 12);
    if (main_w < 180) {
        main_w = sw - main_x - root_margin - 4;
    }
    if (main_w < 120) {
        main_w = 120;
    }
    const int32_t main_h = compact ? 232 : 292;
    const int32_t btn_y = sh - (compact ? 52 : 88);

    printf("UI res=%ldx%ld compact=%d canvas=%ld\r\n",
           (long)sw, (long)sh, compact, (long)g_ui.canvas_size);

    lv_obj_set_style_bg_color(screen, C_BG, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    g_ui.mode = PCB_AI_UI_MODE_ANOMALY;
    printf("UI step root\r\n");
    g_ui.root = make_panel(screen, root_margin, root_margin, sw - (root_margin * 2), sh - (root_margin * 2));
    printf("UI step header\r\n");
    g_ui.label_title = make_label(g_ui.root, compact ? "PCB EdgeAI" : "Anomaly Detection",
                                  12, compact ? 10 : 14, &lv_font_montserrat_18, C_TEXT);
    g_ui.btn_anomaly = make_button(g_ui.root, "Anomaly", compact ? 132 : 244,
                                   header_y, compact ? 84 : 104, compact ? 30 : 34, anomaly_event_cb);
    g_ui.btn_classifier = make_button(g_ui.root, "Classifier", compact ? 224 : 360,
                                      header_y, compact ? 96 : 116, compact ? 30 : 34, classifier_event_cb);
    g_ui.label_status = make_label(g_ui.root, "Starting", compact ? 330 : (sw - 250),
                                   compact ? 14 : 18, &lv_font_montserrat_14, C_MUTED);
    lv_obj_set_width(g_ui.label_status, compact ? 130 : 220);
    lv_label_set_long_mode(g_ui.label_status, LV_LABEL_LONG_CLIP);

    printf("UI step input canvas\r\n");
    g_ui.canvas_input = make_canvas_panel_ex(g_ui.root, input_x, panel_y, "Input patch", s_input_canvas_buf,
                                             &g_ui.panel_input, &g_ui.label_input_title);
    printf("UI step heatmap canvas\r\n");
    g_ui.canvas_heatmap = make_canvas_panel_ex(g_ui.root, heatmap_x, panel_y,
                                               compact ? "Error" : "Reconstruction error", s_heatmap_canvas_buf,
                                               &g_ui.panel_heatmap, &g_ui.label_heatmap_title);

    printf("UI step result panel\r\n");
    g_ui.panel_main = make_panel(g_ui.root, main_x, panel_y, main_w, main_h);
    g_ui.label_sample = make_row_ex(g_ui.panel_main, 18, "Sample", "--/--", &g_ui.label_name_sample);
    g_ui.label_expected = make_row_ex(g_ui.panel_main, 48, "Expected label", "--", &g_ui.label_name_expected);
    g_ui.label_score = make_row_ex(g_ui.panel_main, 78, "Score", "--", &g_ui.label_name_score);
    g_ui.label_threshold_low = make_row_ex(g_ui.panel_main, 108, "Threshold low", "--", &g_ui.label_name_threshold_low);
    g_ui.label_threshold_high = make_row_ex(g_ui.panel_main, 138, "Threshold high", "--", &g_ui.label_name_threshold_high);
    g_ui.label_result = make_row_ex(g_ui.panel_main, 168, "Result", "--", &g_ui.label_name_result);
    g_ui.label_latency = make_row_ex(g_ui.panel_main, 198, "Latency", "--", &g_ui.label_name_latency);
    g_ui.label_compare = make_row_ex(g_ui.panel_main, 228, "Compare", "--", &g_ui.label_name_compare);

    printf("UI step buttons\r\n");
    g_ui.btn_run = make_button(g_ui.root, "Run", main_x, btn_y, compact ? 84 : 112, compact ? 34 : 42, run_event_cb);
    g_ui.btn_next = make_button(g_ui.root, "Next", main_x + (compact ? 94 : 130), btn_y,
                                compact ? 84 : 112, compact ? 34 : 42, next_event_cb);
    update_mode_button_styles();
    printf("UI step done\r\n");
}

void pcb_ai_ui_set_mode(int mode)
{
    if (g_ui.root == NULL) {
        return;
    }

    g_ui.mode = mode;
    if (mode == PCB_AI_UI_MODE_CLASSIFIER) {
        lv_label_set_text(g_ui.label_title, "Defect Classification");
        lv_label_set_text(g_ui.label_name_sample, "Sample");
        lv_label_set_text(g_ui.label_name_expected, "Expected label");
        lv_label_set_text(g_ui.label_name_score, "Predicted");
        lv_label_set_text(g_ui.label_name_threshold_low, "Top1 prob");
        lv_label_set_text(g_ui.label_name_threshold_high, "Top3 #1");
        lv_label_set_text(g_ui.label_name_result, "Top3 #2");
        lv_label_set_text(g_ui.label_name_latency, "Top3 #3");
        lv_label_set_text(g_ui.label_name_compare, "Latency");
        if (g_ui.panel_heatmap != NULL) {
            lv_obj_add_flag(g_ui.panel_heatmap, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        lv_label_set_text(g_ui.label_title, "Anomaly Detection");
        lv_label_set_text(g_ui.label_name_sample, "Sample");
        lv_label_set_text(g_ui.label_name_expected, "Expected label");
        lv_label_set_text(g_ui.label_name_score, "Score");
        lv_label_set_text(g_ui.label_name_threshold_low, "Threshold low");
        lv_label_set_text(g_ui.label_name_threshold_high, "Threshold high");
        lv_label_set_text(g_ui.label_name_result, "Result");
        lv_label_set_text(g_ui.label_name_latency, "Latency");
        lv_label_set_text(g_ui.label_name_compare, "Compare");
        if (g_ui.panel_heatmap != NULL) {
            lv_obj_clear_flag(g_ui.panel_heatmap, LV_OBJ_FLAG_HIDDEN);
        }
    }

    clear_value_labels();
    pcb_ai_ui_clear_heatmap();
    update_mode_button_styles();
}

void pcb_ai_ui_set_status(const char *text)
{
    if (g_ui.label_status != NULL) {
        lv_label_set_text(g_ui.label_status, (text != NULL) ? text : "");
    }
}

void pcb_ai_ui_update_sample_info(int sample_index, int sample_count, int expected_label)
{
    char buf[48];

    if (g_ui.root == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "%d / %d", sample_index, sample_count);
    lv_label_set_text(g_ui.label_sample, buf);

    snprintf(buf, sizeof(buf), "%d", expected_label);
    lv_label_set_text(g_ui.label_expected, buf);

    lv_label_set_text(g_ui.label_score, "--");
    lv_label_set_text(g_ui.label_threshold_low, "--");
    lv_label_set_text(g_ui.label_threshold_high, "--");
    lv_label_set_text(g_ui.label_result, "--");
    lv_label_set_text(g_ui.label_latency, "--");
    lv_label_set_text(g_ui.label_compare, "--");
    lv_obj_set_style_text_color(g_ui.label_result, C_TEXT, 0);
}

void pcb_ai_ui_show_input_patch(const float *input)
{
    const int32_t plane = PATCH_SIZE * PATCH_SIZE;
    int normalized_input = 0;

    if ((g_ui.canvas_input == NULL) || (input == NULL)) {
        return;
    }

    for (int32_t i = 0; i < PATCH_CHANNELS * plane; i += 257) {
        if ((input[i] < -0.01f) || (input[i] > 1.01f)) {
            normalized_input = 1;
            break;
        }
    }

    for (int32_t y = 0; y < PATCH_SIZE; y++) {
        for (int32_t x = 0; x < PATCH_SIZE; x++) {
            const int32_t idx = y * PATCH_SIZE + x;
            float r = input[idx];
            float g = input[plane + idx];
            float b = input[(2 * plane) + idx];
            if (normalized_input) {
                r = r * s_imagenet_std[0] + s_imagenet_mean[0];
                g = g * s_imagenet_std[1] + s_imagenet_mean[1];
                b = b * s_imagenet_std[2] + s_imagenet_mean[2];
            }
            put_scaled_pixel(s_input_canvas_buf, x, y,
                             rgb565_from_u8(float_to_u8(r), float_to_u8(g), float_to_u8(b)));
        }
    }

    lv_obj_invalidate(g_ui.canvas_input);
}

void pcb_ai_ui_clear_heatmap(void)
{
    if (g_ui.canvas_heatmap != NULL) {
        memset(s_heatmap_canvas_buf, 0, sizeof(s_heatmap_canvas_buf));
        lv_obj_invalidate(g_ui.canvas_heatmap);
    }
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

void pcb_ai_ui_update_classifier_result(int sample_index,
                                        int sample_count,
                                        int expected_label,
                                        const char *predicted_class,
                                        float top1_prob,
                                        const char *top3_line0,
                                        const char *top3_line1,
                                        const char *top3_line2,
                                        uint32_t latency_ms)
{
    char buf[48];

    if (g_ui.root == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "%d / %d", sample_index, sample_count);
    lv_label_set_text(g_ui.label_sample, buf);

    snprintf(buf, sizeof(buf), "%d", expected_label);
    lv_label_set_text(g_ui.label_expected, buf);

    lv_label_set_text(g_ui.label_score, (predicted_class != NULL) ? predicted_class : "--");

    snprintf(buf, sizeof(buf), "%.6f", (double)top1_prob);
    lv_label_set_text(g_ui.label_threshold_low, buf);

    lv_label_set_text(g_ui.label_threshold_high, (top3_line0 != NULL) ? top3_line0 : "--");
    lv_label_set_text(g_ui.label_result, (top3_line1 != NULL) ? top3_line1 : "--");
    lv_obj_set_style_text_color(g_ui.label_result, C_TEXT, 0);
    lv_label_set_text(g_ui.label_latency, (top3_line2 != NULL) ? top3_line2 : "--");

    snprintf(buf, sizeof(buf), "%lums", (unsigned long)latency_ms);
    lv_label_set_text(g_ui.label_compare, buf);
}
