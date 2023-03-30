#include "scanning.h"
#include "display.h"
#include "bluetooth.h"
#include "timer.h"
#include "i2c_device.h"
#include "at24cxx.h"
#include "rx8025t.h"
#include "key.h"

void app_main(void)
{
	scanningInit();
	I2CInit();
	if(at24cxxInit())//初始化失败，不停地提示音
	{
		while(1)
		{
			gpio_set_level(BUZZ,1);
			vTaskDelay(100);
			gpio_set_level(BUZZ,0);
			vTaskDelay(500);
		}
	}
	timerInit();
	keyInit();
	btInit();
	displayInit();

	//蜂鸣器响一声
	gpio_set_level(BUZZ,1);
	vTaskDelay(200);
	gpio_set_level(BUZZ,0);
}
