// Copyright 2018 Sean Chen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimNode_SpeedWarping.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimGraphNode_SpeedWarping.generated.h"

struct FAnimNode_SpeedWarping;
/**
*
*/
UCLASS()
class CUSTOMANIMNODEEDITOR_API UAnimGraphNode_SpeedWarping : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Settings")
	FAnimNode_SpeedWarping Node;

public:

	UAnimGraphNode_SpeedWarping();

	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

protected:
	// UAnimGraphNode_Base interface
	virtual void CopyPinDefaultsToNodeData(UEdGraphPin* InPin) override;
	// End of UAnimGraphNode_Base interface

	// UAnimGraphNode_SkeletalControlBase interface
	virtual const FAnimNode_SkeletalControlBase* GetNode() const { return &Node; }
	// End of UAnimGraphNode_SkeletalControlBase interface


};
