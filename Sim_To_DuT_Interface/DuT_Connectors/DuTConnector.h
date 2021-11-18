/**
 * Sim To DuT Interface
 *
 * Copyright (C) 2021 Lukas Wagenlehner
 *
 * This file is part of "Sim To DuT Interface".
 *
 * "Sim To DuT Interface" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Sim To DuT Interface" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Sim To DuT Interface".  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Lukas Wagenlehner
 * @version 1.0
 */

#ifndef SIM_TO_DUT_INTERFACE_DUTCONNECTOR_H
#define SIM_TO_DUT_INTERFACE_DUTCONNECTOR_H

#include <string>
#include <iostream>
#include <set>
#include <memory>
#include "../DuT_Connectors/ConnectorInfo.h"
#include "../Events/SimEvent.h"
#include "../Utility/SharedQueue.h"
#include "ConnectorConfig.h"

namespace sim_interface::dut_connector {
    class DuTConnector {
    public:
        explicit DuTConnector(std::shared_ptr<SharedQueue<SimEvent>> queueDuTToSim,
                              const sim_interface::dut_connector::ConnectorConfig &config);

        // return some basic information from the connector
        virtual ConnectorInfo getConnectorInfo();

        // handle event from the simulation async
        virtual void handleEvent(const SimEvent &simEvent);

// send the event to the simulation
        void sendEventToSim(const SimEvent &simEvent);

    protected:
        // determine if an event needs to be processed
        bool canHandleSimEvent(const SimEvent &simEvent);

    private:
        std::set<std::string> processableOperations;

        std::shared_ptr<SharedQueue<SimEvent>> queueDuTToSim;
    };
}

#endif //SIM_TO_DUT_INTERFACE_DUTCONNECTOR_H