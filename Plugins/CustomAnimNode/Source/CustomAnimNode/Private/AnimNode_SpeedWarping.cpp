// Copyright 2018 Sean Chen. All Rights Reserved.

#include "AnimNode_SpeedWarping.h"
#include "GameFramework/WorldSettings.h"
#include "Animation/AnimInstanceProxy.h"

FAnimNode_SpeedWarping::FAnimNode_SpeedWarping() :
	SpeedWarpingAxisMode(EIKFootRootLocalAxis::Y),
	PelvisAdjustmentAlpha(0.4f),
	MaxIter(3.0f),
	ClampIKUsingFKLeg(true),
	RemainingTime(0.f),
	BoneLocation(FVector::ZeroVector),
	BoneVelocity(FVector::ZeroVector),
	//TODO check if this is useful.
	OwnerVelocity(FVector::ZeroVector)
{
}

void FAnimNode_SpeedWarping::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_SkeletalControlBase::Initialize_AnyThread(Context);

	RemainingTime = 0.0f;
}

void FAnimNode_SpeedWarping::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	FAnimNode_SkeletalControlBase::CacheBones_AnyThread(Context);
}

void FAnimNode_SpeedWarping::UpdateInternal(const FAnimationUpdateContext& Context)
{
	FAnimNode_SkeletalControlBase::UpdateInternal(Context);
	DeltaTime += Context.GetDeltaTime();
	// simulation step
	TimeStep = (Context.GetDeltaTime() / MaxIter) * TimeDilation;
}

void FAnimNode_SpeedWarping::GatherDebugData(FNodeDebugData & DebugData)
{
	ComponentPose.GatherDebugData(DebugData);
}

