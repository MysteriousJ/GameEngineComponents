#include "Gobmesh.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


Matrix4x4 getMatrix4x4(aiMatrix4x4 m)
{
	Matrix4x4 r;
	r[0][0]=m.a1; r[0][1]=m.a2; r[0][2]=m.a3; r[0][3]=m.a4;
	r[1][0]=m.b1; r[1][1]=m.b2; r[1][2]=m.b3; r[1][3]=m.b4;
	r[2][0]=m.c1; r[2][1]=m.c2; r[2][2]=m.c3; r[2][3]=m.c4;
	r[3][0]=m.d1; r[3][1]=m.d2; r[3][2]=m.d3; r[3][3]=m.d4;
	return r;
}

Vec3 getVec3(aiVector3D v)
{
	Vec3 r = {v.x, v.y, v.z};
	return r;
}

Vec2 getVec2(aiVector3D v)
{
	Vec2 r ={v.x, v.y};
	return r;
}

Quaternion getQuaternion(aiQuaternion q)
{
	Quaternion r;
	r.w = q.w;
	r.x = q.x;
	r.y = q.y;
	r.z = q.z;
	return r;
}

// Looks like assimp automatically optimizes the mesh for us!
// Can be called multiple times on the same output mesh, and it will append the new assetMesh at the end of the old one, merging them.
void convertAssimpMesh(Mesh* out_mesh, const aiMesh* assetMesh, Skeleton* skeleton, Matrix4x4 transform)
{
	uint vertexCount = assetMesh->mNumVertices;
	uint faceCount = assetMesh->mNumFaces;

	// Faces
	// If we are appending a mesh onto another mesh, the indices for this face need to be offset, starting at the end of the last mesh's vertices
	uint faceIndexOffset = out_mesh->positions.size();
	for (unsigned int i=0; i<faceCount; i++)
	{
		Face face;
		face.a = faceIndexOffset + assetMesh->mFaces[i].mIndices[0];
		face.b = faceIndexOffset + assetMesh->mFaces[i].mIndices[1];
		face.c = faceIndexOffset + assetMesh->mFaces[i].mIndices[2];
		out_mesh->faces.push_back(face);
	}

	for (uint i=0; i<vertexCount; i++)
	{
		// Positions
		Vec3 position = (transform * getVec3(assetMesh->mVertices[i])).xyz;
		out_mesh->positions.push_back(position);

		// UVs
		if (assetMesh->mTextureCoords[0]) {
			Vec2 uv = getVec2(assetMesh->mTextureCoords[0][i]);
			out_mesh->uvs.push_back(uv);
		}
		else {
			// If the mesh does not have UV coords, set all UVs to 0
			out_mesh->uvs.push_back({0, 0});
		}

		// Normals
		Vec3 normal = (transform * getVec3(assetMesh->mNormals[i])).xyz;
		out_mesh->normals.push_back(normal);
	}

	if (skeleton)
	{
		// Bone indices and weights
		std::vector<std::vector<uint>> indeces(vertexCount);
		std::vector<std::vector<float>> weights(vertexCount);

		if (assetMesh->HasBones())
		{
			// Assimp stores per-vertex information in the mesh's bones
			for (uint boneIndex=0; boneIndex < assetMesh->mNumBones; ++boneIndex)
			{
				aiBone* thisBone = assetMesh->mBones[boneIndex];
				// Each bone has "weights" that dscribe the verteces that are bound to it
				for (uint weightIndex=0; weightIndex < thisBone->mNumWeights; ++weightIndex)
				{
					uint thisVertexIndex = thisBone->mWeights[weightIndex].mVertexId;
					indeces[thisVertexIndex].push_back(findJointIndexWithName(thisBone->mName.C_Str(), *skeleton));
					weights[thisVertexIndex].push_back(thisBone->mWeights[weightIndex].mWeight);
				}
			}

			// Normalize the vertex's weights so they sum to 1.
			for (uint i=0; i<weights.size(); i++)
			{
				float sum = 0;
				for (uint j=0; j<weights[i].size(); j++) {
					sum += weights[i][j];
				}
				for (uint j=0; j<weights[i].size(); j++) {
					weights[i][j] /= sum;
				}
			}
		}

		// Copy the new joint bindings to the mesh
		for (uint i=0; i<vertexCount; ++i)
		{
			out_mesh->jointIndeces.push_back(indeces[i]);
			out_mesh->jointWeights.push_back(weights[i]);
		}
	}
}

