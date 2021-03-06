#include "PendelumArmForces.h"

//Private prototytpes
static void calculateForceOnCylinderChambers(void);
static void calculateLoadForceOnCylinder(void);
static void calculateVerticalForceOnWheelAndTotalMass(void);

static float forceRelationshipFromLoadToGround(uint16 cylinderPoss_mm);
static sint32 convertCylinderLoadForceToVericalOnWheel(uint16 cylinderPoss_mm, sint16 cylinderLoadForce);

static void calculateMassCenterLocation(void);
static void calculateOptimalForceForAllWheels(void);

static int pressureData_Bar[INDEX_SIZE_PRESSURESENS] = {0};
static int posData_mm[SUM_WHEELS] = {0};
void PAFUppdateForceOnWheelsDataUsing(int pressureDataInput[INDEX_SIZE_PRESSURESENS], int posDataInput[SUM_WHEELS]) {
	/*! fixme
	 * Change the functions called in this function so they return and array to send to the next function
	 * to lessend the use of global arrays. The way is exists now is duh to histrical reasons and for compadibility
	 * with sending stuff on CAN
	 */

	/* Update local data */
	int i = 0;
	for (i = 0; i < INDEX_SIZE_PRESSURESENS; i++) {
		pressureData_Bar[i] = pressureDataInput[i];
	}
	int wheel = 0;
	for (wheel = 0; wheel < SUM_WHEELS; wheel++) {
		posData_mm[wheel] = posDataInput[wheel];
	}

	/* Calculate corresponding forces at cylinder chambers and total */
	calculateForceOnCylinderChambers();

	/* Calculate Load force Fa-Fb decaN  (N/10) */
	calculateLoadForceOnCylinder();

	calculateVerticalForceOnWheelAndTotalMass();

	calculateMassCenterLocation();

	calculateOptimalForceForAllWheels();
}

static uint32 forceChamberData[INDEX_SIZE_PRESSURESENS] = {0};
static void calculateForceOnCylinderChambers(void) {
	int x = 0;
	for (x = 0; x < INDEX_SIZE_PRESSURESENS; x++) {
		if (x % 2 == 0) {
			forceChamberData[x] = 100 * pressureData_Bar[x] * (float)CYLINDER_PUSH_AREA_SIDE_A1_m2;
		} else {
			forceChamberData[x] = 100 * pressureData_Bar[x] * (float)CYLINDER_PUSH_AREA_SIDE_B2_m2;
		}
	}
}

static int messuredForceCylinderLoad_deciN[SUM_WHEELS] = {0};
static void calculateLoadForceOnCylinder(void) {
	uint8 x = 0;
	uint8 i = 0;
	for (x = 0; x <SUM_WHEELS; x++) {
		messuredForceCylinderLoad_deciN[x] = (forceChamberData[i] - forceChamberData[i+1]);
		i = i + 2;
	}
}

static sint32 calculatedVerticalForceOnWheel[SUM_WHEELS] = {0};
static sint32 sumOfVerticalForce = 0;
static void calculateVerticalForceOnWheelAndTotalMass(void) {
	int wheel = 0;
	sumOfVerticalForce = 0;
	/* Get vertical force depending on pendulum arm current position */
	for (wheel = 0; wheel < SUM_WHEELS; wheel++) {
		calculatedVerticalForceOnWheel[wheel] = convertCylinderLoadForceToVericalOnWheel(posData_mm[wheel], messuredForceCylinderLoad_deciN[wheel]);
		bool wheelIsConnectedToGround = (calculatedVerticalForceOnWheel[wheel] > 0);
		if (wheelIsConnectedToGround) {
			sumOfVerticalForce = sumOfVerticalForce + calculatedVerticalForceOnWheel[wheel];
		}
	}
}

void PAFSendCylinderChamberForceOnCAN(uint8 CANChannel, uint32 backID, uint32 middleID, uint32 frontID) {
	CANSend_uint16( CANChannel, backID,
			forceChamberData[ANALOG_REAR_RIGHT_PENDULUM_PRESSURE_A],
			forceChamberData[ANALOG_REAR_RIGHT_PENDULUM_PRESSURE_B],
			forceChamberData[ANALOG_REAR_LEFT_PENDULUM_PRESSURE_A],
			forceChamberData[ANALOG_REAR_LEFT_PENDULUM_PRESSURE_B]
	);
	CANSend_uint16( CANChannel, middleID,
			forceChamberData[ANALOG_MID_RIGHT_PENDULUM_PRESSURE_A],
			forceChamberData[ANALOG_MID_RIGHT_PENDULUM_PRESSURE_B],
			forceChamberData[ANALOG_MID_LEFT_PENDULUM_PRESSURE_A],
			forceChamberData[ANALOG_MID_LEFT_PENDULUM_PRESSURE_B]
	);
	CANSend_uint16( CANChannel, frontID,
			forceChamberData[ANALOG_FRONT_RIGHT_PENDULUM_PRESSURE_A],
			forceChamberData[ANALOG_FRONT_RIGHT_PENDULUM_PRESSURE_B],
			forceChamberData[ANALOG_FRONT_LEFT_PENDULUM_PRESSURE_A],
			forceChamberData[ANALOG_FRONT_LEFT_PENDULUM_PRESSURE_B]
	);
}

