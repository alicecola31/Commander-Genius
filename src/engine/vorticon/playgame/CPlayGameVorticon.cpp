/*
 * CPlayGameVorticon.cpp
 *
 *  Created on: 03.10.2009
 *      Author: gerstrong
 *
 */

#include "CPlayGameVorticon.h"
#include "sdl/CTimer.h"
#include "sdl/CVideoDriver.h"
#include "sdl/sound/CSound.h"
#include "sdl/input/CInput.h"
#include "common/CMapLoader.h"
#include "graphics/CGfxEngine.h"
#include "StringUtils.h"
#include "engine/vorticon/ai/CTeleporter.h"

#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }

////
// Creation Routine
////
CPlayGameVorticon::CPlayGameVorticon( CExeFile &ExeFile, char level,
		  char numplayers, Uint8& difficulty, CSaveGameController &SavedGame) :
CPlayGame(ExeFile, level, numplayers, difficulty),
mp_ObjectAI(NULL),
mp_HUD(NULL)
{
	m_level_command = (level==WORLD_MAP_LEVEL_VORTICON) ? GOTO_WORLD_MAP : START_LEVEL;
	mp_Finale = NULL;
	mp_gameoverbmp = NULL;

	if(!m_Player.empty())
		m_Player.clear();

	m_Player.assign(m_NumPlayers, CPlayer(m_Episode, m_Level,
			mp_level_completed,
			m_Object, m_Map));

	for(int i=0 ; i<m_NumPlayers ; i++)
	{
		// Put some important Player properties
		CPlayer &thisPlayer = m_Player.at(i);
		thisPlayer.m_index = i;
		thisPlayer.setDatatoZero();
		thisPlayer.mp_camera->attachObject(&thisPlayer);
	}

	// Create completed level list
	memset(mp_level_completed,false,MAX_LEVELS_VORTICON*sizeof(bool));

	m_showPauseDialog = false;

	if(m_Difficulty==0)
		g_pGfxEngine->Palette.setdarkness(FADE_DARKNESS_EASY);
	else if(m_Difficulty==1)
		g_pGfxEngine->Palette.setdarkness(FADE_DARKNESS);
	else
		g_pGfxEngine->Palette.setdarkness(FADE_DARKNESS_HARD);

	//if(finale) m_level_command = GOTO_FINALE;
}

// Setup all the players, when one level is started
void CPlayGameVorticon::setupPlayers()
{
	m_showKeensLeft=false;
	std::vector<CPlayer>::iterator it_player = m_Player.begin();
	for( ; it_player!=m_Player.end() ; it_player++ )
	for (int i=0 ; i<m_NumPlayers ; i++)
	{
		if( m_Level == WORLD_MAP_LEVEL_VORTICON )
		{
			it_player->m_playingmode = CPlayer::WORLDMAP;
			m_showKeensLeft |= ( it_player->pdie == PDIE_DEAD );
			if(it_player->godmode) it_player->solid = false;
		}
		else
		{
			it_player->m_playingmode = CPlayer::LEVELPLAY;
			it_player->sprite = PSTANDFRAME;
			it_player->solid=true;
		}
		it_player->pdie = PDIE_NODIE;

		// Calibrate Player to the right position, so it won't fall when level starts
		CSprite &sprite = g_pGfxEngine->getSprite(PSTANDFRAME);
		it_player->w = sprite.getWidth()<<STC;
		it_player->h = sprite.getHeight();//<<STC;
		it_player->m_level = m_Level;
		m_Map.m_Dark = false;
		g_pGfxEngine->Palette.setdark(m_Map.m_Dark);

		// Set the pointers to the map and object data
		it_player->setMapData(&m_Map);
		it_player->exists = true;
		if(it_player->m_playingmode == CPlayer::WORLDMAP) it_player->solid=!(it_player->godmode);
	}

	stInventory &inventory = m_Player.at(0).inventory;

	if(mp_HUD) delete mp_HUD;
	mp_HUD = new CHUD(inventory.score, inventory.lives, inventory.charges);
}

