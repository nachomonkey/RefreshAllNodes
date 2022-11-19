// Copyright Epic Games, Inc. All Rights Reserved.

#include "RefreshAllNodes.h"
#include "RefreshAllNodesSettings.h"
#include "RefreshPluginCommands.h"
#include "Framework/Commands/Commands.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "SlateBasics.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet/KismetStringLibrary.h"
#include "PackageTools.h"
#include "Engine/Blueprint.h"
#include "Engine/LevelScriptBlueprint.h"
#include "FileHelpers.h"

#include "Dialog/SCustomDialog.h"
#include "Widgets/Input/SHyperlink.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY(LogRefreshAllNodes);

// Formats text, accounts for proper grammar with plurals
#define BLUEPRINTS_TEXT(x) FText::Format(FText::FromString("{0} blueprint{1}"), x, (x == 1) ? FText::GetEmpty() : FText::FromString("s"))


// Create a dialog that lists the blueprints with errors and lets them be opened
static void ShowProblemBlueprintsDialog(TArray<UBlueprint*> ErroredBlueprints) {
        struct Local {
                static void OnHyperlinkClicked( TWeakObjectPtr<UBlueprint> InBlueprint, TSharedPtr<SCustomDialog> InDialog ) {
                        if (UBlueprint* BlueprintToEdit = InBlueprint.Get()) {
                                GEditor->EditObject(BlueprintToEdit);
                        }

                        if (InDialog.IsValid()) {
                                // Opening the blueprint editor above may end up creating an invisible new window on top of the dialog, 
                                // thus making it not interactable, so we have to force the dialog back to the front
                                InDialog->BringToFront(true);
                        }
                }
        };

        TSharedRef<SVerticalBox> DialogContents = SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .Padding(0, 0, 0, 16)
                [
                        SNew(STextBlock)
                        .Text(FText::FromString("The following blueprints failed to compile:"))
                ];

        TSharedPtr<SCustomDialog> CustomDialog;

        for (UBlueprint* Blueprint : ErroredBlueprints) {
		TWeakObjectPtr<UBlueprint> BlueprintPtr = Blueprint;

                DialogContents->AddSlot()
                        .AutoHeight()
                        .HAlign(HAlign_Left)
                        [
                                SNew(SHyperlink)
                                .Style(FAppStyle::Get(), "Common.GotoBlueprintHyperlink")
                                .OnNavigate(FSimpleDelegate::CreateLambda([BlueprintPtr, &CustomDialog]() { Local::OnHyperlinkClicked(BlueprintPtr, CustomDialog); }))
                                .Text(FText::FromString(Blueprint->GetName()))
                                .ToolTipText(NSLOCTEXT("SourceHyperlink", "EditBlueprint_ToolTip", "Click to edit the blueprint"))
                        ];
        }

        DialogContents->AddSlot()
                .Padding(0, 16, 0, 0)
                [
                        SNew(STextBlock)
                        .Text(FText::FromString("Clicked blueprints will open once this dialog is closed."))
                ];

        CustomDialog = SNew(SCustomDialog)
                .Title(FText::FromString("Blueprint Compilation Errors"))
                .Icon(FAppStyle::Get().GetBrush("NotificationList.DefaultMessage"))
                .Content() [ DialogContents ]
                .Buttons( { SCustomDialog::FButton(FText::FromString("Dismiss")) } );
	CustomDialog->ShowModal();
}

void FRefreshAllNodesModule::StartupModule()  {
	FRefreshPluginCommands::Register();

	RegisterLevelEditorButton();
	RegisterPathViewContextMenuButton();
}

void FRefreshAllNodesModule::ShutdownModule()  {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	LevelEditorExtender->RemoveExtension(LevelEditorExtension.ToSharedRef());
	LevelEditorExtension.Reset();
	ContentBrowserExtension.Reset();
	LevelEditorExtender.Reset();
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
	FSlateIcon IconBrush = FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.RotateMode");

	Builder.BeginSection("RefreshBlueprints", FText::FromString("Refresh All Nodes"));
	Builder.AddMenuEntry(FRefreshPluginCommands::Get().RefreshAllButton, FName(""), FText::FromString("Refresh All Blueprint Nodes"), FText::FromString("Refresh all nodes in every blueprint"), IconBrush);
	Builder.EndSection();
}

void FRefreshAllNodesModule::AddPathViewContextMenuEntry(FMenuBuilder& Builder) {
	FSlateIcon IconBrush = FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.RotateMode");

	Builder.AddMenuEntry(FRefreshPluginCommands::Get().RefreshPathButton, FName(""), FText::FromString("Refresh Blueprints"), FText::FromString("Refresh all nodes in blueprints under this folder"), IconBrush);
}


void FRefreshAllNodesModule::RefreshPathButton_Clicked() {
	// This function is called when the button in the Content Browser right-click context menu is pressed

	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();
	if (Settings->bRefreshLevelBlueprints) {    // Search for UWorld objects if we're searching for level blueprints
		Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	}

	for (const FString& FolderPath : SelectedFolders) {
		Filter.PackagePaths.Add(*FolderPath);
	}

	FindAndRefreshBlueprints(Filter, false);
}

void FRefreshAllNodesModule::RefreshAllButton_Clicked() {
	// This function is called when the main "Refresh All Blueprint Nodes" button in pressed

	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();

	for (FName Path : Settings->AdditionalBlueprintPaths) {
		Filter.PackagePaths.Add(UKismetStringLibrary::Conv_StringToName("/" + Path.ToString()));
	}

	if (Settings->bRefreshLevelBlueprints) {    // Search for UWorld objects if we're searching for level blueprints
		Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	} if (Settings->bRefreshGameBlueprints) {
		Filter.PackagePaths.Add("/Game");
	} if (Settings->bRefreshEngineBlueprints) {
		Filter.PackagePaths.Add("/Engine");
	}

	FindAndRefreshBlueprints(Filter);
}

