#ifndef PID_h
#define PID_h
#define PID_LIBRARY_VERSION	1.1.1

class PID {
    public:

        //Parameter types for some of the functions below
        enum mode_t { AUTOMATIC = 1, MANUAL = 0 };
        enum direction_t { DIRECT = 0, REVERSE = 1 };

        //commonly used functions **************************************************************************
        PID(double*, double*, double*,        // * constructor.  links the PID to the Input, Output, and
            double, double, double,           //   Setpoint.  Initial tuning parameters are also set here
            direction_t);

        void SetMode(mode_t);                 // * sets PID to either MANUAL (0) or AUTOMATIC (1)

        bool Compute();                       // * performs the PID calculation.  it should be
                                            //   called every time loop() cycles. ON/OFF and
                                            //   calculation frequency can be set using SetMode
                                            //   SetSampleTime respectively

        void SetOutputLimits(double, double); //clamps the output to a specific range. 0-255 by default, but
                                            //it's likely the user will want to change this depending on
                                            //the application



        //available but not commonly used functions ********************************************************
        void SetTunings(double, double,       // * While most users will set the tunings once in the
                        double);         	  //   constructor, this function gives the user the option
                                            //   of changing tunings during runtime for Adaptive control
        void SetControllerDirection(          // * Sets the Direction, or "Action" of the controller. DIRECT
                        direction_t);         //   means the output will increase when error is positive. REVERSE
                                            //   means the opposite.  it's very unlikely that this will be needed
                                            //   once it is set in the constructor.
        void SetSampleTime(int);              // * sets the frequency, in Milliseconds, with which
                                            //   the PID calculation is performed.  default is 100
        void SynchITerm();


        //Display functions ****************************************************************
        double GetKp();						  // These functions query the pid for interal values.
        double GetKi();						  //  they were created mainly for the pid front-end,
        double GetKd();						  // where it's important to know what is actually
        double GetITerm();          // Internal value of ITerm

        int GetMode();						  //  inside the PID.
        int GetDirection();					  //

        void Initialize(double i);

    private:

        double dispKp;				// * we'll hold on to the tuning parameters in user-entered
        double dispKi;				//   format for display purposes
        double dispKd;				//

        double kp;                  // * (P)roportional Tuning Parameter
        double ki;                  // * (I)ntegral Tuning Parameter
        double kd;                  // * (D)erivative Tuning Parameter

        int controllerDirection;

        double *myInput;              // * Pointers to the Input, Output, and Setpoint variables
        double *myOutput;             //   This creates a hard link between the variables and the
        double *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                        //   what these values are.  with pointers we'll just know.

        unsigned long lastTime;
        double ITerm, lastInput, lastError;

        unsigned long SampleTime;
        double outMin, outMax;
        bool inAuto;
};

#endif
