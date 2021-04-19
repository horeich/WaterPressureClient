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

#ifndef WATER_PRESSURE_CLIENT_HPP
#define WATER_PRESSURE_CLIENT_HPP

#include "Client.hpp"
#include "PowerSupplyBase.hpp"

#include "ad7680.hpp"
#include "ltc3130.hpp"

namespace mbed
{

class WaterPressureClient : public Client
{

public:

    enum RESULT_STATE
    {
        SUCCESS     = (1 << 0),
        RUNNING     = (1 << 1),
        SUSPENDED   = (1 << 2),
    };

public:

    virtual void Suspend() override;
    virtual void Resume() override;

    WaterPressureClient(
        AD7680* ad7680, PowerSupplyBase* boost, PowerSupplyBase* supply1, PowerSupplyBase* supply2);

    RESULT_STATE ReadPressureAsync(Pressure& pressure);

private:
    float Sample();

private:
    AD7680* _ad7680;                             // <AD7760 16bit AD converter device>
    PowerSupplyBase* _boost;
    PowerSupplyBase* _supply1;
    PowerSupplyBase* _supply2;
    std::unique_ptr<Task<float()>> _readTask; // <Task for async sampling>
};

} // namespace mbed

#endif // WATER_PRESSURE_CLIENT_HPP