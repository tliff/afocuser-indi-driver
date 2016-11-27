/*******************************************************************************
  Copyright(c) 2012 Jasem Mutlaq. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.
 .
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.
 .
 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/
#include "afocuser.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include <memory>


// We declare an auto pointer to focuser.
std::unique_ptr<AFocuser> focuser(new AFocuser());

#define SIM_SEEING  0
#define SIM_FWHM    1
#define FOCUS_MOTION_DELAY  100                /* Focuser takes 100 microsecond to move for each step, completing 100,000 steps in 10 seconds */

void ISPoll(void *p);

void ISGetProperties(const char *dev)
{
        focuser->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
        focuser->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
        focuser->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
        focuser->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
  INDI_UNUSED(dev);
  INDI_UNUSED(name);
  INDI_UNUSED(sizes);
  INDI_UNUSED(blobsizes);
  INDI_UNUSED(blobs);
  INDI_UNUSED(formats);
  INDI_UNUSED(names);
  INDI_UNUSED(n);
}

void ISSnoopDevice (XMLEle *root)
{
    focuser->ISSnoopDevice(root);
}

AFocuser::AFocuser()
{
    ticks=0;
    driver = new AFocuserDriver("/dev/ttyUSB0");
    //SetFocuserCapability(FOCUSER_CAN_ABS_MOVE | FOCUSER_CAN_REL_MOVE | FOCUSER_CAN_ABORT | FOCUSER_HAS_VARIABLE_SPEED);
    SetFocuserCapability(FOCUSER_CAN_REL_MOVE);
}

bool AFocuser::SetupParms()
{
    IDSetNumber(&FWHMNP, NULL);
    return true;
}

bool AFocuser::Connect()
{
    SetTimer(1000);     //  start the timer
    driver->Connect();
    driver->move(6200);
    return true;
}

AFocuser::~AFocuser()
{
    delete driver;
    //dtor
}

const char * AFocuser::getDefaultName()
{
        return (char *)"Focuser Simulator";
}

bool AFocuser::initProperties()
{
    INDI::Focuser::initProperties();

/*    IUFillNumberVector(&SeeingNP,SeeingN,1,getDeviceName(),"SEEING_SETTINGS","Seeing",MAIN_CONTROL_TAB,IP_RW,60,IPS_IDLE);

    IUFillNumber(&FWHMN[0],"SIM_FWHM","arcseconds","%4.2f",0,60,0,7.5);
    IUFillNumberVector(&FWHMNP,FWHMN,1,getDeviceName(), "FWHM","FWHM",MAIN_CONTROL_TAB,IP_RO,60,IPS_IDLE);

    ticks = initTicks = sqrt(FWHMN[0].value - SeeingN[0].value) / 0.75;
*/
    FocusAbsPosN[0].value = FocusAbsPosN[0].max / 2;

    return true;
}

bool AFocuser::updateProperties()
{

    INDI::Focuser::updateProperties();

    if (isConnected())
    {
//        defineNumber(&SeeingNP);
//        defineNumber(&FWHMNP);
        SetupParms();
    }
    else
    {
//        deleteProperty(SeeingNP.name);
//        deleteProperty(FWHMNP.name);
    }

    return true;
}


bool AFocuser::Disconnect()
{
    return true;
}


void AFocuser::TimerHit()
{
    int nexttimer=1000;

    if(isConnected() == false) return;  //  No need to reset timer if we are not connected anymore

    SetTimer(nexttimer);
    return;
}

bool AFocuser::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
  /*      if(strcmp(name,"SEEING_SETTINGS")==0)
        {
            SeeingNP.s = IPS_OK;
            IUUpdateNumber(&SeeingNP, values, names, n);

            IDSetNumber(&SeeingNP,NULL);
            return true;

        }
*/
    }

    //  if we didn't process it, continue up the chain, let somebody else
    //  give it a shot
    return INDI::Focuser::ISNewNumber(dev,name,values,names,n);
}


bool AFocuser::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{

    //  Nobody has claimed this, so, ignore it
    return INDI::Focuser::ISNewSwitch(dev,name,states,names,n);
}

IPState AFocuser::MoveFocuser(FocusDirection dir, int speed, uint16_t duration)
{

    return IPS_ALERT;

}

IPState AFocuser::MoveAbsFocuser(uint32_t targetTicks)
{

    return IPS_ALERT;
}

IPState AFocuser::MoveRelFocuser(FocusDirection dir, uint32_t ticks)
{
    driver->move(ticks * (dir == FOCUS_INWARD ? 1 : -1));

    return IPS_OK;
}

bool AFocuser::SetFocuserSpeed(int speed) {
  return true;
}
