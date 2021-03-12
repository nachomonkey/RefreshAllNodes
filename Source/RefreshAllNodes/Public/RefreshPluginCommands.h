#pragma once

#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FRefreshPluginCommands : public TCommands<FRefreshPluginCommands>  {
	
	public:
		FRefreshPluginCommands()
		       	: TCommands<FRefreshPluginCommands>(FName(TEXT("RefreshPlugin")), FText::FromString("RefreshPlugin Commands"), NAME_None, FEditorStyle::GetStyleSetName())
	       	{
		};
		
		virtual void RegisterCommands() override;
		
		TSharedPtr<FUICommandInfo> RefreshButton;
};
