#include "Algebra.h"
#include <vector>
#include <array>
#include <iostream>
using namespace goblin;

#define SUPPORTED_JOINTS_PER_VERTEX 4
typedef unsigned int uint;
typedef bool bool8;

struct Face
{
	unsigned int a, b, c;
};

struct Mesh
{
	std::vector<Face> faces;
	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;
	std::vector<Vec3> normals;
	// Each vertex is bound to 0 or more joints
	std::vector<std::vector<uint>> jointIndeces;
	std::vector<std::vector<float>> jointWeights;
};

struct Joint
{
	std::string name;
	unsigned int parentIndex;
	Matrix4x4 inverseBindMatrix;
};

struct Skeleton
{
	std::vector<Joint> joints;
	unsigned int rootJointIndex;
};

struct JointAnimation
{
	std::vector<float> scaleKeyTimes;
	std::vector<Vec3> scaleKeys;

	std::vector<float> roateKeyTimes;
	std::vector<Quaternion> rotationKeys;

	std::vector<float> translateKeyTimes;
	std::vector<Vec3> translationKeys;
};

struct SkeletonAnimation
{
	std::string name;
	float duration;
	float keysPerSecond;
	std::vector<JointAnimation> joints;
};

int min(int a, int b)
{
	return (a < b)? a : b;
}

void writeNull(std::ofstream *output, uint numberOfBytes)
{
	char zero = 0;
	for(uint i=0; i<numberOfBytes; ++i) {
		output->write(&zero, sizeof(zero));
	}
}

uint findJointIndexWithName(std::string name, const Skeleton& skeleton)
{
	for (uint i=0; i<skeleton.joints.size(); i++)
	{
		if (skeleton.joints[i].name == name) {
			return i;
		}
	}
	return 0;
}

void outputGOBMESH(const std::string& fileName, const Mesh& mesh)
{
	// Create/open output file
	std::ofstream output(fileName, std::ofstream::binary);

	// Header
	uint faceCount = mesh.faces.size();
	uint vertexCount = mesh.positions.size();
	bool8 hasUVs = (mesh.uvs.size() > 0);
	bool8 hasNormals = (mesh.normals.size() > 0);
	bool8 hasSkeletonBindings = (mesh.jointIndeces.size() > 0);

	output.write((char*)&faceCount, sizeof(faceCount));
	output.write((char*)&vertexCount, sizeof(vertexCount));
	output.write((char*)&hasUVs, sizeof(bool8));
	output.write((char*)&hasNormals, sizeof(bool8));
	output.write((char*)&hasSkeletonBindings, sizeof(bool8));
	// End of header

	// Faces
	output.write((char*)&(mesh.faces[0].a), mesh.faces.size()*sizeof(Face));
	// Positions
	output.write((char*)&(mesh.positions[0].x), mesh.positions.size()*sizeof(Vec3));
	// UVs
	output.write((char*)&(mesh.uvs[0].x), mesh.uvs.size()*sizeof(Vec2));
	// Normals
	output.write((char*)&(mesh.normals[0].x), mesh.normals.size()*sizeof(Vec3));
 
	// Joints
	if (hasSkeletonBindings)
	{
		// Check if any vertex is bound to more than the supported number of joints
		for (uint i=0; i<vertexCount; i++)
		{
			if (mesh.jointIndeces[i].size() > SUPPORTED_JOINTS_PER_VERTEX
			 || mesh.jointWeights[i].size() > SUPPORTED_JOINTS_PER_VERTEX)
			{
				std::cout << "Warning: one or more verteces are bound to more than the supported number of joints, which is " << SUPPORTED_JOINTS_PER_VERTEX << ".";
			}
		}

		// Write indeces
		for (uint i=0; i<mesh.jointIndeces.size(); i++) {
			int usedJointCount = min(mesh.jointIndeces[i].size(), SUPPORTED_JOINTS_PER_VERTEX);
			if (usedJointCount > 0)	output.write((char*)&(mesh.jointIndeces[i][0]), usedJointCount*sizeof(uint));
			// Pad out any unused joint slots with 0s
			writeNull(&output, (SUPPORTED_JOINTS_PER_VERTEX-usedJointCount)*sizeof(uint));
		}
		// Write weights
		for (uint i=0; i<mesh.jointWeights.size(); i++) {
			int usedJointCount = min(mesh.jointWeights[i].size(), SUPPORTED_JOINTS_PER_VERTEX);
			if (usedJointCount > 0) output.write((char*)&(mesh.jointWeights[i][0]), usedJointCount*sizeof(float));
			// Pad out any unused joint slots with 0s
			writeNull(&output, (SUPPORTED_JOINTS_PER_VERTEX-usedJointCount)*sizeof(float));
		}
	}
}

