#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include "XPLMPlanes.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#if IBM
#include <windows.h>
#include <stdio.h>
#endif
#if LIN
#include <GL/gl.h>
#else
#if __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

XPLMDataRef dr_brake_ratio_heading = NULL;
XPLMDataRef dr_right_brake_ratio = NULL;
XPLMDataRef dr_axis_assignments = NULL;
XPLMDataRef dr_left_brake_ratio = NULL;
XPLMDataRef dr_axis_values = NULL;

static int id_right_toe_axis = -1;
static int id_left_toe_axis = -1;
bool FindToeAxis();

int m_menu_parentitem = 0;
XPLMMenuID m_menu_mainmenu = 0;
void MenuEntryClicked(void*, void*);

float FlightLoopCB(float elapsedMe, float elapsedSim, int counter, void* refcon);

PLUGIN_API int XPluginStart( char* outName, char* outSig, char* outDesc)
{
    strcpy(outName,"toe2yaw");
    strcpy(outSig,"signature");
    strcpy(outDesc,"description");

    dr_brake_ratio_heading = XPLMFindDataRef("sim/cockpit2/controls/yoke_heading_ratio");
    dr_right_brake_ratio  = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
    dr_left_brake_ratio   = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
    dr_axis_assignments   = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    dr_axis_values        = XPLMFindDataRef("sim/joystick/joystick_axis_values");

    XPLMSetDatai(XPLMFindDataRef("sim/operation/override/override_joystick_heading"),1);
    XPLMSetDatai(XPLMFindDataRef("sim/operation/override/override_artstab"),1);
    XPLMSetDatai(XPLMFindDataRef("sim/operation/override/override_gearbrake"),1);

    m_menu_parentitem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), outName, NULL, 1);
    m_menu_mainmenu   = XPLMCreateMenu(outName,XPLMFindPluginsMenu(), m_menu_parentitem, MenuEntryClicked, NULL);
    XPLMAppendMenuItem(m_menu_mainmenu, "Enable/Disable", (void*)"onoff", 1);
    XPLMAppendMenuSeparator(m_menu_mainmenu);
//    XPLMAppendMenuItem(m_menu_mainmenu, "Exclusive rudder/yaw control", (void*)"exclusive", 1);
//    XPLMAppendMenuItem(m_menu_mainmenu, "Reasign toe brake axis", (void*)"reasign", 1);

    XPLMRegisterFlightLoopCallback(FlightLoopCB,-1,NULL);

    return 1;
}


PLUGIN_API void	XPluginStop(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
}


//-------------------------------------------------------------------------------------

bool FindToeAxis()
{
    id_left_toe_axis = -1;
    id_right_toe_axis= -1;

    int joy_axis[100];

    XPLMGetDatavi(dr_axis_assignments,joy_axis,0,100);

    for (int i=0; i<100; ++i)
    {
        if (joy_axis[i] == 6)
            id_left_toe_axis = i;
        else if(joy_axis[i] == 7)
            id_right_toe_axis = i;
    }

    return (id_left_toe_axis != -1 && id_left_toe_axis != -1);
}


float FlightLoopCB( float elapsedMe, float elapsedSim, int counter, void* refcon)
{
    if (id_left_toe_axis == -1 || id_right_toe_axis == -1)
        //FIXME: Do it once at startup!
        FindToeAxis();

    if (id_left_toe_axis == -1 || id_right_toe_axis == -1) return -1.0;

    float axis_val[100];
    XPLMGetDatavf(dr_axis_values,axis_val,0,100);

    float left_brake = axis_val[id_left_toe_axis];
    float right_brake= axis_val[id_right_toe_axis];

    float heading = right_brake-left_brake;

    float h = XPLMGetDataf(dr_brake_ratio_heading);

    if (left_brake == 0.0 || right_brake == 0.0 ) {
        XPLMSetDataf(dr_left_brake_ratio,0.0);
        XPLMSetDataf(dr_right_brake_ratio,0.0);
        XPLMSetDataf(dr_brake_ratio_heading,heading);
    }
        else
        {  
            XPLMSetDataf(dr_left_brake_ratio,left_brake);
            XPLMSetDataf(dr_right_brake_ratio,right_brake);
            XPLMSetDataf(dr_brake_ratio_heading,0.0);
        }


    return -1.0;
}


//------------------------------------------------------------------------

void MenuEntryClicked(void* mRef, void* iRef)
{
    char* entry = (char*)iRef;

    if (!strcmp(entry,"onoff")) {
        printf("onoff");
    }
    else
        if (!strcmp(entry,"exclusive")) {
            printf("exclusive");
        }
        else
            if (!strcmp(entry,"reasign")) {
                printf("reasign");
            } 
}


