#define LV_ATTRIBUTE_IMG_WEATHER_VERSION 2

#include "weather_old_gui.h"
#include <stdlib.h>

#if LV_ATTRIBUTE_IMG_WEATHER_VERSION == 1
#include "weather_old_image_1.h"
#else
#include "weather_old_image_2.h"
#endif

#include "lvgl.h"

static lv_obj_t *wc_scr[2];

static lv_obj_t *weather_image = NULL;
static lv_obj_t *temperature_label = NULL;
static lv_obj_t *temperature_symbol = NULL;
static lv_obj_t *humidity_label = NULL;
static lv_obj_t *humidity_symbol = NULL;
static lv_obj_t *main_temp_label = NULL;
static lv_obj_t *temperature_bar = NULL;
static lv_obj_t *humidity_bar = NULL;
static lv_obj_t *temp_bar_label = NULL;
static lv_obj_t *hum_bar_label = NULL;

static lv_obj_t *time_image = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *time_label = NULL;

static lv_obj_t *humidity_main_label = NULL;

// static lv_group_t *g;
static lv_style_t default_style;
static lv_style_t label_style1;
static lv_style_t label_style2;
static lv_style_t label_style3;
static lv_style_t label_style4;
static lv_style_t temp_bar_bg_style;
static lv_style_t temp_bar_indic_style;
static lv_style_t hum_bar_bg_style;
static lv_style_t hum_bar_indic_style;

LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(lv_font_montserrat_24);
LV_FONT_DECLARE(lv_font_montserrat_40);

// 天气图标路径的映射关系

#if LV_ATTRIBUTE_IMG_WEATHER_VERSION == 1
const void *image_map[] = {&Sunny, &Clear, "S:/weather/Fair_2.bin", "S:/weather/Fair_3.bin",
                           &Cloudy, &PartlyCloudy, &PartlyCloudy,
                           "S:/weather/MostlyCloudy_7.bin", "S:/weather/MostlyCloudy_8.bin",
                           &Overcast, "S:/weather/Shower_10.bin", &Thundershower,
                           "S:/weather/ThundershowerWithHail_12.bin", &LightRain,
                           &ModerateRain, &HeavyRain, "S:/weather/Storm_16.bin",
                           "S:/weather/HeavyStorm_17.bin", "S:/weather/SevereStorm_18.bin",
                           "S:/weather/IceRain_19.bin", &Sleet, &SnowFlurry, &LightSnow,
                           &ModerateSnow, &HeavySnow, "S:/weather/Snowstorm_25.bin",
                           "S:/weather/Dust_26.bin", "S:/weather/Sand_27.bin", "S:/weather/Duststorm_28.bin",
                           "S:/weather/Sandstorm_29.bin", "S:/weather/Foggy_30.bin", "S:/weather/Haze_31.bin",
                           "S:/weather/Windy_32.bin", "S:/weather/Blustery_33.bin", "S:/weather/Hurricane_34.bin",
                           "S:/weather/TropicalStorm_35.bin", "S:/weather/Tornado_36.bin", "S:/weather/Cold_37.bin",
                           "S:/weather/Hot_38.bin", "S:/weather/Unknown_99.bin"};
#else
const void *image_map[] = {&Sunny_100, &Clear_150, "S:/weather/Clear_150.bin", "S:/weather/Clear_150.bin",
                           &Cloudy_101, &PartlyCloudy_103, &PartlyCloudy_153,
                           "S:/weather/PartlyCloudy_153.bin", "S:/weather/PartlyCloudy_153.bin",
                           &Overcast_104, "S:/weather/ShowerRain_300.bin", &Thundershower_302,
                           "S:/weather/ThundershowerWithHail_304.bin", &LightRain_305,
                           &ModerateRain_306, &HeavyRain_307, "S:/weather/Storm_310.bin",
                           "S:/weather/HeavyStorm_311.bin", "S:/weather/SevereStorm_312.bin",
                           "S:/weather/FreezingRain_313.bin", &Sleet_404, &SnowFlurry_407, &LightSnow_400,
                           &ModerateSnow_401, &HeavySnow_402, "S:/weather/Snowstorm_403.bin",
                           "S:/weather/Dust_504.bin", "S:/weather/Sand_503.bin", "S:/weather/Duststorm_507.bin",
                           "S:/weather/Sandstorm_508.bin", "S:/weather/Foggy_501.bin", "S:/weather/Haze_502.bin",
                           "S:/weather/Windy_32.bin", "S:/weather/Blustery_33.bin", "S:/weather/Hurricane_34.bin",
                           "S:/weather/TropicalStorm_35.bin", "S:/weather/Tornado_36.bin", "S:/weather/Cold_901.bin",
                           "S:/weather/Hot_900.bin", "S:/weather/Unknown_999.bin"};
