// Copyright 2018 Sean Chen. All Rights Reserved.

#include "AnimGraphNode_OrientationWarping.h"
#include "CustomAnimNodeEditor.h"

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_OrientationWarping::UAnimGraphNode_OrientationWarping()
{

}

FLinearColor UAnimGraphNode_OrientationWarping::GetNodeTitleColor() const
{
	return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UAnimGraphNode_OrientationWarping::GetTooltipText() const
{
	return LOCTEXT("OrientationWarping", "Orientation Warping");
}

FText UAnimGraphNode_OrientationWarping::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("OrientationWarping", "Orientation Warping");
}

FString UAnimGraphNode_OrientationWarping::GetNodeCategory() const
{
	return TEXT("CustomTools");
}

#undef LOCTEXT_NAMESPACE

