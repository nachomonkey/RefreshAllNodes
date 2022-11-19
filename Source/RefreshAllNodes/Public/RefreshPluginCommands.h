#pragma once

#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FRefreshPluginCommands : public TCommands<FRefreshPluginCommands>  {
	
	public:
		FRefreshPluginCommands()
		       	: TCommands<FRefreshPluginCommands>(FName(TEXT("RefreshPlugin")), FText::FromString("RefreshAllNodes Commands"), NAME_None, FAppStyle::GetAppStyleSetName())
	       	{
		};
		
		virtual void RegisterCommands() override;
		
		TSharedPtr<FUICommandInfo> RefreshAllButton;
		TSharedPtr<FUICommandInfo> RefreshPathButton;
};
