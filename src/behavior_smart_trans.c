/*
 *  Smart-Transparent Behavior  —  “&smart_trans”
 *
 *  要求仕様
 *  1. 押下時に Layer-0 以外のすべてのレイヤを無効化
 *     → 事実上「&to 0」と同じ (`zmk_layer_move( 0)` を利用)
 *  2. 同じ key-position の Layer-0 バインドを取得し
 *     その press/release をそのまま再発火
 *  3. &mt / &mo など State-full ビヘイビアも正しく動作
 *     例: Layer-0 が “&mt LCTRL X” なら
 *          └ X を押し hold しつつ C を押せば Ctrl-C 発動
 */

#define FALLBACK_LAYER 0          /* 透過先レイヤを 0 に固定 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zmk/behavior.h>
#include <zmk/layer.h>
#include <zmk/keymap.h>
#include <zmk/behavior_queue.h>

/* ────────────── PRESS ────────────── */
static int smart_trans_pressed(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event)
{
    /* Layer-0 の同位置バインドを取得 */
    const struct zmk_behavior_binding *base_binding =
        zmk_keymap_binding_for_position(FALLBACK_LAYER, event.position);

    if (!base_binding) {
        return ZMK_BEHAVIOR_TRANSPARENT;   /* 何も定義されていなければ素通り */
    }

    /* すべての非 0 レイヤを無効化（&to 0 相当） */
    zmk_layer_move(FALLBACK_LAYER);

    /* Layer-0 バインドを再発火（state-full もそのまま機能） */
    return zmk_behavior_keymap_binding_pressed(
        (struct zmk_behavior_binding *)base_binding, event);
}

/* ────────────── RELEASE ────────────── */
static int smart_trans_released(struct zmk_behavior_binding *binding,
                                struct zmk_behavior_binding_event event)
{
    /* PRESS 時と同じ Layer-0 バインドを取得して release を転送 */
    const struct zmk_behavior_binding *base_binding =
        zmk_keymap_binding_for_position(FALLBACK_LAYER, event.position);

    if (!base_binding) {
        return ZMK_BEHAVIOR_TRANSPARENT;
    }

    return zmk_behavior_keymap_binding_released(
        (struct zmk_behavior_binding *)base_binding, event);
}

/* ────────────── Driver API table ────────────── */
static const struct behavior_driver_api smart_trans_driver_api = {
    .binding_pressed  = smart_trans_pressed,
    .binding_released = smart_trans_released,
};

/* ────────────── Device instantiation ────────────── */
DEVICE_DEFINE(smart_trans_behavior,          /* インスタンスシンボル      */
              "SMART_TRANS",                 /* デバイス名文字列           */
              NULL, NULL, NULL, NULL,        /* 初期化 & PM ハンドラ無し   */
              APPLICATION,                   /* 初期化レベル               */
              CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
              &smart_trans_driver_api);