#endif
const int map_index[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                         10, 11, 12, 13, 14, 15, 16, 17,
                         18, 19, 20, 21, 22, 23, 24, 25,
                         26, 27, 28, 29, 30, 31, 32, 33,
                         34, 35, 36, 37, 38, 99};

void weather_old_obj_del(void);

void weather_old_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&label_style1);
    lv_style_set_text_opa(&label_style1, LV_OPA_COVER);
    lv_style_set_text_color(&label_style1, lv_color_white());
    lv_style_set_text_font(&label_style1, &lv_font_montserrat_24);

    lv_style_init(&label_style2);
    lv_style_set_text_opa(&label_style2, LV_OPA_COVER);
    lv_style_set_text_color(&label_style2, lv_color_white());
    lv_style_set_text_font(&label_style2, &lv_font_montserrat_40);

    lv_style_init(&label_style3);
    lv_style_set_text_opa(&label_style3, LV_OPA_COVER);
    lv_style_set_text_color(&label_style3, lv_color_white());
    lv_style_set_text_font(&label_style3, &lv_font_montserrat_40);

    lv_style_init(&label_style4);
    lv_style_set_text_opa(&label_style4, LV_OPA_COVER);
    lv_style_set_text_color(&label_style4, lv_color_white());
    lv_style_set_text_font(&label_style4, &lv_font_montserrat_20);

    // 温度进度条背景样式
    lv_style_init(&temp_bar_bg_style);
    lv_style_set_border_color(&temp_bar_bg_style, lv_color_white()); // 白色边框
    lv_style_set_border_width(&temp_bar_bg_style, 2);
    lv_style_set_pad_all(&temp_bar_bg_style, 6); // 使指示器更小
    lv_style_set_radius(&temp_bar_bg_style, 6);
    lv_style_set_anim_time(&temp_bar_bg_style, 800); // 增加动画时间到800ms

    // 温度进度条指示器样式
    lv_style_init(&temp_bar_indic_style);
    lv_style_set_bg_opa(&temp_bar_indic_style, LV_OPA_COVER);
    lv_style_set_bg_color(&temp_bar_indic_style, lv_color_white()); // 白色
    lv_style_set_radius(&temp_bar_indic_style, 3);
    lv_style_set_anim_time(&temp_bar_indic_style, 800); // 添加动画时间

    // 湿度进度条背景样式
    lv_style_init(&hum_bar_bg_style);
    lv_style_set_border_color(&hum_bar_bg_style, lv_color_white()); // 白色边框
    lv_style_set_border_width(&hum_bar_bg_style, 2);
    lv_style_set_pad_all(&hum_bar_bg_style, 6); // 使指示器更小
    lv_style_set_radius(&hum_bar_bg_style, 6);
    lv_style_set_anim_time(&hum_bar_bg_style, 800); // 增加动画时间到800ms

    // 湿度进度条指示器样式
    lv_style_init(&hum_bar_indic_style);
    lv_style_set_bg_opa(&hum_bar_indic_style, LV_OPA_COVER);
    lv_style_set_bg_color(&hum_bar_indic_style, lv_color_white()); // 白色
    lv_style_set_radius(&hum_bar_indic_style, 3);
    lv_style_set_anim_time(&hum_bar_indic_style, 800); // 添加动画时间
}

void display_weather_old_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[0])
        return;

    weather_old_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面

    // 天气页初始化
    wc_scr[0] = lv_obj_create(NULL);
    if (NULL == wc_scr[0]) {
        // 处理内存分配失败的情况
        return;
    }
    lv_obj_add_style(wc_scr[0], &default_style, LV_STATE_DEFAULT);

    weather_image = lv_img_create(wc_scr[0]);
    if (NULL == weather_image) {
        return;
    }
    
    // 初始化时就创建标签，避免重复创建
    temp_bar_label = lv_label_create(wc_scr[0]);
    if (temp_bar_label != NULL) {
        lv_obj_add_style(temp_bar_label, &label_style4, LV_STATE_DEFAULT);
    }
    
    hum_bar_label = lv_label_create(wc_scr[0]);
    if (hum_bar_label != NULL) {
        lv_obj_add_style(hum_bar_label, &label_style4, LV_STATE_DEFAULT);
    }
    
    // 创建主要温度显示标签（替代图片）
    main_temp_label = lv_label_create(wc_scr[0]);
    if (main_temp_label != NULL) {
        lv_obj_add_style(main_temp_label, &label_style2, LV_STATE_DEFAULT); // 使用大字体
    }
}

