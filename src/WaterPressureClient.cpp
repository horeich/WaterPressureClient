/*
 * Copyright (c) 2021, HOREICH UG.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WaterPressureClient.hpp"

#define TRACE_GROUP "WPCl"

using namespace mbed;

WaterPressureClient::WaterPressureClient(AD7680* ad7680, PowerSupplyBase* boost, PowerSupplyBase* supply1, PowerSupplyBase* supply2) :
    _ad7680(ad7680),
    _boost(boost),
    _supply1(supply1),
    _supply2(supply2),
    _readTask(CreateTask(this, &WaterPressureClient::Sample))
{   
    Resume();
}

void WaterPressureClient::Resume()
{
    tr_debug("WaterPressureClient::%s", __func__);
    _flags.clear();
}

void WaterPressureClient::Suspend()
{
    tr_debug("WaterPressureClient::%s", __func__);
    _flags.set(CLIENT_SUSPEND);
}

WaterPressureClient::RESULT_STATE WaterPressureClient::ReadPressureAsync(Pressure& pressure)
{
    if (!(_flags.get() & CLIENT_SUSPEND))
    {
        if (_readTask->Run(pressure.get()))
        {
            tr_debug("Avg water pressure [mA]: %f", pressure.get());
            return SUCCESS;
        }
        return RUNNING;
    }
    return SUSPENDED;
}

float WaterPressureClient::Sample()
{
    tr_debug("WaterPressureClient::%s", __func__);

    _supply1->enable(this);
    _supply2->enable(this);
    _boost->enable(this);

    int samples = MBED_CONF_WATER_PRESSURE_CLIENT_NUM_OF_SAMPLES;
    uint32_t index = 0;
    float CMA = 0.0f;

    while (index < samples)
    {
        int state = _flags.wait_any_for(CLIENT_SUSPEND, 0s, false);
        if (state > 0 && CLIENT_SUSPEND)
        {
            tr_debug("=> Water pressure client suspended");
            break;
        }
        float value = _ad7680->read_uint16();
        CMA = (index * CMA + value) / (index + 1);
        index++;
    }

    _ad7680->disable(); // configures pins for deep sleep

    _supply1->disable(this);
    _supply2->disable(this);
    _boost->disable(this);

    float refRes = MBED_CONF_WATER_PRESSURE_CLIENT_REFERENCE_RESISTANCE;
    CMA = _ad7680->reference_voltage() * CMA / 0xFFFF / refRes * 1000.0f;

    return (CMA - 4.0f) * (20.0f/16.0f);
}