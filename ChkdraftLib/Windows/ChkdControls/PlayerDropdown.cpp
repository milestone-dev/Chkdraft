#include "PlayerDropdown.h"

PlayerDropdown::~PlayerDropdown()
{

}

bool PlayerDropdown::CreateThis(HWND hParent, int x, int y, int width, int height, u32 id, bool eightPlayer)
{
    std::vector<std::string> playerStrings;

    if (eightPlayer)
        playerStrings = { "Player 01", "Player 02", "Player 03", "Player 04", "Player 05", "Player 06",
            "Player 07", "Player 08" };
    else
        playerStrings = { "Player 01", "Player 02", "Player 03", "Player 04", "Player 05", "Player 06",
            "Player 07", "Player 08", "Player 09", "Player 10", "Player 11", "Player 12 (Neutral)" };

    if ( DropdownControl::CreateThis(hParent, x, y, width, height, true, false, id, playerStrings, defaultFont) )
    {
        SendMessage(getHandle(), CB_LIMITTEXT, 0, 0);
        DropdownControl::SetSel(0);
        DropdownControl::ClearEditSel();
        return true;
    }
    return false;
}

bool PlayerDropdown::GetPlayerNum(u8 & dest)
{
    bool success = false;
    if ( DropdownControl::GetEditNum<u8>(dest) )
    {
        dest --; // to 0 based
        success = true;
    }
    else if ( auto editText = DropdownControl::GetWinText() )
    {
        for ( auto & c : *editText )
            c = toupper(c);

        int parsedPlayer = 0;

        if (    std::sscanf(editText->c_str(), "PLAYER%d", &parsedPlayer) > 0
             || std::sscanf(editText->c_str(), "P%d", &parsedPlayer) > 0
             || std::sscanf(editText->c_str(), "ID:%d", &parsedPlayer) > 0 )
        {
            dest = parsedPlayer;
            dest --; // to 0 based
            success = true;
        }
    }
    return success;
}

u8 PlayerDropdown::GetPlayerBySelNum()
{
    return (u8)DropdownControl::GetSel();
}