void display_weather_old(const char *title, const char *temperature, int weathercode, lv_scr_load_anim_t anim_type)
{
    display_weather_old_init();
    const void *path = NULL;
    if (weathercode < 39)
    {
        path = image_map[map_index[weathercode]];
    }
    else
    {
        path = image_map[39];
    }

    lv_img_set_src(weather_image, path);

    lv_obj_align(weather_image, LV_ALIGN_CENTER, 0, -30);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(wc_scr[0], anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(wc_scr[0]);
    }
}

void display_sensor_data(const char *temperature, const char *humidity, lv_scr_load_anim_t anim_type)
{
    display_weather_old_init();
    
    // 解析温湿度数值
    float temp_val = atof(temperature);
    float hum_val = atof(humidity);
    
    // 显示主要温度数值（替代图片），保留一位小数
    lv_label_set_text_fmt(main_temp_label, "%.1f°C", temp_val);
    
    // 只在进度条不存在时创建，避免重复创建
    if (temperature_bar == NULL) {
        // 创建温度进度条（横向）
        temperature_bar = lv_bar_create(wc_scr[0]);
        lv_obj_remove_style_all(temperature_bar); // 清除所有样式以获得干净的开始
        lv_obj_add_style(temperature_bar, &temp_bar_bg_style, 0);
        lv_obj_add_style(temperature_bar, &temp_bar_indic_style, LV_PART_INDICATOR);
        lv_obj_set_size(temperature_bar, 180, 20);
        lv_bar_set_range(temperature_bar, 0, 50); // 0到50度
        lv_obj_align(temperature_bar, LV_ALIGN_CENTER, -20, 30);
    }
    
    if (humidity_bar == NULL) {
        // 创建湿度进度条（横向）
        humidity_bar = lv_bar_create(wc_scr[0]);
        lv_obj_remove_style_all(humidity_bar); // 清除所有样式以获得干净的开始
        lv_obj_add_style(humidity_bar, &hum_bar_bg_style, 0);
        lv_obj_add_style(humidity_bar, &hum_bar_indic_style, LV_PART_INDICATOR);
        lv_obj_set_size(humidity_bar, 180, 20);
        lv_bar_set_range(humidity_bar, 0, 100); // 0到100%
        lv_obj_align(humidity_bar, LV_ALIGN_CENTER, -20, 70);
    }
    
    // 更新标签文本，保留一位小数
    lv_label_set_text_fmt(temp_bar_label, "%.1f°C", temp_val);
    lv_label_set_text_fmt(hum_bar_label, "%.1f%%", hum_val);
    
    // 限制数值范围
    if (temp_val < 0) temp_val = 0;
    if (temp_val > 50) temp_val = 50;
    if (hum_val < 0) hum_val = 0;
    if (hum_val > 100) hum_val = 100;
    
    // 布局排列 - 主温度显示在顶部中央
    lv_obj_align(main_temp_label, LV_ALIGN_CENTER, 0, -40);
    
    // 温度数值标签在进度条右侧
    lv_obj_align_to(temp_bar_label, temperature_bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 湿度数值标签在进度条右侧
    lv_obj_align_to(hum_bar_label, humidity_bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 使用动画更新进度条的值，每次都从当前值过渡到新值
    lv_bar_set_value(temperature_bar, (int32_t)temp_val, LV_ANIM_ON);
    lv_bar_set_value(humidity_bar, (int32_t)hum_val, LV_ANIM_ON);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(wc_scr[0], anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(wc_scr[0]);
    }
}

void display_time_old_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[1])
        return;

    weather_old_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面

    // 湿度显示页初始化
    wc_scr[1] = lv_obj_create(NULL);
    if (NULL == wc_scr[1]) {
        // 处理内存分配失败的情况
        return;
    }
    lv_obj_add_style(wc_scr[1], &default_style, LV_STATE_DEFAULT);

    // 创建主要湿度显示标签（替代主温度显示）
    humidity_main_label = lv_label_create(wc_scr[1]);
    if (humidity_main_label != NULL) {
        lv_obj_add_style(humidity_main_label, &label_style2, LV_STATE_DEFAULT); // 使用大字体
    }
    
    // 保留温度进度条相关标签
    temp_bar_label = lv_label_create(wc_scr[1]);
    if (temp_bar_label != NULL) {
        lv_obj_add_style(temp_bar_label, &label_style4, LV_STATE_DEFAULT);
    }
    
    hum_bar_label = lv_label_create(wc_scr[1]);
    if (hum_bar_label != NULL) {
        lv_obj_add_style(hum_bar_label, &label_style4, LV_STATE_DEFAULT);
    }
}

void display_time_old(const char *date, const char *time, lv_scr_load_anim_t anim_type)
{
    display_time_old_init();

    lv_img_set_src(time_image, &rocket);
    lv_obj_align(time_image, LV_ALIGN_TOP_MID, 0, 30);

    lv_label_set_text(date_label, date);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 45);

    lv_label_set_text(time_label, time);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 90);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(wc_scr[1], anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(wc_scr[1]);
    }
}