void PAFSendCylinderLoadForceOnCAN(uint8 CANChannel, uint32 middleAndBackID, uint32 frontID) {

	CANSend_sint16( CANChannel, middleAndBackID,
			messuredForceCylinderLoad_deciN[MR],
			messuredForceCylinderLoad_deciN[ML],
			messuredForceCylinderLoad_deciN[BR],
			messuredForceCylinderLoad_deciN[BL]
	);

	uint8 data_au8_sms_13[4] = {0}; //Load force
	//Construct msg SMS 13
	data_au8_sms_13[0] = messuredForceCylinderLoad_deciN[FR];
	data_au8_sms_13[1] = messuredForceCylinderLoad_deciN[FR] >> 8;
	data_au8_sms_13[2] = messuredForceCylinderLoad_deciN[FL];
	data_au8_sms_13[3] = messuredForceCylinderLoad_deciN[FL] >> 8;

	can_sendData(CANChannel, frontID, CAN_EXD_DU8, 4, data_au8_sms_13);
}

void PAFSendVerticalWheelForceOnCAN(uint8 CANChannel, uint32 middleAndBackID, uint32 frontID) {

	CANSend_sint16( CANChannel, middleAndBackID,
			calculatedVerticalForceOnWheel[MR],
			calculatedVerticalForceOnWheel[ML],
			calculatedVerticalForceOnWheel[BR],
			calculatedVerticalForceOnWheel[BL]
	);

	uint8 data_au8_sms_13[4] = {0}; //Load force
	//Construct msg SMS 13
	data_au8_sms_13[0] = calculatedVerticalForceOnWheel[FR];
	data_au8_sms_13[1] = calculatedVerticalForceOnWheel[FR] >> 8;
	data_au8_sms_13[2] = calculatedVerticalForceOnWheel[FL];
	data_au8_sms_13[3] = calculatedVerticalForceOnWheel[FL] >> 8;

	can_sendData(CANChannel, frontID, CAN_EXD_DU8, 4, data_au8_sms_13);

}

static float forceRelationshipFromLoadToGround(uint16 cylinderPoss_mm) {

	/* Constants */
	const float bxy 	= 0.4991;
	const float l1 		= 1.008;
	const float l3 		= 1.211;
	const float theta3 	= 0.2983;
	const float theta2 	= 0.3370;
	const float pi 		= M_PI;


	float stroke = 0.82 + (cylinderPoss_mm / 1000.0);
	float alpha  = acos( (pow(bxy,2) + pow(l3,2) - pow(stroke,2)) / (2 * bxy * l3) );
	float theta  = alpha + theta2 + theta3;
	float theta1 = theta - theta3;
	float beta1  = acos((pow(stroke,2) + pow(bxy,2) - pow(l3,2)) / (2 * stroke * bxy));
	float gamma  = 0;
	float forceGroundRelationshipConstant = 0;

	if (beta1 < pi / 2 + theta2) {
		gamma = pi / 2 - (beta1 - theta2);
	} else {
		gamma = -(beta1 - (pi / 2 + theta2));
	}

	if (theta1 > pi/2) {
		forceGroundRelationshipConstant = l3 * ((sin(gamma) * cos(theta1 - pi / 2) - cos(gamma) * sin(theta1 - pi / 2)) / (l1 * cos(theta - pi / 2)));
	} else if (gamma > 0) {
		forceGroundRelationshipConstant = l3 * ((sin(gamma) * cos(pi / 2 - theta1) + cos(gamma) * sin(pi / 2 - theta1)) / (l1 * cos(pi / 2 - theta)));
	} else {
		forceGroundRelationshipConstant = l3 * ((sin(gamma) * cos(pi / 2 - theta1) + cos(gamma) * sin(pi / 2 - theta1)) / (l1 * cos(pi / 2 - theta)));
	}
	return forceGroundRelationshipConstant;  /* FG/FL Relationship */
}

