#include "Selections.h"
#include "../Windows/MainWindows/GuiMap.h"

TileNode::~TileNode()
{

}

Selections::Selections(GuiMap & map) : map(map), moved(false), numRecentLocations(0), locationFlags(0), locSelFlags(LocSelFlags::None), selectedLocation(0)
{
    memset((void*)&recentLocations[0], 0, sizeof(recentLocations)/sizeof(u8));
    startDrag.x = -1;
    startDrag.y = -1;
    endDrag.x = -1;
    endDrag.y = -1;
}

Selections::~Selections()
{
    removeTiles();
    removeUnits();
}

void Selections::setStartDrag(s32 x, s32 y)
{
    startDrag.x = x;
    startDrag.y = y;
}

void Selections::setEndDrag(s32 x, s32 y)
{
    endDrag.x = x;
    endDrag.y = y;
}

void Selections::setDrags(s32 x, s32 y)
{
    startDrag.x = x;
    startDrag.y = y;
    endDrag.x = x;
    endDrag.y = y;
}

void Selections::removeTile(TileNode* & tile)
{
    removeTile(tile->xc, tile->yc);
}

void Selections::removeTile(u16 xc, u16 yc)
{
    auto toRemove = selTiles.end();
    for ( auto it = selTiles.begin(); it != selTiles.end(); ++it )
    {
        // If an edge is matched to the tile being removed, un-match the edge
        if ( it->yc == yc ) // Tile is in the same row
        {
            if ( it->xc == xc - 1 ) (u8 &)it->neighbors |= TileNeighbor::Right; // OR 0100, flips on the RIGHT edge bit
            else if ( it->xc == xc + 1 ) (u8 &)it->neighbors |= TileNeighbor::Left; // OR 0001, flips on the LEFT edge bit
        }
        else if ( it->xc == xc ) // Tile is in the same column
        {
            if ( it->yc == yc - 1 ) (u8 &)it->neighbors |= TileNeighbor::Bottom; // OR 1000, flips on the BOTTOM edge bit
            else if ( it->yc == yc + 1 ) (u8 &)it->neighbors |= TileNeighbor::Top; // OR 0010, flips on the TOP edge bit
        }

        if ( it->xc == xc && it->yc == yc )
            toRemove = it;
    }
    
    if ( toRemove != selTiles.end() )
        selTiles.erase(toRemove);
}

void Selections::addTile(u16 value, u16 xc, u16 yc)
{
    TileNode tile;
    tile.xc = xc;
    tile.yc = yc;
    tile.neighbors = TileNeighbor::All;
    tile.value = value;

    for ( auto & selTile : selTiles )
    {
        // If tile edges are touching, remove that border
        if ( selTile.yc == yc ) // Tile is in the same row
        {
            if ( selTile.xc == xc ) // Tile is in the same column: tile is already selected!
            {
                removeTile(xc, yc);
                return; // Iterators are now invalid, ensure loop is exited
            }
            else if ( selTile.xc == xc - 1 ) // 'track' is just left of 'tile'
            {
                (u8 &)tile.neighbors &= TileNeighbor::xLeft; // AND 1110, flips off the LEFT edge bit
                (u8 &)selTile.neighbors &= TileNeighbor::xRight; // AND 1011, flips off the RIGHT edge bit
            }
            else if ( selTile.xc == xc + 1 ) // 'track' is just right of 'tile'
            {
                (u8 &)tile.neighbors &= TileNeighbor::xRight; // AND 1011, flips off the RIGHT edge bit
                (u8 &)selTile.neighbors &= TileNeighbor::xLeft; // AND 1110, flips off the LEFT edge bit
            }
        }
        else if ( selTile.xc == xc ) // Tile is in same column
        {
            if ( selTile.yc == yc - 1 ) // 'track' is just above 'tile'
            {
                (u8 &)tile.neighbors &= TileNeighbor::xTop; // AND 1101, flips off the TOP edge bit
                (u8 &)selTile.neighbors &= TileNeighbor::xBottom; // AND 0111, flips off the BOTTOM edge bit
            }
            else if ( selTile.yc == yc + 1 ) // 'track' is just below 'tile'
            {
                (u8 &)tile.neighbors &= TileNeighbor::xBottom; // AND 0111, flips off the BOTTOM edge bit
                (u8 &)selTile.neighbors &= TileNeighbor::xTop; // AND 1101, flips off the TOP edge bit
            }
        }
    }

    selTiles.insert(selTiles.end(), tile);
}

void Selections::addTile(u16 value, u16 xc, u16 yc, TileNeighbor neighbors)
{
    TileNode tile;
    tile.value = value;
    tile.xc = xc;
    tile.yc = yc;
    tile.neighbors = neighbors;

    selTiles.insert(selTiles.end(), tile);
}

void Selections::removeTiles()
{
    selTiles.clear();
}

u16 Selections::getSelectedLocation()
{
    return selectedLocation;
}

void Selections::selectLocation(u16 index)
{
    selectedLocation = index;
    numRecentLocations = 1;
    recentLocations[0] = u8(index);
}

