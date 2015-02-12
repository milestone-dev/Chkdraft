#include "Forces.h"
#include "Chkdraft.h"

UINT WM_DRAGNOTIFY(WM_NULL);

ForcesWindow::ForcesWindow() : playerBeingDragged(255)
{
	for ( int i=0; i<4; i++ )
		possibleForceNameUpdate[i] = false;
}

bool ForcesWindow::CreateThis(HWND hParent)
{
	if ( getHandle() != NULL )
		return SetParent(hParent);

	if ( ClassWindow::RegisterWindowClass(NULL, NULL, NULL, NULL, NULL, "Forces", NULL, false) &&
		 ClassWindow::CreateClassWindow(NULL, "Forces", WS_VISIBLE|WS_CHILD, 4, 22, 592, 524, hParent, (HMENU)ID_FORCES) )
	{
		GuiMap* map = chkd.maps.curr;
		HWND hForces = getHandle();
		textAboutForces.CreateThis(hForces, 5, 10, 587, 20,
			"Designate player forces, set force names, and force properties. It is recommended to separate computer and human players", 0);

		const char* forceGroups[] = { "Force 1", "Force 2", "Force 3", "Force 4" };
		for ( int y=0; y<2; y++ )
		{
			for ( int x=0; x<2; x++ )
			{
				int force = x+y*2;
				string forceName = "";
				bool allied = false, vision = false, random = false, av = false;
				if ( map != nullptr )
				{
					forceName = map->getForceString(force);
					map->getForceInfo(force, allied, vision, random, av);
				}

				groupForce[force].CreateThis(hForces, 5+293*x, 50+239*y, 288, 234, forceGroups[force], 0);
				editForceName[force].CreateThis(hForces, 20+293*x, 70+239*y, 268, 20, false, ID_EDIT_F1NAME+force);
				editForceName[force].SetText(forceName.c_str());
				dragForces[force].CreateThis(hForces, 20+293*x, 95+239*y, 268, 121, ID_LB_F1PLAYERS+force);
				checkAllied[force].CreateThis(hForces, 15+293*x, 232+239*y, 100, 20, allied, "Allied", ID_CHECK_F1ALLIED+force);
				checkSharedVision[force].CreateThis(hForces, 15+293*x, 252+239*y, 100, 20, vision, "Share Vision", ID_CHECK_F1VISION+force);
				checkRandomizeStart[force].CreateThis(hForces, 125+293*x, 232+239*y, 150, 20, random, "Randomize Start Location", ID_CHECK_F1RANDOM+force);
				checkAlliedVictory[force].CreateThis(hForces, 125+293*x, 252+239*y, 150, 20, av, "Enable Allied Victory", ID_CHECK_F1AV+force);
			}
		}

		if ( WM_DRAGNOTIFY == WM_NULL )
			WM_DRAGNOTIFY = RegisterWindowMessage(DRAGLISTMSGSTRING);

		return true;
	}
	else
		return false;
}

bool ForcesWindow::DestroyThis()
{
	playerBeingDragged = 255;
	return true;
}

