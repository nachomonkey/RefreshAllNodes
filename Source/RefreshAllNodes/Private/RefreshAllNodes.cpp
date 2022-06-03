// Copyright Epic Games, Inc. All Rights Reserved.

#include "RefreshAllNodes.h"
#include "Framework/Commands/Commands.h"
#include "AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
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

#if ENGINE_MAJOR_VERSION < 5
#define ICON_BRUSH_NAME "PropertyWindow.Button_Refresh"
#else
#define ICON_BRUSH_NAME "EditorViewport.RotateMode"
#endif

void FRefreshAllNodesModule::StartupModule()  {
	FRefreshPluginCommands::Register();

	RegisterLevelEditorButton();
	RegisterPathViewContextMenuButton();
}

void FRefreshAllNodesModule::RegisterLevelEditorButton() {
	TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList());

	CommandList->MapAction(FRefreshPluginCommands::Get().RefreshAllButton, FExecuteAction::CreateRaw(this, &FRefreshAllNodesModule::RefreshAllButton_Clicked), FCanExecuteAction());
	
	LevelEditorExtender = MakeShareable(new FExtender());
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	LevelEditorExtension = LevelEditorExtender->AddMenuExtension("WorldSettingsClasses", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FRefreshAllNodesModule::AddLevelEditorMenuEntry));

	auto& MenuExtenders = LevelEditorModule.GetAllLevelEditorToolbarBlueprintsMenuExtenders();
	MenuExtenders.Add(LevelEditorExtender);
   
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(CommandList.ToSharedRef());
}

void FRefreshAllNodesModule::RegisterPathViewContextMenuButton() {
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	ContentBrowserModule.GetAllPathViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FRefreshAllNodesModule::CreateContentBrowserExtender));
}

TSharedRef<FExtender> FRefreshAllNodesModule::CreateContentBrowserExtender(const TArray<FString>& SelectedPaths) {
	SelectedFolders = SelectedPaths;

	TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList());
	CommandList->MapAction(FRefreshPluginCommands::Get().RefreshPathButton, FExecuteAction::CreateRaw(this, &FRefreshAllNodesModule::RefreshPathButton_Clicked), FCanExecuteAction());

	TSharedPtr<FExtender> ContentBrowserExtender = MakeShareable(new FExtender());

	ContentBrowserExtension = ContentBrowserExtender->AddMenuExtension("PathViewFolderOptions", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FRefreshAllNodesModule::AddPathViewContextMenuEntry));
	return ContentBrowserExtender.ToSharedRef();
}

void FRefreshAllNodesModule::AddLevelEditorMenuEntry(FMenuBuilder &Builder) {
	FSlateIcon IconBrush = FSlateIcon(FEditorStyle::GetStyleSetName(), ICON_BRUSH_NAME);

	Builder.BeginSection("RefreshBlueprints", LOCTEXT("RefreshAllNodes", "Refresh All Nodes"));
	Builder.AddMenuEntry(FRefreshPluginCommands::Get().RefreshAllButton, FName(""), FText::FromString("Refresh All Blueprint Nodes"), FText::FromString("Refresh all nodes in every blueprint"), IconBrush);
	Builder.EndSection();
}

void FRefreshAllNodesModule::AddPathViewContextMenuEntry(FMenuBuilder& Builder) {
	FSlateIcon IconBrush = FSlateIcon(FEditorStyle::GetStyleSetName(), ICON_BRUSH_NAME);

	Builder.AddMenuEntry(FRefreshPluginCommands::Get().RefreshPathButton, FName(""), FText::FromString("Refresh Blueprints"), FText::FromString("Refresh all nodes in blueprints under the selected folders"), IconBrush);
}

void FRefreshAllNodesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	LevelEditorExtender->RemoveExtension(LevelEditorExtension.ToSharedRef());
	LevelEditorExtension.Reset();
	ContentBrowserExtension.Reset();
	LevelEditorExtender.Reset();
}

void FRefreshAllNodesModule::RefreshPathButton_Clicked() {
	// This function is called when the button in the Content Browser right-click context menu is pressed

	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();
	if (Settings->RefreshLevelBlueprints) {    // Search for UWorld objects if we're searching for level blueprints
		Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
	}

	for (const FString& FolderPath : SelectedFolders) {
		Filter.PackagePaths.Add(*FolderPath);
	}

	FindAndRefreshBlueprints(Filter, false);
}

void FRefreshAllNodesModule::RefreshAllButton_Clicked() {
	// This function is called when the main "Refresh All Blueprint Nodes" button in pressed

	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();

	for (FName Path : Settings->AdditionalBlueprintPaths) {
		Filter.PackagePaths.Add(UKismetStringLibrary::Conv_StringToName("/" + Path.ToString()));
	}

	if (Settings->RefreshLevelBlueprints) {    // Search for UWorld objects if we're searching for level blueprints
		Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
	} if (Settings->RefreshGameBlueprints) {
		Filter.PackagePaths.Add("/Game");
	} if (Settings->RefreshEngineBlueprints) {
		Filter.PackagePaths.Add("/Engine");
	}

	FindAndRefreshBlueprints(Filter);
}

void FRefreshAllNodesModule::FindAndRefreshBlueprints(const FARFilter& Filter, bool bShouldExclude) {
	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);  // Search for blueprints (and possibly UWorlds)

	TArray<UPackage*> PackagesToSave;

	for (FAssetData Data : AssetData) {
		bool bShouldSkip = false;

		if (bShouldExclude) {
			for (FName Path : Settings->ExcludeBlueprintPaths) {
				if (Data.ObjectPath.ToString().StartsWith(Path.ToString(), ESearchCase::CaseSensitive)) {
					bShouldSkip = true;
					break;
				}
			}
		}

		if (bShouldSkip) {
			continue;
		}

		UBlueprint* Blueprint;
	
		Blueprint = Cast<UBlueprint>(Data.GetAsset());

		if (Blueprint == nullptr)  {  // Try casting to a UWorld (to get level blueprint)
			UWorld* World = Cast<UWorld>(Data.GetAsset());
			if (World) {
				ULevel* Level = World->GetCurrentLevel();
				if (Level) {
					Blueprint = Level->GetLevelScriptBlueprint(true);  // Use the level blueprint
				}
			}
		}

		if (Blueprint == nullptr)  {  // Cannot get a Blueprint (most likely failed to get a level blueprint)
			continue;
		}

		UE_LOG(LogRefreshAllNodes, Display, TEXT("Refreshing Blueprint: %s"), *(Data.ObjectPath.ToString()));

		// Don't save packages that were dirty before refreshing
		bool ShouldTrySave = !Data.GetPackage()->IsDirty();
		
		FBlueprintEditorUtils::RefreshAllNodes(Blueprint);    // Refresh all nodes in this blueprint

		if (ShouldTrySave) {
			PackagesToSave.Add(Data.GetPackage());
		}
	}

	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);   // Save the refreshed blueprints
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRefreshAllNodesModule, RefreshAllNodes)
