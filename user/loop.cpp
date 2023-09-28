#include "main.h"
#include "logUART.h"
#include "sdadc.h"
#include "adc.h"
#include "tim.h"

classLog log;


enum class State {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};

State state; //

extern void BT_Send_String(char *str);

void loop();

float table[][2] = { { 0.0F, 0.958 }, { 10.0F, 1.015 }, { 20.0F, 1.072 }, {
		30.0F, 1.121 }, { 40.0F, 1.180 }, { 50.0F, 1.238 }, { 60.0F, 1.298 }, {
		70.0F, 1.345 }, { 80.0F, 1.398 }, { 90.0F, 1.452 }, { 100.0F, 1.505 }, {
		110.0F, 1.570 }, { 120.0F, 1.618 }, { 130.0F, 1.676 },
		{ 140.0F, 1.731 }, { 150.0F, 1.788 }, { 160.0F, 1.844 },
		{ 170.0F, 1.900 }, { 180.0F, 1.958 }, { 190.0F, 2.010 },
		{ 200.0F, 2.069 }, { 210.0F, 2.124 }, { 220.0F, 2.179 },
		{ 230.0F, 2.237 }, { 240.0F, 2.293 }, { 250.0F, 2.345 },
		{ 260.0F, 2.113 }, { 270.0F, 2.466 }, { 280.0F, 2.521 },
		{ 290.0F, 2.588 }, { 300.0F, 2.621 }, };

float interpolatePressure(float volume) {
	//y = (x - x0) / (x1 - x0) * (y1 - y0) + y0
	if (volume <= 1.0F)
		return 0.0F;
	if (volume >= 2.621F)
		return 300.0F;

	float x0 = 0, x1 = 0;
	float y0 = 0, y1 = 0;
    int i = 0;
	for (i = 0; i < 31; i++) {
		if (volume < table[i][1]) {
			x0 = table[i-1][1];
			x1 = table[i][1];
			y0 = table[i-1][0];
			y1 = table[i][0];
			return (volume - x0) / (x1 - x0) * (y1 - y0) + y0;
		}
	}
	return 0.0F;
}

extern "C" void setup() {
	log.init(&huart1);
	log.w((char*) "Старт");
	loop();
}
volatile uint16_t adc_buffer[50] = {0};

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
  /* This is called after the conversion is completed */
	log.w((char*) "HAL_ADC_ConvCpltCallback");

}

void loop() {



	//ADC1->CR2 |= ADC_CR2_ADON;    //Запуск ADC
	//ADC1->CR2 |= ADC_CR2_SWSTART; //Start conversion of regular channels
	//ADC1->CR2 |= ADC_CR2_DMA;     //Direct memory access mode

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_buffer, 50);
	HAL_TIM_Base_Start_IT(&htim19);
	//TIM19->CR1 |= TIM_CR1_CEN; //Запуск таймера на 480Hz -> ADC1

	//HAL_SDADC_CalibrationStart(&hsdadc1, SDADC_CALIBRATION_SEQ_1);
	//while (__HAL_SDADC_GET_FLAG(&hsdadc1, SDADC_FLAG_CALIBDONE) == RESET);
	//HAL_SDADC_ConvCpltCallback(&hsdadc1);

	// HAL_SDADC_Stop(&hsdadc1);

	//HAL_SDADC_CalibrationStart(&hsdadc1, SDADC_CALIBRATION_SEQ_1);
	HAL_Delay(1000);

	HAL_SDADC_Start(&hsdadc1);
	SDADC1->CR2 |= SDADC_CR2_RSWSTART; // начало преобразования
	HAL_SDADC_PollForConversion(&hsdadc1, 1000);

	while (1) {
		//log.i((char*) "run");

		int16_t adcValue = HAL_SDADC_GetValue(&hsdadc1);
		float adcFValue = (3.281F) * (adcValue) / 32768 * 3.036 / 3.075;
		//char str[32];

		//float pressure = 0.958 + (adcFValue - 0.958)*(300-0.958)/(2.631-0.958);

		float pressure = interpolatePressure(adcFValue);

		char s[128];
		//sprintf (s,"v1=%d f=%f p=%f", adcValue, adcFValue, pressure);
		sprintf (s,"v=%d f=%.3f p=%.1f", adcValue, adcFValue, pressure);
		BT_Send_String(s);



		//log.i("v1=%d f=%f p=%f", adcValue, adcFValue, pressure);

		HAL_Delay(10);
	}

}