bool CPlayGameVorticon::init()
{
	CMapLoader MapLoader( &m_Map, &m_Player );
	MapLoader.m_checkpointset = m_checkpointset;
	MapLoader.mp_objvect = &m_Object;

	m_Map.m_Difficulty = m_Difficulty;

	// load level map
	if( !MapLoader.load( m_Episode, m_Level, m_Gamepath ) ) return false;
	m_SavedGame.setLevel(m_Level);

	//// If those worked fine, continue the initialization
	// draw level map
	m_Map.drawAll();

	// Now Scroll to the position of the player and center him
	m_Map.gotoPos( 32, 64 ); // Assure that the edges are never seen

	setupPlayers();

	// Well, all players are living because they were newly spawn.
	g_pTimer->ResetSecondsTimer();

	g_pInput->flushAll();

	// Initialize the AI
	mp_ObjectAI = new CObjectAI(&m_Map, m_Object, m_Player,
								m_NumPlayers, m_Episode, m_Level,
								m_Difficulty, m_Map.m_Dark);

	// Check if Player meets the conditions to show a cutscene. This also happens, when finale of episode has reached
	verifyFinales();

	// When Level starts it's never dark!
	g_pGfxEngine->Palette.setdark(false);

	if(m_level_command == GOTO_FINALE)
		createFinale();
	else
		if(m_showKeensLeft)	g_pSound->playSound(SOUND_KEENSLEFT, PLAY_NOW);

	// In the case that we are in Episode 3 last Level, show Mortimer Messages
	if( m_Episode == 3 && m_Level == 16 )
	{
		m_MessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString("EP3_MORTIMER"),false, true));
		m_MessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString("EP3_MORTIMER2"),false, true));
		m_MessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString("EP3_MORTIMER3"),false, true));
		m_MessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString("EP3_MORTIMER4"),false, true));
		m_MessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString("EP3_MORTIMER5"),false, true));
		m_MessageBoxes.push_back(new CMessageBoxVort(g_pBehaviorEngine->getString("EP3_MORTIMER6"),false, true));
		g_pSound->playSound(SOUND_MORTIMER);
	}

	return true;
}

////
// Process Routine
////
void CPlayGameVorticon::process()
{
	// Check for fading processes if necessary
	if(g_pGfxEngine->Palette.in_progress())
		g_pGfxEngine->Palette.applyFade();

	/*if(mp_HighScores) // Are we requesting Highscores
	{
		mp_HighScores->process();

		// Blit the background
		g_pVideoDriver->mDrawTasks.add( new BlitScrollSurfaceTask() );

		if(mp_HighScores->destroyed())
		{
			SAFE_DELETE(mp_HighScores);
			m_endgame = true;
		}
	}*/
	else // No, we are in the middle of the game
	{
		if( !m_paused && m_MessageBoxes.empty() ) // Game is not paused, no messages have to be shown and no menu is open
		{
			if (!mp_Finale) // Hasn't the game yet been finished?
			{
				// Perform AIs
				mp_ObjectAI->process();

				/// The following functions must be worldmap dependent
				if( m_Level == WORLD_MAP_LEVEL_VORTICON )
					processOnWorldMap();
				else
					processInLevel();

				// Does one of the players need to pause the game?
				for( int i=0 ; i<m_NumPlayers ; i++ )
				{
					// Did he open the status screen?
					if(m_Player[i].m_showStatusScreen)
						m_paused = true; // this is processed in processPauseDialogs!

					if(!m_Player[0].pdie)
						m_Player[0].processCamera();
				}
			}
			else // In this case the Game has been finished, goto to the cutscenes
			{
				mp_Finale->process();

				if(mp_Finale->getHasFinished())
				{
					SAFE_DELETE(mp_Finale);

					if(!m_gameover)
					{
						//mp_HighScores = new CHighScores(true);
						collectHighScoreInfo();
					}
				}

				m_Player[0].processEvents();
			}
		}

		// Draw all the Stuff here!
		drawAllElements();

		if( m_Level == WORLD_MAP_LEVEL_VORTICON && m_showKeensLeft )
			showKeensLeft();


		// Check if we are in gameover mode. If yes, than show the bitmaps and block the FKeys().
		// Only confirmation button is allowes
		if(m_gameover && !mp_Finale) // game over mode
		{
			if(mp_gameoverbmp != NULL)
			{
				mp_gameoverbmp->process();

				if( g_pInput->getPressedAnyCommand() )
				{
					//mp_HighScores = new CHighScores(true);

					collectHighScoreInfo();
				}
			}
			else // Bitmap must first be created
			{
				CBitmap *pBitmap = g_pGfxEngine->getBitmap("GAMEOVER");
				g_pSound->playSound(SOUND_GAME_OVER, PLAY_NOW);
				mp_gameoverbmp = new CEGABitmap(&m_Map , g_pVideoDriver->getBlitSurface(), pBitmap);
				mp_gameoverbmp->setScrPos( 160-(pBitmap->getWidth()/2), 100-(pBitmap->getHeight()/2) );
			}
		}
		else // No game over
		{
			// Handle special functional keys for paused game, F1 Help, god mode, all items, etc.
			handleFKeys();
		}


		if (g_pVideoDriver->getVidConfig().showfps)
		{
			SDL_Surface *sfc = g_pVideoDriver->mp_VideoEngine->getBlitSurface();
			std::string tempbuf;
#ifdef DEBUG
			tempbuf = "FPS: " + itoa(g_pTimer->getFramesPerSec()) +
								"; x = " + itoa(m_Player[0].getXPosition()) + " ; y = " + itoa(m_Player[0].getYPosition());
#else
			tempbuf = "FPS: " + itoa(g_pTimer->getFramesPerSec());
#endif
			g_pGfxEngine->getFont(1).drawFont(sfc,tempbuf,320-(tempbuf.size()<<3)-1, true);

		}
	}
}


