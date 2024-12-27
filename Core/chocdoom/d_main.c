//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "doomfeatures.h"
#include "sounds.h"

#include "d_iwad.h"

#include "z_zone.h"
#include "w_main.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "p_saveg.h"

#include "i_endoom.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"

#include "g_game.h"

#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "net_client.h"
#include "net_dedicated.h"
#include "net_query.h"

#include "p_setup.h"
#include "r_local.h"
#include "statdump.h"


#include "d_main.h"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);

// Location where savegames are stored

char *          savegamedir;

// location of IWAD and WAD files

char *          iwadfile;


boolean		devparm;	// started game with -devparm
boolean         nomonsters;	// checkparm of -nomonsters
boolean         respawnparm;	// checkparm of -respawn
boolean         fastparm;	// checkparm of -fast

//extern int soundVolume;
//extern  int	sfxVolume;
//extern  int	musicVolume;

extern  boolean	inhelpscreens;

skill_t		startskill;
int             startepisode;
int		startmap;
boolean		autostart;
int             startloadgame;

boolean		advancedemo;

// Store demo, do not accept any inputs
boolean         storedemo;

// "BFG Edition" version of doom2.wad does not include TITLEPIC.
boolean         bfgedition;

// If true, the main game loop has started.
boolean         main_loop_started = false;

char		wadfile[1024];		// primary wad file
char		mapdir[1024];           // directory of development maps

int             show_endoom = 1;


void D_ConnectNetGame(void);
void D_CheckNetGame(void);


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*	ev;
	
    // IF STORE DEMO, DO NOT ACCEPT INPUT
    if (storedemo)
        return;
	
    while ((ev = D_PopEvent()) != NULL)
    {
	if (M_Responder (ev))
	    continue;               // menu ate the event
	G_Responder (ev);
    }
}




//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t     wipegamestate = GS_DEMOSCREEN;
extern  boolean setsizeneeded;
extern  int             showMessages;
void R_ExecuteSetViewSize (void);

