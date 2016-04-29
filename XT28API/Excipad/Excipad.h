/*! \defgroup EXP Excipad
 *  \brief Handle Exipad and Joystick
 *
 *  This module handles the user interface with the Excipad. It handles the following:
 *
 *  1. Configure Excipad
 *  2. Get information about what the user is doing with the Excipad
 *
 *   @{
 */

#ifndef XT28API_EXCIPAD_EXCIPAD_H_
#define XT28API_EXCIPAD_EXCIPAD_H_

#include "api_lib_basic.h"
#include "XT28CANSupport.h"

/*!
 * Excipad Button Enum. Used for Excipad related functions.
 */
typedef enum {
	NONE,		/**< No Button pressed 	*/
	BUTTON_1 ,	/**< Button 1 			*/
	BUTTON_2 ,	/**< Button 2 			*/
	BUTTON_3 ,	/**< Button 3 			*/
	BUTTON_4 ,	/**< Button 4 			*/
	BUTTON_5 ,	/**< Button 5 			*/
	BUTTON_6 ,	/**< Button 6 			*/
	BUTTON_7 ,	/**< Button 7 			*/
	BUTTON_8 ,	/**< Button 8 			*/
	BUTTON_9 ,	/**< Button 9 			*/
	BUTTON_10,	/**< Button 10 			*/
	BUTTON_11,	/**< Button 11 			*/
	BUTTON_12,	/**< Button 12 			*/
	BUTTON_13,	/**< Button 13 			*/
	BUTTON_14,	/**< Button 14 			*/
	BUTTON_15,	/**< Button 15 			*/
	BUTTON_16,	/**< Button 16 			*/
	BUTTON_17,	/**< Button 17 			*/
	BUTTON_18,	/**< Button 18 			*/
	BUTTON_19,	/**< Button 19 			*/
	BUTTON_20,	/**< Button 20 			*/
	BUTTON_21	/**< Button 21 			*/
} exipadButton;



/*! \name Configure */
/*!
 * This function is mandatory to call in the setup of the program if you need to use this module.
 * This function sets up what channel the Excipad is connected. The setup require to use two data boxes in the BODAS C-API. Any channels can be used that
 * is valid by the BODAS API and is not used by any other channel.
 *
 * \param[in] CANChannel The CAN channel the Excipad is connected.
 * \param[in] buttonDataboxNr A data box number, valid range is [1 - 100]
 * \param[in] joystrickDataboxNr A data box number, valid range is [1 - 100]
 */
void EXPConfigureExcipad(uint8 CANChannel, int buttonDataboxNr, int joystrickDataboxYNr, int joystickDataboxXNr);
/** @}*/



/*! \name Get */
/*!
 * Get the last button that was pressed. If the user pressed the button again this function will return NONE,
 * will return a !lastbutton.
 *
 * \return exipadButton last pressed, toggled if pressed again.
 */
exipadButton EXPGetLastPressedButtonWithToggle(void);

/*!
 * Get the currently pressed button
 */
exipadButton EXPGetCurrentlyPressedButton(void);

/*!
 * Check if the user is currently pressing a button on the excipad
 */
bool EXPGetUserIsHoldingAButtonDown(void);

/*!
 *  Get the scaled value of the excipad joy stick.
 *
 * \return Int values in range [-400 - 400]
 */
float EXPGetJoystickScaledValueUppDown(void);

/*!
 *  Get the scaled value of the excipad joystick Left right X
 * @param setButtonState
 */
float EXPGetJoystickXScaledValueLeftRight(void);
/** @}*/



/*! \name Set */
/*!
 *  Sets the last pressed button to state NONE.
 */
void EXPSetButtonStateTo(exipadButton setButtonState);
/** @}*/

#endif /* XT28API_EXCIPAD_EXCIPAD_H_ */
/** @}*/
