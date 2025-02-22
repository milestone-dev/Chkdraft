#include "TileChange.h"
#include "../../../Windows/MainWindows/GuiMap.h"

TileChange::~TileChange()
{

}

std::shared_ptr<TileChange> TileChange::Make(u16 xc, u16 yc, u16 tileValue)
{
    return std::shared_ptr<TileChange>(new TileChange(xc, yc, tileValue));
}

void TileChange::Reverse(void *guiMap)
{
    u16 replacedValue = ((GuiMap*)guiMap)->getTile(xc, yc);
    ((GuiMap*)guiMap)->setTile(xc, yc, tileValue);
    tileValue = replacedValue;
}

int32_t TileChange::GetType()
{
    return UndoTypes::TileChange;
}

TileChange::TileChange(u16 xc, u16 yc, u16 tileValue) : xc(xc), yc(yc), tileValue(tileValue)
{

}