void D_Display (void)
{
    static  boolean		viewactivestate = false;
    static  boolean		menuactivestate = false;
    static  boolean		inhelpscreensstate = false;
    static  boolean		fullscreen = false;
    static  gamestate_t		oldgamestate = -1;
    static  int			borderdrawcount;
    int				nowtime;
    int				tics;
    int				wipestart;
    int				y;
    boolean			done;
    boolean			wipe;
    boolean			redrawsbar;

    if (nodrawers)
    	return;                    // for comparative timing / profiling
		
    redrawsbar = false;
    
    // change the view size if needed
    if (setsizeneeded)
    {
		R_ExecuteSetViewSize ();
		oldgamestate = -1;                      // force background redraw
		borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
		{
		wipe = true;
		// wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
    	wipe = false;

    if (gamestate == GS_LEVEL && gametic)
    	HU_Erase();
    
    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
		if (!gametic)
			break;
		if (automapactive)
			AM_Drawer ();
		if (wipe || (viewheight != 200 && fullscreen) )
			redrawsbar = true;
		if (inhelpscreensstate && !inhelpscreens)
			redrawsbar = true;              // just put away the help screen
		ST_Drawer (viewheight == 200, redrawsbar );
		fullscreen = viewheight == 200;
		break;

      case GS_INTERMISSION:
		WI_Drawer ();
		break;

      case GS_FINALE:
		F_Drawer ();
		break;

      case GS_DEMOSCREEN:
		D_PageDrawer ();
		break;
    }
    
    // draw buffered stuff to screen
    I_UpdateNoBlit ();
    
    // draw the view directly
    if (gamestate == GS_LEVEL && !automapactive && gametic)
    	R_RenderPlayerView (&players[displayplayer]);

    if (gamestate == GS_LEVEL && gametic)
    	HU_Drawer ();
    
    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
    	I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
		viewactivestate = false;        // view was not active
		R_FillBackScreen ();    // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != 320)
    {
		if (menuactive || menuactivestate || !viewactivestate)
			borderdrawcount = 3;
		if (borderdrawcount)
		{
			R_DrawViewBorder ();    // erase old menu stuff
			borderdrawcount--;
		}
    }

    if (testcontrols)
    {
        // Box showing current mouse speed

        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;
    
    // draw pause pic
    if (paused)
    {
		if (automapactive)
			y = 4;
		else
			y = viewwindowy+4;
		V_DrawPatchDirect(viewwindowx + (scaledviewwidth - 68) / 2, y,
							  W_CacheLumpName (DEH_String("M_PAUSE"), PU_CACHE));
    }


    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation


    // normal update
    if (!wipe)
    {
	I_FinishUpdate ();              // page flip or blit buffer
	return;
    }
    
    // wipe update
    // wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    /*wipestart = I_GetTime () - 1;

    do
    {
	do
	{
	    nowtime = I_GetTime ();
	    tics = nowtime - wipestart;
            I_Sleep(1);
	} while (tics <= 0);
        
	wipestart = nowtime;
	// done = wipe_ScreenWipe(wipe_Melt
			    //    , 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
	I_UpdateNoBlit ();
	M_Drawer ();                            // menu is drawn even on top of wipes
	I_FinishUpdate ();                      // page flip or blit buffer
    } while (!done);*/
}

//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
    int i;

    M_ApplyPlatformDefaults();

    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
    M_BindWeaponControls();
    M_BindMapControls();
    M_BindMenuControls();
    M_BindChatControls(MAXPLAYERS);

    key_multi_msgplayer[0] = HUSTR_KEYGREEN;
    key_multi_msgplayer[1] = HUSTR_KEYINDIGO;
    key_multi_msgplayer[2] = HUSTR_KEYBROWN;
    key_multi_msgplayer[3] = HUSTR_KEYRED;

#ifdef FEATURE_MULTIPLAYER
    NET_BindVariables();
#endif

    M_BindVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindVariable("sfx_volume",             &sfxVolume);
    M_BindVariable("music_volume",           &musicVolume);
    M_BindVariable("show_messages",          &showMessages);
    M_BindVariable("screenblocks",           &screenblocks);
    M_BindVariable("detaillevel",            &detailLevel);
    M_BindVariable("snd_channels",           &snd_channels);
    M_BindVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
    M_BindVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    M_BindVariable("show_endoom",            &show_endoom);

    // Multiplayer chat macros

    /*for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindVariable(buf, &chat_macros[i]);
    }*/
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // Drone players don't need mouse focus

    if (drone)
        return false;

    // when menu is active or game is paused, release the mouse 
 
    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}

//
//  D_DoomLoop
//
void D_DoomLoop (void)
{
    main_loop_started = true;

    TryRunTics();

    I_SetWindowTitle(gamedescription);
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_InitGraphics();
    I_EnableLoadingDisk();

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();

    if (testcontrols)
    {
        wipegamestate = gamestate;
    }

    while (1)
    {
		// frame syncronous IO operations
		I_StartFrame ();

		TryRunTics (); // will run at least one tic

		S_UpdateSounds (players[consoleplayer].mo);// move positional sounds

		// Update display, next frame, with current state.
		if (screenvisible)
		{
			D_Display ();
		}
    }
}



//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char                    *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
	D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
    V_DrawPatch (0, 0, W_CacheLumpName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;

    // The Ultimate Doom executable changed the demo sequence to add
    // a DEMO4 demo.  Final Doom was based on Ultimate, so also
    // includes this change; however, the Final Doom IWADs do not
    // include a DEMO4 lump, so the game bombs out with an error
    // when it reaches this point in the demo sequence.

    // However! There is an alternate version of Final Doom that
    // includes a fixed executable.

    if (gameversion == exe_ultimate || gameversion == exe_final)
      demosequence = (demosequence+1)%7;
    else
      demosequence = (demosequence+1)%6;
    
    switch (demosequence)
    {
      case 0:
	if ( gamemode == commercial )
	    pagetic = TICRATE * 11;
	else
	    pagetic = 170;
	gamestate = GS_DEMOSCREEN;
	pagename = DEH_String("TITLEPIC");
	if ( gamemode == commercial )
	  S_StartMusic(mus_dm2ttl);
	else
	  S_StartMusic (mus_intro);
	break;
      case 1:
	G_DeferedPlayDemo(DEH_String("demo1"));
	break;
      case 2:
	pagetic = 200;
	gamestate = GS_DEMOSCREEN;
	pagename = DEH_String("CREDIT");
	break;
      case 3:
	G_DeferedPlayDemo(DEH_String("demo2"));
	break;
      case 4:
	gamestate = GS_DEMOSCREEN;
	if ( gamemode == commercial)
	{
	    pagetic = TICRATE * 11;
	    pagename = DEH_String("TITLEPIC");
	    S_StartMusic(mus_dm2ttl);
	}
	else
	{
	    pagetic = 200;

	    if ( gamemode == retail )
	      pagename = DEH_String("CREDIT");
	    else
	      pagename = DEH_String("HELP2");
	}
	break;
      case 5:
	G_DeferedPlayDemo(DEH_String("demo3"));
	break;
        // THE DEFINITIVE DOOM Special Edition demo
      case 6:
	G_DeferedPlayDemo(DEH_String("demo4"));
	break;
    }

    // The Doom 3: BFG Edition version of doom2.wad does not have a
    // TITLETPIC lump. Use INTERPIC instead as a workaround.
    if (bfgedition && !strcasecmp(pagename, "TITLEPIC")
        && W_CheckNumForName("titlepic") < 0)
    {
        pagename = DEH_String("INTERPIC");
    }
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo ();
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static char *banners[] =
{
    // doom2.wad
    "                         "
    "DOOM 2: Hell on Earth v%i.%i"
    "                           ",
    // doom1.wad
    "                            "
    "DOOM Shareware Startup v%i.%i"
    "                           ",
    // doom.wad
    "                            "
    "DOOM Registered Startup v%i.%i"
    "                           ",
    // Registered DOOM uses this
    "                          "
    "DOOM System Startup v%i.%i"
    "                          ",
    // doom.wad (Ultimate DOOM)
    "                         "
    "The Ultimate DOOM Startup v%i.%i"
    "                        ",
    // tnt.wad
    "                     "
    "DOOM 2: TNT - Evilution v%i.%i"
    "                           ",
    // plutonia.wad
    "                   "
    "DOOM 2: Plutonia Experiment v%i.%i"
    "                           ",
};

//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// 

static char *GetGameName(char *gamename)
{
    size_t i;
    char *deh_sub;
    return "";
}

//
// Find out what version of Doom is playing.
//

void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.

   /* if (gamemission == none)
    {
        unsigned int i;

        for (i=0; i<numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i].name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            } 
            else if (!strncasecmp(lumpinfo[i].name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
        {
            // Still no idea.  I don't think this is going to work.

            I_Error("Unknown or invalid IWAD file.");
        }
    }*/
    
    gamemission = doom2;
	gamemode = commercial;
    // Make sure gamemode is set up correctly

   /* if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (W_CheckNumForName("E4M1") > 0)
        {
            // Ultimate Doom

            gamemode = retail;
        } 
        else if (W_CheckNumForName("E3M1") > 0)
        {
            gamemode = registered;
        }
        else
        {
            gamemode = shareware;
        }
    }
    else
    {
        int p;

        // Doom 2 of some kind.
        gamemode = commercial;

    }*/
}

// Set the gamedescription string

void D_SetGameDescription(void)
{
    boolean is_freedoom = W_CheckNumForName("FREEDOOM") >= 0,
            is_freedm = W_CheckNumForName("FREEDM") >= 0;

    gamedescription = "Unknown";

    if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (is_freedoom)
        {
            gamedescription = GetGameName("Freedoom: Phase 1");
        }
        else if (gamemode == retail)
        {
            // Ultimate Doom

            gamedescription = GetGameName("The Ultimate DOOM");
        }
        else if (gamemode == registered)
        {
            gamedescription = GetGameName("DOOM Registered");
        }
        else if (gamemode == shareware)
        {
            gamedescription = GetGameName("DOOM Shareware");
        }
    }
    else
    {
        // Doom 2 of some kind.  But which mission?

        if (is_freedoom)
        {
            if (is_freedm)
            {
                gamedescription = GetGameName("FreeDM");
            }
            else
            {
                gamedescription = GetGameName("Freedoom: Phase 2");
            }
        }
        else if (logical_gamemission == doom2)
        {
            gamedescription = GetGameName("DOOM 2: Hell on Earth");
        }
        else if (logical_gamemission == pack_plut)
        {
            gamedescription = GetGameName("DOOM 2: Plutonia Experiment"); 
        }
        else if (logical_gamemission == pack_tnt)
        {
            gamedescription = GetGameName("DOOM 2: TNT - Evilution");
        }
    }
}

