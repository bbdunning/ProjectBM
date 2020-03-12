#pragma once
#ifndef __INPUTHANDLER_H
#define __INPUTHANDLER_H

class InputHandler {
    public:
        InputHandler();
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
        bool Leftflag;
        bool Rightflag;
	    float lightX;
};

#endif