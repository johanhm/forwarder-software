/*!
 * \page ADCONFIG  Configurations
 *
 * TBA
 */


/*! \defgroup ADCFG	Configurations
 * \ingroup SUPPORT
 *  \brief Pre-defined active dampening setups.
 *
 *  @{
 */

#ifndef XT28API_ACTIVEDAMPENING_ADCCONFIGURATIONS_H_
#define XT28API_ACTIVEDAMPENING_ADCCONFIGURATIONS_H_

#include "ActiveDampening.h"
#include "PendelumArmActuate.h"
#include "PendelumArmPosition.h"
#include "PendelumArmForces.h"
#include "PendelumArmPressure.h"
#include "IMU.h"
#include "api_lib_basic.h"


void ADCFGNivPIDAndSkyhookSetup(bool state);

/*!
 * A Setup that does the following:
 *
 * 1. Get PID leveling signals
 * 2. Get skyhook signals
 * 3. Add signals together from 1 and 2
 * 4. Input signal array into Sliding mode force controller, that returns an array of suggested flow
 * 5. Set reference in flow for pendulum arm actuate
 * 6. Actuate
 *
 * For this controller to work in a reasonable speed the PID leveling and Skyhook parameters need to be set high enough in order to be able to
 * Compete with a reference force. This setup does not use a reference force for each wheel.
 *
 *
 * @param state Set to true to use this setup, false and its deactivated.
 */
void ADCFGPesudoForcePIDSkyhookSlidingMode(bool state);

/*!
 * A setup that does the following:
 *
 * 1. Get PID leveling signals
 * 2. Get skyhook signals
 * 3. __Get reference force for all wheels__
 * 4. Add signals from 1, 2 and 3 together
 * 5. Input signal array into Sliding mode force controller, that returns an array of suggested flow
 * 6. Set reference in flow for pendulum arm actuate
 * 7. Actuate
 *
 * For this controller to work in a reasonable speed the PID leveling and Skyhook parameters need to be set high enough in order to be able to
 * Compete with a reference force.
 *
 * @param state Set to true to use this setup, false and its deactivated.
 */
void ADCFGPesudoForceWithOptimalForceRefPIDSkyhookSlidingMode(bool state);

/*!
 * A setup that does the following:
 *
 * 1. Get PID leveling signals
 * 2. Get PD Force signals
 * 3. Add signals from 1 and 2 together
 * 4. Set reference for pendulum arm in Current
 * 5. Actuate and pray.
 *
 * @param setHeightRef
 * @param setPhiRef
 * @param setThetaRef
 * @param forceGainPercent
 */
void ADCFGNivPIDAndForcePID(float setHeightRef, float setPhiRef, float setThetaRef, float forceGainPercent);

/*!
 * Is this a smart way do handle setting constants?
 * @param heightGain
 * @param phiGain
 * @param thetaGain
 * @param forceGain
 */
void ADCFGNivPIDAndForcePIDCfg(float heightGain, float phiGain, float thetaGain, float forceGain);

/*!
 * hello
 * @param state if on
 */
void ADCFGForceControllerTest(bool state);


#endif /* XT28API_ACTIVEDAMPENING_ADCCONFIGURATIONS_H_ */
 /** @}*/