void display_humidity_data(const char *humidity, const char *temperature, lv_scr_load_anim_t anim_type)
{
    display_time_old_init();
    
    // 解析温湿度数值
    float hum_val = atof(humidity);
    float temp_val = atof(temperature);
    
    // 显示主要湿度数值，保留一位小数
    lv_label_set_text_fmt(humidity_main_label, "%.1f%%", hum_val);
    
    // 只在进度条不存在时创建，避免重复创建
    if (temperature_bar == NULL) {
        // 创建温度进度条（横向）
        temperature_bar = lv_bar_create(wc_scr[1]);
        lv_obj_remove_style_all(temperature_bar); // 清除所有样式以获得干净的开始
        lv_obj_add_style(temperature_bar, &temp_bar_bg_style, 0);
        lv_obj_add_style(temperature_bar, &temp_bar_indic_style, LV_PART_INDICATOR);
        lv_obj_set_size(temperature_bar, 180, 20);
        lv_bar_set_range(temperature_bar, 0, 50); // 0到50度
        lv_obj_align(temperature_bar, LV_ALIGN_CENTER, -20, 30);
    }
    
    if (humidity_bar == NULL) {
        // 创建湿度进度条（横向）
        humidity_bar = lv_bar_create(wc_scr[1]);
        lv_obj_remove_style_all(humidity_bar); // 清除所有样式以获得干净的开始
        lv_obj_add_style(humidity_bar, &hum_bar_bg_style, 0);
        lv_obj_add_style(humidity_bar, &hum_bar_indic_style, LV_PART_INDICATOR);
        lv_obj_set_size(humidity_bar, 180, 20);
        lv_bar_set_range(humidity_bar, 0, 100); // 0到100%
        lv_obj_align(humidity_bar, LV_ALIGN_CENTER, -20, 70);
    }
    
    // 更新标签文本，保留一位小数
    lv_label_set_text_fmt(temp_bar_label, "%.1f°C", temp_val);
    lv_label_set_text_fmt(hum_bar_label, "%.1f%%", hum_val);
    
    // 限制数值范围
    if (temp_val < 0) temp_val = 0;
    if (temp_val > 50) temp_val = 50;
    if (hum_val < 0) hum_val = 0;
    if (hum_val > 100) hum_val = 100;
    
    // 布局排列 - 主湿度显示在顶部中央
    lv_obj_align(humidity_main_label, LV_ALIGN_CENTER, 0, -40);
    
    // 温度数值标签在进度条右侧
    lv_obj_align_to(temp_bar_label, temperature_bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 湿度数值标签在进度条右侧
    lv_obj_align_to(hum_bar_label, humidity_bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 使用动画更新进度条的值，每次都从当前值过渡到新值
    lv_bar_set_value(temperature_bar, (int32_t)temp_val, LV_ANIM_ON);
    lv_bar_set_value(humidity_bar, (int32_t)hum_val, LV_ANIM_ON);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(wc_scr[1], anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(wc_scr[1]);
    }
}

void weather_old_obj_del(void)
{
    if (NULL != wc_scr[0])
    {
        lv_obj_clean(wc_scr[0]);
        weather_image = NULL;
        temperature_label = NULL;
        temperature_symbol = NULL;
        humidity_label = NULL;
        humidity_symbol = NULL;
        temperature_bar = NULL;
        humidity_bar = NULL;
        temp_bar_label = NULL;
        hum_bar_label = NULL;
        main_temp_label = NULL;
    }

    if (NULL != wc_scr[1])
    {
        lv_obj_clean(wc_scr[1]);
        time_image = NULL;
        date_label = NULL;
        time_label = NULL;
        humidity_main_label = NULL;
    }
}

void weather_old_gui_del(void)
{
    for (int pos = 0; pos < 2; ++pos)
    {
        if (NULL != wc_scr[pos])
        {
            lv_obj_clean(wc_scr[pos]); // 清空此前页面
            wc_scr[pos] = NULL;

            weather_image = NULL;
            temperature_label = NULL;
            temperature_symbol = NULL;
            humidity_label = NULL;
            humidity_symbol = NULL;
            temperature_bar = NULL;
            humidity_bar = NULL;
            temp_bar_label = NULL;
            hum_bar_label = NULL;
            main_temp_label = NULL;

            time_image = NULL;
            date_label = NULL;
            time_label = NULL;
            humidity_main_label = NULL;
        }
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&label_style1);
    // lv_style_reset(&label_style2);
    // lv_style_reset(&label_style3);
    // lv_style_reset(&label_style4);
}