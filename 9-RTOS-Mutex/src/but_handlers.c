#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"

static void power_btn_handler(lv_event_t *e) {}
static void m_btn_handler(lv_event_t *e) {}
static void clock_btn_handler(lv_event_t *e) {}


static void up_btn_handler(lv_event_t *e) {
	lv_event_code_t code = lv_event_get_code(e);
    char *c;
    int temp;
    if (code == LV_EVENT_CLICKED) {
        c = lv_label_get_text(label_set_value);
        temp = atoi(c);
        lv_label_set_text_fmt(label_set_value, "%02d", temp + 1);
    }
}

static void down_btn_handler(lv_event_t *e) {
	lv_event_code_t code = lv_event_get_code(e);
    char *c;
    int temp;
    if (code == LV_EVENT_CLICKED) {
        c = lv_label_get_text(label_set_value);
        temp = atoi(c);
        lv_label_set_text_fmt(label_set_value, "%02d", temp - 1);
    }
}