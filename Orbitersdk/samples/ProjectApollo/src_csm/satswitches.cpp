/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005

  ORBITER vessel module: Saturn-specific switches

  Project Apollo is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Project Apollo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Project Apollo; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  See http://nassp.sourceforge.net/license/ for more details.

  **************************************************************************/

// To force orbitersdk.h to use <fstream> in any compiler version
#pragma include_alias( <fstream.h>, <fstream> )
#include "Orbitersdk.h"
#include <stdio.h>
#include <math.h>
#include "soundlib.h"

#include "resource.h"

#include "nasspdefs.h"
#include "nasspsound.h"

#include "toggleswitch.h"
#include "apolloguidance.h"
#include "dsky.h"
#include "csmcomputer.h"
#include "IMU.h"
#include "lvimu.h"

#include "saturn.h"


void SaturnToggleSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s, int xoffset, int yoffset)

{
	ToggleSwitch::Init(xp, yp, w, h, surf, bsurf, row, xoffset, yoffset);
	sat = s;
}

void SaturnGuardedPushSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s, int xoffset, int yoffset, int lxoffset, int lyoffset)

{
	GuardedPushSwitch::Init(xp, yp, w, h, surf, bsurf, row, xoffset, yoffset, lxoffset, lyoffset);
	sat = s;
}

bool LESMotorFireSwitch::SwitchTo(int newState, bool dontspring)

{
	if (GuardedPushSwitch::SwitchTo(newState,dontspring) && Toggled())
	{
		ClearToggled();
		sat->JettisonLET(true);

		return true;
	}

	return false;
}

bool XLunarSwitch::SwitchTo(int newState, bool dontspring)


{
	if (ToggleSwitch::SwitchTo(newState,dontspring)) {
		// Do nothing, handling is done in the IU
		return true;
	}
	return false;
}

void SaturnThreePosSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s)

{
	ThreePosSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	sat = s;
}


SaturnRCSValveTalkback::SaturnRCSValveTalkback()

{
	valve = 0;
}


void SaturnRCSValveTalkback::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SwitchRow &row, RCSValve *v, bool failopen)

{
	IndicatorSwitch::Init(xp, yp, w, h, surf, row, failopen);
	valve = v;	
}

int SaturnRCSValveTalkback::GetState()

{
	if (valve && SRC && (SRC->Voltage() > SP_MIN_DCVOLTAGE))
		state = valve->IsOpen() ? 1 : 0;
	else 
		// SM RCS helium and prim. propellant talkbacks fail open
		state = (failOpen ? 1 : 0);

	return state;
}


void SaturnH2PressureMeter::Init(int i, SURFHANDLE surf, SwitchRow &row, Saturn *s)

{
	MeterSwitch::Init(row);
	Index = i;
	NeedleSurface = surf;
	Sat = s;
}

double SaturnH2PressureMeter::QueryValue()

{
	TankPressures press;
	Sat->GetTankPressures(press);

	if (Index == 1) 
		return press.H2Tank1PressurePSI;
	else
		return press.H2Tank2PressurePSI;
}

