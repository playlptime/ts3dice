/*
 * TeamSpeak 3 dice plugin
 *
 */



#ifdef WINDOWS
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <randomc.h>
#include "mersenne.cpp"
#include "public_errors.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"
#include "plugin_events.h"
#include "dice_plugin.h"
#include <math.h>

static struct TS3Functions ts3Functions;

#ifdef WINDOWS
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); dest[destSize-1] = '\0'; }
#endif

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128

static char* pluginCommandID = NULL;
/* Define and init the random number class instance.  No return values, so assume it works :( */
CRandomMersenne RanGen((int)time(0));

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
    return "Dice Roller";
}

/* Plugin version */
const char* ts3plugin_version() {
    return "1.0.5";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return 8;
}

/* Plugin author */
const char* ts3plugin_author() {
    return "TheCraiggers";
}

/* Plugin description */
const char* ts3plugin_description() {
    return "This plugin rolls dice for you and anybody else that asks.\n\nDice commands are required to begin and end with parentheses.  For example, '(3d6+4)'.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {

    /* Your plugin init code here:
	   We don't have any init code, so lets just do a test to the console to make sure it works. */
	printf("RandomNumber Test: %d\n", RanGen.IRandom(0,99));

    return 0;  /* 0 = success, 1 = failure */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
    /* Your plugin cleanup code here */

	/* Free pluginCommandID if we registered it */
	if(pluginCommandID) {
		free(pluginCommandID);
		pluginCommandID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	printf("PLUGIN: offersConfigure\n");
	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
	return PLUGIN_OFFERS_NO_CONFIGURE;  /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window.If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
    printf("PLUGIN: configure\n");
}

/*
* If the plugin wants to use plugin commands, it needs to register a command ID. This function will be automatically called after
* the plugin was initialized. This function is optional. If you don't use plugin commands, the function can be omitted.
* Note the passed commandID parameter is no longer valid after calling this function, so you *must* copy it and store it in the plugin.
*/
//void ts3plugin_registerPluginCommandID(const char* commandID) {
//	const size_t sz = strlen(commandID) + 1;
//	pluginCommandID = (char*)malloc(sz);
//	memset(pluginCommandID, 0, sz);
//	_strcpy(pluginCommandID, sz, commandID);  /* The commandID buffer will invalidate after existing this function */
//	printf("PLUGIN: registerPluginCommandID: %s\n", pluginCommandID);
//}

/* Plugin command keyword. Return NULL or "" if not used. */
const char* ts3plugin_commandKeyword() {
	return "";
}

/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {

    if(newStatus == STATUS_CONNECTION_ESTABLISHED) {  /* connection established and we have client and channels available */
        anyID myID;

        /* Get client's ID */
        if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
            ts3Functions.logMessage("Error querying client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            return;
        }
		else
		{
			//Bring up a PM tab for yourself
			if(ts3Functions.requestSendPrivateTextMsg(serverConnectionHandlerID, "This is your private dice rolling area.", myID, NULL) != ERROR_ok) {
				ts3Functions.logMessage("Error requesting send text message", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			}
		}
    }
}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored) {
    printf("PLUGIN: onTextMessageEvent %llu %d %d %s %s %d\n", (long long unsigned int)serverConnectionHandlerID, targetMode, fromID, fromName, message, ffIgnored);

	/* Friend/Foe manager has ignored the message, so ignore here as well. */
	if(ffIgnored) {
		return 0; /* Client will ignore the message anyways, so return value here doesn't matter */
	}

	//Dice Rolling code begins here.
	//Lets check to see if its a dice command first
	if(message[0] == '(') 
	{
		//Looks like it is.  Start gathering info
		int NumOfDice = 0;
		int DiceSides = 0;
		int Modifier = 0;
		int ModIsNegitive = 0;
		int *CurrentNumber = &NumOfDice;
		int i;
		int foo;
		int ResultsSize = 21;

		for (i=1; message[i]; i++)
		{
			printf("pos: %d, is: %c or %d\n", i, message[i],(int)message[i]);
			if (message[i] == 'd' || message[i] == 'D')
			{
				CurrentNumber = &DiceSides;
				printf("Found a D\n");
				ResultsSize = ResultsSize + 2;
			}
			if (message[i] == '+')
			{
				ResultsSize = ResultsSize + 4;
				CurrentNumber = &Modifier;
			}
			if (message[i] == '-')
			{
				CurrentNumber = &Modifier;
				ModIsNegitive = 1;
				ResultsSize = ResultsSize + 4;
			}
			if (message[i]>47 && message[i]<58)
			{
				//Looks like we have a number.  Stuff it somewhere.
				foo = message[i] - 48;
				//if (foo==0)
					//foo = 10;
				printf("Found a number: %d, CurrentNumber: %d\n", foo,*CurrentNumber);
				*CurrentNumber = (*CurrentNumber * 10) + foo;
				printf("After adding: %d\n", *CurrentNumber);
				ResultsSize++;
			}
			if (message[i] == ')' && NumOfDice && DiceSides)
			{
				//The command is finished.  Compute it here.
				if (ModIsNegitive)
					Modifier = Modifier * -1;

				printf("Dice: %d D %d %d\n", NumOfDice,DiceSides,Modifier);
				int *Dice = 0;
				int Total = 0;
				Dice = new int[NumOfDice];
				for (foo = 1; foo < NumOfDice+1; foo++)
				{
					Dice[foo] = RanGen.IRandom(1,DiceSides);
					Total = Total + Dice[foo];
					printf("%d, %d\n", Dice[foo], Total);
					ResultsSize = ResultsSize + (log((double)Dice[foo]+1) + 2);
					if (Dice[foo] == 1 || Dice[foo] == DiceSides)
						ResultsSize = ResultsSize + 30;
				}

				printf("ResultsSize: %d\n",ResultsSize);

				//Lets create our string containing the results of the Dice roll
				char *Results;
				char buff[100];

				Results = new char[ResultsSize];
				Results[0] = 0;
				strcat (Results, "[B]{");
				strcat (Results, _itoa(NumOfDice,buff,10));
				strcat (Results, "D");
				strcat (Results, itoa(DiceSides,buff,10));

				strcat (Results, "}[/B] = [");
				for (foo = 1; foo<NumOfDice+1; foo++)
				{
					if (Dice[foo] == 1)
					{
						strcat(Results, "[COLOR=#ff0000][B]");
						strcat (Results, itoa(Dice[foo],buff,10));
						strcat(Results, "[/B][/COLOR]");
					}
					else if (Dice[foo] == DiceSides)
					{
						strcat(Results, "[COLOR=#00AA00][B]");
						strcat (Results, itoa(Dice[foo],buff,10));
						strcat(Results, "[/B][/COLOR]");
					}
					else
						strcat (Results, itoa(Dice[foo],buff,10));
					if (foo < NumOfDice)
						strcat (Results, ", ");
				}
				strcat (Results, "] = ");
				strcat (Results, itoa(Total,buff,10));

				if (Modifier)
					if (ModIsNegitive)
					{
						Modifier = Modifier * -1;
						strcat (Results, " - ");
						strcat (Results, itoa(Modifier,buff,10));
						strcat (Results, " = ");
						strcat (Results, itoa(Total - Modifier,buff,10));
					}
					else
					{
						strcat (Results, " + ");
						strcat (Results, itoa(Modifier,buff,10));
						strcat (Results, " = ");
						strcat (Results, itoa(Total + Modifier,buff,10));
					}

				printf(">%s<\n",Results);
				switch (targetMode)	{
					case TextMessageTarget_CLIENT:
						//Looks like a PM.  Respond to just that person.
						anyID myID;
						if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
							ts3Functions.logMessage("Error querying own client id", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
							return 0;
						}
						if(ts3Functions.requestSendPrivateTextMsg(serverConnectionHandlerID, Results, fromID, NULL) != ERROR_ok) {
							ts3Functions.logMessage("Error requesting send text message", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
						}
						break;

					case TextMessageTarget_CHANNEL:
						if(ts3Functions.requestSendChannelTextMsg(serverConnectionHandlerID, Results, fromID, NULL) != ERROR_ok) 
						{
							ts3Functions.logMessage("Error requesting send text message to channel", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
						}
						break;

					case TextMessageTarget_SERVER:
						if(ts3Functions.requestSendServerTextMsg(serverConnectionHandlerID, Results, NULL) != ERROR_ok) 
						{
							ts3Functions.logMessage("Error requesting send text message to channel", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
						}
						break;
				}
				if (!message[i+1] == 0){
					printf("More!");
					ts3plugin_onTextMessageEvent(serverConnectionHandlerID, targetMode, toID, fromID, fromName, fromUniqueIdentifier, (const char*) &message[i+1], ffIgnored);
				}
				return 0;
			}
		}
	}

    return 0;  /* 0 = handle normally, 1 = client will ignore the text message */
}
