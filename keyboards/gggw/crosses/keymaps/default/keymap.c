// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "features/achordion.h"

#ifdef CONSOLE_ENABLE
#    include "print.h"
#endif /* ifdef CONSOLE_ENABLE */

/*
 * Keycodes, combos, and layers! oh my!
 */

enum CROSSES_LAYERS {
    _BASE,
    _NUM,
    _NAV,
    _MEDIA,
    _FUNC,
    _MOUS,
};

enum my_keycodes {
    VIMS = SAFE_RANGE,
    LARR,
    FARR,
    EPIP,
    LVBC,
    RVBC,
    BARR,
    MSE_INC,
    MSE_DEC,
    DRAG_SCROLL,
};

/*
 * Pointer Storage
 */
typedef union {
    uint32_t raw;
    struct {
        uint16_t mse_cpi : 16;
    } __attribute__((packed));
} global_user_config_t;

global_user_config_t global_user_config = {0};

const uint16_t MIN_DEFAULT_DPI = 400;
const uint16_t MAX_DEFAULT_DPI = 10000;

void write_config_to_eeprom(global_user_config_t* config) {
    eeconfig_update_user(config->raw);
}

uint16_t get_pointer_dpi(global_user_config_t* config) {
    uint16_t current = config->mse_cpi;

    if (current < MIN_DEFAULT_DPI) {
        return MIN_DEFAULT_DPI;
    }

    if (current > MAX_DEFAULT_DPI) {
        return MAX_DEFAULT_DPI;
    }

    return current;
}

void update_pointer_cpi(global_user_config_t* config) {
    pointing_device_set_cpi(get_pointer_dpi(config));
}

void change_pointer_dpi(global_user_config_t* config, bool inc) {
    uint16_t current = config->mse_cpi;
    uint16_t requested = current += inc ? 50 : -50;

    if (requested < MIN_DEFAULT_DPI) {

#ifdef CONSOLE_ENABLE
        dprintf("%u is beyond bounds - omitting operation", requested);
#endif // CONSOLE_ENABLE
        return;
    }

    if (requested > MAX_DEFAULT_DPI) {
#ifdef CONSOLE_ENABLE
        dprintf("%u is beyond bounds - omitting operation", requested);
#endif // CONSOLE_ENABLE
        return;
    }

    config->mse_cpi += inc ? 10 : -10;
    pointing_device_set_cpi(get_pointer_dpi(config));
    write_config_to_eeprom(&global_user_config);
}


void debug_config_to_console(global_user_config_t* config) {
#ifdef CONSOLE_ENABLE
    dprintf("(crosses) process_record_user: config = {\n"
            "\traw = 0x%lu,\n"
            "\t{\n"
            "\t\tmse_cpi=0x%X (%u)\n"
            "\t}\n"
            "}\n",
            (unsigned long)config->raw, config->mse_cpi, get_pointer_dpi(config));
#endif // CONSOLE_ENABLE
}

void eeconfig_init_user(void) {
    global_user_config.raw = 0;
    global_user_config.mse_cpi = MIN_DEFAULT_DPI;

    write_config_to_eeprom(&global_user_config);
    debug_config_to_console(&global_user_config);
}

/*
 * Pointing Device Config
 */

bool set_scrolling = false;

float scroll_acc_h = 0;
float scroll_acc_v = 0;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (set_scrolling) {
        // Calculate and accumulate scroll values based on mouse movement and divisors
        scroll_acc_h += (float)mouse_report.x / SCROLL_DIVISOR_H;
        scroll_acc_v += (float)mouse_report.y / SCROLL_DIVISOR_V;

        // Assign integer parts of accumulated scroll values to the mouse report
        mouse_report.h = (int8_t)scroll_acc_h;
        mouse_report.v = (int8_t)scroll_acc_v;

        // Update accumulated scroll values by subtracting the integer parts
        scroll_acc_h -= (int8_t)scroll_acc_h;
        scroll_acc_v -= (int8_t)scroll_acc_v;

        // Clear the X and Y values of the mouse report
        mouse_report.x = 0;
        mouse_report.y = 0;
    }

    return mouse_report;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    if (get_highest_layer(state) != 5) {
        set_scrolling = false;
    }

    return state;
}

