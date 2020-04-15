// Copyright 2018 Sean Chen. All Rights Reserved.

#include "AnimNode_OrientationWarping.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"

FAnimMode_OrientationWarping::FAnimMode_OrientationWarping()
{
}

void FAnimMode_OrientationWarping::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	BasePose.Initialize(Context);
	const FBoneContainer& BoneContainer = Context.AnimInstanceProxy->GetRequiredBones();
	IKFootRootBone.Initialize(BoneContainer);
	FBoneReference(FName("pelvis")).Initialize(BoneContainer);
	for (FBoneRef & Bone : SpineBones)
	{
		Bone.Bone.Initialize(BoneContainer);
	}
}

void FAnimMode_OrientationWarping::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	BasePose.CacheBones(Context);
}

void FAnimMode_OrientationWarping::Update_AnyThread(const FAnimationUpdateContext & Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	BasePose.Update(Context);
}

void FAnimMode_OrientationWarping::Evaluate_AnyThread(FPoseContext & Output)
{
	BasePose.Evaluate(Output);

	check(!FMath::IsNaN(LocomotionAngle) && FMath::IsFinite(LocomotionAngle));


	if (!FMath::IsNearlyZero(LocomotionAngle, KINDA_SMALL_NUMBER))
	{
		const FBoneContainer& BoneContainer = Output.AnimInstanceProxy->GetRequiredBones();
		if (IKFootRootBone.IsValidToEvaluate(BoneContainer))
		{
			// Prepare convert Quat and BoneContainer.
			const FQuat MeshToComponentQuat(FRotator(0.f, 0.f, 0.f));
			FComponentSpacePoseContext CSOutput(Output.AnimInstanceProxy);
			CSOutput.Pose.InitPose(Output.Pose);

			//Build our desired rotation for IK root bone.
			FRotator DeltaRotation(0.0f, 0.0f, 0.f);
			switch (Settings.YawRotationAxis)
			{
			case EAxis::X:
				DeltaRotation.Roll = LocomotionAngle;
			case EAxis::Y:
				DeltaRotation.Pitch = LocomotionAngle;
			case EAxis::Z:
				DeltaRotation.Yaw = LocomotionAngle;
			default:
				break;
			}
			const FQuat DeltaQuat(DeltaRotation);
			// Convert our rotation from Component Space to Mesh Space.
			const FQuat MeshSpaceDeltaQuat = MeshToComponentQuat.Inverse() * DeltaQuat * MeshToComponentQuat;
			// Apply rotation to IK root bone.
			FCompactPoseBoneIndex RotateBoneIndex = IKFootRootBone.GetCompactPoseIndex(BoneContainer);
			Output.Pose[RotateBoneIndex].SetRotation(Output.Pose[RotateBoneIndex].GetRotation() * MeshSpaceDeltaQuat);
			Output.Pose[RotateBoneIndex].NormalizeRotation();


			// Do the same things like IK foot root bone to pelvis, but in the reversed orientation.
			FCompactPoseBoneIndex PelvisBoneIndex(1);
			FTransform PelvisBoneTM = CSOutput.Pose.GetComponentSpaceTransform(PelvisBoneIndex);

			const FRotator PelvisDeltaRotation(DeltaRotation.Pitch * Settings.BodyOrientationAlpha, DeltaRotation.Yaw * Settings.BodyOrientationAlpha, DeltaRotation.Roll * Settings.BodyOrientationAlpha);
			const FQuat PelvisDeltaQuat(PelvisDeltaRotation);
			const FQuat MeshSpacePelvisDeltaQuat = PelvisBoneTM.GetRotation().Inverse() * PelvisDeltaQuat * PelvisBoneTM.GetRotation();
			Output.Pose[PelvisBoneIndex].ConcatenateRotation(MeshSpacePelvisDeltaQuat);
			Output.Pose[PelvisBoneIndex].NormalizeRotation();

			// Apply rotation to spine
			if (SpineBones.Num())
			{
				for (int32 i = 0; i < SpineBones.Num(); i++)
				{
					if (SpineBones[i].Bone.IsValidToEvaluate(BoneContainer))
					{
						FCompactPoseBoneIndex SpineBoneIndex = SpineBones[i].Bone.GetCompactPoseIndex(BoneContainer);
						FTransform SpineBoneTM = CSOutput.Pose.GetComponentSpaceTransform(SpineBoneIndex);
						const FRotator SpineDeltaRotation((-PelvisDeltaRotation.Pitch / SpineBones.Num()), (-PelvisDeltaRotation.Yaw / SpineBones.Num()), (-PelvisDeltaRotation.Roll / SpineBones.Num()));
						const FQuat SpineDeltaQuat(SpineDeltaRotation);
						const FQuat MeshSpaceSpineDeltaQuat = SpineBoneTM.GetRotation().Inverse() * SpineDeltaQuat * SpineBoneTM.GetRotation();
						Output.Pose[SpineBoneIndex].ConcatenateRotation(MeshSpaceSpineDeltaQuat);
						Output.Pose[SpineBoneIndex].NormalizeRotation();
						//GEngine->AddOnScreenDebugMessage((i + 2), 10, FColor::Yellow, (FString::Printf(TEXT(" TargetID: %d, TargetName: %s"), SpineBones[i].Bone.GetCompactPoseIndex((BoneContainer)).GetInt(), *SpineBones[i].Bone.BoneName.ToString())));
					}
				}
			}
		}
	}

}

void FAnimMode_OrientationWarping::GatherDebugData(FNodeDebugData & DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine);

	BasePose.GatherDebugData(DebugData);
}