void CPlayGameVorticon::handleFKeys()
{
	int i;

	// CTSpace Cheat
	if (g_pInput->getHoldedKey(KC) &&
			g_pInput->getHoldedKey(KT) &&
			g_pInput->getHoldedKey(KSPACE))
	{
		g_pInput->flushAll();
		for(i=0;i<m_NumPlayers;i++)
		{
			m_Player[i].pfiring = false;
			if (m_Player[i].m_playingmode)
			{
				CPhysicsSettings &Phy = g_pBehaviorEngine->getPhysicsSettings();

				if(Phy.misc.ctspace_keys)
				{
					m_Player[i].give_keycard(DOOR_YELLOW);
					m_Player[i].give_keycard(DOOR_RED);
					m_Player[i].give_keycard(DOOR_GREEN);
					m_Player[i].give_keycard(DOOR_BLUE);
				}
				else
				{
					m_Player[i].take_keycard(DOOR_YELLOW);
					m_Player[i].take_keycard(DOOR_RED);
					m_Player[i].take_keycard(DOOR_GREEN);
					m_Player[i].take_keycard(DOOR_BLUE);
				}

				m_Player[i].inventory.charges = Phy.misc.ctspace_ammo;
				m_Player[i].inventory.HasPogo = true;
				m_Player[i].inventory.lives += 5;

				std::string Text = g_pBehaviorEngine->getString("CTSPACECHEAT");

				m_MessageBoxes.push_back(new CMessageBoxVort(Text));
				m_paused = true;
			}
		}
		g_pVideoDriver->AddConsoleMsg("All items cheat");
	}

	// GOD cheat -- toggle god mode
	if ( g_pInput->getHoldedKey(KG) && g_pInput->getHoldedKey(KO) && g_pInput->getHoldedKey(KD) )
	{
		std::vector<CPlayer>::iterator it_player = m_Player.begin();
		for( ; it_player != m_Player.end() ; it_player++)
		{
			it_player->godmode ^= 1;
			// If player on map, make disable the solid property of the players
			if(m_Level == 80)
				it_player->solid = !it_player->godmode;
			it_player->performCollisions();
		}

		g_pVideoDriver->DeleteConsoleMsgs();
		if (m_Player[0].godmode)
			g_pVideoDriver->AddConsoleMsg("God mode ON");
		else
			g_pVideoDriver->AddConsoleMsg("God mode OFF");

		g_pSound->playSound(SOUND_GUN_CLICK, PLAY_FORCE);

		// Show a message like in the original game
		m_MessageBoxes.push_back(new CMessageBoxVort(m_Player[0].godmode ? g_pBehaviorEngine->getString("GODMODEON") : g_pBehaviorEngine->getString("GODMODEOFF")));
		m_paused = true;
		g_pInput->flushKeys();
	}

	if(g_pInput->getPressedKey(KP) && m_MessageBoxes.empty())
	{
		g_pSound->playSound(SOUND_GUN_CLICK, PLAY_FORCE);
		m_MessageBoxes.push_back(new CMessageBoxVort("Game Paused"));
	}

	// Menus will only open if Keen is solid or in god mode. This means neither dying nor teleporting
	if( m_Player[0].solid || ( m_Player[0].godmode && !m_Player[0].dying ) )
	{
		// F2 - Sound Menu
		if ( g_pInput->getPressedKey(KF2) )
		{
			//mp_Menu = new CMenuVorticon( ACTIVE, m_Map,
				//	m_SavedGame,  m_restartVideo, m_hideobjects );
			//mp_Menu->init(AUDIO);
		}
		// F3 - Controls Menu
		else if ( g_pInput->getPressedKey(KF3) )
		{
			//mp_Menu = new CMenuVorticon( ACTIVE, m_Map,
				//	m_SavedGame,  m_restartVideo, m_hideobjects );
			//mp_Menu->init(CONTROLS);
		}
		// F5 - save game
		else if ( g_pInput->getPressedKey(KF5) )
		{
			//mp_Menu = new CMenuVorticon( ACTIVE, m_Map,
				//	m_SavedGame,  m_restartVideo, m_hideobjects );
			//mp_Menu->init(SAVE);
		}
	}
}

