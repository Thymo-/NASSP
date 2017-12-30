/***************************************************************************
This file is part of Project Apollo - NASSP
Copyright 2017

Lunar Module Caution and Warning Electronics Assembly

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

#pragma once

class LEM;

class LEM_CWEA : public e_object {
public:
		LEM_CWEA(SoundLib &s);
		void Init(LEM *s, e_object *cwea_pwr, e_object *ma_pwr);
		void SaveState(FILEHANDLE scn, char *start_str, char *end_str);
		void LoadState(FILEHANDLE scn, char *end_str);
		bool IsPowered();
		void TimeStep(double simdt);
		void SystemTimeStep(double simdt);
		void SetMasterAlarm(bool alarm);
		void SetAlarmTone(bool tone);

		bool CheckMasterAlarmMouseClick(int event);
		void RenderMasterAlarm(SURFHANDLE surf, SURFHANDLE alarmLit, SURFHANDLE border);
		void RedrawLeft(SURFHANDLE sf, SURFHANDLE ssf);
		void RedrawRight(SURFHANDLE sf, SURFHANDLE ssf);

protected:
		int LightStatus[5][8];		// 1 = lit, 2 = not
		int CabinLowPressLt;		// FF for this
		int WaterWarningDisabled;   // FF for this
		bool MasterAlarm;
		bool AlarmTone;

		SoundLib &soundlib;
		Sound MasterAlarmSound;

		LEM *lem;					// Pointer at LEM
};