void pointing_device_init_user(void) {
    set_auto_mouse_layer(_MOUS);
    set_auto_mouse_enable(true);
}

enum combos {
    WY_TAB,
    DH_CAPS,
    HCOMA_MINS,
    WSPC_VIM,
    FU_QUOTE,
    PL_DQUOTE,
    CCOM_LARR,
    XDOT_FARR,
    GM_EPIP,
    COMD_UNDERS,
    ZDOT_BARR,
};

const uint16_t PROGMEM wy_combo[]     = {KC_W, KC_Y, COMBO_END};
const uint16_t PROGMEM dh_combo[]     = {KC_D, KC_H, COMBO_END};
const uint16_t PROGMEM hcomma_combo[] = {KC_H, KC_COMM, COMBO_END};
const uint16_t PROGMEM wspc_combo[]   = {KC_W, KC_SPC, COMBO_END};
const uint16_t PROGMEM fu_combo[]     = {KC_F, KC_U, COMBO_END};
const uint16_t PROGMEM pl_combo[]     = {KC_P, KC_L, COMBO_END};
const uint16_t PROGMEM ccom_combo[]   = {KC_C, KC_COMM, COMBO_END};
const uint16_t PROGMEM xdot_combo[]   = {KC_X, KC_DOT, COMBO_END};
const uint16_t PROGMEM gm_combo[]     = {KC_G, KC_M, COMBO_END};
const uint16_t PROGMEM comd_combo[]   = {KC_COMM, KC_DOT, COMBO_END};
const uint16_t PROGMEM zdot_combo[]   = {KC_Z, KC_DOT, COMBO_END};

combo_t key_combos[] = {
    [WY_TAB] = COMBO(wy_combo, KC_TAB),
    [DH_CAPS] = COMBO(dh_combo, CW_TOGG),
    [HCOMA_MINS] = COMBO(hcomma_combo, KC_MINS),
    [WSPC_VIM] = COMBO(wspc_combo, VIMS),
    [FU_QUOTE] = COMBO(fu_combo, KC_QUOT),
    [PL_DQUOTE] = COMBO(pl_combo, S(KC_QUOT)),
    [CCOM_LARR] = COMBO(ccom_combo, LARR),
    [XDOT_FARR] = COMBO(xdot_combo, FARR),
    [GM_EPIP] = COMBO(gm_combo, EPIP),
    [COMD_UNDERS] = COMBO(comd_combo, S(KC_MINS)),
    [ZDOT_BARR] = COMBO(zdot_combo, BARR),
};

/*
 * Keymaps!
 */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//    ┌───────────┬───────────┬───────────┬───────────┬────────────┐                      ┌──────┬───────────┬───────────┬───────────┬───────────┐
//    │     q     │     w     │     f     │     p     │     b      │                      │  j   │     l     │     u     │     y     │     ;     │
//    ├───────────┼───────────┼───────────┼───────────┼────────────┤                      ├──────┼───────────┼───────────┼───────────┼───────────┤
//    │ LGUI_T(a) │ LALT_T(r) │ LCTL_T(s) │ LSFT_T(t) │     g      │                      │  m   │ RSFT_T(n) │ RCTL_T(e) │ RALT_T(i) │ RGUI_T(o) │
//    ├───────────┼───────────┼───────────┼───────────┼────────────┤                      ├──────┼───────────┼───────────┼───────────┼───────────┤
//    │     z     │     x     │     c     │     d     │     v      │                      │  k   │     h     │     ,     │     .     │ LT(3, /)  │
//    └───────────┴───────────┴───────────┼───────────┼────────────┼─────┐   ┌────────────┼──────┼───────────┼───────────┴───────────┴───────────┘
//                                        │   MO(5)   │ LT(2, esc) │ spc │   │ LT(1, ent) │ bspc │   MO(4)   │
//                                        └───────────┴────────────┴─────┘   └────────────┴──────┴───────────┘
[_BASE] = LAYOUT_default(
  KC_Q         , KC_W         , KC_F         , KC_P         , KC_B          ,                              KC_J    , KC_L         , KC_U         , KC_Y         , KC_SCLN       ,
  LGUI_T(KC_A) , LALT_T(KC_R) , LCTL_T(KC_S) , LSFT_T(KC_T) , KC_G          ,                              KC_M    , RSFT_T(KC_N) , RCTL_T(KC_E) , RALT_T(KC_I) , RGUI_T(KC_O)  ,
  KC_Z         , KC_X         , KC_C         , KC_D         , KC_V          ,                              KC_K    , KC_H         , KC_COMM      , KC_DOT       , LT(3, KC_SLSH),
                                               MO(5)        , LT(2, KC_ESC) , KC_SPC ,     LT(1, KC_ENT) , KC_BSPC , MO(4)
),

