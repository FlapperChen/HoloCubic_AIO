#include "weather_old.h"
#include "weather_old_gui.h"
#include "ESP32Time.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include <Wire.h>
#include <SparkFun_Qwiic_Humidity_AHT20.h>

#define TIME_API "https://acs.m.taobao.com/gw/mtop.common.getTimestamp/"
#define WEATHER_PAGE_SIZE 2

// AHT20 I2C引脚定义
#define AHT20_SDA_PIN 21
#define AHT20_SCL_PIN 22

// AHT20 传感器 - 使用独立的I2C总线 (GPIO21/GPIO22)
// 注意：AHT20使用GPIO21(SDA)和GPIO22(SCL)，与MPU6050分离

struct SensorData
{
    float temperature;
    float humidity;
};

// 传感器的持久化配置
#define WEATHER_OLD_CONFIG_PATH "/weather_old.cfg"
struct WT_Config
{
    unsigned long sensorUpdateInterval;   // 传感器更新的时间间隔(s)
    unsigned long timeUpdateInterval;     // 日期时钟更新的时间间隔(s)
};

static void write_config(const WT_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->sensorUpdateInterval);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->timeUpdateInterval);
    w_data += tmp;
    g_flashCfg.writeFile(WEATHER_OLD_CONFIG_PATH, w_data.c_str());
}

static void read_config(WT_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(WEATHER_OLD_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    
    // 强制重置配置，确保使用正确的默认值
    cfg->sensorUpdateInterval = 5000;   // 传感器更新间隔5秒 (5000毫秒)
    cfg->timeUpdateInterval = 900000;   // 时间更新间隔900秒 (15分钟)
    write_config(cfg);
    
    // 安全检查：确保配置值不为0或过小
    if (cfg->sensorUpdateInterval < 1000) {
        cfg->sensorUpdateInterval = 5000;
    }
    if (cfg->timeUpdateInterval < 60000) {
        cfg->timeUpdateInterval = 900000;
    }
}

struct WeatherAppRunData
{
    unsigned long preSensorMillis;    // 上一回更新传感器时的毫秒数
    unsigned long preTimeMillis;      // 更新时间计数器
    long long m_preNetTimestamp;      // 上一次的网络时间戳
    long long m_errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long m_preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag;   // 强制更新标志
    unsigned int forceSensorUpdate;   // 专门用于传感器的强制更新标志
    int clock_page;                   // 时钟桌面的播放记录

    ESP32Time g_rtc;                  // 用于时间解码
    SensorData sensorData;            // 保存传感器数据
};

static WT_Config cfg_data;
static WeatherAppRunData *run_data = NULL;
static AHT20 humiditySensor;
static TwoWire AHT20_Wire = TwoWire(1); // 使用I2C总线1

static SensorData getSensorData(void)
{
    return run_data->sensorData;
}

static long long getTimestamp(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return 0;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    // start connection and send HTTP headerFFF
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            // 以网络时间戳为准
            run_data->m_preNetTimestamp = atoll(time.c_str()) + run_data->m_errorNetTimestamp;
            run_data->m_preLocalTimestamp = GET_SYS_MILLIS();
        }
    }
    else
    {
        // 得不到网络时间戳时
        run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (GET_SYS_MILLIS() - run_data->m_preLocalTimestamp);
        run_data->m_preLocalTimestamp = GET_SYS_MILLIS();
    }
    http.end();

    return run_data->m_preNetTimestamp;
}

static void UpdateSensorData(SensorData *sensorData, lv_scr_load_anim_t anim_type)
{
    char temperature[10] = {0};
    char humidity[10] = {0};
    sprintf(temperature, "%.1f", sensorData->temperature);
    sprintf(humidity, "%.1f", sensorData->humidity);
    display_sensor_data(temperature, humidity, anim_type);
}

static void UpdateTime_RTC(long long timestamp, lv_scr_load_anim_t anim_type)
{
    run_data->g_rtc.setTime(timestamp / 1000);
    String date = run_data->g_rtc.getDate(String("%Y-%m-%d"));
    String time = run_data->g_rtc.getTime(String("%H:%M:%S"));
    display_time_old(date.c_str(), time.c_str(), anim_type);
}

static int weather_init(AppController *sys)
{
    Serial.println("Weather Old App: Starting initialization...");
    
    weather_old_gui_init();
    // 获取配置信息
    read_config(&cfg_data);
    
    Serial.println("Weather Old App: GUI and config initialized");
    
    // 初始化 AHT20 传感器 - 使用独立的I2C总线
    // AHT20使用GPIO21(SDA)和GPIO22(SCL)，与MPU6050(GPIO32/GPIO33)分离
    
    // 初始化AHT20专用的I2C总线
    AHT20_Wire.begin(AHT20_SDA_PIN, AHT20_SCL_PIN);
    AHT20_Wire.setClock(400000); // 设置I2C时钟频率为400kHz
    
    // 延时确保I2C总线稳定
    delay(100);
    
    // 初始化AHT20传感器
    if (false == humiditySensor.begin(AHT20_Wire))
    {
        Serial.println("AHT20 sensor initialization failed!");
        // 即使传感器初始化失败，也不阻止应用启动
        // 将使用默认的传感器数据
    }
    else
    {
        Serial.println("AHT20 sensor initialized successfully.");
        // 等待传感器完全准备好
        delay(500);
    }
    
    // 初始化运行时参数
    run_data = (WeatherAppRunData *)calloc(1, sizeof(WeatherAppRunData));
    if (NULL == run_data)
    {
        Serial.println("Weather Old App: Failed to allocate memory for run_data!");
        return -1; // 内存分配失败
    }
    
    Serial.println("Weather Old App: Memory allocated successfully");
    
    run_data->m_preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    run_data->m_errorNetTimestamp = 2;
    run_data->m_preLocalTimestamp = 0; // 上一次的本地机器时间戳
    run_data->clock_page = 0;          // 时钟桌面的播放记录
    // 初始化时间记录，设置为当前时间减去间隔，确保第一次检查时会立即更新
    unsigned long currentMillis = GET_SYS_MILLIS();
    run_data->preSensorMillis = currentMillis - cfg_data.sensorUpdateInterval - 1000; // 减去额外1秒确保触发
    run_data->preTimeMillis = currentMillis - cfg_data.timeUpdateInterval - 1000;
    run_data->coactusUpdateFlag = 0x00; // 不使用强制更新，依赖时间间隔
    run_data->forceSensorUpdate = 0x00; // 初始化传感器强制更新标志

    // 初始化传感器数据为合理的默认值
    run_data->sensorData.temperature = 25.0;  // 默认25°C
    run_data->sensorData.humidity = 50.0;     // 默认50%湿度
    
    Serial.println("Weather Old App: Initialization completed successfully");
    return 0;
}

