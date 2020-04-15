// Copyright 2018 Sean Chen. All Rights Reserved.

#include "AnimGraphNode_SpeedWarping.h"
#include "UnrealWidget.h"
#include "Kismet2/CompilerResultsLog.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_SpeedWarping

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_SpeedWarping::UAnimGraphNode_SpeedWarping()
{

}


FText UAnimGraphNode_SpeedWarping::GetTooltipText() const
{
	return LOCTEXT("SpeedWarping", "Speed Warping");
}

FLinearColor UAnimGraphNode_SpeedWarping::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.1f);
}

FText UAnimGraphNode_SpeedWarping::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("SpeedWarping", "Speed Warping");
}

void UAnimGraphNode_SpeedWarping::CopyPinDefaultsToNodeData(UEdGraphPin * InPin)
{

}

#undef LOCTEXT_NAMESPACE