//    ┌──────┬───┬───┬───────────┬──────┐               ┌──────┬───────────┬───────────┬──────┬─────┐
//    │ S(8) │ 7 │ 8 │     9     │  /   │               │  \   │   S(9)    │   S(0)    │ S(\) │     │
//    ├──────┼───┼───┼───────────┼──────┤               ├──────┼───────────┼───────────┼──────┼─────┤
//    │  -   │ 4 │ 5 │ LSFT_T(6) │ S(=) │               │ S(5) │ RSFT_T([) │ RCTL_T(]) │ S(;) │  ;  │
//    ├──────┼───┼───┼───────────┼──────┤               ├──────┼───────────┼───────────┼──────┼─────┤
//    │  `   │ 1 │ 2 │     3     │  =   │               │  [   │     ]     │           │      │     │
//    └──────┴───┴───┼───────────┼──────┼─────┐   ┌─────┼──────┼───────────┼───────────┴──────┴─────┘
//                   │           │  0   │     │   │     │      │           │
//                   └───────────┴──────┴─────┘   └─────┴──────┴───────────┘
[_NUM] = LAYOUT_default(
  S(KC_8) , KC_7 , KC_8 , KC_9         , KC_SLSH   ,                         KC_BSLS , S(KC_9)         , S(KC_0)         , S(KC_BSLS) , KC_TRNS,
  KC_MINS , KC_4 , KC_5 , LSFT_T(KC_6) , S(KC_EQL) ,                         S(KC_5) , RSFT_T(KC_LBRC) , RCTL_T(KC_RBRC) , S(KC_SCLN) , KC_SCLN,
  KC_GRV  , KC_1 , KC_2 , KC_3         , KC_EQL    ,                         KC_LBRC , KC_RBRC         , KC_TRNS         , KC_TRNS    , KC_TRNS,
                          KC_TRNS      , KC_0      , KC_TRNS ,     KC_TRNS , KC_TRNS , KC_TRNS
),

//    ┌──────┬──────┬──────┬───────┬─────┐               ┌─────┬──────┬──────┬──────┬──────┐
//    │      │      │      │       │     │               │     │      │      │      │      │
//    ├──────┼──────┼──────┼───────┼─────┤               ├─────┼──────┼──────┼──────┼──────┤
//    │ lgui │ lalt │ lctl │ lsft  │     │               │     │ left │ down │  up  │ rght │
//    ├──────┼──────┼──────┼───────┼─────┤               ├─────┼──────┼──────┼──────┼──────┤
//    │      │      │ copy │ paste │     │               │     │ home │ end  │ pgdn │ pgup │
//    └──────┴──────┴──────┼───────┼─────┼─────┐   ┌─────┼─────┼──────┼──────┴──────┴──────┘
//                         │       │     │     │   │     │     │      │
//                         └───────┴─────┴─────┘   └─────┴─────┴──────┘
[_NAV] = LAYOUT_default(
  KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS ,                         KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
  KC_LGUI , KC_LALT , KC_LCTL , KC_LSFT , KC_TRNS ,                         KC_TRNS , KC_LEFT , KC_DOWN , KC_UP   , KC_RGHT,
  KC_TRNS , KC_TRNS , KC_COPY , KC_PSTE , KC_TRNS ,                         KC_TRNS , KC_HOME , KC_END  , KC_PGDN , KC_PGUP,
                                KC_TRNS , KC_TRNS , KC_TRNS ,     KC_TRNS , KC_TRNS , KC_TRNS
),