void Selections::selectLocation(s32 clickX, s32 clickY, bool canSelectAnywhere)
{
    size_t numLocations = map.numLocations();
    u16 firstRecentlySelected = NO_LOCATION;
    bool madeSelection = false;
    
    for ( u16 i=1; i<=numLocations; i++ )
    {
        if ( i != selectedLocation && (i != Chk::LocationId::Anywhere || canSelectAnywhere) )
        {
            const auto & location = map.getLocation(i);
            s32 locLeft = std::min(location.left, location.right),
                locRight = std::max(location.left, location.right),
                locTop = std::min(location.top, location.bottom),
                locBottom = std::max(location.top, location.bottom);

            if ( clickX >= locLeft && clickX <= locRight &&
                 clickY >= locTop && clickY <= locBottom )
            {
                bool recentlySelected = false;
                for ( u8 recentIndex=0; recentIndex<numRecentLocations; recentIndex++ )
                {
                    if ( i == recentLocations[recentIndex] )
                    {
                        recentlySelected = true;
                        break;
                    }
                }

                if ( recentlySelected ) // Location has been recently selected, flag if no earlier location was flagged
                {
                    if ( firstRecentlySelected == NO_LOCATION )
                        firstRecentlySelected = i;
                }
                else // Location hasn't been recently selected, select it
                {
                    selectedLocation = i;
                    if ( numRecentLocations < Chk::TotalLocations )
                    {

                        recentLocations[numRecentLocations] = u8(i);
                        numRecentLocations ++;
                    }
                    else
                    {
                        recentLocations[0] = u8(i);
                        numRecentLocations = 1;
                    }
                    madeSelection = true;
                    break;
                }
            }
        }
    }

    if ( !madeSelection )
    {
        if ( firstRecentlySelected != NO_LOCATION )
        {
            selectedLocation = firstRecentlySelected;
            recentLocations[0] = u8(firstRecentlySelected);
            numRecentLocations = 1;
        }
        else // Reset recent locations
        {
            selectedLocation = NO_LOCATION;
            recentLocations[0] = u8(selectedLocation);
            numRecentLocations = 1;
        }
    }
}

void Selections::addUnit(u16 index)
{
    if ( !unitIsSelected(index) )
        selUnits.insert(selUnits.begin(), index);
}

void Selections::removeUnit(u16 index)
{
    auto toErase = std::find(selUnits.begin(), selUnits.end(), index);
    if ( toErase != selUnits.end() )
        selUnits.erase(toErase);
}

void Selections::removeUnits()
{
    selUnits.clear();
}

void Selections::ensureFirst(u16 index)
{
    if ( selUnits.size() > 0 && selUnits[0] != index )
    {
        auto toErase = std::find(selUnits.begin(), selUnits.end(), index);
        if ( toErase != selUnits.end() )
        {
            selUnits.erase(toErase);
            selUnits.insert(selUnits.begin(), index);
        }
    }
}

void Selections::sendSwap(u16 oldIndex, u16 newIndex)
{
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex == newIndex )
            unitIndex = oldIndex | UnitSortFlags::Swapped;
        else if ( unitIndex == oldIndex )
            unitIndex = newIndex;
    }
}

void Selections::sendMove(u16 oldIndex, u16 newIndex) // The item is being moved back to its oldIndex from its newIndex
{
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex == newIndex )
            unitIndex = oldIndex | UnitSortFlags::Moved;
        else if ( newIndex > unitIndex && oldIndex <= unitIndex ) // The moved unit was somewhere ahead of track and is now behind track
            unitIndex++; // Selected unit index needs to be moved forward
        else if ( newIndex < unitIndex && oldIndex >= unitIndex ) // The moved unit was somewhere behind track and is now ahead of track
            unitIndex--; // Selected unit index needs to be moved backward
    }
}

void Selections::finishSwap()
{
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex & UnitSortFlags::Swapped )
            unitIndex &= UnitSortFlags::Unswap;
    }
}

void Selections::finishMove()
{
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex & UnitSortFlags::Moved )
            unitIndex &= UnitSortFlags::Unmove;
    }
}

bool Selections::unitIsSelected(u16 index)
{
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex == index )
            return true;
    }
    return false;
}

u16 Selections::numUnits()
{
    if ( selUnits.size() < u16_max )
        return (u16)selUnits.size();
    else
        return u16_max;
}

u16 Selections::numUnitsUnder(u16 index)
{
    u16 numUnitsBefore = 0;
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex < index )
            numUnitsBefore++;
    }
    return numUnitsBefore;
}

std::vector<TileNode> & Selections::getTiles()
{
    return selTiles;
}

TileNode Selections::getFirstTile()
{
    TileNode tile;
    tile.xc = 0;
    tile.yc = 0;
    tile.neighbors = TileNeighbor::None;
    tile.value = 0;

    if ( selTiles.size() > 0 )
    {
        tile.xc = selTiles[0].xc;
        tile.yc = selTiles[0].yc;
        tile.neighbors = selTiles[0].neighbors;
        tile.value = selTiles[0].value;
    }

    return tile;
}

std::vector<u16> & Selections::getUnits()
{
    return selUnits;
}

u16 Selections::getFirstUnit()
{
    if ( selUnits.size() > 0 )
        return selUnits[0];
    else
        return 0;
}

u16 Selections::getHighestIndex()
{
    int highestIndex = -1;
    for ( u16 & unitIndex : selUnits )
    {
        if ( (int)unitIndex > highestIndex )
            highestIndex = (int)unitIndex;
    }

    if ( highestIndex >= 0 )
        return highestIndex;
    else
        return (u16)highestIndex;
}

u16 Selections::getLowestIndex()
{
    u16 highestIndex = u16_max;
    for ( u16 & unitIndex : selUnits )
    {
        if ( unitIndex < highestIndex )
            highestIndex = unitIndex;
    }
    return highestIndex;
}

void Selections::sortUnits(bool ascending)
{
    if ( ascending )
        std::sort(selUnits.begin(), selUnits.end());
    else // Sort descending
        std::sort(selUnits.begin(), selUnits.end(), std::greater<u16>());
}
