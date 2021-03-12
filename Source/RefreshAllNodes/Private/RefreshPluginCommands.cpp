#include "RefreshPluginCommands.h"
#include "RefreshAllNodes.h"

void FRefreshPluginCommands::RegisterCommands() {
#define LOCTEXT_NAMESPACE ""
	UI_COMMAND(RefreshButton, "Refresh All Blueprints", "Refresh all nodes in every Blueprint", EUserInterfaceActionType::Button, FInputChord());
#undef LOCTEXT_NAMESPACE
}