void FAnimNode_SpeedWarping::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext & Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);
	const UWorld * TheWorld = Output.AnimInstanceProxy->GetAnimInstanceObject()->GetWorld();
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	//get bone index and varify them exists.
	FCompactPoseBoneIndex IKFootRootBoneCompactPoseIndex = IkFootRootBone.GetCompactPoseIndex(BoneContainer);
	FCompactPoseBoneIndex PelvisBoneCompactPoseIndex = PelvisBone.GetCompactPoseIndex(BoneContainer);
	if (IKFootRootBoneCompactPoseIndex == INDEX_NONE || PelvisBoneCompactPoseIndex == INDEX_NONE)
	{
		return;
	}

	// Every leg apply a spring force to pelvis.
	TArray<FVector> TSpringForce;
	TArray<FIKFootLocation> TIKFootLocations;

	if (!FeetDefinitions.Num() == 0)
	{
		//Calculate each foot's real position.
		for (FIKBones & Each : FeetDefinitions)
		{
			bool bInvalidLimb = false;
			FCompactPoseBoneIndex IKFootBoneCompactPoseIndex = Each.IKFootBone.GetCompactPoseIndex(BoneContainer);
			FCompactPoseBoneIndex FKFootBoneCompactPoseIndex = Each.FKFootBone.GetCompactPoseIndex(BoneContainer);
			FCompactPoseBoneIndex UpperLimbIndex(INDEX_NONE);
			const FCompactPoseBoneIndex LowerLimbIndex = BoneContainer.GetParentBoneIndex(FKFootBoneCompactPoseIndex);

			// if any leg is invalid, cancel the whole process.
			if (LowerLimbIndex == INDEX_NONE)
			{
				bInvalidLimb = true;
			}
			else
			{
				UpperLimbIndex = BoneContainer.GetParentBoneIndex(LowerLimbIndex);
				if (UpperLimbIndex == INDEX_NONE)
				{
					bInvalidLimb = true;
				}
			}
			if (bInvalidLimb)
			{
				return;
			}

			// Get Local Space transforms for our bones.
			const FTransform IKBoneLocalTransform = Output.Pose.GetLocalSpaceTransform(IKFootBoneCompactPoseIndex);
			const FTransform EndBoneLocalTransform = Output.Pose.GetLocalSpaceTransform(FKFootBoneCompactPoseIndex);
			const FTransform LowerLimbLocalTransform = Output.Pose.GetLocalSpaceTransform(LowerLimbIndex);
			const FTransform UpperLimbLocalTransform = Output.Pose.GetLocalSpaceTransform(UpperLimbIndex);

			// Get Component Space transforms for our bones.
			FTransform IKBoneCSTransform = Output.Pose.GetComponentSpaceTransform(IKFootBoneCompactPoseIndex);
			FTransform EndBoneCSTransform = Output.Pose.GetComponentSpaceTransform(FKFootBoneCompactPoseIndex);
			FTransform LowerLimbCSTransform = Output.Pose.GetComponentSpaceTransform(LowerLimbIndex);
			FTransform UpperLimbCSTransform = Output.Pose.GetComponentSpaceTransform(UpperLimbIndex);

			// Get current position of root of limb. 
			// All position are in Component space.
			const FVector RootPos = UpperLimbCSTransform.GetTranslation();
			const FVector InitialJointPos = LowerLimbCSTransform.GetTranslation();
			const FVector InitialEndPos = EndBoneCSTransform.GetTranslation();
			const FVector OriginIKPos = IKBoneCSTransform.GetTranslation();

			// Length of limbs.
			float LowerLimbLength = (EndBoneCSTransform.GetLocation() - LowerLimbCSTransform.GetLocation()).Size();
			float UpperLimbLength = (LowerLimbCSTransform.GetLocation() - UpperLimbCSTransform.GetLocation()).Size();

			// Get scale pivot
			const FVector ScalePivot = FVector(RootPos.X, RootPos.Y, OriginIKPos.Z);

			// Calculate new IKPos
			FVector NewIKPos = OriginIKPos;
			switch (SpeedWarpingAxisMode)
			{
			case EIKFootRootLocalAxis::NONE:
				break;
			case EIKFootRootLocalAxis::X:
				NewIKPos.X = ((OriginIKPos - ScalePivot).X * SpeedScaling) + ScalePivot.X;
			case EIKFootRootLocalAxis::Y:
				NewIKPos.Y = ((OriginIKPos - ScalePivot).Y * SpeedScaling) + ScalePivot.Y;
			case EIKFootRootLocalAxis::Z:
				NewIKPos.Z = ((OriginIKPos - ScalePivot).Z * SpeedScaling) + ScalePivot.Z;
			default:
				break;
			}

			// Clamp Ik foot position use total leg length.
			FVector ActualIKPos = NewIKPos;
			if (ClampIKUsingFKLeg)
			{
				float OriginIKLength = (OriginIKPos - RootPos).Size();
				float NewIkLength = (NewIKPos - RootPos).Size();
				float CutoffPercent = 1 - (1 / SpeedScaling);
				float TotalLimbLength = LowerLimbLength + UpperLimbLength;
				float MaxScaledLimbLength = OriginIKLength + (TotalLimbLength - OriginIKLength) * CutoffPercent;
				FVector IKDirection = (NewIKPos - RootPos).GetSafeNormal();
				if (NewIkLength > OriginIKLength)
				{
					FVector AdjustedIKPos = NewIKPos + IKDirection * (MaxScaledLimbLength - NewIkLength);
					ActualIKPos = (AdjustedIKPos - RootPos).Size() > (ActualIKPos - RootPos).Size() ? ActualIKPos : AdjustedIKPos;
				}
			}

			// Set Ik foot's actual position.
			IKBoneCSTransform.SetTranslation(ActualIKPos);
			OutBoneTransforms.Add(FBoneTransform(IKFootBoneCompactPoseIndex, IKBoneCSTransform));

			// Record origin and actual IK foot position.
			FIKFootLocation IKFootLocation;
			IKFootLocation.LimbRootLocation = RootPos;
			IKFootLocation.OriginLocation = OriginIKPos;
			IKFootLocation.ActualLocation = ActualIKPos;
			IKFootLocation.NewLocation = NewIKPos;
			TIKFootLocations.Add(IKFootLocation);
		}
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Calculate influence upon pelvis by this leg, according to actual Ik foot position.
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	for (FIKFootLocation EachIK : TIKFootLocations)
	{
		// Init values first time
		RemainingTime = DeltaTime;
		BoneLocation = EachIK.OriginLocation;
		BoneVelocity = FVector::ZeroVector;
		//		GEngine->AddOnScreenDebugMessage(17, 10, FColor::Green, FString::Printf(TEXT("OriginIKSize: %0.5f"), EachIK.OriginLocation.Size()));

		while (RemainingTime > TimeStep)
		{
			// Calculate error vector.
			FVector const IKDirection = (EachIK.ActualLocation - EachIK.LimbRootLocation).GetSafeNormal();
			float IKStretch = ((EachIK.ActualLocation - EachIK.LimbRootLocation).Size() - (EachIK.OriginLocation - EachIK.LimbRootLocation).Size());
			FVector const Error = IKDirection * IKStretch;
			FVector const SpringForce = PelvisAdjustmentInterp.Stiffness * Error;
			FVector const DampingForce = PelvisAdjustmentInterp.Dampen * BoneVelocity;

			// Calculate force based on error and velocity
			FVector const Acceleration = SpringForce - DampingForce;

			// Integrate velocity
			// Make sure damping with variable frame rate actually dampens velocity. Otherwise Spring will go nuts.
			float const CutOffDampingValue = 1.f / TimeStep;
			if (PelvisAdjustmentInterp.Dampen > CutOffDampingValue)
			{
				float const SafetyScale = CutOffDampingValue / PelvisAdjustmentInterp.Dampen;
				BoneVelocity += SafetyScale * (Acceleration * TimeStep);
			}
			else
			{
				BoneVelocity += (Acceleration * TimeStep);
			}

			// Integrate position
			FVector const OldBoneLocation = BoneLocation;
			FVector const DeltaMove = (BoneVelocity * TimeStep);
			BoneLocation += DeltaMove;

			// Update velocity to reflect post processing done to bone location.
			BoneVelocity = (BoneLocation - OldBoneLocation) / TimeStep;

			check(!BoneLocation.ContainsNaN());
			check(!BoneVelocity.ContainsNaN());

			// End of calculation.
			RemainingTime -= TimeStep;
		}
		TSpringForce.Add(BoneVelocity);
	}
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// Calculate resultant force.
	FVector ResultantForce = FVector(0.0f, 0.0f, 0.0f);
	for (FVector const EachVector : TSpringForce)
	{
		ResultantForce += EachVector;
	}

	//Calculate actual adjusted pelvis posiiton.
	FTransform PelvisBoneCSTransform = Output.Pose.GetComponentSpaceTransform(PelvisBoneCompactPoseIndex);
	FVector OriginPelvisLocation = PelvisBoneCSTransform.GetLocation();
	FVector NewPelvisLocation = OriginPelvisLocation + ResultantForce;
	FVector ActralPelvisLocation = OriginPelvisLocation + (ResultantForce * PelvisAdjustmentAlpha);

	// Set new pelvis transform.
	PelvisBoneCSTransform.SetTranslation(ActralPelvisLocation);
	OutBoneTransforms.Insert(FBoneTransform(PelvisBoneCompactPoseIndex, PelvisBoneCSTransform), 0);

}