void ForcesWindow::RefreshWindow()
{
	HWND hWnd = getHandle();
	GuiMap* map = chkd.maps.curr;
	if ( map != nullptr )
	{
		for ( int force=0; force<4; force++ )
		{
			string forceName;
			bool allied = false, vision = false, random = false, av = false;
			forceName = map->getForceString(force);
			map->getForceInfo(force, allied, vision, random, av);

			SetWindowText(GetDlgItem(hWnd, ID_EDIT_F1NAME+force), forceName.c_str());
			if ( allied ) SendMessage(GetDlgItem(hWnd, ID_CHECK_F1ALLIED+force), BM_SETCHECK, BST_CHECKED  , NULL);
			else		  SendMessage(GetDlgItem(hWnd, ID_CHECK_F1ALLIED+force), BM_SETCHECK, BST_UNCHECKED, NULL);
			if ( vision ) SendMessage(GetDlgItem(hWnd, ID_CHECK_F1VISION+force), BM_SETCHECK, BST_CHECKED  , NULL);
			else		  SendMessage(GetDlgItem(hWnd, ID_CHECK_F1VISION+force), BM_SETCHECK, BST_UNCHECKED, NULL);
			if ( random ) SendMessage(GetDlgItem(hWnd, ID_CHECK_F1RANDOM+force), BM_SETCHECK, BST_CHECKED  , NULL);
			else		  SendMessage(GetDlgItem(hWnd, ID_CHECK_F1RANDOM+force), BM_SETCHECK, BST_UNCHECKED, NULL);
			if ( av		) SendMessage(GetDlgItem(hWnd, ID_CHECK_F1AV	   +force), BM_SETCHECK, BST_CHECKED  , NULL);
			else		  SendMessage(GetDlgItem(hWnd, ID_CHECK_F1AV	   +force), BM_SETCHECK, BST_UNCHECKED, NULL);
		}

		for ( int i=0; i<4; i++ )
		{
			HWND hListBox = GetDlgItem(hWnd, ID_LB_F1PLAYERS+i);
			if ( hListBox != NULL )
				while ( SendMessage(hListBox, LB_DELETESTRING, 0, NULL) != LB_ERR );
		}

		for ( int player=0; player<8; player++ )
		{
			u8 force(0), color(0), race(0), displayOwner(map->getDisplayOwner(player));
			if ( map->getPlayerForce(player, force) )
			{
				map->getPlayerColor(player, color);
				map->getPlayerRace(player, race);
				stringstream ssplayer;
				ssplayer << "Player " << player+1 << " - " << playerColors[color] << " - "
						 << playerRaces[race] << " (" << playerOwners[displayOwner] << ")";
				HWND hListBox = GetDlgItem(hWnd, ID_LB_F1PLAYERS+force);
				if ( hListBox != NULL )
					SendMessage(hListBox, LB_ADDSTRING, NULL, (LPARAM)ssplayer.str().c_str());
			}
		}
	}
}

