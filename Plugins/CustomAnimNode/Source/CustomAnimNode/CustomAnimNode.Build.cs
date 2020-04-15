// Copyright 2018 Sean Chen. All Rights Reserved.

using UnrealBuildTool;

public class CustomAnimNode : ModuleRules
{
    public CustomAnimNode(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

   //     PublicIncludePaths.AddRange(
   //         new string[] {
   //             "CustomAnimNode/Public"
			//	// ... add public include paths required here ...
			//}
   //         );


   //     PrivateIncludePaths.AddRange(
   //         new string[] {
   //             "CustomAnimNode/Private",
			//	// ... add other private include paths required here ...
			//}
   //         );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "AnimationCore",
                "AnimGraphRuntime"
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
