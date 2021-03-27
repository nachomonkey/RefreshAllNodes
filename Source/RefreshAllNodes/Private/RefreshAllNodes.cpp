// Copyright Epic Games, Inc. All Rights Reserved.

#include "RefreshAllNodes.h"
#include "Framework/Commands/Commands.h"

#include "AssetRegistryModule.h"
#include "SlateBasics.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet/KismetStringLibrary.h"
#include "Engine/Blueprint.h"
#include "Engine/LevelScriptBlueprint.h"
#include "RefreshAllNodesSettings.h"
#include "FileHelpers.h"

DEFINE_LOG_CATEGORY(LogRefreshAllNodes);

#define LOCTEXT_NAMESPACE "FRefreshAllNodesModule"

void FRefreshAllNodesModule::StartupModule()  {

	FRefreshPluginCommands::Register();

	TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList());

	CommandList->MapAction(FRefreshPluginCommands::Get().RefreshButton, FExecuteAction::CreateRaw(this, &FRefreshAllNodesModule::RefreshButton_Clicked), FCanExecuteAction());
	
	MenuExtender = MakeShareable(new FExtender());
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	Extension = MenuExtender->AddMenuExtension("WorldSettingsClasses", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FRefreshAllNodesModule::AddMenuExtension));

	auto& MenuExtenders = LevelEditorModule.GetAllLevelEditorToolbarBlueprintsMenuExtenders();
	MenuExtenders.Add(MenuExtender);
   
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(CommandList.ToSharedRef());
}

void FRefreshAllNodesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	MenuExtender->RemoveExtension(Extension.ToSharedRef());
	Extension.Reset();
	MenuExtender.Reset();
}


void FRefreshAllNodesModule::RefreshButton_Clicked() {
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	TArray<FAssetData> AssetData;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();

	for (FName Path : Settings->AdditionalBlueprintPaths)
		Filter.PackagePaths.Add(UKismetStringLibrary::Conv_StringToName("/" + Path.ToString()));

	if (Settings->RefreshLevelBlueprints)    // Search for UWorld objects if we're searching for level blueprints
		Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
	if (Settings->RefreshGameBlueprints)
		Filter.PackagePaths.Add("/Game");
	if (Settings->RefreshEngineBlueprints)
		Filter.PackagePaths.Add("/Engine");

	AssetRegistryModule.Get().GetAssets(Filter, AssetData);  // Search for blueprints (and possibly UWorlds)
	TArray<UPackage*> PackagesToSave;

	for (FAssetData Data : AssetData) {
		bool ShouldSkip = false;

		for (FName Path : Settings->ExcludeBlueprintPaths) {
			if (Data.ObjectPath.ToString().StartsWith(Path.ToString(), ESearchCase::CaseSensitive)) {
				ShouldSkip = true;
				break;
			}
		}

		if (ShouldSkip)
			continue;

		UBlueprint* Blueprint;
	
		Blueprint = Cast<UBlueprint>(Data.GetAsset());

		if (Blueprint == nullptr)  {  // Try casting to a UWorld (to get level blueprint)
			UWorld* World = Cast<UWorld>(Data.GetAsset());
			if (World) {
				ULevel* Level = World->GetCurrentLevel();
				if (Level)
					Blueprint = Level->GetLevelScriptBlueprint(true);  // Use the level blueprint
			}
		}

		if (Blueprint == nullptr)  {  // Cannot get a Blueprint (most likely failed to get a level blueprint)
			continue;
		}

		UE_LOG(LogRefreshAllNodes, Display, TEXT("Refreshing Blueprint: %s"), *(Data.ObjectPath.ToString()));

		// Don't save packages that were dirty before refreshing
		bool ShouldTrySave = !Data.GetPackage()->IsDirty();
		
		FBlueprintEditorUtils::RefreshAllNodes(Blueprint);    // Refresh all nodes in this blueprint

		if (ShouldTrySave)
			PackagesToSave.Add(Data.GetPackage());
	}

	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);   // Save the refreshed blueprints
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRefreshAllNodesModule, RefreshAllNodes)
