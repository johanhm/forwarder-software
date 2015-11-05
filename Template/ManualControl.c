
// Private functions used only in manual control
void modes(can_Message_ts* msg_s);
void caseSwitch(can_Message_ts* msg_s);
bool toggleVariable(bool toggleTarget);
void setVariablesZero(void);


void manual_Control_Task(void)
{
	// Temporary CAN message structs.
	can_Message_ts msg_s_Excipad;		// create a struct called msg_s containing the CAN msg
	can_Message_ts msg_s_joystick;		// create a struct called msg_s containing the CAN msg

	uint8 LeftExcipad_au8[8];
	uint8 LeftExcipadNumBytes_u8 = 0;
	uint8 statusExcipad = can_getDatabox(CAN_3, 3, LeftExcipad_au8, &LeftExcipadNumBytes_u8);
	if (0 == statusExcipad)	// Databox 3 = CAN ID for Left excipad buttons
	{
		uint8 i = 0;
		//Construct the fake msg to be sent to switchCase function
		for(i = 0; i < 8; i++)
		{
			msg_s_Excipad.data_au8[i] = LeftExcipad_au8[i];
		}
		msg_s_Excipad.id_u32 = CAN_ID_LEFT_EXCIPAD_BUTTONS;
		//end constuck the fake msg.

		//start of checking logic for messages
		uint16 statusSum = 0;
		for(i = 0; i < 8; i++)
		{
			statusSum = statusSum+LeftExcipad_au8[i];
		}
		if (statusSum >= 1)
		{
			if (LeftExcipad_au8[2] == MSG_ENABLE_PENDULUM_ARM_ALL_DOWN)
			{
				caseSwitch(&msg_s_Excipad);
			}
			else if (LeftExcipad_au8[2] == MSG_ENABLE_PENDULUM_ARM_ALL_UP)
			{
				caseSwitch(&msg_s_Excipad);
			}
			else if (preStatusExcipad >= 1)
			{
				//do nothing
			}
			else
			{
				caseSwitch(&msg_s_Excipad);
			}

		}
		preStatusExcipad = statusSum; //global variable
		//end of checking logic
	}

	uint8 JoystickY_au8[8];
	uint8 JoystickYNumBytes_u8;

	//store the data from the right Excipad
	//removed address from JoystickY_au8
	if (0 == can_getDatabox(CAN_3, 4, JoystickY_au8, &JoystickYNumBytes_u8))	// DATABOX FOR CAN_ID_JOYSTICK_Y
	{
		//sned debugg msg
		//if (0 == can_sendData(CAN_1, 0xffffff, CAN_EXD_DU8, 8, JoystickY_au8)) {}
		//end send debugg msg
		//start of fake msg construktion for the modes function
		uint8 i = 0;
		for(i = 0; i<8; i++)
		{
			msg_s_joystick.data_au8[i] = JoystickY_au8[i];
		}
		msg_s_joystick.id_u32 = CAN_ID_JOYSTICK_Y;
		modes(&msg_s_joystick);
		//end of fake msg construktion for the modes function
	}

	if (zButton == 1)
	{
		++zButtonCounter;
	}
	if((zButtonCounter > 5) && (zButton==1) )
	{
		--zRampUp;

		if(zRampUp < 2)
		{
			setVariablesZero();
			zRampUp = 2;
			zButton = 0;
			zButtonCounter = 0;
		}
	}

}

