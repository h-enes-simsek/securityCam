#ifndef ServoHandler_H
#define ServoHandler_H

#include <ESP32Servo.h>

// pinout config for esp32 cam ai-thinker
#define SERVO_EL_PIN	14   // IO14, for elevation
#define SERVO_TR_PIN 	15 	 // IO15, for traverse
#define DUMMY_PIN_1		2    // dummy pin to attach unused servo (to occupy pwm channel)
#define DUMMY_PIN_2		13   // dummy pin to attach unused servo (to occupy pwm channel)

// This wrapper class handles 2-axis servo motors.
class ServoHandler 
{
	public:
	ServoHandler();
	ServoHandler(int min, int max, int initialTr, int initialEl);
	void servoRotateTraverse(int val);
	void servoRotateElevation(int val);

	private:
	int controlBoundaries(int angle);
	Servo mServoDummy1; // only used to occupy pwm channel
	Servo mServoDummy2; // only used to occupy pwm channel
	Servo mServoTr;
	Servo mServoEl; 
	
	int mServoPosTr; // in deg, to save servo position in traverse
	int mServoPosEl; // in deg, to save servo position in elevation
};

#endif