void convertAssimpMeshesInNodeTree(Mesh* out_mesh, const aiScene* scene, const aiNode* node, Skeleton* skeleton, Matrix4x4 parentTransform)
{
	Matrix4x4 transform = parentTransform * getMatrix4x4(node->mTransformation);
	// Convert meshes in this node
	for (uint i=0; i<node->mNumMeshes; ++i) {
		uint meshIndex = node->mMeshes[i];
		aiMesh* mesh = scene->mMeshes[meshIndex];
		convertAssimpMesh(out_mesh, mesh, skeleton, transform);
	}
	// Recursively search child nodes
	for (uint i=0; i<node->mNumChildren; ++i) {
		convertAssimpMeshesInNodeTree(out_mesh, scene, node->mChildren[i], skeleton, transform);
	}
}

void convertAssimpMeshesInScene(Mesh* out_mesh, const aiScene* scene, Skeleton* skeleton)
{
	convertAssimpMeshesInNodeTree(out_mesh, scene, scene->mRootNode, skeleton, Matrix4x4::identity);
}

// Recursivly reads a tree of joints
void readJoint(Skeleton* out_skeleton, const aiNode* jointNode, uint parentIndex, Matrix4x4 parentTransform)
{
	Matrix4x4 transform = parentTransform * getMatrix4x4(jointNode->mTransformation);
	Joint joint;
	joint.name = jointNode->mName.C_Str();
	joint.parentIndex = parentIndex;
	joint.inverseBindMatrix = inverse(transform);
	uint jointIndex = out_skeleton->joints.size();
	out_skeleton->joints.push_back(joint);

	for (uint i=0; i < jointNode->mNumChildren; i++) {
		readJoint(out_skeleton, jointNode->mChildren[i], jointIndex, transform);
	}
}

void convertAssimpSkeleton(Skeleton* out_skeleton, aiNode* assimpSkeleton)
{
	out_skeleton->rootJointIndex = 0;
	readJoint(out_skeleton, assimpSkeleton, 0, Matrix4x4::identity);
}

double max(double a, double b) {
	return a>b? a : b;
}

void convertAssimpAnimation(SkeletonAnimation* out_animation, aiAnimation* assimpAnimation, const Skeleton& skeleton)
{
	// The animation must have the same number of joints as the skeleton it's for
	out_animation->joints.resize(skeleton.joints.size());
	out_animation->keysPerSecond = float(assimpAnimation->mTicksPerSecond);

	double lastScaleTime = 0;
	double lastRotationTime = 0;
	double lastPositionTime = 0;

	// Loop through the joints ("channels") in the animation
	for (uint i=0; i<assimpAnimation->mNumChannels; i++) {
		JointAnimation joint;
		aiNodeAnim* assimpJoint = assimpAnimation->mChannels[i];

		// Get the keys in the joint's timeline
		// Scale
		for (uint j=0; j<assimpJoint->mNumScalingKeys; j++) {
			joint.scaleKeys.push_back(getVec3(assimpJoint->mScalingKeys[j].mValue));
			joint.scaleKeyTimes.push_back(float(assimpJoint->mScalingKeys[j].mTime/assimpAnimation->mTicksPerSecond));
			lastScaleTime = max(lastScaleTime, assimpJoint->mScalingKeys[j].mTime);
		}
		// Rotation
		for (uint j=0; j<assimpJoint->mNumRotationKeys; j++) {
			joint.rotationKeys.push_back(getQuaternion(assimpJoint->mRotationKeys[j].mValue));
			joint.roateKeyTimes.push_back(float(assimpJoint->mRotationKeys[j].mTime/assimpAnimation->mTicksPerSecond));
			lastRotationTime = max(lastRotationTime, assimpJoint->mRotationKeys[j].mTime);
		}
		// Translation
		for (uint j=0; j<assimpJoint->mNumPositionKeys; j++) {
			joint.translationKeys.push_back(getVec3(assimpJoint->mPositionKeys[j].mValue));
			joint.translateKeyTimes.push_back(float(assimpJoint->mPositionKeys[j].mTime/assimpAnimation->mTicksPerSecond));
			lastPositionTime = max(lastPositionTime, assimpJoint->mPositionKeys[j].mTime);
		}

		uint jointIndex = findJointIndexWithName(assimpJoint->mNodeName.C_Str(), skeleton);
		out_animation->joints[jointIndex] = joint;
	}

	// The duration is the time of the last key in the animation
	double lastKeyTime = max(lastScaleTime, max(lastRotationTime, lastPositionTime));
	out_animation->duration = float(lastKeyTime/assimpAnimation->mTicksPerSecond);
}


aiNode* getSkeleton(const aiScene* scene)
{
	aiNode* root = scene->mRootNode;
	// Search through all of the nodes in the scene's root node for a skeleton.
	for (uint i=0; i < root->mNumChildren; i++)
	{
		// If the node doesn't have any meshes, assume it's a skeleton
		if (root->mChildren[i]->mNumMeshes == 0) {
			return root->mChildren[i];
		}
	}
	return 0;
}