static sint32 convertCylinderLoadForceToVericalOnWheel(uint16 cylinderPoss_mm, sint16 cylinderLoadForce) {
	float relativeConstant = forceRelationshipFromLoadToGround(cylinderPoss_mm);
	sint32 forceOnWheelVertical_N = relativeConstant * cylinderLoadForce;
	return forceOnWheelVertical_N;
}

sint32 PAFConvertVerticalForceOnWheelToCylinderLoadForce(uint16 cylinderPoss_mm, sint32 verticalForce) {
	float relativeConstant = forceRelationshipFromLoadToGround(cylinderPoss_mm);
	sint32 forceOnCylinder_N = verticalForce / relativeConstant;
	return forceOnCylinder_N;
}

// MARK: Mass center functions
static float massCenterLocationX_m = 0;
static float massCenterLocationY_m = 0;
static void calculateMassCenterLocation(void) {
	float lengthOfForwarder_m = LENGTH_OF_FORWARDER_m;
	float lengthToMidOfForwarder_m = LENGTH_TO_MID_OFF_FORWARDER_m;
	float widthOfForwarder_m = WIDTH_OF_FORWARDER_m;

	float sumOfForcesOnWheels_N = sumOfVerticalForce;

	massCenterLocationX_m = 1 / sumOfForcesOnWheels_N * (calculatedVerticalForceOnWheel[FR] + calculatedVerticalForceOnWheel[MR] + calculatedVerticalForceOnWheel[BR]) * widthOfForwarder_m;
	massCenterLocationY_m = 1 / sumOfForcesOnWheels_N * ((calculatedVerticalForceOnWheel[ML] + calculatedVerticalForceOnWheel[MR]) * lengthToMidOfForwarder_m + (calculatedVerticalForceOnWheel[BL] + calculatedVerticalForceOnWheel[BR]) * lengthOfForwarder_m);

}

void PAFSendMassCenterLocationOnCAN(uint CANChannel, uint32 ID) {
	float lengthOfForwarder_m = LENGTH_OF_FORWARDER_m;
	float widthOfForwarder_m = WIDTH_OF_FORWARDER_m;
	int convertToPercent = 100;

	sint16 massCenterLocationX_s16_10m = (massCenterLocationX_m / widthOfForwarder_m  * convertToPercent);
	sint16 massCenterLocationY_s16_10m = (massCenterLocationY_m / lengthOfForwarder_m * convertToPercent);

	CANSend_sint16(CANChannel, ID,
			massCenterLocationX_s16_10m,
			massCenterLocationY_s16_10m,
			0,
			0
	);
}

static int forceReferenceOptimalDistrubutionVertical_N[SUM_WHEELS] = {0};
static int forceRefOptDispForCylinderLoad_N[SUM_WHEELS] = {0};
static void calculateOptimalForceForAllWheels(void) {
	const float lengthOfForwarder_m 		= LENGTH_OF_FORWARDER_m;
	const float lengthToMidOfForwarder_m 	= LENGTH_TO_MID_OFF_FORWARDER_m;
	const float widthOfForwarder_m 			= WIDTH_OF_FORWARDER_m;

	/* Dynamic center of FORCE */
	float massCenterLocationX_mLocal = massCenterLocationX_m;
	float massCenterLocationY_mLocal = massCenterLocationY_m;
	/* end */

	/* kMid Scaling constant calculations */
	const float maxValueAmplitude = 0.3388; /* got this value from finding the point where kM, kF and kB intercet */
	const float offset = asin(1) - (float)lengthToMidOfForwarder_m / (lengthOfForwarder_m - lengthToMidOfForwarder_m) * M_PI / 2;

	float omega = (float)massCenterLocationY_mLocal / lengthToMidOfForwarder_m * (float)M_PI / 2.0;
	float kMidScalingConstant = sin(omega) * maxValueAmplitude;
	if (massCenterLocationY_mLocal > lengthToMidOfForwarder_m) {
		omega = massCenterLocationY_mLocal / (lengthOfForwarder_m - lengthToMidOfForwarder_m) * M_PI / 2 + offset;
		kMidScalingConstant = sin(omega) * maxValueAmplitude;
	}

	/* Calculate scaling constants */
	float kFront = (lengthOfForwarder_m - massCenterLocationY_mLocal - (lengthOfForwarder_m - lengthToMidOfForwarder_m) * kMidScalingConstant) / lengthOfForwarder_m;
	float kLeft  = 1 - massCenterLocationX_mLocal / widthOfForwarder_m;
	float kMid   = kMidScalingConstant;
	float kRear  = (massCenterLocationY_mLocal - lengthToMidOfForwarder_m * kMidScalingConstant) / lengthOfForwarder_m;
	float kRight = massCenterLocationX_mLocal / widthOfForwarder_m;

	/* Optimal force reference vector vertical */
	forceReferenceOptimalDistrubutionVertical_N[FL]  = kFront * kLeft  * sumOfVerticalForce;
	forceReferenceOptimalDistrubutionVertical_N[FR]  = kFront * kRight * sumOfVerticalForce;
	forceReferenceOptimalDistrubutionVertical_N[ML]  = kMid   * kLeft  * sumOfVerticalForce;
	forceReferenceOptimalDistrubutionVertical_N[MR]  = kMid   * kRight * sumOfVerticalForce;
	forceReferenceOptimalDistrubutionVertical_N[BL]  = kRear  * kLeft  * sumOfVerticalForce;
	forceReferenceOptimalDistrubutionVertical_N[BR]  = kRear  * kRight * sumOfVerticalForce;

	/* Convert to optimalFOrceRef on cylinder. The check of calculations is
	 * correct calculate sum of vertical reference and compare to weight */
	int wheel = 0;
	for (wheel = 0; wheel < 6; wheel++) {
		forceRefOptDispForCylinderLoad_N[wheel] = PAFConvertVerticalForceOnWheelToCylinderLoadForce(posData_mm[wheel], forceReferenceOptimalDistrubutionVertical_N[wheel]);
	}
}

