// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserDelegates.h"
#include "LevelEditor.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRefreshAllNodes, Log, All);

#define LOCTEXT_NAMESPACE "RefreshAllNodes"

class FRefreshAllNodesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FExtender> LevelEditorExtender;
	TSharedPtr<const FExtensionBase> LevelEditorExtension;
	TSharedPtr<const FExtensionBase> ContentBrowserExtension;

	TArray<FString> SelectedFolders;

	UPROPERTY()
	TArray<UBlueprint*> ProblemBlueprints;

	void RegisterLevelEditorButton();
	void RegisterPathViewContextMenuButton();
	
	TSharedRef<FExtender> CreateContentBrowserExtender(const TArray<FString>& SelectedPaths);

	void RefreshAllButton_Clicked();
	void RefreshPathButton_Clicked();
	
	void FindAndRefreshBlueprints(const FARFilter& Filter, bool bShouldExclude = true);
		
	void AddLevelEditorMenuEntry(FMenuBuilder &Builder);
	void AddPathViewContextMenuEntry(FMenuBuilder &Builder);
};

#undef LOCTEXT_NAMESPACE
