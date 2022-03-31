/*==============================================================================
	Giving credit where credit due
	Some ideas borrowed from Kory Gill 'New Study Template'
	https://github.com/korygill/technical-analysis
==============================================================================*/

#include "sierrachart.h"
SCDLLName("GC5150 Studies")
const SCString ContactInformation = "GC5150, @gc5150 (twitter)";

SCDateTime GetNow(SCStudyInterfaceRef sc)
{
	if (sc.IsReplayRunning())
		return sc.CurrentDateTimeForReplay;
	else
		return sc.CurrentSystemDateTime;
}

/*==============================================================================
	This study clears the Recent Bid and Ask Volume at the session start time(s)
	Sierra added member functions to accomplish this per request
	https://www.sierrachart.com/SupportBoard.php?ThreadID=71671
------------------------------------------------------------------------------*/
SCSFExport scsf_AutoClearRecentBidAskVolume(SCStudyInterfaceRef sc)
{
	// Session Start Times

	// Session 1
	SCInputRef Input_StartTimeSession1 = sc.Input[0];
	SCInputRef Input_StartTimeSession1Enabled = sc.Input[1];

	// Session 2
	SCInputRef Input_StartTimeSession2 = sc.Input[2];
	SCInputRef Input_StartTimeSession2Enabled = sc.Input[3];

	// Session 3
	SCInputRef Input_StartTimeSession3 = sc.Input[4];
	SCInputRef Input_StartTimeSession3Enabled = sc.Input[5];

	// Also clear current traded volume option
	SCInputRef Input_AlsoClearCurrentTradedVolume = sc.Input[6];

	// Track if session is cleared within the second avoid multiple clearing within the same second
	int &ClearedSession = sc.GetPersistentIntFast(1);

	// Current time independent of replay
	SCDateTime CurrentTime;

	if (sc.SetDefaults)
	{
		// Set the configuration and defaults
		sc.GraphName = "DOM: Auto Clear Recent Bid/Ask Volume";
		SCString studyDescription;
		studyDescription.Format("%s by %s", sc.GraphName.GetChars(), ContactInformation.GetChars());
		sc.StudyDescription = studyDescription;
		sc.AutoLoop = 0;
		sc.GraphRegion = 0;

		// Session 1 - Set to pull from Session Times (Day) via StartTime1
		Input_StartTimeSession1.Name = "Session 1: Start Time";
		Input_StartTimeSession1.SetDescription("Auto Clear Recent Bid/Ask Volume at Session 1 Start Time");
		Input_StartTimeSession1.SetTime(sc.StartTime1);

		Input_StartTimeSession1Enabled.Name = "Session 1: Auto Clear Enabled";
		Input_StartTimeSession1Enabled.SetDescription("Auto Clear for Session 1 Start Time Enabled");
		Input_StartTimeSession1Enabled.SetYesNo(1);

		// Session 2 - Set to pull from Session Times (Evening) via StartTime2
		Input_StartTimeSession2.Name = "Session 2: Start Time";
		Input_StartTimeSession2.SetDescription("Auto Clear Recent Bid/Ask Volume at Session 2 Start Time");
		Input_StartTimeSession2.SetTime(sc.StartTime2);

		Input_StartTimeSession2Enabled.Name = "Session 2: Auto Clear Enabled";
		Input_StartTimeSession2Enabled.SetDescription("Auto Clear for Session 2 Start Time Enabled");
		Input_StartTimeSession2Enabled.SetYesNo(0);

		// Session 3 - Ad Hoc Time
		Input_StartTimeSession3.Name = "Session 3: Start Time";
		Input_StartTimeSession3.SetDescription("Auto Clear Recent Bid/Ask Volume at Session 3 Start Time");
		Input_StartTimeSession3.SetTime(HMS_TIME(2, 0, 0));

		Input_StartTimeSession3Enabled.Name = "Session 3: Auto Clear Enabled";
		Input_StartTimeSession3Enabled.SetDescription("Auto Clear for Session 3 Start Time Enabled");
		Input_StartTimeSession3Enabled.SetYesNo(0);

		// Clear Current Traded Volume Option
		Input_AlsoClearCurrentTradedVolume.Name = "Also Clear Current Traded Volume";
		Input_AlsoClearCurrentTradedVolume.SetDescription("If enabled this also clears Current Traded Volume along with Recent Bid/Ask Volume");
		Input_AlsoClearCurrentTradedVolume.SetYesNo(1);

		return;
	}

	CurrentTime = GetNow(sc); // Get time once instead of calling it possibly 6 times later...

	// Return if cleared flag set and current time within same second as session times.
	// Logic is that first run clears the vol as wanted if we are at any of the session times and they are enabled
	// Run code, clear, set cleared flag. If we run again in the same time frame (only down to seconds) it won't trigger again
	// If chart update interval is such that it runs this several times a second it can clear many times and we only need to do once
	if (
		ClearedSession == 1 && CurrentTime.GetTimeInSeconds() == Input_StartTimeSession1.GetTime() || // Session 1
		ClearedSession == 1 && CurrentTime.GetTimeInSeconds() == Input_StartTimeSession2.GetTime() || // Session 2
		ClearedSession == 1 && CurrentTime.GetTimeInSeconds() == Input_StartTimeSession3.GetTime())	  // Session 3
		return;
	else
		ClearedSession = 0; // Reset flag as we are no longer within same session (second) and flag was previously set

	// Logic Check! If we made it this far we have cleared flag reset and can now check against session times and if they are enabled
	// If any of the three sessions are set/enabled and we hit that time, then clear recent bid/ask and also current traded if enabled
	if (
		((CurrentTime.GetTimeInSeconds() == Input_StartTimeSession1.GetTime()) && Input_StartTimeSession1Enabled.GetYesNo()) || // Session 1
		((CurrentTime.GetTimeInSeconds() == Input_StartTimeSession2.GetTime()) && Input_StartTimeSession2Enabled.GetYesNo()) || // Session 2
		((CurrentTime.GetTimeInSeconds() == Input_StartTimeSession3.GetTime()) && Input_StartTimeSession3Enabled.GetYesNo())	// Session 3
	)
	{
		// Clear Recent Bid/Ask Vol
		// New Feature requested and added recently 3/15/2022
		// https://www.sierrachart.com/SupportBoard.php?ThreadID=71671
		sc.ClearRecentBidAskVolume();

		// Also clear current traded volume if enabled
		if (Input_AlsoClearCurrentTradedVolume.GetYesNo() == 1)
			sc.ClearCurrentTradedBidAskVolume();

		ClearedSession = 1; // Set flag as we have cleared for this session time
	}
}