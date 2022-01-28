#include "ServoHandler.h"

ServoHandler::ServoHandler()
{	
	// min max values should be configured for servo motor (in msec, pwm width)
	// for sg90, 400-2400 provides about 160 deg. (but actually it should be 180 deg according to servo documentation)
	mServoTr.attach(SERVO_TR_PIN, 400, 2400);
	mServoEl.attach(SERVO_EL_PIN, 400, 2400);
	
	// attach it but do not use dummy servos, because these pwm channels will be use used by camera
	mServoDummy1.attach(DUMMY_PIN_1, 1000, 2000); // dummy numbers are used
	mServoDummy2.attach(DUMMY_PIN_2, 1000, 2000); // dummy numbers are used
	
	// initial position for servos (in degree)
	mServoPosTr = 90;
	mServoPosEl = 90;
	
	// servo frequency (in Hz)
	mServoTr.setPeriodHertz(50);    
	mServoEl.setPeriodHertz(50);
}

ServoHandler::ServoHandler(int minInMsec, int maxInMsec, int initialTr, int initialEl)
{	
	// min max values should be configured for servo motor (in msec, pwm width)
	// for sg90, 400-2400 provides about 160 deg. (but actually it should be 180 deg according to servo documentation)
	mServoTr.attach(SERVO_TR_PIN, minInMsec, maxInMsec);
	mServoEl.attach(SERVO_EL_PIN, minInMsec, maxInMsec);
	
	// attach it but do not use dummy servos, because these pwm channels will be use used by camera
	mServoDummy1.attach(DUMMY_PIN_1, 1000, 2000); // dummy numbers are used
	mServoDummy2.attach(DUMMY_PIN_2, 1000, 2000); // dummy numbers are used
	
	// initial position for servos (in degree)
	mServoPosTr = initialTr;
	mServoPosEl = initialEl;
	
	// servo frequency (in Hz)
	mServoTr.setPeriodHertz(50);    
	mServoEl.setPeriodHertz(50);
}

// rotate servo in the traverse direction for "val" degree
void ServoHandler::servoRotateTraverse(int val)
{
	mServoTr.write(mServoPosTr + val);
	mServoPosTr += val;
	mServoPosTr = controlBoundaries(mServoPosTr);
}

// rotate servo in the elevation direction for "val" degree
void ServoHandler::servoRotateElevation(int val)
{
	mServoEl.write(mServoPosEl + val);
	mServoPosEl += val;
	mServoPosEl = controlBoundaries(mServoPosEl);
}

int ServoHandler::controlBoundaries(int angle)
{
	if(angle < 0)
		return 0;
	else if(angle > 180)
		return 180;
	else
		return angle;
}

/* ___important___ info about dummy servos
The servo library we’re using automatically assigns a PWM channel to each servo motor 
(mServoDummy1 → PWM channel 0; mServoDummy2 → PWM channel 1; mServoPosTr → PWM channel 2; mServoPosEl → PWM channel 3)

The first channels are being used by the camera, so if we change those PWM channels’ properties, 
we’ll get errors with the camera. 
*/