static boolean D_AddFile(char *filename)
{
    wad_file_t *handle;

    handle = W_AddFile(filename);

    return handle != NULL;
}


// Initialize the game version

static void InitGameVersion(void)
{
    gamemode = 2;
    gamemission = 1;
}

void PrintGameVersion(void)
{
    /*int i;

    for (i=0; gameversions[i].description != NULL; ++i)
    {
        if (gameversions[i].version == gameversion)
        {
            printf("Emulating the behavior of the "
                   "'%s' executable.\n", gameversions[i].description);
            break;
        }
    }*/
}

// Function called at exit to display the ENDOOM screen

static void D_Endoom(void)
{
    byte *endoom;

    // Don't show ENDOOM if we have it disabled, or we're running
    // in screensaver or control test mode. Only show it once the
    // game has actually started.

    if (!show_endoom || !main_loop_started
     || screensaver_mode > 0)
    {
        return;
    }

    endoom = W_CacheLumpName(DEH_String("ENDOOM"), PU_STATIC);

    I_Endoom(endoom);
}


//
// D_DoomMain
//
void D_DoomMain (void)
{
    int p;
    char file[256];
    char demolumpname[9];
#if ORIGCODE
    int numiwadlumps;
#endif

    //I_AtExit(D_Endoom, false);

    // print banner

    //I_PrintBanner(PACKAGE_STRING);
    
    DEBUGTXT("ZINIT");

    DEH_printf("Z_Init: Init zone memory allocation daemon. \n");
    Z_Init ();

    memcpy(states, state_init, sizeof(state_init));


    //!
    // @vanilla
    //
    // Disable monsters.
    //
	
    nomonsters = 0;

    //!
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = 0;

    //!
    // @vanilla
    //
    // Monsters move faster.
    //

    fastparm = 0;

    //! 
    // @vanilla
    //
    // Developer mode.  F1 saves a screenshot in the current working
    // directory.
    //

    devparm = 0;

	DEBUGTXT("I_DisplayFPSDots");
    I_DisplayFPSDots(devparm);


    if (devparm)
	DEH_printf(D_DEVSTR);
    
    // find which dir to use for config files

    {
        // Auto-detect the configuration dir.

        M_SetConfigDir(NULL);
    }

    //!
    // @arg <x>
    // @vanilla
    //
    // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
    // x defaults to 200.  Values are rounded up to 10 and down to 400.
    //

    
    // init subsystems
    V_Init ();

    M_SetConfigFilenames("default.cfg", PROGRAM_PREFIX "doom.cfg");
    D_BindVariables();
    M_LoadDefaults();
    
    DEBUGTXT("M_LoadDefaults");

    // Save configuration at exit.
    // I_AtExit(M_SaveDefaults, false);

    // Find main IWAD file and load it.
    iwadfile = D_FindIWAD(IWAD_MASK_DOOM, &gamemission);

    modifiedgame = false;

    D_AddFile(iwadfile);
#if ORIGCODE
    numiwadlumps = numlumps;
#endif

    W_CheckCorrectIWAD(doom);

    // Now that we've loaded the IWAD, we can figure out what gamemission
    // we're playing and which version of Vanilla Doom we need to emulate.
    D_IdentifyVersion();
    InitGameVersion();


	DEBUGTXT("modifiedgame");

    // Load PWAD files.
    modifiedgame = 0;

    // Debug:
//    W_PrintDirectory();


    //I_AtExit((atexit_func_t) G_CheckDemoStatus, true);

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // Load DEHACKED lumps from WAD files - but only if we give the right
    // command line parameter.


    // Set the gamedescription string. This is only possible now that
    // we've finished loading Dehacked patches.
    D_SetGameDescription();

    {
        savegamedir = "";
    }



    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(true);
    I_InitMusic();
    
    DEBUGTXT("I_InitMusic");


    // get skill / episode / map from parms
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;


    // Check for load game parameter
    // We do this here and save the slot number, so that the network code
    // can override it or send the load slot to other players.

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in slot s.
    //

	startloadgame = -1;
    
    DEBUGTXT("M_Init");

    M_Init ();
    
    DEBUGTXT("R_Init");

    R_Init ();
    
    DEBUGTXT("P_Init");

    P_Init ();

    S_Init (sfxVolume * 8, musicVolume * 8);

	DEBUGTXT("D_CheckNetGame");

    DEH_printf("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    DEH_printf("HU_Init: Setting up heads up display.\n");
    HU_Init ();

    DEH_printf("ST_Init: Init status bar.\n");
    ST_Init ();
    
    DEBUGTXT("ST_Init");


/*
    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
		singledemo = true;              // quit after one demo
		G_DeferedPlayDemo (demolumpname);
		D_DoomLoop ();  // never returns
    } else {
        singledemo = false;
    }*/
   /* 
    if (startloadgame >= 0)
    {
        M_StringCopy(file, P_SaveGameFile(startloadgame), sizeof(file));
        G_LoadGame(file);
    }
*/
    if (gameaction != ga_loadgame )
    {
		if (autostart || netgame)
			G_InitNew (startskill, startepisode, startmap);
		else
		{
			DEBUGTXT("D_StartTitle");
			D_StartTitle ();                // start up intro loop
		}
    }



    D_DoomLoop ();  // never returns
}

