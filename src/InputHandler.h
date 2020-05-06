#pragma once
#ifndef __INPUTHANDLER_H
#define __INPUTHANDLER_H

class InputHandler {
    public:
        InputHandler();
        void init();
        void setKeyFlags(int key, int action);
        bool Wflag;
        bool Sflag;
        bool Aflag;
        bool Dflag;
        bool Spaceflag;
        bool Ctrlflag;
        bool Shiftflag;
        bool Upflag;
        bool Downflag;
        bool Cflag;
        bool Leftflag;
        bool Rightflag;
        bool R;
        bool Q;
        bool E=false;
	    float lightX;
	    float up_start_time;
	    float down_start_time;
	    float up_time;
	    float down_time;
	    float right_start_time;
	    float left_start_time;
	    float right_time;
	    float left_time;
        float space_time;
        float space_start_time;
        float shift_start_time;
        bool jump;

		bool kp5;
        bool kp1;
        bool kp2;
		bool kp3;

        bool controllerPresent;
};

#endif