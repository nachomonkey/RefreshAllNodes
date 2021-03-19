#include "RefreshPluginCommands.h"
#include "RefreshAllNodes.h"

void FRefreshPluginCommands::RegisterCommands() {
#define LOCTEXT_NAMESPACE ""
	UI_COMMAND(RefreshButton, "Refresh All Blueprints", "Refresh all nodes in every Blueprint", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control|EModifierKey::Shift, EKeys::R));
#undef LOCTEXT_NAMESPACE
}
