This is a plugin for the Teamspeak 3 VOIP program to enable dice rolls to be made in the client.  Very useful for those wishing to play old-school pencil and paper RPGs online.

My friends and I like using VOIP applications for our weekly traditional RPG games, as it makes gameplay so much faster than when playing in IRC. To help facilitate our games (and perhaps yours) I've made a small plugin to roll dice.

### Usage ###
Once the plugin is loaded, simply type a typical 'dice command' enclosed by parentheses. For example, (3d6).

You may also use modifiers to act as bonuses or penalties, like this (1d20-2).

Anybody may use the above commands, not just the person that installed the plugin! For this reason, it is usually a good idea to only have one person at a time in your group with this plugin, and unless all dice rolling will be public, it should be the GM.

### Features ###

  * Private channel for rolls the GM doesn't want the players to see.
  * Dice rolls can be made in channels or private messages, so the GM and a player may roll dice in private.
  * Only one user (preferably the GM) need load the plugin.
  * Highlighting of natural 1's and natural perfects to help comparing failures and successes for those games that use that system.
  * Uses Mersenne-Twister random algorithm to help make it as random as possible.


### Known Issues ###
Private dice rolling window doesn't appear if you enable the plugin while already connected to a server. Either enable the plugin before connecting, or disconnect and reconnect after enabling the plugin.

### Planned Changes ###
A configuration window for ... configurations.
A popup window with buttons representing dice commands for quick access to often used rolls.
I'm not entirely happy with how the results look. As I get feedback from the community and my game buddies, I'll hopefully find something better.

Let me know if you would like to help with these.


---


## Installation ##
  1. Download the correct file from the downloads section.  Pay special attention to the 32bit or 64bit suffix- match it to the installation of Teamspeak you have installed.
  1. Open the zip file in your favorite archiving program and decompress it into your Teamspeak 3 plugin directory.  For most people, that will be `C:\Program Files\TeamSpeak 3 Client\plugins`
  1. Start Teamspeak 3.  Click Settings->Plugins.  Click the checkbox next to the Dice Roller plugin.
  1. Restart Teamspeak so the private dice rolling area can be created.