// The Ending and mortimer cutscenes for example
void CPlayGameVorticon::verifyFinales()
{
	// first we need to know which Episode we were on
	if(m_Episode == 1)
	{
		bool hasBattery, hasWiskey, hasJoystick, hasVaccum;
		hasBattery = hasWiskey = hasJoystick = hasVaccum = false;

		// Check if one of the Players has the items
		for( int i=0 ;i < m_NumPlayers ; i++)
		{
			hasBattery |= m_Player[i].inventory.HasBattery;
			hasWiskey |= m_Player[i].inventory.HasWiskey;
			hasJoystick |= m_Player[i].inventory.HasJoystick;
			hasVaccum |= m_Player[i].inventory.HasVacuum;
		}

		// If they have have the items, we can go home
		if(hasBattery && hasWiskey && hasJoystick && hasVaccum)
			createFinale();
	}
	else if(m_Episode == 2)
	{
		bool allCitiesSaved;
		allCitiesSaved = mp_level_completed[4];
		allCitiesSaved &= mp_level_completed[6];
		allCitiesSaved &= mp_level_completed[7];
		allCitiesSaved &= mp_level_completed[9];
		allCitiesSaved &= mp_level_completed[11];
		allCitiesSaved &= mp_level_completed[13];
		allCitiesSaved &= mp_level_completed[15];
		allCitiesSaved &= mp_level_completed[16];

		if(allCitiesSaved)
			createFinale();
	}
	else if(m_Episode == 3)
	{
		if(mp_level_completed[16] == true) // If this level is completed, Mortimer has been killed!
			createFinale();
	}
}

void CPlayGameVorticon::createFinale()
{
	if(m_Episode == 1)
	{
		mp_Finale = new CEndingEp1(m_Map, m_Player, m_hideobjects, m_Object);
	}
	else if(m_Episode == 2)
	{
		mp_Finale = new CEndingEp2(m_Map, m_Player, m_Object);
	}
	else if(m_Episode == 3)
	{
		mp_Finale = new CEndingEp3(m_Map, m_Player, m_Object);
	}
}

