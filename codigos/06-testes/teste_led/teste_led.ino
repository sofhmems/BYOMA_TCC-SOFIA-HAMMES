#include <stdio.h>
#include <stdlib.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"


#include "rom/ets_sys.h"


// ======================================================
// CONFIGURAÇÕES
// ======================================================


// LED do GP2Y1010AU0F
#define LED_PIN ((gpio_num_t)27)


// Saída Vo do GP2Y1010AU0F
// GPIO34 = ADC1_CHANNEL_6
#define ADC_CHANNEL ADC1_CHANNEL_6


// VREF padrão do ADC
#define DEFAULT_VREF 1100


// Número de amostras para média
#define NO_OF_SAMPLES 64


// ------------------------------------------------------
// CALIBRAÇÃO DO SENSOR
// ------------------------------------------------------
//
// Tensão aproximada do sensor em ambiente sem poeira.
//
// IMPORTANTE:
// Depois de testar o sensor, ajuste esse valor conforme
// a tensão medida em ambiente limpo.
//
// Exemplo:
// 0.60 V -> coloque 600
//


#define BASELINE_VOLTAGE 600.0


// Sensibilidade aproximada:
// 0.5 V para 0.1 mg/m3
//
// Portanto:
// concentração = (V - Vbaseline) * 0.2
//


#define SENSOR_SENSITIVITY 0.2


// ======================================================
// VARIÁVEL DE CALIBRAÇÃO
// ======================================================


static esp_adc_cal_characteristics_t *adc_chars = NULL;


// ======================================================
// INICIALIZAÇÃO DO ADC
// ======================================================


void init_adc()
{
    adc1_config_width(ADC_WIDTH_BIT_12);


    adc1_config_channel_atten(
        ADC_CHANNEL,
        ADC_ATTEN_DB_0
    );


    adc_chars = (esp_adc_cal_characteristics_t *)calloc(
        1,
        sizeof(esp_adc_cal_characteristics_t)
    );


    if (adc_chars == NULL)
    {
        return;
    }


    esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_0,
        ADC_WIDTH_BIT_12,
        DEFAULT_VREF,
        adc_chars
    );
}


// ======================================================
// INICIALIZAÇÃO DO LED
// ======================================================


void init_led()
{
    gpio_reset_pin(LED_PIN);


    gpio_set_direction(
        LED_PIN,
        GPIO_MODE_OUTPUT
    );


    gpio_set_level(
        LED_PIN,
        0
    );
}


// ======================================================
// LEITURA DO SENSOR
// ======================================================


float read_particle_level()
{
    uint32_t adc_reading = 0;


    // --------------------------------------------------
    // Faz várias leituras para reduzir ruído
    // --------------------------------------------------


    for (int i = 0; i < NO_OF_SAMPLES; i++)
    {
        adc_reading += adc1_get_raw(ADC_CHANNEL);
    }


    // Média
    adc_reading /= NO_OF_SAMPLES;


    // --------------------------------------------------
    // Converte ADC para milivolts
    // --------------------------------------------------


    uint32_t voltage_mv =
        esp_adc_cal_raw_to_voltage(
            adc_reading,
            adc_chars
        );


    // Converte para volts
    float voltage = voltage_mv / 1000.0;


    // --------------------------------------------------
    // Calcula concentração de partículas
    // --------------------------------------------------


    float particles =
        (voltage - (BASELINE_VOLTAGE / 1000.0))
        * SENSOR_SENSITIVITY;


    // --------------------------------------------------
    // Evita valores negativos
    // --------------------------------------------------


    if (particles < 0)
    {
        particles = 0;
    }


    return particles;
}


// ======================================================
// TAREFA DO SENSOR
// ======================================================


void dust_sensor_task(void *pvParameters)
{
    (void)pvParameters;


    while (1)
    {
        // Liga LED
        gpio_set_level(
            LED_PIN,
            1
        );


        // Aguarda 280 us
        ets_delay_us(280);


        // Lê partículas
        float particles = read_particle_level();


        // Desliga LED
        gpio_set_level(
            LED_PIN,
            0
        );


        // ------------------------------------------------
        // MOSTRA SOMENTE O NÍVEL DE PARTÍCULAS
        // ------------------------------------------------


        Serial.print("Particulas: ");
        Serial.print(particles, 3);
        Serial.println(" mg/m3");


        // Aguarda 500 ms
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}


// ======================================================
// SETUP
// ======================================================


void setup()
{
    Serial.begin(115200);


    delay(1000);


    init_led();


    init_adc();


    xTaskCreate(
        dust_sensor_task,
        "Dust Sensor",
        4096,
        NULL,
        2,
        NULL
    );
}


// ======================================================
// LOOP
// ======================================================


void loop()
{
    delay(1000);
}
