/*******************************************************************************
 \project   INFM_HIL_Interface
 \file      BmwCodec.cpp
 \brief     Implements the CAN codec for the BMW.
 \author    Matthias Bank, Marco Keul
 \version   1.0.0
 \date      02.12.2021
 ******************************************************************************/
#ifndef SIM_TO_DUT_INTERFACE_BMWCODEC_H
#define SIM_TO_DUT_INTERFACE_BMWCODEC_H


/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
// Project includes
#include "CodecUtilities.h"
#include "../CANConnectorCodec.h"

// System includes


/*******************************************************************************
 * CLASS DECLARATIONS
 ******************************************************************************/
namespace sim_interface::dut_connector::can{

    class BmwCodec : public CANConnectorCodec{

    public:
        // Function members

        /**
         * Constructor.
         */
        BmwCodec();

        /**
        * Converts an simulation event to a CAN/CANFD payload.
        *
        * @param event - The simulation event we want to transform into a CAN/CANFD frame payload.
        *
        * @return The CAN/CANFD frame payload.
        */
        std::vector<__u8> convertSimEventToFrame(SimEvent event) override;

        /**
        * Converts a CAN/CANFD frame to the corresponding simulation events.
        *
        * @param frame   - The frame that we want to transform.
        * @param isCanfd - Flag for CANFD frames.
        *
        * @return The simulation events that were contained in the CAN/CANFD frame.
        */
        std::vector<SimEvent> convertFrameToSimEvent(struct canfd_frame frame, bool isCanfd) override;

        // Data members

    private:
        // Function members

        /**
         * Test/Demo-function.
         *
         * @param frame
         * @param isCanfd
         * @return
         */
        std::vector<SimEvent> handleHazardFrame(struct canfd_frame frame, bool isCanfd);

        /**
         * Test/Demo-function.
         *
         * @param frame
         * @param isCanfd
         * @return
         */
        std::vector<SimEvent> handleBrakeFrame(struct canfd_frame frame, bool isCanfd);

        // Data members
        bool hostIsBigEndian;

    };

}


#endif //SIM_TO_DUT_INTERFACE_BMWCODEC_H
/*******************************************************************************
 * END OF FILE
 ******************************************************************************/