void CPlayGameVorticon::teleportPlayerFromLevel(CPlayer &player, int origx, int origy)
{
	int destx, desty;

	CTeleporter *teleporter = new CTeleporter(&m_Map, m_Player, origx, origy);
	player.beingteleported = true;
	player.solid = false;
	destx = g_pBehaviorEngine->getTeleporterTableAt(5).x;
	desty = g_pBehaviorEngine->getTeleporterTableAt(5).y;
	teleporter->solid = false;
	teleporter->direction = TELEPORTING_SCROLL;
	teleporter->destx = destx>>TILE_S;
	teleporter->desty = desty>>TILE_S;
	teleporter->whichplayer = player.m_index;
	m_Object.push_back(teleporter);
}

void CPlayGameVorticon::collectHighScoreInfo()
{
	if(m_Episode == 1)
	{
		bool extra[4];

		extra[0] = m_Player[0].inventory.HasJoystick;
		extra[1] = m_Player[0].inventory.HasBattery;
		extra[2] = m_Player[0].inventory.HasVacuum;
		extra[3] = m_Player[0].inventory.HasWiskey;

		//mp_HighScores->writeEP1HighScore(m_Player[0].inventory.score, extra);
	}
	else if(m_Episode == 2)
	{
		// episode 2: game is won when all cities are saved
		int saved_cities=0;
		if (mp_level_completed[4]) saved_cities++;
		if (mp_level_completed[6]) saved_cities++;
		if (mp_level_completed[7]) saved_cities++;
		if (mp_level_completed[13]) saved_cities++;
		if (mp_level_completed[11]) saved_cities++;
		if (mp_level_completed[9]) saved_cities++;
		if (mp_level_completed[15]) saved_cities++;
		if (mp_level_completed[16]) saved_cities++;

		//mp_HighScores->writeEP2HighScore(m_Player[0].inventory.score, saved_cities);
	}
	//else
		//mp_HighScores->writeHighScoreCommon(m_Player[0].inventory.score);
}

// This function draws the objects that need to be seen on the screen
void CPlayGameVorticon::drawObjects()
{
	if(m_hideobjects) return;

	std::vector<CObject*>::iterator it_obj = m_Object.begin();
	for(; it_obj!=m_Object.end() ; it_obj++)
	{
		(*it_obj)->draw();
	}

	// We draw the Player as last, because we want to see him in front of the other objects
	std::vector<CPlayer>::iterator it_player = m_Player.begin();
	std::vector<CPlayer>::iterator it_end = ( m_Level!=WORLD_MAP_LEVEL_VORTICON) ? m_Player.end() : m_Player.begin()+1;
	for (; it_player != it_end ; it_player++)
	{
		if(!it_player->beingteleported)
			it_player->draw();
	}

}

void CPlayGameVorticon::drawAllElements()
{
	// Animate the tiles of the map
	m_Map.animateAllTiles();

	// Blit the background
	g_pVideoDriver->mDrawTasks.add( new BlitScrollSurfaceTask() );

	// Draw all objects to the screen
	drawObjects();

	// Draw masked tiles here!
	m_Map.drawForegroundTiles();

	if(mp_option[OPT_HUD].value && !mp_Finale &&
			!m_paused )
	{	// Draw the HUD
		mp_HUD->render();
	}

	// Render the dialogs which are seen when the game is paused
	if( m_paused || !m_MessageBoxes.empty() )
	{
		// Finally draw Dialogs like status screen, game paused, etc.
		processPauseDialogs();
	}


}
////
// Cleanup Routine
////
void CPlayGameVorticon::cleanup()
{
	SAFE_DELETE(mp_ObjectAI);
	std::vector<CObject*>::iterator obj = m_Object.begin();
	for( ; obj != m_Object.end() ; obj++ )
		delete (*obj);
	m_Object.clear();
}

CPlayGameVorticon::~CPlayGameVorticon()
{
	m_Player.clear();
	if(mp_Finale) delete mp_Finale;
	mp_Finale = NULL;
	if(mp_gameoverbmp) delete mp_gameoverbmp;
	mp_gameoverbmp = NULL;
	SAFE_DELETE(mp_ObjectAI);
	SAFE_DELETE(mp_HUD);
}
