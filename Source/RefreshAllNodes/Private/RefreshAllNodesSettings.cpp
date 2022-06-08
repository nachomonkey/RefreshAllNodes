#include "RefreshAllNodesSettings.h"

#define LOCTEXT_NAMESPACE "RefreshAllNodes"

URefreshAllNodesSettings::URefreshAllNodesSettings() {
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("Refresh All Nodes");

	bRefreshGameBlueprints = true;
	bCompileBlueprints = true;
}

#if WITH_EDITOR

FText URefreshAllNodesSettings::GetSectionText() const {
	return LOCTEXT("SettingsDisplayName", "Refresh All Nodes");
}

#endif

#undef LOCTEXT_NAMESPACE
