// Copyright 2018 Sean Chen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BoneContainer.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_SpeedWarping.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EIKFootRootLocalAxis : uint8
{
	NONE	UMETA(DisplayName = "NONE"),
	X		UMETA(DisplayName = "IKFootRootLocalX"),
	Y		UMETA(DisplayName = "IKFootRootLocalY"),
	Z		UMETA(DisplayName = "IKFootRootLocalZ")
};

USTRUCT(BlueprintType)
struct FIKBones
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = Settings)
		FBoneReference IKFootBone;

	UPROPERTY(EditAnywhere, Category = Settings)
		FBoneReference FKFootBone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
		int32 NumBonesInLimb;

};

USTRUCT(BlueprintType)
struct FIKFootLocation
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = Settings)
		FVector LimbRootLocation;

	UPROPERTY(EditAnywhere, Category = Settings)
		FVector OriginLocation;

	UPROPERTY(EditAnywhere, Category = Settings)
		FVector ActualLocation;

	UPROPERTY(EditAnywhere, Category = Settings)
		FVector NewLocation;

	FIKFootLocation() :
		LimbRootLocation(FVector(0.f, 0.f, 0.f)),
		OriginLocation(FVector(0.f, 0.f, 0.f)),
		ActualLocation(FVector(0.f, 0.f, 0.f)),
		NewLocation(FVector(0.f, 0.f, 0.f))
	{

	}

};

USTRUCT(BlueprintType)
struct FPelvisAdjustmentInterp
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		float Stiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		float Dampen;

	FPelvisAdjustmentInterp() :
		Stiffness(1.0f),
		Dampen(1.0f)
	{}

};

USTRUCT(BlueprintInternalUseOnly)
struct CUSTOMANIMNODE_API FAnimNode_SpeedWarping : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

public:
	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, Category = Settings)
		FBoneReference IkFootRootBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		TArray<FIKBones> FeetDefinitions;

	UPROPERTY(EditAnywhere, Category = Settings)
		FBoneReference PelvisBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		EIKFootRootLocalAxis SpeedWarpingAxisMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
		float SpeedScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		float PelvisAdjustmentAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		float MaxIter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		FPelvisAdjustmentInterp PelvisAdjustmentInterp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		bool ClampIKUsingFKLeg;


	/** Delta time */
	float DeltaTime;

	/** Internal use - Amount of time we need to simulate. */
	float RemainingTime;

	/** Internal use - Current timestep */
	float TimeStep;

	/** Internal use - Current time dilation */
	float TimeDilation;

	/** Did we have a non-zero ControlStrength last frame. */
	bool bHadValidStrength;

	/** World-space location of the bone. */
	FVector BoneLocation;

	/** World-space velocity of the bone. */
	FVector BoneVelocity;

	/** Velocity of the owning actor */
	FVector OwnerVelocity;

public:
	FAnimNode_SpeedWarping();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	// End of FAnimNode_Base interface

	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

private:
	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	// Resused bone transform array to avoid reallocating in skeletal controls
	TArray<FBoneTransform> BoneTransforms;
};
