#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <assert.h>
#include "Algebra.h"
#include "AssimpConvert.h"

void convertFile(std::string fileName)
{
	// The output file name and location is the same as the input file,
	// but may be longer, and has the extention replaced with .gob*
	int endSubStrPos = fileName.find_last_of('.');
	std::string outputFileName = fileName.substr(0, endSubStrPos);

	/*aiLogStream logStream;
	logStream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&logStream);*/

	const aiScene* assetScene = 0;
	assetScene = aiImportFile(fileName.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

	if (!assetScene) {
		std::cout << aiGetErrorString() << '\n';
		return;
	}

	Skeleton* outputSkeleton = 0;
	aiNode* skeletonNode = getSkeleton(assetScene);
	if (skeletonNode) {
		outputSkeleton = new Skeleton;
		convertAssimpSkeleton(outputSkeleton, skeletonNode);
		outputGOBSKEL(outputFileName + ".gobskel", *outputSkeleton);

		// Output each animation to an individual file.
		for (unsigned int i=0; i < assetScene->mNumAnimations; i++)
		{
			SkeletonAnimation animation;
			convertAssimpAnimation(&animation, assetScene->mAnimations[i], *outputSkeleton);

			// Hack around dumb animation names that Blender exports
			std::string animationName = assetScene->mAnimations[i]->mName.C_Str();
			int lastPipeChar = animationName.find_last_of('|');
			animationName = animationName.substr(lastPipeChar+1);

			// Check that the animation has the same number of joints as the skeleton
			if (animation.joints.size() != outputSkeleton->joints.size()) {
				std::cout << "Error: the skeleton and the animation '" << assetScene->mAnimations[i]->mName.C_Str() << "' have different numbers of joints.\n";
			}
			else {
				outputGOBSKELANIM(outputFileName + "_" + animationName + ".gobskelanim", animation);
			}
		}
	}

	if (assetScene->mNumMeshes > 0)
	{
		// If the input file contains multiple meshes, we'll merge them together into one.
		Mesh outputMesh;
		convertAssimpMeshesInScene(&outputMesh, assetScene, outputSkeleton);
		outputGOBMESH(outputFileName + ".gobmesh", outputMesh);
	}

	delete outputSkeleton;
	aiReleaseImport(assetScene);
}

int main(int argCount, const char* args[])
{
	for (int i=1; i<argCount; ++i)
	{
		convertFile(args[i]);
	}

	return 0;
}