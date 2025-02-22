#include "ChkdPlugins.h"
#include "../Chkdraft.h"
#include "../../MappingCoreLib/TextTrigCompiler.h"
#include "Settings.h"

LRESULT CALLBACK PluginProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_COPYDATA:
            {
                COPYDATASTRUCT* dataStruct = (COPYDATASTRUCT*)lParam;
                u32 length = (u32)dataStruct->cbData;
                const void* copyData = dataStruct->lpData;

                u32 dwData = (u32)dataStruct->dwData;
                u16 copyID = LOWORD(dwData),
                    mapID = HIWORD(dwData);

                if ( copyID >= PLUGIN_MSG_START && copyID <= PLUGIN_MSG_END )
                {
                    GuiMapPtr map = chkd.maps.GetMap(mapID);
                    if ( map != nullptr )
                    {
                        switch ( copyID )
                        {
                            case UPDATE_CHK_FILE:
                                {
                                    if ( map->deserialize((Chk::SerializedChk*)copyData) )
                                    {
                                        map->refreshScenario();
                                        map->notifyChange(false);
                                        return TRUE;
                                    }
                                }
                                break;
                            case REPLACE_TRIGGERS_TEXT:
                                {
                                    std::unique_ptr<char> inputText = std::unique_ptr<char>(new char[size_t(length)+1]);
                                    std::memcpy(inputText.get(), copyData, length);
                                    if ( inputText.get()[length-2] != '\0' ) // Ensure NUL-terminated
                                        inputText.get()[length-1] = '\0';

                                    std::string textBuf(inputText.get());

                                    TextTrigCompiler compiler(Settings::useAddressesForMemory, Settings::deathTableStart);
                                    if ( compiler.compileTriggers(textBuf, *map, chkd.scData, 0, map->numTriggers()) )
                                    {
                                        map->notifyChange(false);
                                        return TRUE;
                                    }
                                    else
                                        WinLib::Message("Compilation failed.", "Error!");
                                }
                                break;
                        }
                    }
                }
            }
            break;

        case SAVE_MAP:
            {
                GuiMapPtr map = chkd.maps.GetMap((u16)wParam);
                if ( map != nullptr )
                {
                    if ( map->SaveFile(false) )
                        return chkd.maps.GetMapID(map);
                }
            }
            break;

        case COPY_CHK_FILE:
            {
                GuiMapPtr map = chkd.maps.GetMap((u16)lParam);
                if ( map != nullptr )
                {
                    u16 mapID = chkd.maps.GetMapID(map);
                    COPYDATASTRUCT copyData;
                    copyData.dwData = (ULONG_PTR)MAKELONG(COPY_CHK_FILE, mapID);
                    std::vector<u8> chkFile = map->serialize();
                    copyData.lpData = PVOID(&chkFile[0]);
                    copyData.cbData = DWORD(chkFile.size() - sizeof(Chk::ChkHeader));
                    SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&copyData);
                    return mapID;
                }
            }
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam); // Valid occasion to use this method
            break;
    }
    return 0;
}