static void weather_process(AppController *sys,
                            const ImuAction *act_info)
{
    // 安全检查：确保运行数据已正确初始化
    if (NULL == run_data)
    {
        Serial.println("Weather Old App: run_data is NULL, exiting...");
        sys->app_exit();
        return;
    }
    
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        run_data->clock_page = (run_data->clock_page + 1) % WEATHER_PAGE_SIZE;
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        // 以下等效与 clock_page = (clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        run_data->clock_page = (run_data->clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
    }
    else if (GO_FORWORD == act_info->active)
    {
        // 后仰时，强制更新当前页面的数据
        run_data->coactusUpdateFlag = 0x01;
        run_data->forceSensorUpdate = 0x01;
    }

    if (0 == run_data->clock_page) // 更新传感器数据
    {
        // 检查是否需要更新传感器数据（使用简单的时间间隔检查）
        unsigned long currentMillis = GET_SYS_MILLIS();
        unsigned long timeSinceLastUpdate = currentMillis - run_data->preSensorMillis;
        bool timeIntervalReached = (timeSinceLastUpdate >= cfg_data.sensorUpdateInterval);
        bool shouldUpdateSensor = (0x01 == run_data->forceSensorUpdate) || timeIntervalReached;
        
        if (shouldUpdateSensor)
        {
            // 更新时间记录
            run_data->preSensorMillis = currentMillis;
            
            // 尝试从AHT20传感器读取数据
            bool dataValid = false;
            float temp = -999.0;
            float humi = -999.0;
            
            // 检查传感器是否有新数据可用
            if (humiditySensor.available())
            {
                temp = humiditySensor.getTemperature();
                humi = humiditySensor.getHumidity();
                dataValid = true;
            }
            else
            {
                // 如果没有新数据，强制触发测量
                delay(50); // 给传感器一些时间进行测量
                temp = humiditySensor.getTemperature();
                humi = humiditySensor.getHumidity();
                dataValid = true;
            }
            
            // 检查数据是否合理 (温度范围 -40 到 85°C, 湿度范围 0 到 100%)
            if (dataValid && temp > -40 && temp < 85 && humi >= 0 && humi <= 100)
            {
                run_data->sensorData.temperature = temp;
                run_data->sensorData.humidity = humi;
            }
            else
            {
                // 如果数据无效，保持之前的有效值不变
                Serial.println("Invalid sensor data received, keeping previous values");
            }
            run_data->forceSensorUpdate = 0x00; // 重置传感器强制更新标志
        }
        
        SensorData sensorData = getSensorData();
        UpdateSensorData(&sensorData, anim_type);
    }

    // 界面刷新
    if (1 == run_data->clock_page) // 更新湿度显示页面
    {
        // 获取传感器数据
        SensorData sensorData = getSensorData();
        
        // 格式化温湿度数据并显示
        char temperature[10] = {0};
        char humidity[10] = {0};
        sprintf(temperature, "%.1f", sensorData.temperature);
        sprintf(humidity, "%.1f", sensorData.humidity);
        display_humidity_data(humidity, temperature, anim_type);
    }

    delay(300);
}

static void weather_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int weather_exit_callback(void *param)
{
    Serial.println("Weather Old App: Starting exit process...");
    
    weather_old_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
        Serial.println("Weather Old App: run_data freed");
    }
    
    // 停止AHT20传感器相关的I2C总线（可选）
    // AHT20_Wire.end(); // 如果需要完全释放I2C资源可以取消注释
    
    Serial.println("Weather Old App: Exit completed");
    return 0;
}

static void weather_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        int event_id = (int)message;
        if (1 == run_data->clock_page && run_data->clock_page == event_id)
        {
            long long timestamp = getTimestamp(TIME_API) + TIMEZERO_OFFSIZE; // nowapi时间API
            UpdateTime_RTC(timestamp, LV_SCR_LOAD_ANIM_NONE);
        }
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "sensorUpdateInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.sensorUpdateInterval);
        }
        else if (!strcmp(param_key, "timeUpdateInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.timeUpdateInterval);
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "sensorUpdateInterval"))
        {
            cfg_data.sensorUpdateInterval = atol(param_val);
        }
        else if (!strcmp(param_key, "timeUpdateInterval"))
        {
            cfg_data.timeUpdateInterval = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ weather_old_app = {WEATHER_OLD_APP_NAME, &app_weather_old, "",
                           weather_init, weather_process, weather_background_task,
                           weather_exit_callback, weather_message_handle};
