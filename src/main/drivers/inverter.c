/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef INVERTER

#include "gpio.h"

#include "inverter.h"

void initInverter(void)
{
    struct {
        GPIO_TypeDef *gpio;
        gpio_config_t cfg;
    } gpio_setup = {
        .gpio = INVERTER_GPIO,
        // configure for Push-Pull
        .cfg = { INVERTER_PIN, Mode_Out_PP, Speed_2MHz } 
    };

#if defined(ANYFC) || defined(COLIBRI) || defined(REVO) || defined(VRBRAIN)
    RCC_AHB1PeriphClockCmd(INVERTER_PERIPHERAL, ENABLE);
#else
    RCC_APB2PeriphClockCmd(INVERTER_PERIPHERAL, ENABLE);
#endif
    gpioInit(gpio_setup.gpio, &gpio_setup.cfg);
}

#endif