void PAFSendOptimalForceRefOnCAN(uint8 CANChannel, uint32 frontAndMiddleID, uint32 backID) {
	sint16 forceReferenceDispSum_N = 0;
	/* Convert to optimalFOrceRef on cylinder. */
	int wheel = 0;
	for (wheel = 0; wheel < 6; wheel++) {
		forceReferenceDispSum_N = forceReferenceDispSum_N + forceReferenceOptimalDistrubutionVertical_N[wheel];
	}

	CANSend_sint16(CANChannel, frontAndMiddleID,
			forceRefOptDispForCylinderLoad_N[FL],
			forceRefOptDispForCylinderLoad_N[FR],
			forceRefOptDispForCylinderLoad_N[ML],
			forceRefOptDispForCylinderLoad_N[MR]
	);
	CANSend_sint16(CANChannel, backID,
			forceRefOptDispForCylinderLoad_N[BL],
			forceRefOptDispForCylinderLoad_N[BR],
			sumOfVerticalForce,
			forceReferenceDispSum_N
	);
}

void PAFSendForceErrorPercentageOnCAN(uint8 CANChannel, uint32 frontAndMiddleID, uint32 backID) {
	sint32 forceErrorInPercent[SUM_WHEELS] = {0};
	int wheel = 0;
	int convertToPercent = 100;
	for (wheel = 0; wheel < SUM_WHEELS; wheel++) {
		forceErrorInPercent[wheel] = -(float)( (float)(forceRefOptDispForCylinderLoad_N[wheel] - messuredForceCylinderLoad_deciN[wheel])
				/
				(float)forceRefOptDispForCylinderLoad_N[wheel] ) * convertToPercent;
	}
	CANSend_sint16(CANChannel, frontAndMiddleID,
			forceErrorInPercent[FL],
			forceErrorInPercent[FR],
			forceErrorInPercent[ML],
			forceErrorInPercent[MR]
	);
	CANSend_sint16(CANChannel, backID,
			forceErrorInPercent[BL],
			forceErrorInPercent[BR],
			0,
			0
	);
}

int PAFGetOptimalReferenceForceForWheel_N(uint8 wheelNumber) {
	return forceRefOptDispForCylinderLoad_N[wheelNumber];
}

void PAFGetOptimalReferenceForceArray_N(int forceOptRefOutputArray[static SUM_WHEELS]) {
	int wheel = 0;
	for (wheel = 0; wheel < SUM_WHEELS; wheel++) {
		forceOptRefOutputArray[wheel] = forceRefOptDispForCylinderLoad_N[wheel];
	}
}

int PAFGetMessuredCylinderLoadForceForWheel_dN(uint8 wheelNumber) {
	return messuredForceCylinderLoad_deciN[wheelNumber];
}

void PAFGetMessuredCylinderLoadForceArray_dN(int forceCylLoadOutputArray[static SUM_WHEELS]) {
	int wheel = 0;
	for (wheel = 0; wheel < SUM_WHEELS; wheel++) {
		forceCylLoadOutputArray[wheel] = messuredForceCylinderLoad_deciN[wheel];
	}
}




