// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "RefreshPluginCommands.h"
#include "LevelEditor.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRefreshAllNodes, Log, All);

#define LOCTEXT_NAMESPACE "RefreshAllNodes"

class FRefreshAllNodesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FExtender> MenuExtender;
	TSharedPtr<const FExtensionBase> Extension;

	void RefreshButton_Clicked();
		
	void AddMenuExtension(FMenuBuilder &Builder) {
#if ENGINE_MAJOR_VERSION < 5
		FSlateIcon IconBrush = FSlateIcon(FEditorStyle::GetStyleSetName(), "PropertyWindow.Button_Refresh");
#else
		FSlateIcon IconBrush = FSlateIcon(FEditorStyle::GetStyleSetName(), "EditorViewport.RotateMode");
#endif
		Builder.BeginSection("RefreshBlueprints", LOCTEXT("RefreshAllNodes", "Refresh Blueprints"));
		Builder.AddMenuEntry(FRefreshPluginCommands::Get().RefreshButton, FName(""), FText::FromString("Refresh All Blueprint Nodes"), FText::FromString("Refresh all nodes in every blueprint"), IconBrush);
		Builder.EndSection();
	}

	TSharedRef<FExtender> OnExtendLevelEditorViewMenu(const TSharedRef<FUICommandList> CommandList)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		Extender->AddMenuExtension("BlueprintClass", EExtensionHook::Before, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FRefreshAllNodesModule::AddMenuExtension));

		return Extender;
	}
};

#undef LOCTEXT_NAMESPACE