void SaturnH2PressureMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (Index == 1) 
		oapiBlt(drawSurface, NeedleSurface,  0, (110 - (int)(v / 400.0 * 104.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface, 53, (110 - (int)(v / 400.0 * 104.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


void SaturnO2PressureMeter::Init(int i, SURFHANDLE surf, SwitchRow &row, Saturn *s, ToggleSwitch *o2PressIndSwitch)

{
	MeterSwitch::Init(row);
	Index = i;
	NeedleSurface = surf;
	Sat = s;
	O2PressIndSwitch = o2PressIndSwitch;
}

double SaturnO2PressureMeter::QueryValue()

{
	TankPressures press;
	Sat->GetTankPressures(press);

	if (Index == 1) 
		if (O2PressIndSwitch->IsUp())  
			return press.O2Tank1PressurePSI;
		else
			return press.O2SurgeTankPressurePSI;
	else
		return press.O2Tank2PressurePSI;
}

void SaturnO2PressureMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (Index == 1) 
		DoDrawSwitch(drawSurface, NeedleSurface, v, 86, 0);
	else
		DoDrawSwitch(drawSurface, NeedleSurface, v, 139, 10);
}

void SaturnO2PressureMeter::DoDrawSwitch(SURFHANDLE surf, SURFHANDLE needle, double value, int xOffset, int xNeedle)

{
	if (value < 100.0)
		oapiBlt(surf, needle, xOffset, 110, xNeedle, 0, 10, 10, SURF_PREDEF_CK);

	else if (value <= 500.0) 
		oapiBlt(surf, needle, xOffset, 110 - (int)((value - 100.0) * 0.065), xNeedle, 0, 10, 10, SURF_PREDEF_CK);

	else if (value <= 850.0)
		oapiBlt(surf, needle, xOffset, 84 - (int)((value - 500.0) * 0.07714), xNeedle, 0, 10, 10, SURF_PREDEF_CK);

	else if (value <= 900.0)
		oapiBlt(surf, needle, xOffset, 57 - (int)((value - 850.0) * 0.38), xNeedle, 0, 10, 10, SURF_PREDEF_CK);

	else if (value <= 950.0)
		oapiBlt(surf, needle, xOffset, 38 - (int)((value - 900.0) * 0.42), xNeedle, 0, 10, 10, SURF_PREDEF_CK);

	else if (value <= 1050.0)
		oapiBlt(surf, needle, xOffset, 17 - (int)((value - 950.0) * 0.13), xNeedle, 0, 10, 10, SURF_PREDEF_CK);

	else
		oapiBlt(surf, needle, xOffset, 4, xNeedle, 0, 10, 10, SURF_PREDEF_CK);
}


void SaturnCryoQuantityMeter::Init(char *sub, int i, SURFHANDLE surf, SwitchRow &row, Saturn *s)

{
	MeterSwitch::Init(row);
	Substance = sub;
	Index = i;
	NeedleSurface = surf;
	Sat = s;
}

double SaturnCryoQuantityMeter::QueryValue()

{
	TankQuantities q;
	Sat->GetTankQuantities(q);

	if (!strcmp("H2", Substance)) {
		if (Index == 1) 
			return q.H2Tank1Quantity;
		else
			return q.H2Tank2Quantity;
	} else {
		if (Index == 1) 
			return q.O2Tank1Quantity;
		else
			return q.O2Tank2Quantity;
	}
}

void SaturnCryoQuantityMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (!strcmp("H2", Substance)) {
		if (Index == 1) 
			oapiBlt(drawSurface, NeedleSurface,  172, (110 - (int)(v * 104.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
		else
			oapiBlt(drawSurface, NeedleSurface,  225, (110 - (int)(v * 104.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	} else {
		if (Index == 1) 
			oapiBlt(drawSurface, NeedleSurface,  258, (110 - (int)(v * 104.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
		else {
			//
			// Apollo 13 O2 tank 2 quantity display failed offscale high around 46:45.
			//

			#define O2FAILURETIME	(46.0 * 3600.0 + 45.0 * 60.0)

			if (Sat->GetApolloNo() == 13) {
				if (Sat->GetMissionTime() >= (O2FAILURETIME + 5.0)) {
					v = 1.05;
				}
				else if (Sat->GetMissionTime() >= O2FAILURETIME) {
					v += (1.05 - value) * ((Sat->GetMissionTime() - O2FAILURETIME) / 5.0);
				}
			}
			oapiBlt(drawSurface, NeedleSurface,  311, (110 - (int)(v * 104.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
		}
	}
}


RCSQuantityMeter::RCSQuantityMeter()

{
	source = 0;
	SMRCSIndSwitch = 0;
	NeedleSurface = 0;
}

void RCSQuantityMeter::Init(SURFHANDLE surf, SwitchRow &row, PropellantRotationalSwitch *s, ToggleSwitch *indswitch)

{
	MeterSwitch::Init(row);
	source = s;
	SMRCSIndSwitch = indswitch;
	NeedleSurface = surf;
}

double RCSQuantityMeter::QueryValue()

{
	if (!source) return 0.0;

	// only SM data are available/displayed
	SMRCSPropellantSource *ps = source->GetSMSource();
	if (!ps) return 0;

	if (SMRCSIndSwitch->IsDown()) {		
		return ps->GetPropellantQuantityToDisplay();
	}
	return ps->GetHeliumTempF() / 100.;
}

void RCSQuantityMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface,  150, (108 - (int)(v * 104.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


RCSFuelPressMeter::RCSFuelPressMeter()

{
	source = 0;
	NeedleSurface = 0;
}

void RCSFuelPressMeter::Init(SURFHANDLE surf, SwitchRow &row, PropellantRotationalSwitch *s)

{
	MeterSwitch::Init(row);
	source = s;
	NeedleSurface = surf;
}

double RCSFuelPressMeter::QueryValue()

{
	if (!source) return 0;

	SMRCSPropellantSource *ps = source->GetSMSource();
	if (ps) return ps->GetPropellantPressurePSI();

	CMRCSPropellantSource *cmps = source->GetCMSource();
	if (cmps) return cmps->GetPropellantPressurePSI();

	return 0;
}

void RCSFuelPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface,  95, (108 - (int)(v / 400.0 * 104.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


RCSHeliumPressMeter::RCSHeliumPressMeter()

{
	source = 0;
	NeedleSurface = 0;
}

void RCSHeliumPressMeter::Init(SURFHANDLE surf, SwitchRow &row, PropellantRotationalSwitch *s)

{
	MeterSwitch::Init(row);
	source = s;
	NeedleSurface = surf;
}

double RCSHeliumPressMeter::QueryValue()

{
	if (!source) return 0;

	SMRCSPropellantSource *ps = source->GetSMSource();
	if (ps) return ps->GetHeliumPressurePSI();

	CMRCSPropellantSource *cmps = source->GetCMSource();
	if (cmps) return cmps->GetHeliumPressurePSI();

	return 0;
}

void RCSHeliumPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface,  59, (108 - (int)(v / 5000.0 * 104.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


RCSTempMeter::RCSTempMeter()

{
	source = 0;
	NeedleSurface = 0;
}

void RCSTempMeter::Init(SURFHANDLE surf, SwitchRow &row, PropellantRotationalSwitch *s)

{
	MeterSwitch::Init(row);
	source = s;
	NeedleSurface = surf;
}

double RCSTempMeter::QueryValue()

{
	if (!source) return 0;

	SMRCSPropellantSource *ps = source->GetSMSource();
	if (ps) return ps->GetPackageTempF();

	CMRCSPropellantSource *cmps = source->GetCMSource();
	if (cmps) return cmps->GetHeliumTempF();

	return 0;
}

void RCSTempMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface,  4, (108 - (int)(v / 300.0 * 104.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


PropellantSource::PropellantSource(PROPELLANT_HANDLE &h) : source_prop(h)

{
	our_vessel = 0;
}

PROPELLANT_HANDLE PropellantSource::Handle()

{
	return source_prop;
}

double PropellantSource::Quantity()

{
	if (source_prop && our_vessel) {
		return our_vessel->GetPropellantMass(source_prop) / our_vessel->GetPropellantMaxMass(source_prop);
	}

	return 0.0;
}


PropellantRotationalSwitch::PropellantRotationalSwitch()

{
	int i;

	for (i = 0; i < 7; i++) {
		CMSources[i] = 0;
		SMSources[i] = 0;
	}
}

void PropellantRotationalSwitch::SetCMSource(int num, CMRCSPropellantSource *s)

{
	if (num >= 0 && num < 7)
		CMSources[num] = s; 
}

void PropellantRotationalSwitch::SetSMSource(int num, SMRCSPropellantSource *s)

{
	if (num >= 0 && num < 7)
		SMSources[num] = s; 
}

CMRCSPropellantSource *PropellantRotationalSwitch::GetCMSource()

{
	return CMSources[GetState()];
}

SMRCSPropellantSource *PropellantRotationalSwitch::GetSMSource()

{
	return SMSources[GetState()];
}


void SaturnFuelCellMeter::Init(SURFHANDLE surf, SwitchRow &row, Saturn *s, RotationalSwitch *fuelCellIndicatorsSwitch)

{
	MeterSwitch::Init(row);
	NeedleSurface = surf;
	Sat = s;
	FuelCellIndicatorsSwitch = fuelCellIndicatorsSwitch;
}


double SaturnFuelCellH2FlowMeter::QueryValue()

{
	FuelCellStatus fc;
	Sat->GetFuelCellStatus(FuelCellIndicatorsSwitch->GetState(), fc);

	return fc.H2FlowLBH; 
}

void SaturnFuelCellH2FlowMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (v < 0.05)
		oapiBlt(drawSurface, NeedleSurface, 0, (111 - (int)(v / 0.05 * 21.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
	else if (v < 0.15)
		oapiBlt(drawSurface, NeedleSurface, 0, (90 - (int)((v - 0.05) / 0.1 * 65.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface, 0, (25 - (int)((v - 0.15) / 0.05 * 21.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnFuelCellO2FlowMeter::QueryValue()

{
	FuelCellStatus fc;
	Sat->GetFuelCellStatus(FuelCellIndicatorsSwitch->GetState(), fc);

	return fc.O2FlowLBH; 
}

void SaturnFuelCellO2FlowMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (v < 0.4)
		oapiBlt(drawSurface, NeedleSurface, 53, (111 - (int)(v / 0.4 * 21.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	else if (v < 1.2)
		oapiBlt(drawSurface, NeedleSurface, 53, (90 - (int)((v - 0.4) / 0.8 * 65.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface, 53, (25 - (int)((v - 1.2) / 0.4 * 21.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnFuelCellTempMeter::QueryValue()

{
	FuelCellStatus fc;
	Sat->GetFuelCellStatus(FuelCellIndicatorsSwitch->GetState(), fc);

	return fc.TempF; 
}

void SaturnFuelCellTempMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (v < 400.0)
		oapiBlt(drawSurface, NeedleSurface, 86, (109 - (int)((v - 100.0) / 300.0 * 53.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
	else if (v < 500.0)
		oapiBlt(drawSurface, NeedleSurface, 86, (56 - (int)((v - 400.0) / 100.0 * 40.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface, 86, (16 - (int)((v - 500.0) / 50.0 * 12.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnFuelCellCondenserTempMeter::QueryValue()

{
	FuelCellStatus fc;
	Sat->GetFuelCellStatus(FuelCellIndicatorsSwitch->GetState(), fc);

	return fc.CondenserTempF; 
}

void SaturnFuelCellCondenserTempMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface, 139, (109 - (int)((v - 150.0) / 100.0 * 103.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


void SaturnCabinMeter::Init(SURFHANDLE surf, SwitchRow &row, Saturn *s)

{
	MeterSwitch::Init(row);
	NeedleSurface = surf;
	Sat = s;
}


double SaturnSuitTempMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	return KelvinToFahrenheit(atm.SuitTempK);
}

void SaturnSuitTempMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface,  1, (110 - (int)((v - 20.0) / 75.0 * 104.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnCabinTempMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	return KelvinToFahrenheit(atm.CabinTempK);
}

void SaturnCabinTempMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface,  53, (110 - (int)((v - 40.0) / 80.0 * 104.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnSuitPressMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	return atm.SuitPressurePSI;
}

void SaturnSuitPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (v < 6.0)
		oapiBlt(drawSurface, NeedleSurface,  101, (108 - (int)(v / 6.0 * 55.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface,  101, (53 - (int)((v - 6.0) / 10.0 * 45.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnCabinPressMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	return atm.CabinPressurePSI;
}

void SaturnCabinPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (v < 6.0)
		oapiBlt(drawSurface, NeedleSurface,  153, (108 - (int)(v / 6.0 * 55.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface,  153, (53 - (int)((v - 6.0) / 10.0 * 45.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


double SaturnPartPressCO2Meter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	return atm.SuitCO2MMHG;
}

void SaturnPartPressCO2Meter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (v < 10.0)
		oapiBlt(drawSurface, NeedleSurface,  215, (109 - (int)(v / 10.0 * 55.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	else if (v < 15.0)
		oapiBlt(drawSurface, NeedleSurface,  215, (54 - (int)((v - 10.0) / 5.0 * 19.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	else if (v < 20.0)
		oapiBlt(drawSurface, NeedleSurface,  215, (35 - (int)((v - 15.0) / 5.0 * 15.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	else
		oapiBlt(drawSurface, NeedleSurface,  215, (20 - (int)((v - 20.0) / 10.0 * 14.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}

void SaturnRoundMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s)

{
	RoundMeter::Init(p0, p1, row);
	Sat = s;
}

double SaturnSuitComprDeltaPMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	// Suit compressor pressure difference
	return (atm.SuitPressurePSI - atm.SuitReturnPressurePSI);
}

void SaturnSuitComprDeltaPMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - .5) / .5 * 60.0;
	DrawNeedle(drawSurface, 45, 22, 20.0, (180.0 - v) * RAD);
}


double SaturnLeftO2FlowMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	// O2 main regulator output flow 
	/// \todo Is this the correct flow for that meter? No documentation found yet...
	
	return atm.CabinRegulatorFlowLBH + atm.O2DemandFlowLBH + atm.DirectO2FlowLBH + 
		   atm.SuitTestFlowLBH + atm.CabinRepressFlowLBH + atm.EmergencyCabinRegulatorFlowLBH;
}

void SaturnLeftO2FlowMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - .6) / .4 * 60.0;	
	DrawNeedle(drawSurface, 0, 22, 20.0, v * RAD);
}


double SaturnSuitCabinDeltaPMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	// Suit cabin pressure difference
	return (atm.SuitReturnPressurePSI - atm.CabinPressurePSI) * (INH2O / PSI);
}

void SaturnSuitCabinDeltaPMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v / 5.0) * 60.0;
	DrawNeedle(drawSurface, 0, 22, 20.0, v * RAD);
	oapiBlt(drawSurface, FrameSurface, 0, 13, 0, 0, 46, 18, SURF_PREDEF_CK);
}


double SaturnRightO2FlowMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	// O2 main regulator output flow 	
	return atm.CabinRegulatorFlowLBH + atm.O2DemandFlowLBH + atm.DirectO2FlowLBH + 
		   atm.SuitTestFlowLBH + atm.CabinRepressFlowLBH + atm.EmergencyCabinRegulatorFlowLBH;
}

void SaturnRightO2FlowMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - .6) / .4 * 60.0;
	DrawNeedle(drawSurface, 45, 22, 20.0, (180.0 - v) * RAD);
	oapiBlt(drawSurface, FrameSurface, 0, 13, 0, 0, 46, 18, SURF_PREDEF_CK);
}


void SaturnEcsRadTempInletMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, RotationalSwitch *ecsindicatorsswitch)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	ECSIndicatorsSwitch = ecsindicatorsswitch;
}

double SaturnEcsRadTempInletMeter::QueryValue()

{
	if (ECSIndicatorsSwitch->GetState() == 1) {
		PrimECSCoolingStatus pcs;
		Sat->GetPrimECSCoolingStatus(pcs);
		return pcs.RadiatorInletTempF; 
	} else {
		SecECSCoolingStatus scs;
		Sat->GetSecECSCoolingStatus(scs);
		return scs.RadiatorInletTempF;
	}
}

void SaturnEcsRadTempInletMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 90.0) / 30.0 * 60.0;	
	DrawNeedle(drawSurface, 0, 22, 20.0, v * RAD);
}


double SaturnEcsRadTempPrimOutletMeter::QueryValue()

{
	PrimECSCoolingStatus pcs;
	Sat->GetPrimECSCoolingStatus(pcs);

	return pcs.RadiatorOutletTempF; 
}

void SaturnEcsRadTempPrimOutletMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 25.0) / 75.0 * 60.0;	
	DrawNeedle(drawSurface, 45, 22, 20.0, (180.0 - v) * RAD);
}


double SaturnEcsRadTempSecOutletMeter::QueryValue()

{
	SecECSCoolingStatus scs;
	Sat->GetSecECSCoolingStatus(scs);

	return scs.RadiatorOutletTempF; 
}

void SaturnEcsRadTempSecOutletMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 50.0) / 20.0 * 60.0;	
	DrawNeedle(drawSurface, 0, 22, 20.0, v * RAD);
}


void SaturnGlyEvapTempOutletMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, RotationalSwitch *ecsindicatorsswitch)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	ECSIndicatorsSwitch = ecsindicatorsswitch;
}

double SaturnGlyEvapTempOutletMeter::QueryValue()

{
	if (ECSIndicatorsSwitch->GetState() == 1) {
		PrimECSCoolingStatus pcs;
		Sat->GetPrimECSCoolingStatus(pcs);
		return pcs.EvaporatorOutletTempF; 
	} else {
		SecECSCoolingStatus scs;
		Sat->GetSecECSCoolingStatus(scs);
		return scs.EvaporatorOutletTempF;
	}
}

void SaturnGlyEvapTempOutletMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 50.0) / 20.0 * 60.0;	
	DrawNeedle(drawSurface, 45, 22, 20.0, (180.0 - v) * RAD);
}


void SaturnGlyEvapSteamPressMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, RotationalSwitch *ecsindicatorsswitch)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	ECSIndicatorsSwitch = ecsindicatorsswitch;
}

double SaturnGlyEvapSteamPressMeter::QueryValue()

{
	if (ECSIndicatorsSwitch->GetState() == 1) {
		PrimECSCoolingStatus pcs;
		Sat->GetPrimECSCoolingStatus(pcs);
		return pcs.EvaporatorSteamPressurePSI; 
	} else {
		SecECSCoolingStatus scs;
		Sat->GetSecECSCoolingStatus(scs);
		return scs.EvaporatorSteamPressurePSI;
	}
}

void SaturnGlyEvapSteamPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 0.15) / 0.1 * 60.0;	
	DrawNeedle(drawSurface, 0, 22, 20.0, v * RAD);
}


void SaturnGlycolDischPressMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, RotationalSwitch *ecsindicatorsswitch)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	ECSIndicatorsSwitch = ecsindicatorsswitch;
}

double SaturnGlycolDischPressMeter::QueryValue()

{
	if (ECSIndicatorsSwitch->GetState() == 1) {
		PrimECSCoolingStatus pcs;
		Sat->GetPrimECSCoolingStatus(pcs);
		return pcs.RadiatorInletPressurePSI; 
	} else {
		SecECSCoolingStatus scs;
		Sat->GetSecECSCoolingStatus(scs);
		return scs.RadiatorInletPressurePSI;
	}
}

void SaturnGlycolDischPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 30.0) / 30.0 * 60.0;	
	DrawNeedle(drawSurface, 45, 22, 20.0, (180.0 - v) * RAD);
}


void SaturnAccumQuantityMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, RotationalSwitch *ecsindicatorsswitch)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	ECSIndicatorsSwitch = ecsindicatorsswitch;
}

double SaturnAccumQuantityMeter::QueryValue()

{
	if (ECSIndicatorsSwitch->GetState() == 1) {
		PrimECSCoolingStatus pcs;
		Sat->GetPrimECSCoolingStatus(pcs);
		return pcs.AccumulatorQuantityPercent; 
	} else {
		SecECSCoolingStatus scs;
		Sat->GetSecECSCoolingStatus(scs);
		return scs.AccumulatorQuantityPercent;
	}
}

void SaturnAccumQuantityMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 0.5) / 0.5 * 60.0;	
	DrawNeedle(drawSurface, 0, 22, 20.0, v * RAD);
}


void SaturnH2oQuantityMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, ToggleSwitch *h2oqtyindswitch, CircuitBrakerSwitch *cba, CircuitBrakerSwitch *cbb)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	H2oQtyIndSwitch = h2oqtyindswitch;
	CbA = cba;
	CbB = cbb;
}

double SaturnH2oQuantityMeter::QueryValue()

{
	ECSWaterStatus ws;
	Sat->GetECSWaterStatus(ws);

	if (!CbA->IsPowered() && !CbB->IsPowered())
		return 0;
	else if (H2oQtyIndSwitch->IsUp())
		return ws.PotableH2oTankQuantityPercent; 
	else 
		return ws.WasteH2oTankQuantityPercent;
}

void SaturnH2oQuantityMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (v - 0.5) / 0.5 * 60.0;	
	DrawNeedle(drawSurface, 45, 22, 20.0, (180.0 - v) * RAD);
}


double SaturnAccelGMeter::QueryValue()

{
	return Sat->GetAccelG();
}

void SaturnAccelGMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (-v * 180.0 / 12.0) + 180.0;
	DrawNeedle (drawSurface, 40, 40, 35.0, v * RAD);
}


void DirectO2RotationalSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, h_Pipe *p)

{
	RotationalSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	Pipe = p;
}

/*bool DirectO2RotationalSwitch::CheckMouseClick(int event, int mx, int my)

{
	if (RotationalSwitch::CheckMouseClick(event, mx, my)) {
		CheckValve();
		return true;
	}
	return false;
}*/

bool DirectO2RotationalSwitch::SwitchTo(int newValue)

{
	if (RotationalSwitch::SwitchTo(newValue)) {
		CheckValve();
		return true;
	}
	return false;
}

void DirectO2RotationalSwitch::CheckValve()

{
	if (GetState() == 6) {
		Pipe->in->h_open = SP_VALVE_CLOSE;
		Pipe->flowMax = 0;
	
	} else if (GetState() == 5) {
		Pipe->in->h_open = SP_VALVE_OPEN;
		Pipe->flowMax = 0.1 / LBH;

	} else if (GetState() == 4) {
		Pipe->in->h_open = SP_VALVE_OPEN;
		Pipe->flowMax = 0.2 / LBH;

	} else if (GetState() == 3) {
		Pipe->in->h_open = SP_VALVE_OPEN;
		Pipe->flowMax = 0.31 / LBH;

	} else if (GetState() == 2) {
		Pipe->in->h_open = SP_VALVE_OPEN;
		Pipe->flowMax = 0.41 / LBH;

	} else if (GetState() == 1) {
		Pipe->in->h_open = SP_VALVE_OPEN;
		Pipe->flowMax = 0.53 / LBH;

	} else if (GetState() == 0) {
		Pipe->in->h_open = SP_VALVE_OPEN;
		Pipe->flowMax = 0.67 / LBH;
	}
}


void SaturnEcsGlycolPumpsSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Pump *p,
									  CircuitBrakerSwitch* ac1a, CircuitBrakerSwitch* ac1b, CircuitBrakerSwitch* ac1c,
									  CircuitBrakerSwitch* ac2a, CircuitBrakerSwitch* ac2b, CircuitBrakerSwitch* ac2c)

{
	RotationalSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	GlycolPump = p;
	ACBus1.WireToBuses(ac1a, ac1b, ac1c);
	ACBus2.WireToBuses(ac2a, ac2b, ac2c);
	CheckPump();
}

/*bool SaturnEcsGlycolPumpsSwitch::CheckMouseClick(int event, int mx, int my)

{
	if (RotationalSwitch::CheckMouseClick(event, mx, my)) {
		CheckPump();
		return true;
	}
	return false;
}*/

bool SaturnEcsGlycolPumpsSwitch::SwitchTo(int newValue)

{
	if (RotationalSwitch::SwitchTo(newValue)) {
		CheckPump();
		return true;
	}
	return false;
}

void SaturnEcsGlycolPumpsSwitch::LoadState(char *line)

{
	RotationalSwitch::LoadState(line);
	CheckPump();
}

void SaturnEcsGlycolPumpsSwitch::CheckPump()

{
	if (GetState() == 2) 
		GlycolPump->WireTo(NULL);
	else if (GetState() == 1 || GetState() == 3) 
		GlycolPump->WireTo(&ACBus1);
	else
		GlycolPump->WireTo(&ACBus2);
}


void SaturnSuitCompressorSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row,
									  CircuitBrakerSwitch* ac1a, CircuitBrakerSwitch* ac1b, CircuitBrakerSwitch* ac1c,
									  CircuitBrakerSwitch* ac2a, CircuitBrakerSwitch* ac2b, CircuitBrakerSwitch* ac2c)

{
	ACBus1.WireToBuses(ac1a, ac1b, ac1c);
	ACBus2.WireToBuses(ac2a, ac2b, ac2c);
	ThreeSourceSwitch::Init(xp, yp, w, h, surf, bsurf, row, &ACBus1, NULL, &ACBus2);
}

void DCBusIndicatorSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SwitchRow &row, DCBusController *d, int fc)

{
	IndicatorSwitch::Init(xp, yp, w, h, surf, row);
	dcbus = d;
	fuelcell = fc;
}

int DCBusIndicatorSwitch::GetState()

{
	if (dcbus->IsFuelCellConnected(fuelcell) || !dcbus->IsBusContPowered(fuelcell))
		return 1;
	else
		return 0;
}


SaturnDCAmpMeter::SaturnDCAmpMeter(double minVal, double maxVal, double vMin, double vMax) :
	ElectricMeter(minVal, maxVal, vMin, vMax)

{
}

void SaturnDCAmpMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, PowerStateRotationalSwitch *dcindicatorswitch)

{
	ElectricMeter::Init(p0, p1, row, dcindicatorswitch);
	DCIndicatorSwitch = dcindicatorswitch;
}

double SaturnDCAmpMeter::QueryValue()

{
	// Battery Charger
	if (DCIndicatorSwitch->GetState() == 7)
		return DCIndicatorSwitch->Current() * 20.0;

	return DCIndicatorSwitch->Current();
}

void BMAGPowerRotationalSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, BMAG *Unit)

{
	RotationalSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	bmag = Unit;
	
	CheckBMAGPowerState();
}

void BMAGPowerRotationalSwitch::CheckBMAGPowerState()

{
	switch (GetState()) {
	case 0: // OFF
		bmag->SetPower(false, false);
		break;
	case 1: // WARM UP
		bmag->SetPower(true, false);
		break;
	case 2: // ON
		bmag->SetPower(true, true);
		break;
	}
}

/*bool BMAGPowerRotationalSwitch::CheckMouseClick(int event, int mx, int my)

{
	if (RotationalSwitch::CheckMouseClick(event, mx, my)) {		
		CheckBMAGPowerState();
		return true;
	}

	return false;
}*/

bool BMAGPowerRotationalSwitch::SwitchTo(int newValue)

{
	if (RotationalSwitch::SwitchTo(newValue)) {
		CheckBMAGPowerState();
		return true;
	}

	return false;
}

void BMAGPowerRotationalSwitch::LoadState(char *line)

{
	RotationalSwitch::LoadState(line);
	CheckBMAGPowerState();
}


void SaturnSPSPercentMeter::Init(SURFHANDLE blackFontSurf, SURFHANDLE whiteFontSurf, SwitchRow &row, Saturn *s)

{
	MeterSwitch::Init(row);
	BlackFontSurface = blackFontSurf;
	WhiteFontSurface = whiteFontSurf;
	Sat = s;
}

void SaturnSPSPercentMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	int percent = (int) (v * 1000.0);

	// What should the panel display with full tanks? Looks like 99.9 is the maximum.
	if (percent > 999) {
		percent = 999;
	}

	int digit1 = percent / 100;
	percent -= (digit1 * 100);

	int digit2 = percent / 10;
	int digit3 = percent - (digit2 * 10);

	oapiBlt(drawSurface, BlackFontSurface, 0, 0, 10 * digit1, 0, 10, 12);
	oapiBlt(drawSurface, BlackFontSurface, 13, 0, 10 * digit2, 0, 10, 12);
	oapiBlt(drawSurface, WhiteFontSurface, 26, 0, 11 * digit3, 0, 11, 12);
}


double SaturnSPSOxidPercentMeter::QueryValue()

{
	return Sat->GetSPSPropellant()->GetOxidPercent(); 
}


double SaturnSPSFuelPercentMeter::QueryValue()

{
	return Sat->GetSPSPropellant()->GetFuelPercent(); 
}


double SaturnSPSOxidUnbalMeter::QueryValue()

{
	return Sat->GetSPSPropellant()->GetOxidUnbalanceLB();
}

void SaturnSPSOxidUnbalMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = v / 450.0 * 90.0;
	DrawNeedle(drawSurface, 30, 31, 28.0, (180.0 - v) * RAD);
}


void SaturnSPSPropellantPressMeter::Init(SURFHANDLE surf, SwitchRow &row, Saturn *s, bool fuel)

{
	MeterSwitch::Init(row);
	NeedleSurface = surf;
	Sat = s;
	Fuel = fuel;
}

double SaturnSPSPropellantPressMeter::QueryValue()

{
	// Fuel and oxidizer have the same pressure for now.
	return Sat->GetSPSPropellant()->GetPropellantPressurePSI();
}

void SaturnSPSPropellantPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (Fuel) {
		oapiBlt(drawSurface, NeedleSurface, 86, (109 - (int)(v / 250.0 * 103.0)), 0, 0, 10, 10, SURF_PREDEF_CK);	
	} else {
		oapiBlt(drawSurface, NeedleSurface, 139, (109 - (int)(v / 250.0 * 103.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
	}
}


void SaturnSPSTempMeter::Init(SURFHANDLE surf, SwitchRow &row, Saturn *s)

{
	MeterSwitch::Init(row);
	NeedleSurface = surf;
	Sat = s;
}

double SaturnSPSTempMeter::QueryValue()

{
	return Sat->GetSPSPropellant()->GetPropellantLineTempF();
}

void SaturnSPSTempMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface, 0, (109 - (int)(v / 200.0 * 103.0)), 0, 0, 10, 10, SURF_PREDEF_CK);
}


void SaturnSPSHeliumNitrogenPressMeter::Init(SURFHANDLE surf, SwitchRow &row, Saturn *s, ThreePosSwitch *spspressindswitch)

{
	MeterSwitch::Init(row);
	NeedleSurface = surf;
	Sat = s;
	SPSPressIndSwitch = spspressindswitch;
}

double SaturnSPSHeliumNitrogenPressMeter::QueryValue()

{
	if (SPSPressIndSwitch->IsUp())
		return Sat->GetSPSPropellant()->GetHeliumPressurePSI();
	else if (SPSPressIndSwitch->IsCenter())
		return Sat->GetSPSEngine()->GetNitrogenPressureAPSI();
	else 
		return Sat->GetSPSEngine()->GetNitrogenPressureBPSI();			
}

void SaturnSPSHeliumNitrogenPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface, 53, (109 - (int)(v / 5000.0 * 103.0)), 10, 0, 10, 10, SURF_PREDEF_CK);
}


void SaturnLVSPSPcMeter::Init(HPEN p0, HPEN p1, SwitchRow &row, Saturn *s, ToggleSwitch *lvspspcindicatorswitch, SURFHANDLE frameSurface)

{
	SaturnRoundMeter::Init(p0, p1, row, s);
	LVSPSPcIndicatorSwitch = lvspspcindicatorswitch;
	FrameSurface = frameSurface;
}

double SaturnLVSPSPcMeter::QueryValue()

{
	//
	// Note: on a real Saturn, it shows a percentage of pressure difference measured by 
	// the Q-ball of the LES. For now it shows AoA x 10
	//

	if (LVSPSPcIndicatorSwitch->IsDown()) {
		return Sat->GetSPSEngine()->GetChamberPressurePSI();

	} else {
		if (Sat->LETAttached() && Sat->GetDynPressure() > 100.0) {
			return fabs((10.0 / RAD) * Sat->GetAOA());
		} else {
			return 0;
		}
	}
}

void SaturnLVSPSPcMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = (155.0 - v) / 160.0 * 270.0;	
	DrawNeedle(drawSurface, 48, 45, 20.0, (v - 45.0) * RAD);
	oapiBlt(drawSurface, FrameSurface, 0, 0, 0, 0, 95, 91, SURF_PREDEF_CK);
}


void SaturnGPFPIMeter::Init(SURFHANDLE surf, SwitchRow &row, Saturn *s, ToggleSwitch *gpfpiindswitch, int xoffset)

{
	MeterSwitch::Init(row);
	NeedleSurface = surf;
	Sat = s;
	GPFPIIndicatorSwitch = gpfpiindswitch;
	xOffset = xoffset;
}

double SaturnGPFPIMeter::AdjustForPower(double val) 

{ 
	if (ACSource && DCSource) {
		if (ACSource->Voltage() > SP_MIN_ACVOLTAGE && DCSource->Voltage() > SP_MIN_DCVOLTAGE)
			return val;
	}
	return 0; 
}

void SaturnGPFPIMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	oapiBlt(drawSurface, NeedleSurface, xOffset,      93 - (int)v, 10, 1, 7, 8, SURF_PREDEF_CK);
	oapiBlt(drawSurface, NeedleSurface, xOffset + 12, 93 - (int)v,  3, 1, 7, 8, SURF_PREDEF_CK);
}


double SaturnGPFPIPitchMeter::QueryValue()

{
	if (GPFPIIndicatorSwitch->IsUp()) {
		
		LVTankQuantities LVq;
		Sat->GetLVTankQuantities(LVq);
		
		if (Sat->GetStage() > LAUNCH_STAGE_TWO_ISTG_JET) {  
			return 89.0 * LVq.SIVBOxQuantity / LVq.S4BOxMass;
		} 
		else {
			return 89.0 * LVq.SIIQuantity / LVq.SIIFuelMass;
		}
	}
	else {
		return (10.0 * Sat->GetSPSEngine()->pitchGimbalActuator.GetPosition()) + 44.0;
	}
}


double SaturnGPFPIYawMeter::QueryValue()

{
	if (GPFPIIndicatorSwitch->IsUp()) {
		
		LVTankQuantities LVq;
		Sat->GetLVTankQuantities(LVq);
		
		return 89.0 * LVq.SIVBFuelQuantity / LVq.S4BFuelMass;
	}
	else {
		return (10.0 * Sat->GetSPSEngine()->yawGimbalActuator.GetPosition()) + 44.0;
	}
}


void FDAIPowerRotationalSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, FDAI *F1, FDAI *F2,
    					 		     e_object *dc1, e_object *dc2, e_object *ac1, e_object *ac2,
									 SaturnGPFPIMeter *gpfpiPitch1, SaturnGPFPIMeter *gpfpiPitch2, SaturnGPFPIMeter *gpfpiYaw1, SaturnGPFPIMeter *gpfpiYaw2)
{
	RotationalSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	FDAI1 = F1;
	FDAI2 = F2;

	DCSource1 = dc1;
	DCSource2 = dc2;
	ACSource1 = ac1;
	ACSource2 = ac2;

	GPFPIPitch1 =gpfpiPitch1;
	GPFPIPitch2 =gpfpiPitch2;
	GPFPIYaw1 =gpfpiYaw1;
	GPFPIYaw2 =gpfpiYaw2;

	CheckFDAIPowerState();
}

//
// Wire up the FDAIs to the appropriate power source based on switch position. We wire them to this
// switch, and higher level code can wire the switch to a suitable power source.
//

void FDAIPowerRotationalSwitch::CheckFDAIPowerState()

{
	switch (GetState()) {
	case 0:
		FDAI1->WireTo(NULL, NULL);
		FDAI2->WireTo(NULL, NULL);

		GPFPIPitch1->WireTo(NULL, NULL);
		GPFPIPitch2->WireTo(NULL, NULL);
		GPFPIYaw1->WireTo(NULL, NULL);
		GPFPIYaw2->WireTo(NULL, NULL);
		break;

	case 1:
		FDAI1->WireTo(DCSource1, ACSource1);
		FDAI2->WireTo(NULL, NULL);

		GPFPIPitch1->WireTo(DCSource1, ACSource1);
		GPFPIPitch2->WireTo(NULL, NULL);
		GPFPIYaw1->WireTo(DCSource1, ACSource1);
		GPFPIYaw2->WireTo(NULL, NULL);
		break;

	case 2:
		FDAI1->WireTo(NULL, NULL);
		FDAI2->WireTo(DCSource2, ACSource2);

		GPFPIPitch1->WireTo(NULL, NULL);
		GPFPIPitch2->WireTo(DCSource2, ACSource2);
		GPFPIYaw1->WireTo(NULL, NULL);
		GPFPIYaw2->WireTo(DCSource2, ACSource2);
		break;

	case 3:
		FDAI1->WireTo(DCSource1, ACSource1);
		FDAI2->WireTo(DCSource2, ACSource2);

		GPFPIPitch1->WireTo(DCSource1, ACSource1);
		GPFPIPitch2->WireTo(DCSource2, ACSource2);
		GPFPIYaw1->WireTo(DCSource1, ACSource1);
		GPFPIYaw2->WireTo(DCSource2, ACSource2);
		break;
	}
}

/*bool FDAIPowerRotationalSwitch::CheckMouseClick(int event, int mx, int my)

{
	if (RotationalSwitch::CheckMouseClick(event, mx, my)) {
		CheckFDAIPowerState();
		return true;
	}

	return false;
}*/

bool FDAIPowerRotationalSwitch::SwitchTo(int newValue)

{
	if (RotationalSwitch::SwitchTo(newValue)) {
		CheckFDAIPowerState();
		return true;
	}

	return false;
}

void FDAIPowerRotationalSwitch::LoadState(char *line)

{
	RotationalSwitch::LoadState(line);
	CheckFDAIPowerState();
}


//
// CMACInverterSwitch allows you to connect the CM AC inverters to the CM AC busses.
//

void CMACInverterSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row,int bus,int inv, Saturn *ship)

{
	ToggleSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	acbus = bus;
	acinv = inv;
	sat = ship;
	UpdateSourceState();
}

/*bool CMACInverterSwitch::CheckMouseClick(int event, int mx, int my)

{
	if (ToggleSwitch::CheckMouseClick(event, mx, my))
	{
		UpdateSourceState();
		return true;
	}

	return false;
}*/

bool CMACInverterSwitch::SwitchTo(int newState, bool dontspring)

{
	if (ToggleSwitch::SwitchTo(newState,dontspring)) {
		UpdateSourceState();
		return true;
	}
	return false;
}

void CMACInverterSwitch::UpdateSourceState()
{	 
	if (IsUp()) {
		switch(acbus){
		case 1: // AC1
			switch(acinv){
			case 1:
				// Inverter 3 beats inverter 1
				if(sat->AcBus1Switch3.GetState() == TOGGLESWITCH_UP){
					return; // LOCKOUT
				}
				sat->ACBus1PhaseA.WireTo(&sat->Inverter1->PhaseA);
				sat->ACBus1PhaseB.WireTo(&sat->Inverter1->PhaseB);
				sat->ACBus1PhaseC.WireTo(&sat->Inverter1->PhaseC);
				break;
			case 2:
				// Inverter 1 beats inverter 2
				if(sat->AcBus1Switch1.GetState() == TOGGLESWITCH_UP){
					return; // LOCKOUT
				}
				sat->ACBus1PhaseA.WireTo(&sat->Inverter2->PhaseA);
				sat->ACBus1PhaseB.WireTo(&sat->Inverter2->PhaseB);
				sat->ACBus1PhaseC.WireTo(&sat->Inverter2->PhaseC);
				break;
			case 3:
				// Inverter 2 beats 3
				if(sat->AcBus1Switch2.GetState() == TOGGLESWITCH_UP){
					return; // LOCKOUT
				}
				sat->ACBus1PhaseA.WireTo(&sat->Inverter3->PhaseA);
				sat->ACBus1PhaseB.WireTo(&sat->Inverter3->PhaseB);
				sat->ACBus1PhaseC.WireTo(&sat->Inverter3->PhaseC);
				break;
			}
			break;
		case 2: // AC2
			switch(acinv){
			case 1:
				if(sat->AcBus2Switch3.GetState() == TOGGLESWITCH_UP){
					return; // LOCKOUT
				}
				sat->ACBus2PhaseA.WireTo(&sat->Inverter1->PhaseA);
				sat->ACBus2PhaseB.WireTo(&sat->Inverter1->PhaseB);
				sat->ACBus2PhaseC.WireTo(&sat->Inverter1->PhaseC);
				break;
			case 2:
				if(sat->AcBus2Switch1.GetState() == TOGGLESWITCH_UP){
					return; // LOCKOUT
				}
				sat->ACBus2PhaseA.WireTo(&sat->Inverter2->PhaseA);
				sat->ACBus2PhaseB.WireTo(&sat->Inverter2->PhaseB);
				sat->ACBus2PhaseC.WireTo(&sat->Inverter2->PhaseC);
				break;
			case 3:
				if(sat->AcBus2Switch2.GetState() == TOGGLESWITCH_UP){
					return; // LOCKOUT
				}
				sat->ACBus2PhaseA.WireTo(&sat->Inverter3->PhaseA);
				sat->ACBus2PhaseB.WireTo(&sat->Inverter3->PhaseB);
				sat->ACBus2PhaseC.WireTo(&sat->Inverter3->PhaseC);
				break;
			}
			break;
		}
	}
	else if (IsDown()) {
		// SHUTTING DOWN CASE
		switch(acbus){
		case 1: // AC1
			switch(acinv){
			case 1:
				if(sat->ACBus1PhaseA.SRC != &sat->Inverter1->PhaseA){ return; } // LOCKOUT
				sat->ACBus1PhaseA.WireTo(NULL);
				sat->ACBus1PhaseB.WireTo(NULL);
				sat->ACBus1PhaseC.WireTo(NULL);
				// Check inv 2
				if(sat->AcBus1Switch2.GetState() == TOGGLESWITCH_UP){ sat->AcBus1Switch2.UpdateSourceState(); }
				break;
			case 2:
				if(sat->ACBus1PhaseA.SRC != &sat->Inverter2->PhaseA){ return; } // LOCKOUT
				sat->ACBus1PhaseA.WireTo(NULL);
				sat->ACBus1PhaseB.WireTo(NULL);
				sat->ACBus1PhaseC.WireTo(NULL);
				if(sat->AcBus1Switch3.GetState() == TOGGLESWITCH_UP){ sat->AcBus1Switch3.UpdateSourceState(); }
				break;
			case 3:
				if(sat->ACBus1PhaseA.SRC != &sat->Inverter3->PhaseA){ return; } // LOCKOUT
				sat->ACBus1PhaseA.WireTo(NULL);
				sat->ACBus1PhaseB.WireTo(NULL);
				sat->ACBus1PhaseC.WireTo(NULL);
				if(sat->AcBus1Switch1.GetState() == TOGGLESWITCH_UP){ sat->AcBus1Switch1.UpdateSourceState(); }
				break;
			}
			break;
		case 2: // AC2
			switch(acinv){
			case 1:
				if(sat->ACBus2PhaseA.SRC != &sat->Inverter1->PhaseA){ return; } // LOCKOUT
				sat->ACBus2PhaseA.WireTo(NULL);
				sat->ACBus2PhaseB.WireTo(NULL);
				sat->ACBus2PhaseC.WireTo(NULL);
				if(sat->AcBus2Switch2.GetState() == TOGGLESWITCH_UP){ sat->AcBus2Switch2.UpdateSourceState(); }
				break;
			case 2:
				if(sat->ACBus2PhaseA.SRC != &sat->Inverter2->PhaseA){ return; } // LOCKOUT
				sat->ACBus2PhaseA.WireTo(NULL);
				sat->ACBus2PhaseB.WireTo(NULL);
				sat->ACBus2PhaseC.WireTo(NULL);
				if(sat->AcBus2Switch3.GetState() == TOGGLESWITCH_UP){ sat->AcBus2Switch3.UpdateSourceState(); }
				break;
			case 3:
				if(sat->ACBus2PhaseA.SRC != &sat->Inverter3->PhaseA){ return; } // LOCKOUT
				sat->ACBus2PhaseA.WireTo(NULL);
				sat->ACBus2PhaseB.WireTo(NULL);
				sat->ACBus2PhaseC.WireTo(NULL);
				if(sat->AcBus2Switch1.GetState() == TOGGLESWITCH_UP){ sat->AcBus2Switch1.UpdateSourceState(); }
				break;
			}
			break;
		}		
	}
}

void CMACInverterSwitch::LoadState(char *line)

{
	ToggleSwitch::LoadState(line);
	UpdateSourceState();
}


/*bool SaturnSCContSwitch::CheckMouseClick(int event, int mx, int my) 

{
	if (SaturnToggleSwitch::CheckMouseClick(event, mx, my)) {
		SetSCControl(sat);
		return true;
	}
	return false;
}*/

bool SaturnSCContSwitch::SwitchTo(int newState, bool dontspring)

{
	if (SaturnToggleSwitch::SwitchTo(newState,dontspring)) {
		SetSCControl(sat);
		return true;
	}
	return false;
}


void THCRotarySwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s)

{
	RotationalSwitch::Init(xp, yp, w, h, surf, bsurf, row);
	sat = s;
}

/*bool THCRotarySwitch::CheckMouseClick(int event, int mx, int my) 

{
	if (RotationalSwitch::CheckMouseClick(event, mx, my)) {
		SetSCControl(sat);
		return true;
	}
	return false;
}*/

bool THCRotarySwitch::SwitchTo(int newState)

{
	if (RotationalSwitch::SwitchTo(newState)) {
		SetSCControl(sat);
		return true;
	}
	return false;
}

void SaturnSCControlSetter::SetSCControl(Saturn *sat) {

	if (sat->SCContSwitch.IsUp() && !sat->THCRotary.IsClockwise()) {
		sat->agc.SetInputChannelBit(031, 15, true);
	} else {
		sat->agc.SetInputChannelBit(031, 15, false);
	}
}


SaturnEMSDvSetSwitch::SaturnEMSDvSetSwitch(Sound &clicksound) : ClickSound(clicksound) {

	position = 0;
}

void SaturnEMSDvDisplay::Init(SURFHANDLE digits, SwitchRow &row, Saturn *s)

{
	MeterSwitch::Init(row);
	Digits = digits;
	Sat = s;
}

void SaturnEMSDvDisplay::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	if (Voltage() < SP_MIN_DCVOLTAGE) return;
	if (Sat->ems.IsOff()) return; 

	if (v < 0) {
		oapiBlt(drawSurface, Digits, 0, 0, 161, 0, 10, 19);
	}

	int i, Curdigit;
	char buffer[100];
	sprintf(buffer, "%7.1f", fabs(v));
	for (i = 0; i < 7; i++) {
		if (buffer[i] >= '0' && buffer[i] <= '9') {
			Curdigit = buffer[i] - '0';
			oapiBlt(drawSurface, Digits, (i == 6 ? 0 : 10) + 16 * i, 0, 16 * Curdigit, 0, 16, 19);
		} else if (buffer[i] == '.') {
			oapiBlt(drawSurface, Digits, 10 + 16 * i, 0, 200, 0, 4, 19);
		}
	}
}

double SaturnEMSDvDisplay::QueryValue() {
	return Sat->ems.GetdVRangeCounter();
}

int SaturnEMSDvDisplay::GetState() {
	return (int) (Sat->ems.GetdVRangeCounter() * 10.);
}

void SaturnEMSDvDisplay::SetState(int value) {
	Sat->ems.dVRangeCounter = value / 10.;
}


void SaturnEMSScrollDisplay::Init(SwitchRow &row, Saturn *s) {
	MeterSwitch::Init(row);
	Sat = s;
}

int SaturnEMSScrollDisplay::GetState() {
	return (int) (Sat->ems.ScrollPosition);
}

void SaturnEMSScrollDisplay::SetState(int value) {
	Sat->ems.ScrollPosition = value;
}


void SaturnPanel382Cover::Init(SwitchRow &row, Saturn *s) {
	MeterSwitch::Init(row);
	Sat = s;
}

int SaturnPanel382Cover::GetState() {
	return Sat->panel382Enabled;
}

void SaturnPanel382Cover::SetState(int value) {
	Sat->panel382Enabled = value;
}


void SaturnPanel600::Init(SwitchRow &row, Saturn *s) {
	MeterSwitch::Init(row);
	Sat = s;
}

int SaturnPanel600::GetState() {
	return Sat->hatchPanel600EnabledLeft;
}

void SaturnPanel600::SetState(int value) {

	if (value == 0) value = -1;

	Sat->hatchPanel600EnabledLeft = value;
	Sat->hatchPanel600EnabledRight = value;
}


void SaturnPanelOrdeal::Init(SwitchRow &row, Saturn *s) {
	MeterSwitch::Init(row);
	Sat = s;
}

int SaturnPanelOrdeal::GetState() {
	return Sat->ordealEnabled;
}

void SaturnPanelOrdeal::SetState(int value) {

	if (value == 0) value = -1;

	Sat->ordealEnabled = value;
}


void SaturnASCPSwitch::Init(SwitchRow &row, Saturn *s, int axis) {
	MeterSwitch::Init(row);
	Sat = s;
	Axis = axis;
}

int SaturnASCPSwitch::GetState() {
	return (int) (Sat->ascp.output.data[Axis] * 10.);
}

void SaturnASCPSwitch::SetState(int value) {
	Sat->ascp.output.data[Axis] = value / 10.;
}


void SaturnAbortSwitch::Init(SwitchRow &row, Saturn *s) {
	MeterSwitch::Init(row);
	Sat = s;
}

int SaturnAbortSwitch::GetState() {
	return (Sat->bAbort ? TOGGLESWITCH_UP : TOGGLESWITCH_DOWN);
}

void SaturnAbortSwitch::SetState(int value) {
	// Nothing for now
}


bool SaturnEMSDvSetSwitch::CheckMouseClick(int event, int mx, int my) {

	int oldPos = position;
	switch(event) {
		case PANEL_MOUSE_LBPRESSED:
			if (my < 44)
				position = 1;
			else
				position = 3;			
			break;

		case PANEL_MOUSE_RBPRESSED:
			if (my < 44)
				position = 2;
			else
				position = 4;
			break;

		case PANEL_MOUSE_LBUP:
		case PANEL_MOUSE_RBUP:
			position = 0;
			break;
	}
	if (position && position != oldPos)
		ClickSound.play();
	return true;
}


void SaturnCabinPressureReliefLever::InitGuard(SURFHANDLE surf, SoundLib *soundlib) { 
	
	guardSurface = surf;

	if (!guardClick.isValid())
		soundlib->LoadSound(guardClick, GUARD_SOUND, INTERNAL_ONLY);
}

void SaturnCabinPressureReliefLever::DrawSwitch(SURFHANDLE drawSurface) {

	oapiBlt(drawSurface, guardSurface, 0, 0, guardState * 152, 0, 152, 79, SURF_PREDEF_CK);
	ThumbwheelSwitch::DrawSwitch(drawSurface);
}

bool SaturnCabinPressureReliefLever::CheckMouseClick(int event, int mx, int my) {

	if (event & PANEL_MOUSE_RBDOWN) {
		if (mx <= 152 &&  my <= 79) {			
			if (guardState) {
				guardState = 0;
			} else {
				guardState = 1;
			}
			guardClick.play();
			return true;

		} else {
			return false;
		}
	} else {
		bool r = ThumbwheelSwitch::CheckMouseClick(event, mx, my);
		if (state == 3 && guardState == 0) {
			state = 2;
			return false;
		} else {
			return r;
		}
	}
}

bool SaturnCabinPressureReliefLever::SwitchTo(int newState)
{
	if (ThumbwheelSwitch::SwitchTo(newState))
	{
		if(state == 3 && guardState == 0)
		{
			state = 2;
			return false;
		}
		else
			return true;
	}
	return false;
}

void SaturnCabinPressureReliefLever::SaveState(FILEHANDLE scn) {

	char buffer[100];

	sprintf(buffer, "%i %i", state, guardState); 
	oapiWriteScenario_string(scn, name, buffer);
}

void SaturnCabinPressureReliefLever::LoadState(char *line) {

	char buffer[100];
	int st, gst;

	sscanf(line, "%s %i %i", buffer, &st, &gst); 
	if (!strnicmp(buffer, name, strlen(name))) {
		state = st;
		guardState = gst;
	}
}


void OpticsHandcontrollerSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s) {

	sat = s;
	HandcontrollerSwitch::Init(xp, yp, w, h, surf, bsurf, row);
}

bool OpticsHandcontrollerSwitch::CheckMouseClick(int event, int mx, int my) {

	if (HandcontrollerSwitch::CheckMouseClick(event, mx, my)) {
		sat->optics.OpticsManualMovement &= 0xF0; 
		if (state == 1) {
			// Optics Right
			sat->optics.OpticsManualMovement |= 0x08; 
		} else if (state == 2) {
			// Optics Left
			sat->optics.OpticsManualMovement |= 0x04;
		} else if (state == 3) {
			// Optics Down
			sat->optics.OpticsManualMovement |= 0x02; 
		} else if (state == 4) {
			// Optics Up
			sat->optics.OpticsManualMovement |= 0x01; 
		}
		return true;
	}
	return false;
}


void MinImpulseHandcontrollerSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s) {

	sat = s;
	HandcontrollerSwitch::Init(xp, yp, w, h, surf, bsurf, row);
}

bool MinImpulseHandcontrollerSwitch::CheckMouseClick(int event, int mx, int my) {

	if (HandcontrollerSwitch::CheckMouseClick(event, mx, my)) {
		unsigned int c = sat->agc.GetInputChannel(032);
		c &= 077700;
		if (state == 1) {
			// roll right
			c |= 020;			
		} else if (state == 2) {
			// roll left
			c |= 040;			
		} else if (state == 3) {
			// pitch down
			c |= 02;
		} else if (state == 4) {
			// pitch up
			c |= 01;
		} else if (state == 5) {
			// yaw left
			c |= 010;			
		} else if (state == 6) {
			// yaw right
			c |= 04;			
		}		
		sat->agc.SetInputChannel(032, c);
		return true;
	}
	return false;
}

void DSEIndicatorSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SwitchRow &row, DSE *d, bool failopen)
{
	dse = d;
	IndicatorSwitch::Init(xp, yp, w, h, surf, row, failopen);
}

int DSEIndicatorSwitch::GetState()
{
	return dse->TapeMotion() ? 1 : 0;
}


//
// "Special offset" of 130 px to avoid overlapping with the O2DemandRegulatorRotary
//

void SuitTestSwitch::DrawSwitch(SURFHANDLE drawSurface) {

	if (position) {
		// Find best bitmap for position angle
		int index = -1;
		for (int i = 0; i < RotationalSwitchBitmapCount; i++) {
			if (index == -1) {
				index = i;
			} else if (AngleDiff(bitmaps[i].angle, position->GetAngle()) < AngleDiff(bitmaps[index].angle, position->GetAngle())) {
				index = i;
			}
		}
		oapiBlt(drawSurface, switchSurface, x, y, (bitmaps[index].xOffset * width) + 130, bitmaps[index].yOffset * height, width - 130, height, SURF_PREDEF_CK);
	}
}

void SuitTestSwitch::DrawFlash(SURFHANDLE DrawSurface) {

	if (!visible)
		return;

	if (switchBorder)
		oapiBlt(DrawSurface, switchBorder, x, y, 0, 0, width - 130, height, SURF_PREDEF_CK);
}

bool SuitTestSwitch::CheckMouseClick(int event, int mx, int my) {
	
	return RotationalSwitch::CheckMouseClick(event, mx + 130, my);
}


double SaturnOxygenRepressPressMeter::QueryValue()

{
	AtmosStatus atm;
	Sat->GetAtmosStatus(atm);

	return atm.O2RepressPressurePSI;
}

void SaturnOxygenRepressPressMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface)

{
	v = 115.0 - v / 1200.0 * 50.0 ;	
	DrawNeedle(drawSurface, 55, 440, 60.0, v * RAD);
	oapiBlt(drawSurface, FrameSurface, 0, 413, 0, 110, 90, 30, SURF_PREDEF_CK);
}

// LM power switch
void CSMLMPowerSwitch::Init(int xp, int yp, int w, int h, SURFHANDLE surf, SURFHANDLE bsurf, SwitchRow &row, Saturn *s){
	sat = s;
	ThreePosSwitch::Init(xp, yp, w, h, surf, bsurf, row);
}

bool CSMLMPowerSwitch::CheckMouseClick(int event, int mx, int my)

{
	if (SaturnThreePosSwitch::CheckMouseClick(event, mx, my)) {
		// First -- Are we docked?
		if(sat->dockingprobe.IsHardDocked() == false){ return true; }
		// Umbilical connected? (Supposed to be by docking!)
		if(sat->CSMToLEMConnector.connectedTo == NULL){ return true; }
		ConnectorMessage msg;
		ConnectorMessageValue mval;
		msg.destination = LEM_CSM_POWER;		
		msg.messageType = 42;
		switch(state){
			case THREEPOSSWITCH_UP:
				// Connect the bus
				sat->CSMToLEMPowerDrain.Enable();
				sat->CSMToLEMPowerDrain.WireTo(&sat->LMUmbilicalFeeder);				
				// Turn on LEM				
				mval.iValue = 1; // Relay State 1 = Deny Descent ECA operation
				msg.val1 = mval;
				sat->CSMToLEMConnector.SendMessage(msg);
				break;
			case THREEPOSSWITCH_CENTER:
				// Turn off LEM (but don't disconnect it)
				sat->CSMToLEMPowerDrain.Disable();
				sat->CSMToLEMPowerDrain.WireTo(NULL);
				break;
			case THREEPOSSWITCH_DOWN:
				// Reset LEM				
				mval.iValue = 0; // Relay State 0 = Permit Descent ECA operation
				msg.val1 = mval;
				sat->CSMToLEMConnector.SendMessage(msg);
				// Ensure disconnected
				sat->CSMToLEMPowerDrain.Disable();
				sat->CSMToLEMPowerDrain.WireTo(NULL);
				break;
		}

		return true;
	}
	return false;
}

bool CSMLMPowerSwitch::SwitchTo(int newState)
{
	if (SaturnThreePosSwitch::SwitchTo(newState)) {
		return true;
	}
	return false;
}

void OrdealRotationalSwitch::DrawSwitch(SURFHANDLE drawSurface) {

	RotationalSwitch::DrawSwitch(drawSurface);

	if (mouseDown) {
		RECT rt;
		char label[100];
		sprintf(label, "%d", value);

		HDC hDC = oapiGetDC(drawSurface);
		HFONT font = CreateFont(22, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
		SelectObject(hDC, font);
		SetTextColor(hDC, RGB(255, 255, 255));
		SetTextAlign(hDC, TA_CENTER);
		SetBkMode(hDC, OPAQUE);
		SetBkColor(hDC, RGB(146, 146, 146));
		
		if (GetState() == 0) {
			rt.left = 29 + x;
			rt.top = 24 + y;
			rt.right = 60 + x;
			rt.bottom = 55 + y;
			ExtTextOut(hDC, 44 + x, 28 + y, ETO_OPAQUE, &rt, label, strlen(label), NULL);

		} else if (GetState() == 1) {
			rt.left = 35 + x;
			rt.top = 30 + y;
			rt.right = 59 + x;
			rt.bottom = 52 + y;
			ExtTextOut(hDC, 49 + x, 31 + y, ETO_OPAQUE, &rt, label, strlen(label), NULL);

		} else if (GetState() == 2) {
			rt.left = 29 + x;
			rt.top = 29 + y;
			rt.right = 60 + x;
			rt.bottom = 59 + y;
			ExtTextOut(hDC, 44 + x, 34 + y, ETO_OPAQUE, &rt, label, strlen(label), NULL);

		} else if (GetState() == 3) {
			TextOut(hDC, 42 + x, 36 + y, label, strlen(label));
		
		} else if (GetState() == 4) {
			rt.left = 28 + x;
			rt.top = 30 + y;
			rt.right = 54 + x;
			rt.bottom = 60 + y;
			ExtTextOut(hDC, 37 + x, 34 + y, ETO_OPAQUE, &rt, label, strlen(label), NULL);

		} else if (GetState() == 5) {
			TextOut(hDC, 32 + x, 31 + y, label, strlen(label));

		} else if (GetState() == 6) {
			rt.left = 25 + x;
			rt.top = 24 + y;
			rt.right = 55 + x;
			rt.bottom = 54 + y;
			ExtTextOut(hDC, 39 + x, 28 + y, ETO_OPAQUE, &rt, label, strlen(label), NULL);
		} 

		DeleteObject(font);
		oapiReleaseDC(drawSurface, hDC);
	}
}

bool OrdealRotationalSwitch::CheckMouseClick(int event, int mx, int my) {

	if (event & PANEL_MOUSE_LBDOWN) {
		// Check whether it's actually in our switch region.
		if (mx < x || my < y)
			return false;

		if (mx > (x + width) || my > (y + height))
			return false;

		lastX = mx;
		mouseDown = true;

	} else if (((event & PANEL_MOUSE_LBPRESSED) != 0) && mouseDown) {
		if (abs(mx - lastX) >= 2) {
			value += (int) ((mx - lastX) / 2.);
			value = min(max(value, 10), 310);
			lastX = mx;
		}

	} else if (event & PANEL_MOUSE_LBUP) {
		mouseDown = false;
		return false;
	}
	SetValue((int) ((value / 50.) + 0.5));
	return true;
}

void OrdealRotationalSwitch::SaveState(FILEHANDLE scn) {

	if (position) {
		oapiWriteScenario_int (scn, name, value); 
	}		
}

void OrdealRotationalSwitch::LoadState(char *line) {

	char buffer[100];
	int val;

	sscanf(line, "%s %i", buffer, &val); 
	if (!strnicmp(buffer, name, strlen(name))) {
		value = val;
		SetValue((int) ((value / 50.) + 0.5));
	}
}

double SaturnHighGainAntennaPitchMeter::QueryValue(){
	return Sat->hga.Pitch; 
}

void SaturnHighGainAntennaPitchMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface){
	// Gauge runs from -110 to 110 degrees. Sign matches.
	v = v * 1.222222;
	DrawNeedle(drawSurface, 30, 25, 20.0, v * RAD);
}

double SaturnHighGainAntennaStrengthMeter::QueryValue(){
	return Sat->hga.SignalStrength; 
}

void SaturnHighGainAntennaStrengthMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface){
	// Gauge runs from 180 to 0. Sign reversed.
	v = 180-(v*1.8);
	DrawNeedle(drawSurface, 107, 25, 20.0, v * RAD);
}

double SaturnHighGainAntennaYawMeter::QueryValue(){
	return Sat->hga.Yaw; 
}

void SaturnHighGainAntennaYawMeter::DoDrawSwitch(double v, SURFHANDLE drawSurface){
	// Gauge runs from 220 to -40. Sign reversed.
	v = 220-(v*0.722222);
	DrawNeedle(drawSurface, 185, 25, 20.0, v * RAD);
}

double SaturnLMDPGauge::QueryValue(){
	return 4; // FIXME: Return actual data when actual data available
}

void SaturnLMDPGauge::DoDrawSwitch(double v, SURFHANDLE drawSurface){
	// Gauge runs from 122.5 to 57.5, sign reversed.
	v = 122.5-((v+1)*13);	
	DrawNeedle(drawSurface, 43, 77, 20.0, v * RAD);
}

void SaturnLMDPGauge::DrawNeedle (SURFHANDLE surf, int x, int y, double rad, double angle){
	// Needle function by Rob Conley from Mercury code
	// This one needs a longer and offset needle

	double dx = rad * cos(angle), dy = rad * sin(angle);
	HGDIOBJ oldObj;

	HDC hDC = oapiGetDC (surf);
	oldObj = SelectObject (hDC, Pen1);
	MoveToEx (hDC, x + (int)(2*dx+0.5), y - (int)(2*dy+0.5), 0); 
	LineTo (hDC, x + (int)(3*dx+0.5), y - (int)(3*dy+0.5));
	SelectObject (hDC, oldObj);
	oapiReleaseDC (surf, hDC);
}