// Changes output value sent to pendulum arm solenoids.
void modes(can_Message_ts* msg_s)
{
	volatile uint8 mode = DEFAULT_MODE;
	//volatile uint8 data = msg_s->data_au8;
	uint8 i,counter = 0;
	/*
	float JoyFloat=0;  //Used for intermediate calculations
   uint32 JOYx=0;
   uint32 JOYx1=0;
	 */
	for(i = 0; i < sizeofButtonStatus; i++)
	{
		if(buttonStatus[i])
		{
			mode = i;
			++counter;
		}
	}
	if (counter==0) mode = DEFAULT_MODE;

	uint16 joystickInput = JOYSTICK_Y_MID_POINT;
	if(msg_s->id_u32 == CAN_ID_JOYSTICK_Y) {
		joystickInput = ((msg_s->data_au8[1] << 8) | msg_s->data_au8[0]);		// Add the two 8bits byte0 and byte1 to a 16bit.
	}
	// joystickInput is between 350-4300.
	sint32 xLow = 24,  xHigh = -23, cLow = -28200, cHigh = 83167, scaling = 100;
	if((joystickInput < JOYSTICK_Y_LOW_POINT) || (joystickInput > JOYSTICK_Y_HIGH_POINT))
	{
		joystickInput = JOYSTICK_Y_MID_POINT;		// Set mid-point
	}
	sint32 scaledJoystickInputLow = (joystickInput*xLow+cLow)/scaling;				// Scaling x to avoid float (c = 885, x = -0,33)
	sint32 scaledJoystickInputHigh = (joystickInput*xHigh+cHigh)/scaling;			// Scaling x to avoid float (c = -58, x = 19)
	if (scaledJoystickInputHigh > 750) scaledJoystickInputHigh = 750;				//Sanity check input value
	if (scaledJoystickInputHigh < 330) scaledJoystickInputHigh = 330;				//Sanity check input value
	if (scaledJoystickInputLow > 750)  scaledJoystickInputLow = 750;					//Sanity check input value
	if (scaledJoystickInputLow < 330) scaledJoystickInputLow = 330;					//Sanity check input value

	sint16 JoyREF=0;

	//Scale Joystick input for new actuate function Joystick 350 to 4300  to -300 to 300 signal
	if((joystickInput<JOYSTICK_Y_HIGH_DEADBAND) && (joystickInput>JOYSTICK_Y_LOW_DEADBAND)){JoyREF=0;}

	else if(joystickInput>JOYSTICK_Y_HIGH_DEADBAND){JoyREF=((float)(joystickInput-JOYSTICK_Y_HIGH_DEADBAND)/(4300-JOYSTICK_Y_HIGH_DEADBAND)*(float)300);}  //Linear scaling
	else if(joystickInput<JOYSTICK_Y_LOW_DEADBAND){
		JoyREF=((float)(joystickInput-JOYSTICK_Y_LOW_DEADBAND)/(JOYSTICK_Y_HIGH_DEADBAND-350)*(float)300);

	}

	sint16 REFINV=0;
	REFINV=JoyREF-(2*JoyREF);

	switch(mode)
	{
	case INDEX_CYLINDER_FRONT_RIGHT:								//----1----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[FR]=JoyREF;
		break;

	case INDEX_CYLINDER_FRONT_LEFT:								//----2----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[FL]=JoyREF;
		break;

	case INDEX_CYLINDER_MID_RIGHT:									//----3----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[MR]=JoyREF;
		break;

	case INDEX_CYLINDER_MID_LEFT:									//----4----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[ML]=JoyREF;
		break;

	case INDEX_CYLINDER_REAR_RIGHT:									//----5----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[BR]=JoyREF;
		break;

	case INDEX_CYLINDER_REAR_LEFT:									//----6----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[BL]=JoyREF;
		break;

	case INDEX_CYLINDER_ALL_DOWN:									//----7----
		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[FR]=zRampUp;
		referenceSoleonidOutputCurrent_ma[MR]=zRampUp;
		referenceSoleonidOutputCurrent_ma[BR]=zRampUp;
		referenceSoleonidOutputCurrent_ma[FL]=zRampUp;
		referenceSoleonidOutputCurrent_ma[ML]=zRampUp;
		referenceSoleonidOutputCurrent_ma[BL]=zRampUp;



		break;

	case INDEX_CYLINDER_ALL_UP:										//----8----

		defaultSafety=0;
		referenceSoleonidOutputCurrent_ma[FR]=-zRampUp;
		referenceSoleonidOutputCurrent_ma[MR]=-zRampUp;
		referenceSoleonidOutputCurrent_ma[BR]=-zRampUp;
		referenceSoleonidOutputCurrent_ma[FL]=-zRampUp;
		referenceSoleonidOutputCurrent_ma[ML]=-zRampUp;
		referenceSoleonidOutputCurrent_ma[BL]=-zRampUp;
		break;

	case INDEX_CYLINDER_TILT_Y:										//----9---- (Pitch)

		if(joystickInput > JOYSTICK_Y_HIGH_DEADBAND)				//Low signal
		{
			defaultSafety=0;
			referenceSoleonidOutputCurrent_ma[FR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[FL]=JoyREF;
			referenceSoleonidOutputCurrent_ma[BR]=REFINV;
			referenceSoleonidOutputCurrent_ma[BL]=REFINV;

		}
		else if(joystickInput < JOYSTICK_Y_LOW_DEADBAND)			//High signal
		{

			defaultSafety=0;
			referenceSoleonidOutputCurrent_ma[FR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[FL]=JoyREF;
			referenceSoleonidOutputCurrent_ma[BR]=REFINV;
			referenceSoleonidOutputCurrent_ma[BL]=REFINV;
		}
		else if(joystickInput < JOYSTICK_Y_HIGH_DEADBAND && joystickInput > JOYSTICK_Y_LOW_DEADBAND)
		{
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
		}
		break;

	case INDEX_CYLINDER_TILT_X:										//----9---- (roll)
		if(joystickInput > JOYSTICK_Y_HIGH_DEADBAND)			//Low signal
		{
			defaultSafety=0;
			referenceSoleonidOutputCurrent_ma[FR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[MR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[BR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[FL]=REFINV;
			referenceSoleonidOutputCurrent_ma[ML]=REFINV;
			referenceSoleonidOutputCurrent_ma[BL]=REFINV;

		}
		else if(joystickInput < JOYSTICK_Y_LOW_DEADBAND)			//High signal
		{


			defaultSafety=0;
			referenceSoleonidOutputCurrent_ma[FR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[MR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[BR]=JoyREF;
			referenceSoleonidOutputCurrent_ma[FL]=REFINV;
			referenceSoleonidOutputCurrent_ma[ML]=REFINV;
			referenceSoleonidOutputCurrent_ma[BL]=REFINV;
		}
		else if(joystickInput < JOYSTICK_Y_HIGH_DEADBAND && joystickInput > JOYSTICK_Y_LOW_DEADBAND)
		{
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
		}
		break;

	case INDEX_SIMPLE_CONTROL:

		//Simple control enabeld
		break;

	}
}

// Switches active mode for the forwarder pendulum arms.
void caseSwitch(can_Message_ts* msg_s)
{
	volatile bool status = 0;
	uint8 i=0;
	if(msg_s->id_u32 == CAN_ID_LEFT_EXCIPAD_BUTTONS) 						//Check CAN ID
	{
		if(msg_s -> data_au8[3] == MSG_ENABLE_PENDULUM_ARM_FRONT_RIGHT) 		//MSG_ENABLE_PENDULUM_ARM_FRONT_RIGHT
		{
			status = toggleVariable(INDEX_CYLINDER_FRONT_RIGHT);
			for (i = 0; i < 6; i++) {
				referenceSoleonidOutputCurrent_ma[i]=0;
			}					//Set all PWM-outputs (to solenoids) to zero
			setVariablesZero();												//Reset buttonStatus to ensure only one is active
			buttonStatus[INDEX_CYLINDER_FRONT_RIGHT] = status;				//Set active bit for buttonStatus
			ACTIVE_FORCE_CONTROL = 0; //disable controller
		}
		else if(msg_s->data_au8[3] == MSG_ENABLE_PENDULUM_ARM_FRONT_LEFT)	//MSG_ENABLE_PENDULUM_ARM_FRONT_LEFT
		{
			status = toggleVariable(INDEX_CYLINDER_FRONT_LEFT);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			buttonStatus[INDEX_CYLINDER_FRONT_LEFT] = status;
			ACTIVE_FORCE_CONTROL=0; //disable controller
		}
		else if(msg_s->data_au8[4] == MSG_ENABLE_PENDULUM_ARM_MID_RIGHT)	//MSG_ENABLE_PENDULUM_ARM_MID_RIGHT
		{
			status = toggleVariable(INDEX_CYLINDER_MID_RIGHT);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			buttonStatus[INDEX_CYLINDER_MID_RIGHT] = status;
			ACTIVE_FORCE_CONTROL=0; //disable controller
		}
		else if(msg_s->data_au8[3] == MSG_ENABLE_PENDULUM_ARM_MID_LEFT)		//MSG_ENABLE_PENDULUM_ARM_MID_LEFT
		{
			status = toggleVariable(INDEX_CYLINDER_MID_LEFT);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			buttonStatus[INDEX_CYLINDER_MID_LEFT] = status;
			ACTIVE_FORCE_CONTROL=0; //disable controller
		}

		else if(msg_s->data_au8[5] == MSG_ENABLE_PENDULUM_ARM_REAR_RIGHT)	//MSG_ENABLE_PENDULUM_ARM_REAR_RIGHT
		{
			status = toggleVariable(INDEX_CYLINDER_REAR_RIGHT);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			buttonStatus[INDEX_CYLINDER_REAR_RIGHT] = status;
			ACTIVE_FORCE_CONTROL=0; //disable controller
		}
		else if(msg_s->data_au8[4] == MSG_ENABLE_PENDULUM_ARM_REAR_LEFT)	//MSG_ENABLE_PENDULUM_ARM_REAR_LEFT
		{
			status = toggleVariable(INDEX_CYLINDER_REAR_LEFT);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			buttonStatus[INDEX_CYLINDER_REAR_LEFT] = status;
			ACTIVE_FORCE_CONTROL=0; //disable controller
		}
		else if(msg_s->data_au8[2] == MSG_ENABLE_PENDULUM_ARM_ALL_DOWN)		//MSG_ENABLE_PENDULUM_ARM_ALL_DOWN
		{
			setVariablesZero();
			ACTIVE_FORCE_CONTROL=0; //disable controller
			buttonStatus[INDEX_CYLINDER_ALL_DOWN] = 1;
			zButton = 1;
			zButtonCounter = 0;
			++zRampUp;
			if(zRampUp > 50)
			{
				zRampUp = 50;
			}
		}
		else if(msg_s->data_au8[2] == MSG_ENABLE_PENDULUM_ARM_ALL_UP)		//MSG_ENABLE_PENDULUM_ARM_ALL_UP
		{
			setVariablesZero();
			ACTIVE_FORCE_CONTROL=0; //disable controller
			buttonStatus[INDEX_CYLINDER_ALL_UP] = 1;
			zButton = 1;
			zButtonCounter = 0;
			++zRampUp;
			if(zRampUp > 50)
			{
				zRampUp = 50;
			}
		}
		else if(msg_s->data_au8[1] == MSG_ENABLE_PENDULUM_ARM_TILT_Y)		//MSG_ENABLE_PENDULUM_ARM_MID_LEFT
		{
			status = toggleVariable(INDEX_CYLINDER_TILT_Y);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			ACTIVE_FORCE_CONTROL=0; //disable controller
			buttonStatus[INDEX_CYLINDER_TILT_Y] = status;
		}
		else if(msg_s->data_au8[1] == MSG_ENABLE_PENDULUM_ARM_TILT_X)		//MSG_ENABLE_PENDULUM_ARM_MID_LEFT
		{
			status = toggleVariable(INDEX_CYLINDER_TILT_X);
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			ACTIVE_FORCE_CONTROL=0; //disable controller
			buttonStatus[INDEX_CYLINDER_TILT_X] = status;
		}
		else if(msg_s->data_au8[2] == MSG_ENABLE_SIMPLE_CONTROL)		//MSG_ENABLE_PENDULUM_simple controll
		{
			status = toggleVariable(INDEX_SIMPLE_CONTROL);

			ACTIVE_FORCE_CONTROL=status;  //Enable or disable force control
			for(i=0;i<6;i++){
				referenceSoleonidOutputCurrent_ma[i]=0;}
			setVariablesZero();
			buttonStatus[INDEX_SIMPLE_CONTROL] = status;
		}
	}
}

bool toggleVariable(bool toggleTarget)
{
	if (buttonStatus[toggleTarget])
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void setVariablesZero(void)
{
	uint8 i;
	for( i = 0; i < sizeofButtonStatus; i++)
	{
		buttonStatus[i] = 0;
	}
}


// CONFIGURATION FUNCTIONS, SHOULD MOVE TO OTHER FILE

void can_1_BusOffCallback(uint16 status_u16)
{
	static uint16 temp_u16;

	temp_u16 = status_u16;
} // can_1_BusOffCallback

void can_3_BusOffCallback(uint16 status_u16)
{
	static uint16 temp_u16;

	temp_u16 = status_u16;
} // can_3_BusOffCallback


void appl_configOutputs(void)
{
	/* POH's */
	out_cfg(OUT_1_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_2_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_3_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_4_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_5_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_6_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_7_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_8_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_9_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_10_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_11_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);
	out_cfg(OUT_12_POH_CL, cfg_debounce, cfg_frequency, cfg_minLoad, cfg_maxLoad);

	out_cfgPI(OUT_1_POH_CL,157,136);
	out_cfgPI(OUT_2_POH_CL,157,136);
	out_cfgPI(OUT_3_POH_CL,157,136);
	out_cfgPI(OUT_4_POH_CL,157,136);
	out_cfgPI(OUT_5_POH_CL,157,136);
	out_cfgPI(OUT_6_POH_CL,157,136);
	out_cfgPI(OUT_7_POH_CL,157,136);
	out_cfgPI(OUT_8_POH_CL,157,136);
	out_cfgPI(OUT_9_POH_CL,157,136);
	out_cfgPI(OUT_10_POH_CL,157,136);
	out_cfgPI(OUT_11_POH_CL,157,136);
	out_cfgPI(OUT_12_POH_CL,157,136);


} // appl_configOutputs

void appl_configInputs(void)
{
	/* inputs angle sensors 1-6 */
	in_cfgVoltageInput(IN_1_AIV, 1000, 4000, 100, 200, 4800, 200); //angle sensor Front right
	in_cfgVoltageInput(IN_2_AIV, 1000, 4000, 100, 200, 4800, 200); //angle sensor Front left
	in_cfgVoltageInput(IN_3_AIV, 1000, 4000, 100, 200, 4800, 200); //angle sensor Mid right
	in_cfgVoltageInput(IN_4_AIV, 1000, 4000, 100, 200, 4800, 200); //angle sensor Mid left
	in_cfgVoltageInput(IN_5_AIV, 1000, 4000, 100, 200, 4800, 200); //angle sensor Rear right
	in_cfgVoltageInput(IN_6_AIV, 1000, 4000, 100, 200, 4800, 200); //angle sensor Rear left
	/* ... */

	/* pressure sensors 7-18 */
	in_cfgVoltageInput(IN_7_AIV, 1000, 4000, 100, 200, 4800, 200);  //Front right A
	in_cfgVoltageInput(IN_8_AIV, 1000, 4000, 100, 200, 4800, 200);  //Front right B
	in_cfgVoltageInput(IN_9_AIV, 1000, 4000, 100, 200, 4800, 200);  //Front left A
	in_cfgVoltageInput(IN_10_AIV, 1000, 4000, 100, 200, 4800, 200); //Front left B
	in_cfgVoltageInput(IN_11_AIV, 1000, 4000, 100, 200, 4800, 200); //Mid right A
	in_cfgVoltageInput(IN_12_AIV, 1000, 4000, 100, 200, 4800, 200); //Mid right B
	in_cfgVoltageInput(IN_13_AIV, 1000, 4000, 100, 200, 4800, 200); //Mid left A
	in_cfgVoltageInput(IN_14_AIV, 1000, 4000, 100, 200, 4800, 200); //Mid left B
	in_cfgVoltageInput(IN_15_AIV, 1000, 4000, 100, 200, 4800, 200); //Rear right A
	in_cfgVoltageInput(IN_16_AIV, 1000, 4000, 100, 200, 4800, 200); //Rear right B
	in_cfgVoltageInput(IN_17_AIV, 1000, 4000, 100, 200, 4800, 200); //Rear left A
	in_cfgVoltageInput(IN_18_AIV, 1000, 4000, 100, 200, 4800, 200); //Rear left B

	/* inputs AIC_AI10V - activate current measuring */
	in_setCurrentMeasurement(1, ON);

} // appl_configInputs

/**************************************************************************************************/


/***************************************************************************************************
 *  FUNCTION:      appl_setVpOnFirst
 */
/**\brief         All VP's are switched on.
 *
 * This function must be called cyclically.
 *
 * \param[in]      -
 *
 * \return         TRUE: Setting VPs on was finished. All VPs are switched on.
 *                 FALSE: Setting VPs on was not finished.
 */
/**************************************************************************************************/
void appl_Task_1(void) {
  uint16 diag_u16;
  safout_ts safout_s;
  uint16 appl_moError_u16;
  bool appl_VpFirstOnStatus_l;


  /* trigger check point 0 */
  sys_triggerTC(0);

  /* switch on VP's (power supplies for outputs) for the first time */
  appl_VpFirstOnStatus_l = appl_setVpOnFirst();
  if (FALSE != appl_VpFirstOnStatus_l) {
	  //nothing
  }

  /*
   * evaluation from the shut down test after startup - the channel is only available after
   * an execcuted shut down test !!!
   */

  diag_u16 = safout_getStatus (SAFOUT_41_POH);

  if (SDT_EXECUTE_DU16 == (diag_u16 & SDT_EXECUTE_DU16)) {
    // execution from a shut down test pending (no power supply for the outputs) or
    // still running - channel is not available
    // ...
  } else {
    if (SDT_FAILED_DU16 == (diag_u16 & SDT_FAILED_DU16)) {
      // shut down failed - channel is not available

      // short circuits to the supply side from the shut down switch(es) could be ignored
      // for a improved availability
      safout_ignoreError(SAFOUT_41_POH);
      // ...
    }

    if (SDT_CURRENT_TIMEOUT_DU16 == (diag_u16 & SDT_CURRENT_TIMEOUT_DU16))
    {
      // no current flow through the shut down switch(es)within the stated time (open load etc.)
      //
      // ...
    }
  }


  // ...


  /*
   * shut down test is finished, channel is available - evaluation from the diagnosis status
   */

  diag_u16 = safout_getStatusxt (SAFOUT_41_POH, &safout_s);

  if (NO_AVAILABILITY_DU16 == (diag_u16 & NO_AVAILABILITY_DU16))
  {
    // channel is deactivated - check the extended information for the error evaluation

    // ...

    // short circuits to the supply side from the shut down switch could be ignored
    // for a improved availability
    safout_ignoreError(SAFOUT_41_POH);

    // ...

  }
  else
  {
    // channel is available
    safout (SAFOUT_41_POH, OUT_1_POH_CL, 400);

    // ...
  }

  // Check if there is a hardware monitor error
  appl_moError_u16 = mo_getError();
  if (0 != appl_moError_u16)
  {
    // There is a harware monitor error

    // Inform the User by using CAN_1 (note: Transmitter of CAN_2, CAN_3 and CAN_4 are disabled
    // at several errors)

    switch (appl_moError_u16)
    {
      case MOF_VSS_1_DU16:
      {
        // sensor supply 1: voltage is out of range
        // reaction: e.g. ignore all signals of sensors which are supplied by VSS_1
        break;
      }

      case MOF_3V3_1V5_HW_DU16  :
      {
        // ECU internal voltages 1,5V/3,3V (one or both) are out of range
        // reaction: e.g.
        // - set outputs for an emergency mode such as speed < 5km/h:
        //   out(..);
        // - muting (include reset of hardware ciruit for 3V3/1V5 cut off and unlock VP on
        //   and EMCY on):
        //   mo_ignoreError();
        // - set VPs  on:
        //   sys_setVP(VP_1, ON);
        //   sys_setVP(VP_2, ON);
        // - set EMCY on:
        //   emcy_setPowerOn();
        // - If ECU internal voltages 1,5V/3,3V are in range than there is power on outputs.
        break;
      }

      //...

      default:
      {
        break;
      }
    } // switch
  } // if

  // ...
} // appl_Task_1

void appl_IdleTask_1(void)
{
  // ...
} // appl_IdleTask_1

bool appl_setVpOnFirst(void)
{
	static uint8 firstVpOnState_u8 = 0;
	uint16 status_u16;
	bool returnValue_l = TRUE;

	if (0xFF != firstVpOnState_u8)
	{
		returnValue_l = FALSE;
		switch (firstVpOnState_u8)
		{
		case 0:
		{
			status_u16 = sys_setVP(VP_1, ON);

			switch (status_u16)
			{
			case SYSSETVP_OK_DU16:
			{
				// ok
				// VP_1 is ready (and Shut off path test are finished). Goto VP_2.
				firstVpOnState_u8 = 1;
				break;
			}

			case SYSSETVP_ON_LOCKED_DU16:
			{
				// Setting VP ON is locked (because of hardware monitor error)
				// Unlock is possible by calling mo_ignoreError() (muting of hardware monitor errors)
				// Setting VP on must be repeated.
				// mo_ignoreError();
				break;
			}

			case SYSSETVP_SOP_TEST_RUNNING_DU16:
			{
				// SOP test is running. It must be waited.
				break;
			}

			case SYSSETVP_INH_ACTIVE_DU16:
			{
				// Power supply is disabled by inhibit or output-enable input pin
				// Setting VP on must be repeated.
				break;
			}

			case SYSSETVP_NOUBATT_DU16:
			{
				// No power supply connected (at KL30).
				// Setting VP on must be repeated.
				break;
			}

			case SYSSETVP_NOVP_DU16:
			{
				// Power supply cannot be switched on for switch-off tests.
				// Check for error messages indicating that VP is switched off due to hardware or
				// monitoring errors.
				// Setting VP on must be repeated.
				break;
			}

			case SYSSETVP_UNEXPECTED_VP_DU16:
			{
				// Unexpected state of power supply for power stages previous to switch-off tests.
				// Setting VP on must be repeated.
				break;
			}

			default:
			{
				break;
			}
			} // switch

			break;
		} // case

		case 1:
		{
			status_u16 = sys_setVP(VP_2, ON);

			switch (status_u16)
			{
			case SYSSETVP_OK_DU16:
			{
				// ok
				// First time is finished.
				firstVpOnState_u8 = 0xFF;
				returnValue_l = TRUE;
				break;
			}

			case SYSSETVP_ON_LOCKED_DU16:
			{
				// Setting VP ON is locked (because of hardware monitor error)
				// Unlock is possible by calling mo_ignoreError() (muting of hardware monitor errors)
				// Setting VP on must be repeated.
				// mo_ignoreError();
				break;
			}

			default:
			{
				break;
			}
			} // switch
			break;
		} // case

		default:
		{
			break;
		}
		} // switch
	} // if
	// else: It is not the first time. So nothing must be done.

	return returnValue_l;
} // appl_setVpOnFirst


void appl_AfterRunFunc(void)
{
  // ...
} // appl_AfterRunFunc

void appl_updateDiagData(void)
{
  // ...
} //appl_updateDiagData(void)

void appl_EmergencyTask(void)
{
	sys_triggerTC(0);
}

void appl_ErrorHandler(uint16 errorCode_u16, uint8 errorParam_u8)
{
  static uint16 tempErrCode_u16;
  static uint8  tempErrParam_u8;

  tempErrCode_u16 = errorCode_u16;
  tempErrParam_u8 = errorParam_u8;
} // appl_ErrorHandler


void appl_setDefaults(void)
{
  // ...
} // appl_setDefaults




