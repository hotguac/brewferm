/**********************************************************************************************
 * Arduino PID Library - Version 1.1.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * Adapted for Particle by Dan Rice <dan@zoombody.com>
 *
 * Adapted from Paritcle version for BrewFerm by Joe Kokosa
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/

#include "application.h"
#include "jpid.h"

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(double* Input,
         double* Output,
         double* Setpoint,
         double Kp,
         double Ki,
         double Kd,
         direction_t ControllerDirection) {

    myOutput = Output;
    myInput = Input;
    mySetpoint = Setpoint;

    inAuto = false;

    PID::SetOutputLimits(0, 255);		//default output limit corresponds to
                                                //the arduino pwm limits

    SampleTime = 100;		//default Controller Sample Time is 0.1 seconds

    PID::SetControllerDirection(ControllerDirection);
    PID::SetTunings(Kp, Ki, Kd);

    lastTime = millis() - SampleTime;

    lastInput = 0;
    lastError = 0;
}

/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool PID::Compute()
{
    if (!inAuto) {
        return false;
    }

    unsigned long now = millis();
    unsigned long timeChange = (now - lastTime);

    if (timeChange>=SampleTime)    {
      /*Compute all the working error variables*/
	    double input = *myInput;
        double error = *mySetpoint - input;

        ITerm += (ki * error);

        if (ITerm > outMax) {
            ITerm = outMax;
        } else {
            if (ITerm < outMin) {
                ITerm = outMin;
            }
        }

        lastError = error;

        double dInput = (input - lastInput);

        /*Compute PID Output*/
        double output = (kp * error) + ITerm - (kd * dInput);

        if (output > outMax) {
            output = outMax;
        } else {
            if(output < outMin) {
                output = outMin;
            }
        }

        *myOutput = output;

        /*Remember some variables for next time*/
        lastInput = input;
        lastTime = now;

        return true;
    } else {
        return false;
    }
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID::SetTunings(double Kp, double Ki, double Kd)
{
    if (Kp<0 || Ki<0 || Kd<0) {
       return;
    }

    dispKp = Kp;
    dispKi = Ki;
    dispKd = Kd;

    double SampleTimeInSec = ((double)SampleTime)/1000;

    kp = Kp;
    ki = Ki * SampleTimeInSec;
    kd = Kd / SampleTimeInSec;

    if (controllerDirection == REVERSE)
    {
        kp = (0 - kp);
        ki = (0 - ki);
        kd = (0 - kd);
    }
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void PID::SetSampleTime(int NewSampleTime)
{
    if (NewSampleTime > 0) {
        double ratio  = (double)NewSampleTime
                        / (double)SampleTime;

        ki *= ratio;
        kd /= ratio;

        SampleTime = (unsigned long)NewSampleTime;
    }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID::SetOutputLimits(double Min, double Max)
{
    if (Min >= Max) {
        return;
    }

    outMin = Min;
    outMax = Max;

    if (inAuto)
    {
        if (*myOutput > outMax) {
            *myOutput = outMax;
        }
        else {
            if (*myOutput < outMin) {
                *myOutput = outMin;
            }
        }

        if (ITerm > outMax) {
            ITerm = outMax;
        }
        else {
            if (ITerm < outMin) {
                ITerm = outMin;
            }
        }
    }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to MANUAL (0) or AUTOMATIC (1)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID::SetMode(mode_t Mode)
{
    bool newAuto = (Mode == AUTOMATIC);

    if (newAuto == !inAuto) {  /*we just went from manual to auto*/
        PID::Initialize(*myInput);
    }

    inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize(double i)
{
    ITerm = i; // *myOutput; chg 1/28/2020
    lastInput = *myInput;

    if (ITerm > outMax) {
        ITerm = outMax;
    }
    else {
        if (ITerm < outMin) {
            ITerm = outMin;
        }
    }
}

void PID::SynchITerm()
{
    ITerm = *myInput; // *mySetpoint; chg 1/28/2020
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(direction_t Direction)
{
    if (inAuto && Direction !=controllerDirection) {
        kp = (0 - kp);
        ki = (0 - ki);
        kd = (0 - kd);
    }

    controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
double PID::GetKp()
{
    return  dispKp;
}

//---------------------------------------------------------------------------
// Expose internal variable
//---------------------------------------------------------------------------
double PID::GetKi()
{
    return  dispKi;
}

//---------------------------------------------------------------------------
// Expose internal variable
//---------------------------------------------------------------------------
double PID::GetKd()
{
    return  dispKd;
}

//---------------------------------------------------------------------------
// Expose internal variable
//---------------------------------------------------------------------------
double PID::GetITerm()
{
    return ITerm;
}

//---------------------------------------------------------------------------
// Expose internal variable
//---------------------------------------------------------------------------
int PID::GetMode()
{
    return  inAuto ? AUTOMATIC : MANUAL;
}

//---------------------------------------------------------------------------
// Expose internal variable
//---------------------------------------------------------------------------
int PID::GetDirection()
{
    return controllerDirection;
}