void outputGOBSKEL(const std::string& fileName, Skeleton& skeleton)
{
	// Create/open output file
	std::ofstream output(fileName, std::ofstream::binary);

	/* .gobskel file format
	uint number of joints
	uint root joint index
	x joints (uint parentIndex, Matrix4x4 inverseBindTtansform)
	*/

	// Number of joints
	uint jointCount = skeleton.joints.size();
	output.write((char*)&jointCount, sizeof(jointCount));

	// Root joint index
	output.write((char*)&skeleton.rootJointIndex, sizeof(unsigned int));

	// Joints
	for (uint i=0; i<skeleton.joints.size(); i++) {
		output.write((char*)&(skeleton.joints[i].parentIndex), sizeof(uint));
		output.write((char*)&(skeleton.joints[i].inverseBindMatrix), 4*4*sizeof(float));
	}
}

void outputGOBSKELANIM(const std::string& fileName, SkeletonAnimation& animation)
{
	// Create/open output file
	std::ofstream output(fileName, std::ofstream::binary);
	if (!output.is_open()) {
		std::cout << "Failed to create file " + fileName + ".\n";
		return;
	}

	/* File format
	Header {
		float32 duration
		uint32 joint count
	}
	for each joint {
		uint32 number of scale keys
		uint32 number of rotation keys
		uint32 number of translation keys
		float scale key times [number of scale keys]
		float rotate key times [number of rotation keys]
		float translate key times [number of translation keys]
		Vec3 scale key values [number of scale keys]
		Quaternion rotation key values [number of rotation keys]
		Vec3 translation key values [number of translation keys]
	}
	*/

	output.write((char*)&(animation.duration), sizeof(animation.duration));
	uint numberOfJoints = animation.joints.size();
	output.write((char*)&numberOfJoints, sizeof(numberOfJoints));

	for (uint i=0; i<animation.joints.size(); i++)
	{
		// Scale
		uint scaleKeyCount = animation.joints[i].scaleKeys.size();
		output.write((char*)&scaleKeyCount, sizeof(scaleKeyCount));
		if (scaleKeyCount > 0) {
			output.write((char*)&(animation.joints[i].scaleKeyTimes[0]),
				scaleKeyCount*sizeof(float));
			output.write((char*)&(animation.joints[i].scaleKeys[0]),
				scaleKeyCount*sizeof(Vec3));
		}
		// Rotation
		uint rotateKeyCount = animation.joints[i].rotationKeys.size();
		output.write((char*)&rotateKeyCount, sizeof(rotateKeyCount));
		if (rotateKeyCount > 0) {
			output.write((char*)&(animation.joints[i].roateKeyTimes[0]),
				rotateKeyCount*sizeof(float));
			output.write((char*)&(animation.joints[i].rotationKeys[0]),
				rotateKeyCount*sizeof(Quaternion));
		}
		// Translation
		uint translateKeyCount = animation.joints[i].translationKeys.size();
		output.write((char*)&translateKeyCount, sizeof(translateKeyCount));
		if (translateKeyCount > 0) {
			output.write((char*)&(animation.joints[i].translateKeyTimes[0]),
				translateKeyCount*sizeof(float));
			output.write((char*)&(animation.joints[i].translationKeys[0]),
				translateKeyCount*sizeof(Vec3));
		}
	}
}