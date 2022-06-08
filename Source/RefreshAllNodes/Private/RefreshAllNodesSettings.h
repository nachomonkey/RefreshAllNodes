#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"
#include "RefreshAllNodesSettings.generated.h"

/**
 * Configure the Refresh All Nodes plugin
 */
UCLASS(config=Engine, defaultconfig)
class URefreshAllNodesSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	public:
		URefreshAllNodesSettings();

#if WITH_EDITOR
		virtual FText GetSectionText() const override;
#endif
		/** Determines whether blueprints should be compiled after being refreshed.
		  * Enabling compilation will allow the plugin to catch errors in the blueprints,
		  * but it will take more time to process.
		  */
		UPROPERTY(config, EditAnywhere, Category=Behavior)
		bool bCompileBlueprints;

		/** Should the plugin refresh level blueprints? */
		UPROPERTY(config, EditAnywhere, Category=Search)
		bool bRefreshLevelBlueprints;

		/** Should the plugin refresh blueprints in this project's game content folder? */
		UPROPERTY(config, EditAnywhere, Category=Search)
		bool bRefreshGameBlueprints;

		/** Should the plugin refresh blueprints in the engine's content folder? */
		UPROPERTY(config, EditAnywhere, Category=Search)
		bool bRefreshEngineBlueprints;

		/**
		 * Additional paths to search in for blueprints to refresh, good for plugins
		 * Example: Add "Paper2D" to not search the Paper2D plugin for blueprints to refresh.
		 */
		UPROPERTY(config, EditAnywhere, Category=Search)
		TArray<FName> AdditionalBlueprintPaths;

		/**
		 * Blueprint paths to not search in for blueprints to refresh, good for
		 * blueprints with little dependencies that take lots of resources to
		 * load. Example: Add "/Game/Marketplace" to not search the "Marketplace"
		 * directory in the Content folder.
		 */
		UPROPERTY(config, EditAnywhere, Category=Search)
		TArray<FName> ExcludeBlueprintPaths;
};