bool FAnimNode_SpeedWarping::IsValidToEvaluate(const USkeleton * Skeleton, const FBoneContainer & RequiredBones)
{
	return (IkFootRootBone.IsValidToEvaluate(RequiredBones));
}

void FAnimNode_SpeedWarping::InitializeBoneReferences(const FBoneContainer & RequiredBones)
{
	IkFootRootBone.Initialize(RequiredBones);
	PelvisBone.Initialize(RequiredBones);
	if (!FeetDefinitions.Num() == 0)
	{
		for (FIKBones & Each : FeetDefinitions)
		{
			Each.IKFootBone.Initialize(RequiredBones);
			Each.FKFootBone.Initialize(RequiredBones);
		}
	}
}

void FAnimNode_SpeedWarping::PreUpdate(const UAnimInstance * InAnimInstance)
{
	const USkeletalMeshComponent* SkelComp = InAnimInstance->GetSkelMeshComponent();
	const UWorld* World = SkelComp->GetWorld();
	check(World->GetWorldSettings());
	TimeDilation = World->GetWorldSettings()->GetEffectiveTimeDilation();

	AActor* SkelOwner = SkelComp->GetOwner();
	if (SkelComp->GetAttachParent() != NULL && (SkelOwner == NULL))
	{
		SkelOwner = SkelComp->GetAttachParent()->GetOwner();
		OwnerVelocity = SkelOwner->GetVelocity();
	}
	else
	{
		OwnerVelocity = FVector::ZeroVector;
	}
}