//    ┌──────┬──────┬──────┬──────┬──────┐               ┌─────┬─────┬─────┬─────┬─────┐
//    │      │      │      │      │      │               │     │     │     │     │     │
//    ├──────┼──────┼──────┼──────┼──────┤               ├─────┼─────┼─────┼─────┼─────┤
//    │ mprv │ vold │ mply │ volu │ mnxt │               │     │     │     │     │     │
//    ├──────┼──────┼──────┼──────┼──────┤               ├─────┼─────┼─────┼─────┼─────┤
//    │      │      │      │      │      │               │     │     │     │     │     │
//    └──────┴──────┴──────┼──────┼──────┼─────┐   ┌─────┼─────┼─────┼─────┴─────┴─────┘
//                         │      │      │     │   │     │     │     │
//                         └──────┴──────┴─────┘   └─────┴─────┴─────┘
[_MEDIA] = LAYOUT_default(
  KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS ,                         KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
  KC_MPRV , KC_VOLD , KC_MPLY , KC_VOLU , KC_MNXT ,                         KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
  KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS ,                         KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
                                KC_TRNS , KC_TRNS , KC_TRNS ,     KC_TRNS , KC_TRNS , KC_TRNS
),

//    ┌─────┬─────┬─────┬─────┬─────┐               ┌─────┬──────┬──────┬──────┬──────┐
//    │ f11 │ f12 │ f13 │ f14 │ f15 │               │     │      │      │      │      │
//    ├─────┼─────┼─────┼─────┼─────┤               ├─────┼──────┼──────┼──────┼──────┤
//    │ f6  │ f7  │ f8  │ f9  │ f10 │               │     │ rsft │ rctl │ ralt │ rgui │
//    ├─────┼─────┼─────┼─────┼─────┤               ├─────┼──────┼──────┼──────┼──────┤
//    │ f1  │ f2  │ f3  │ f4  │ f5  │               │     │      │      │      │      │
//    └─────┴─────┴─────┼─────┼─────┼─────┐   ┌─────┼─────┼──────┼──────┴──────┴──────┘
//                      │     │     │     │   │     │     │      │
//                      └─────┴─────┴─────┘   └─────┴─────┴──────┘
[_FUNC] = LAYOUT_default(
  KC_F11 , KC_F12 , KC_F13 , KC_F14  , KC_F15  ,                         KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
  KC_F6  , KC_F7  , KC_F8  , KC_F9   , KC_F10  ,                         KC_TRNS , KC_RSFT , KC_RCTL , KC_RALT , KC_RGUI,
  KC_F1  , KC_F2  , KC_F3  , KC_F4   , KC_F5   ,                         KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
                             KC_TRNS , KC_TRNS , KC_TRNS ,     KC_TRNS , KC_TRNS , KC_TRNS
),