void FRefreshAllNodesModule::FindAndRefreshBlueprints(const FARFilter& Filter, bool bShouldExclude) {
	const URefreshAllNodesSettings* Settings = GetDefault<URefreshAllNodesSettings>();

	TArray<FAssetData> AssetData;
	TArray<UPackage*> PackagesToSave;

	// ProblemBlueprints is filled with blueprints when there are errors, but only emptied here...
	// It really should be emptied after the ProblemNotification fades out.
	ProblemBlueprints.Empty();
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Search for applicable assets (UBlueprints, possibly UWorlds)
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	int NumAssets = AssetData.Num();

	TSharedPtr<SNotificationItem> RefreshingNotification;

	// Create different popups depending on whether there are blueprints to refresh or not
	if (NumAssets) {
		FNotificationInfo Info(FText::Format(FText::FromString("Refreshing {0}..."), BLUEPRINTS_TEXT(NumAssets)));
		Info.ExpireDuration = 5;
		Info.bFireAndForget = false;
		RefreshingNotification = FSlateNotificationManager::Get().AddNotification(Info);
		RefreshingNotification->SetCompletionState(SNotificationItem::CS_Pending);
	} else {
		FNotificationInfo Info(FText::FromString("No blueprints were refreshed"));
		Info.ExpireDuration = 1.5f;
	
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	// Loop through the assets, get to the blueprints, and refresh them
	for (FAssetData Data : AssetData) {
		bool bShouldSkip = false;

		FString AssetPathString = Data.GetObjectPathString();

		// Skip if this is in an excluded path and we are refreshing all blueprints
		if (bShouldExclude) {
			for (FName Path : Settings->ExcludeBlueprintPaths) {
				if (AssetPathString.StartsWith(Path.ToString(), ESearchCase::CaseSensitive)) {
					bShouldSkip = true;
					break;
				}
			}
		}
		if (bShouldSkip) {
			continue;
		}

		TWeakObjectPtr<UBlueprint> Blueprint = Cast<UBlueprint>(Data.GetAsset());
		
		// Try casting to a UWorld (to get level blueprint)
		if (Blueprint == nullptr)  {
			TWeakObjectPtr<UWorld> World = Cast<UWorld>(Data.GetAsset());
			if (World != nullptr) {
				TWeakObjectPtr<ULevel> Level = World->GetCurrentLevel();

				if (Level != nullptr) {
					// Use the level blueprint
					Blueprint = Level->GetLevelScriptBlueprint(true);
				}
			}
		}
	      
		// Skip if there is no blueprint
		if (Blueprint == nullptr)  {
			continue;
		}

		UE_LOG(LogRefreshAllNodes, Display, TEXT("Refreshing Blueprint: %s"), *AssetPathString);
	    
		// Refresh all nodes in this blueprint
		FBlueprintEditorUtils::RefreshAllNodes(Blueprint.Get());

		if (Settings->bCompileBlueprints) {

			// Compile blueprint
			UE_LOG(LogRefreshAllNodes, Display, TEXT("Compiling Blueprint: %s"), *AssetPathString);
			FKismetEditorUtilities::CompileBlueprint(Blueprint.Get(), EBlueprintCompileOptions::BatchCompile | EBlueprintCompileOptions::SkipSave);

			// Check if the blueprint failed to compile
			if (!Blueprint->IsUpToDate() && Blueprint->Status != BS_Unknown) {
				UE_LOG(LogRefreshAllNodes, Error, TEXT("Failed to compile %s"), *AssetPathString);
				ProblemBlueprints.Add(Blueprint.Get());
			}
		}

		PackagesToSave.Add(Data.GetPackage());
	}

	// Save the refreshed blueprints
	bool bSuccess = UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);

	// If the saving fails, log and error and raise a notification
	if (!bSuccess) {
		UE_LOG(LogRefreshAllNodes, Error, TEXT("Failed to save packages"));
		FNotificationInfo Info(FText::FromString("Failed to save packages"));
		Info.ExpireDuration = 10.f;
	
		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Fail);
	}

	// Set the popup to "success" state
	if (RefreshingNotification.IsValid()) {
		RefreshingNotification->SetText(FText::Format(FText::FromString("Refreshed {0}"), BLUEPRINTS_TEXT(NumAssets)));
		RefreshingNotification->SetCompletionState(SNotificationItem::CS_Success);
		RefreshingNotification->ExpireAndFadeout();
	}

	// If there were errors in compilation, create a new popup with an option to see which blueprints failed to compile
	if (ProblemBlueprints.Num()) {
		auto ShowBlueprints = [this]
	      	{	
			if (ProblemBlueprints.Num()) {
				ShowProblemBlueprintsDialog(ProblemBlueprints);
			}
		};

		FNotificationInfo Info(FText::Format(FText::FromString("{0} failed to compile"), BLUEPRINTS_TEXT(ProblemBlueprints.Num())));
		Info.ExpireDuration = 15;
		Info.Image = FAppStyle::GetBrush("Icons.Warning");

		TSharedPtr<SNotificationItem> ProblemNotification;
	       	ProblemNotification = FSlateNotificationManager::Get().AddNotification(Info);
		ProblemNotification->SetHyperlink(FSimpleDelegate::CreateLambda(ShowBlueprints), FText::FromString("Show blueprints"));
	}
}

IMPLEMENT_MODULE(FRefreshAllNodesModule, RefreshAllNodes)