LRESULT ForcesWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		case WM_SHOWWINDOW:
			if ( wParam == TRUE )
				RefreshWindow();
			else
			{
				for ( int i=0; i<4; i++ )
					CheckReplaceForceName(i);
			}
			break;

		case WM_COMMAND:
			switch ( HIWORD(wParam) )
			{
				case BN_CLICKED:
					{
						GuiMap* map = chkd.maps.curr;
						LRESULT state = SendMessage((HWND)lParam, BM_GETCHECK, NULL, NULL);
						if ( map != nullptr )
						{
							switch ( LOWORD(wParam) )
							{
								case ID_CHECK_F1ALLIED: case ID_CHECK_F2ALLIED: case ID_CHECK_F3ALLIED: case ID_CHECK_F4ALLIED:
									{
										int force = LOWORD(wParam)-ID_CHECK_F1ALLIED;
										if ( state == BST_CHECKED )
											map->setForceAllied(force, true);
										else
											map->setForceAllied(force, false);

										chkd.maps.curr->notifyChange(false);
									}
									break;

								case ID_CHECK_F1VISION: case ID_CHECK_F2VISION: case ID_CHECK_F3VISION: case ID_CHECK_F4VISION:
									{
										int force = LOWORD(wParam)-ID_CHECK_F1VISION;
										if ( state == BST_CHECKED )
											map->setForceVision(force, true);
										else
											map->setForceVision(force, false);

										chkd.maps.curr->notifyChange(false);
									}
									break;

								case ID_CHECK_F1RANDOM: case ID_CHECK_F2RANDOM: case ID_CHECK_F3RANDOM: case ID_CHECK_F4RANDOM:
									{
										int force = LOWORD(wParam)-ID_CHECK_F1RANDOM;
										if ( state == BST_CHECKED )
											map->setForceRandom(force, true);
										else
											map->setForceRandom(force, false);

										chkd.maps.curr->notifyChange(false);
									}
									break;

								case ID_CHECK_F1AV: case ID_CHECK_F2AV: case ID_CHECK_F3AV: case ID_CHECK_F4AV:
									{
										int force = LOWORD(wParam)-ID_CHECK_F1AV;
										if ( state == BST_CHECKED )
											map->setForceAv(force, true);
										else
											map->setForceAv(force, false);

										chkd.maps.curr->notifyChange(false);
									}
									break;
							}
						}
					}
					break;

				case EN_CHANGE:
					if ( LOWORD(wParam) >= ID_EDIT_F1NAME && LOWORD(wParam) <= ID_EDIT_F4NAME )
						possibleForceNameUpdate[LOWORD(wParam)-ID_EDIT_F1NAME] = true;
					break;

				case EN_KILLFOCUS:
					if ( LOWORD(wParam) >= ID_EDIT_F1NAME && LOWORD(wParam) <= ID_EDIT_F4NAME )
						CheckReplaceForceName(LOWORD(wParam)-ID_EDIT_F1NAME);
					break;
			}
			break;

		default:
			{
				if ( WM_DRAGNOTIFY != WM_NULL && msg == WM_DRAGNOTIFY )
				{
					DRAGLISTINFO* dragInfo = (DRAGLISTINFO*)lParam;
					switch ( dragInfo->uNotification )
					{
						case DL_BEGINDRAG:
							{
								int index = LBItemFromPt(dragInfo->hWnd, dragInfo->ptCursor, FALSE);
								LRESULT length = SendMessage(dragInfo->hWnd, LB_GETTEXTLEN, index, NULL)+1;
								char* str;
								try { str = new char[length]; } catch ( std::bad_alloc ) { return FALSE; }
								length = SendMessage(dragInfo->hWnd, LB_GETTEXT, index, (LPARAM)str);
								if ( length != LB_ERR && length > 8 && str[7] >= '1' && str[7] <= '8' )
								{
									playerBeingDragged = str[7]-'1';
									for ( int id=ID_LB_F1PLAYERS; id<=ID_LB_F4PLAYERS; id++ )
									{
										HWND hForceLb = GetDlgItem(hWnd, id);
										if ( hForceLb != dragInfo->hWnd && hForceLb != NULL )
											SendMessage(GetDlgItem(hWnd, id), LB_SETCURSEL, -1, NULL);
									}
									return TRUE;
								}
								else
									return FALSE;
							}
							break;

						case DL_CANCELDRAG:
							playerBeingDragged = 255;
							return DefWindowProc(hWnd, msg, wParam, lParam);
							break;

						case DL_DRAGGING:
							{
								HWND hUnder = WindowFromPoint(dragInfo->ptCursor);
								if ( hUnder != NULL )
								{
									LONG windowID = GetWindowLong(hUnder, GWL_ID);
									if ( windowID >= ID_LB_F1PLAYERS && windowID <= ID_LB_F4PLAYERS )
										return DL_MOVECURSOR;
								}
								return DL_STOPCURSOR;
							}
							break;

						case DL_DROPPED:
							{
								HWND hUnder = WindowFromPoint(dragInfo->ptCursor);
								if ( hUnder != NULL && playerBeingDragged < 8 )
								{
									LONG windowID = GetWindowLong(hUnder, GWL_ID);
									if ( windowID >= ID_LB_F1PLAYERS && windowID <= ID_LB_F4PLAYERS )
									{
										int force = windowID-ID_LB_F1PLAYERS;
										GuiMap* map = chkd.maps.curr;
										if ( map != nullptr )
										{
											map->setPlayerForce(playerBeingDragged, force);
											RefreshWindow();
											stringstream ssPlayer;
											ssPlayer << "Player " << playerBeingDragged+1;
											SendMessage(GetDlgItem(hWnd, ID_LB_F1PLAYERS+force), LB_SELECTSTRING, -1, (LPARAM)ssPlayer.str().c_str());
											map->notifyChange(false);
										}
									}
								}
								playerBeingDragged = 255;
								return DefWindowProc(hWnd, msg, wParam, lParam);
							}
							break;

						default:
							return DefWindowProc(hWnd, msg, wParam, lParam);
							break;
					}
				}
				else
					return DefWindowProc(hWnd, msg, wParam, lParam);
			}
			break;
	}
	return 0;
}

void ForcesWindow::CheckReplaceForceName(int force)
{
	string newMapForce;
	if ( force < 4 && possibleForceNameUpdate[force] == true && editForceName[force].GetEditText(newMapForce) )
	{
		u16* mapForceString;
		if ( chkd.maps.curr->FORC().getPtr<u16>(mapForceString, 8+2*force, 2) &&
			 parseEscapedString(newMapForce) &&
			 chkd.maps.curr->replaceString(newMapForce, *mapForceString, false, true) )
		{
			chkd.maps.curr->notifyChange(false);
		}
		possibleForceNameUpdate[force] = false;
	}
}
