// Copyright 2018 Sean Chen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_OrientationWarping.h"
#include "AnimGraphNode_OrientationWarping.generated.h"

struct FAnimMode_OrientationWarping;
/**
*
*/
UCLASS()
class CUSTOMANIMNODEEDITOR_API UAnimGraphNode_OrientationWarping : public UAnimGraphNode_Base
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "Settings")
		FAnimMode_OrientationWarping Node;

	//~ Begin UEdGraphNode Interface.
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UAnimGraphNode_Base Interface
	virtual FString GetNodeCategory() const override;
	//~ End UAnimGraphNode_Base Interface

	UAnimGraphNode_OrientationWarping();


};
