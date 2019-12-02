/***
Servo Definitions Header File

The following header files contains definitions for address control of the
servos in the project

The number of servos and the tick time need to be inserted in the fist 2
definitions. 

Each servo has a number identity. This refers to the PWM channel they are
connected to. If the servos are attached to a different channel, update the
value

Servos belong to PWM groups. Make sure like servos are in the same group so
that the correct period is used.

There are definitions for the range of motion of each servo. To change the 
range of motion of any servo, change the tick limit values. The corresponding 
ServoCount will then be a percentage of the that motion. Ranges do not need to
be symmetic

DO NOT PUT PROTOTYPES HERE. THEY BELONG IN Servo.h
***/
#ifndef ServoDefs_H
#define ServoDefs_H

#define NUM_SERVOS 5
#define TICK_US 0.8 //1 tick is 0.8 us

//Servo Numeric Identifier (matches PWM channel)
#define CANNON_SERVO 0
#define TIMER_SERVO 1
#define O2_GAUGE_SERVO 2 
#define POWER_GAUGE_SERVO 3
#define TOT_SERVO  4

//PWM Channel Group designations
#define CANNON_TIMER_GROUP 0
#define GAUGE_GROUP 1
#define TOT_GROUP 2

//Tick Limits (Servo hardware dependent)
#define CANNON_HI_LIM 3000
#define CANNON_LO_LIM 800

#define TIMER_HI_LIM 2400
#define TIMER_LO_LIM 1500

#define O2_GAUGE_HI_LIM 3200
#define O2_GAUGE_LO_LIM 800

#define POWER_GAUGE_HI_LIM 2700
#define POWER_GAUGE_LO_LIM 910

#define TOT_HI_LIM 3200
#define TOT_LO_LIM 800

//Group Periods
#define CANNON_TIMER_PERIOD_MS 20
#define GAUGE_GROUP_PERIOD_MS 20
#define TOT_GROUP_PERIOD_MS 20

#endif /*ServoDefs_H*/