//    ┌──────┬─────────┬─────────┬─────────────┬────┐             ┌────────┬─────────┬─────────┬────┬─────────┐
//    │ lsft │  lalt   │   no    │     no      │ no │             │ EE_CLR │ MSE_INC │ MSE_DEC │ no │   no    │
//    ├──────┼─────────┼─────────┼─────────────┼────┤             ├────────┼─────────┼─────────┼────┼─────────┤
//    │ lctl │ MS_BTN3 │ MS_BTN2 │   MS_BTN1   │ no │             │   no   │   no    │   no    │ no │   no    │
//    ├──────┼─────────┼─────────┼─────────────┼────┤             ├────────┼─────────┼─────────┼────┼─────────┤
//    │  no  │   no    │   no    │ DRAG_SCROLL │ no │             │   no   │   no    │   no    │ no │ DB_TOGG │
//    └──────┴─────────┴─────────┼─────────────┼────┼────┐   ┌────┼────────┼─────────┼─────────┴────┴─────────┘
//                               │             │ no │ no │   │ no │   no   │   no    │
//                               └─────────────┴────┴────┘   └────┴────────┴─────────┘
[_MOUS] = LAYOUT_default(
  KC_LSFT , KC_LALT , KC_NO   , KC_NO       , KC_NO ,                     EE_CLR , MSE_INC , MSE_DEC , KC_NO , KC_NO  ,
  KC_LCTL , MS_BTN3 , MS_BTN2 , MS_BTN1     , KC_NO ,                     KC_NO  , KC_NO   , KC_NO   , KC_NO , KC_NO  ,
  KC_NO   , KC_NO   , KC_NO   , DRAG_SCROLL , KC_NO ,                     KC_NO  , KC_NO   , KC_NO   , KC_NO , DB_TOGG,
                                KC_TRNS     , KC_NO , KC_NO ,     KC_NO , KC_NO  , KC_NO
)
};

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (!process_achordion(keycode, record)) {
        return false;
    }
    switch (keycode) {
        case RSFT_T(KC_LBRC):
            if (record->tap.count && record->event.pressed) {
                tap_code16(S(KC_LBRC));
                return false;
            }
            break;

        case RCTL_T(KC_RBRC):
            if (record->tap.count && record->event.pressed) {
                tap_code16(S(KC_RBRC));
                return false;
            }
            break;

        case VIMS:
            if (record->event.pressed) {
                tap_code16(KC_ESC);
                SEND_STRING(":w\n");

                return false;
            }
            break;

        case EPIP:
            if (record->event.pressed) {
                SEND_STRING("|> ");

                return false;
            }
            break;

        case LARR:
            if (record->event.pressed) {
                SEND_STRING("-> ");

                return false;
            }
            break;

        case FARR:
            if (record->event.pressed) {
                SEND_STRING("=> ");

                return false;
            }
            break;

        case BARR:
            if (record->event.pressed) {
                SEND_STRING("<- ");

                return false;
            }
            break;
        case MSE_INC:
            if (record->event.pressed) {
                change_pointer_dpi(&global_user_config, true);
                debug_config_to_console(&global_user_config);

                return false;
            }
            break;
        case MSE_DEC:
            if (record->event.pressed) {
                change_pointer_dpi(&global_user_config, false);
                debug_config_to_console(&global_user_config);

                return false;
            }
            break;
        case DRAG_SCROLL:
            set_scrolling = record->event.pressed;
            break;
    }

    return true;
}

bool achordion_chord(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record, uint16_t other_keycode, keyrecord_t* other_record) {
    switch (tap_hold_keycode) {
        case 0x4128:
            return true;
            break;
    }

    return achordion_opposite_hands(tap_hold_record, other_record);
}

void matrix_scan_user(void) {
    achordion_task();
}

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (!is_keyboard_master()) {
        return rotation;
    }

    return OLED_ROTATION_180;
}

static void render_logo(void) {
    static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0xA0, 0xA1, 0xA2,
        0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA,
        0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2,
        0xB3, 0xB4, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5,
        0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD,
        0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };

    oled_write_P(qmk_logo, false);
}

bool oled_task_user(void) {
    render_logo();

    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR(">>> BASE "), false);
            break;
        case _NUM:
            oled_write_P(PSTR(">>> PROG "), false);
            break;
        case _NAV:
            oled_write_P(PSTR(">>> NAVI "), false);
            break;
        case _MEDIA:
            oled_write_P(PSTR(">>> MEDIA"), false);
            break;
        case _FUNC:
            oled_write_P(PSTR(">>> FUNC "), false);
            break;
        case _MOUS:
            oled_write_P(PSTR(">>> MOUSE"), false);
            break;
        default:
            break;
    }
    // oled_write_ln_P(get_u16_str(get_pointer_dpi(&global_user_config), '0'), false);
    return false;
}
#endif /* ifdef OLED_ENABLE */

void keyboard_post_init_user(void) {
    global_user_config.raw = eeconfig_read_user();
    update_pointer_cpi(&global_user_config);
    write_config_to_eeprom(&global_user_config);
}
