#pragma once
#include <windows.h>
// Call once on the player's GUI thread after its main window exists.
// Replaces only the text in the three built-in skin sprite sheets in memory.
void StartEnglishSkinLabels(HMODULE extensionModule);
