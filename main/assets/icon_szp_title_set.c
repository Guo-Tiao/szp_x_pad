/*******************************************************************************
 * Size: 16 px
 * Bpp: 2
 * Opts: --bpp 2 --size 16 --no-compress --font iconfont.ttf --range 60490,59244,59951,58963 --format lvgl -o icon_szp_title_set.c
 ******************************************************************************/

#include "lvgl.h"

#ifndef ICON_SZP_TITLE_SET
#define ICON_SZP_TITLE_SET 1
#endif

#if ICON_SZP_TITLE_SET

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+E653 "" */
    0x24, 0x0, 0x0, 0x0, 0x2d, 0x2f, 0xfe, 0x0,
    0xf, 0x4b, 0xff, 0xe0, 0x7f, 0xd0, 0x2, 0xfd,
    0xb8, 0xb4, 0x0, 0x2e, 0x10, 0x3d, 0x28, 0x4,
    0x2, 0xff, 0x4b, 0x80, 0x2, 0xe2, 0xd2, 0x80,
    0x0, 0x0, 0xb4, 0x0, 0x0, 0xb, 0xfd, 0x0,
    0x0, 0xb, 0xeb, 0x40, 0x0, 0x2, 0x82, 0xd0,
    0x0, 0x0, 0x0, 0xb0, 0x0, 0x0, 0x0, 0x0,

    /* U+E76C "" */
    0x0, 0x6f, 0xf9, 0x0, 0x7, 0xff, 0xff, 0xd0,
    0x3f, 0xe4, 0x1b, 0xfc, 0xbd, 0x0, 0x0, 0x7e,
    0x30, 0x6, 0x90, 0xc, 0x0, 0xbf, 0xfe, 0x0,
    0x3, 0xfe, 0xbf, 0xc0, 0x1, 0xd0, 0x7, 0x40,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 0xf0, 0x0,
    0x0, 0x7, 0xd0, 0x0, 0x0, 0x1, 0x40, 0x0,

    /* U+EA2F "" */
    0x0, 0x0, 0x0, 0x0, 0x3, 0x40, 0x0, 0x0,
    0xf8, 0x0, 0xb0, 0x37, 0xc0, 0xb, 0x4c, 0x2c,
    0x0, 0x79, 0x2d, 0x0, 0x2, 0xc8, 0x0, 0x0,
    0x5d, 0x0, 0x1, 0xe8, 0xe0, 0x1, 0xd3, 0xb,
    0x0, 0x0, 0xc5, 0x74, 0x0, 0x3f, 0x0, 0x0,
    0xe, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+EC4A "" */
    0x0, 0x0, 0x0, 0x0, 0xd, 0x0, 0x0, 0x0,
    0xf8, 0x0, 0x0, 0xc, 0xe0, 0x0, 0x70, 0xc3,
    0x42, 0x42, 0xcc, 0xa2, 0xc, 0xb, 0xe8, 0x18,
    0xc0, 0x2e, 0x1, 0x8c, 0x2, 0xf0, 0x18, 0xc0,
    0x7e, 0xc3, 0x4c, 0x1c, 0xcb, 0x1, 0x87, 0xc,
    0x34, 0x20, 0x0, 0xce, 0x0, 0x0, 0xf, 0x80,
    0x0, 0x0, 0xe0, 0x0, 0x0, 0x4, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 256, .box_w = 16, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 56, .adv_w = 256, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 256, .box_w = 13, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 150, .adv_w = 256, .box_w = 14, .box_h = 16, .ofs_x = 1, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x119, 0x3dc, 0x5f7
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 58963, .range_length = 1528, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 4, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t icon_szp_title_set = {
#else
lv_font_t icon_szp_title_set = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if ICON_SZP_TITLE_SET*/

