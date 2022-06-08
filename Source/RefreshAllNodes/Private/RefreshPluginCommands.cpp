#include "RefreshPluginCommands.h"
#include "RefreshAllNodes.h"

void FRefreshPluginCommands::RegisterCommands() {
#define LOCTEXT_NAMESPACE ""
	UI_COMMAND(RefreshAllButton, "Refresh All Blueprint Nodes", "Refresh all nodes in every blueprint", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RefreshPathButton, "Refresh Blueprints", "Refresh all nodes in blueprints under this folder", EUserInterfaceActionType::Button, FInputChord());
#undef LOCTEXT_NAMESPACE
}
