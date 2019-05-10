#ifndef GOBLIN_SKELETON_ANIMATION_HEADER
#define GOBLIN_SKELETON_ANIMATION_HEADER

#include "Goblin3D.h"
#include <assert.h>
#include <vector>

namespace goblin {

struct Skeleton
{
	// The array of joints is a tree.
	// Every joint must come after its parent in the array.
	// The root joint is at 0, and its parent index is itself.
	struct Joint {
		unsigned int parentIndex;
		Matrix4x4 modelSpaceBindPoseInverse;
	};

	Joint* joints;
	unsigned int jointCount;
	unsigned int rootJointIndex;
};

void createSkeletonFromGOBSKEL(Skeleton* out_skeleton, char* bytes, size_t byteCount);
void destroySkeleton(Skeleton* skeleton);

/* Contains a list of joints, each of which has it's own timeline.
Duration is the length of the joint's timeline that is the longest.
The joints are at the same indices they are in the skeleton this animation is for.
*/
struct SkeletonAnimation
{
	/* Keeps the scale, rotate, and translate channels in separate arrays.
	Each keyframe has a time it occures, and a corresponding value.
	e.g. the value at scaleKeyValue[5] should happen at scaleKeyTime[5]
	Key times are in seconds.
	Key values are transforms relative to the joint's parent.
	*/
	struct JointAnimation
	{
		unsigned int scaleKeyCount;
		float* scaleKeyTimes;
		Vec3* scaleKeyValues;

		unsigned int rotateKeyCount;
		float* rotateKeyTimes;
		Quaternion* rotateKeyValues;

		unsigned int translateKeyCount;
		float* translateKeyTimes;
		Vec3* translateKeyValues;
	};

	float duration;
	unsigned int keysPerSecond;
	unsigned int jointCount;
	JointAnimation* jointAnimations;
};

void createSkeletonAnimationFromGOBSKELANIM(SkeletonAnimation* out_animation, char* bytes, size_t byteCount);
void destroySkeletonAnimation(SkeletonAnimation* animation);

// A "JointPose" is a relative offset from a joint's parent.

/* Search an array of floats to find the closest indices that 
have less than and greater than the specified values.
Animation keys are sorted in order of low to high, starting at 0. 
If at the start or end of the animation, firstkey == secondkey. */
void findAnimationKeys(
	unsigned int* out_firstKey,
	unsigned int* out_secondKey,
	float time,
	float* arrayOfTimes,
	unsigned int numberOfTimes);

/* Fills an array with the pose of each joint in the animation, relative to its parent joint.
Time is clamped to [start of animation, end of animation].
Length of out_jointTransformsArray must be at least the number of joints in the animation. */
void sampleSkeletonAnimation(
	Transform* out_jointTransformsArray,
	float time,
	const SkeletonAnimation& animation);

/* Builds an array of transforms that contain only the difference in 
position, rotation, and scale between the reference pose and the target pose.
Use the difference poses in additive blending.
Inputs and output can be the same array. */
void buildDifferenceSkeletonPose(
	Transform *out_differenceJointPoses,
	Transform *referenceJointPoses,
	Transform *targetJointPoses,
	unsigned int jointCount);

/* Adds the transforms of addedJointPoses to baseJointPoses.
Since joint poses sampled from a skeleton animaiton contain the entire transform
from the joint's parent, the added poses should be generated from buildDifferenceSkeletonPoses().
Inputs and output can be the same array. */
void additiveBlendSkeletonPoses(
	Transform *out_blendedJointPoses,
	Transform *baseJointPoses,
	Transform *addedJointPoses,
	unsigned int jointCount,
	float addWeight);

/* Convenience shortcut for sampling the animations,
getting the difference poses, and additive blending. */
void blendSkeletonAnimation(
	Transform *out_blendedJointPoses,
	SkeletonAnimation* animation,
	float animationTime,
	float weight,
	SkeletonAnimation *referencePose = 0,
	float referencePoseTime = 0);

/* Converts each joint in a sampled pose into model space.
For each joint in the skeleton, jointPoses contains the offset-transform from its parent.
Goes through each joint and applies the transform of its parent.
Do any blending of poses before this step.
Input and output can be the same array. */
void jointPosesToModelSpace(
	Transform *out_modelSpaceJoints,
	Transform *jointPoses,
	Skeleton& skeleton);

/* Builds the matrices need to transform a vertex by a joint in the vertex shader.
A skinning matrix transforms a model-space vertex position by moving it into
the joint's pose space, then applying all of its parent's offsets from
each other until it makes it back into model-space, in the modified position. */
void buildSkinningMatrix(
	Matrix4x4* out_matrixArray,
	Transform* modelSpaceJoints,
	Skeleton& skeleton);




// Implementation =============================================================

void createSkeletonFromGOBSKEL(Skeleton* out_skeleton, char* bytes, size_t byteCount)
{
	/*
	The joints must be ordered so that every joint comes after its
	parent in the array.

	File format:
	Header {
	uint32 number of joints
	uint32 root joint index (deprecated)
	}
	for each joint {
	uint32 parent index
	float32[16] model space inverse bind pose matrix
	}
	*/
	BinaryReader b(bytes, byteCount);

	b.readInto(&out_skeleton->jointCount, sizeof(out_skeleton->jointCount));
	b.readInto(&out_skeleton->rootJointIndex, sizeof(out_skeleton->rootJointIndex));
	out_skeleton->joints = new Skeleton::Joint[out_skeleton->jointCount];

	for (unsigned int i=0; i<out_skeleton->jointCount; i++)
	{
		Skeleton::Joint joint;
		b.readInto(&joint.parentIndex, sizeof(joint.parentIndex));
		b.readInto(&joint.modelSpaceBindPoseInverse, sizeof(joint.modelSpaceBindPoseInverse));
		assert(joint.parentIndex >= 0 && joint.parentIndex < out_skeleton->jointCount);

		out_skeleton->joints[i] = joint;
	}
}

void destroySkeleton(Skeleton* skeleton)
{
	if (skeleton->jointCount == 0) {
		return;
	}
	delete[] skeleton->joints;
	Skeleton zero={};
	*skeleton = zero;
}

void createSkeletonAnimationFromGOBSKELANIM(SkeletonAnimation* out_animation, char* bytes, size_t byteCount)
{
	/* File format
	Header {
	float32 duration
	uint32 joint count
	}
	for each joint {
	uint32 number of scale keys
	float32 scale key times [number of scale keys]
	Vector3 scale key values [number of scale keys]

	uint32 number of rotation keys
	float32 rotate key times [number of rotation keys]
	Quaternion rotation key values [number of rotation keys]

	uint32 number of translation keys
	float32 translate key times [number of translation keys]
	Vector3 translation key values [number of translation keys]
	}
	*/

	BinaryReader b(bytes, byteCount);

	b.readInto(&out_animation->duration, sizeof(out_animation->duration));
	b.readInto(&out_animation->jointCount, sizeof(out_animation->jointCount));

	out_animation->jointAnimations = new SkeletonAnimation::JointAnimation[out_animation->jointCount];

	// Read each joint
	for (unsigned int i=0; i<out_animation->jointCount; i++)
	{
		SkeletonAnimation::JointAnimation joint;

		// Read key count, times, and values for the joint
		// Scale
		b.readInto(&joint.scaleKeyCount, sizeof(joint.scaleKeyCount));
		joint.scaleKeyTimes = new float[joint.scaleKeyCount];
		joint.scaleKeyValues = new Vec3[joint.scaleKeyCount];
		b.readInto(joint.scaleKeyTimes, joint.scaleKeyCount*sizeof(float));
		b.readInto(joint.scaleKeyValues, joint.scaleKeyCount*sizeof(Vec3));

		// Rotation
		b.readInto(&joint.rotateKeyCount, sizeof(joint.rotateKeyCount));
		joint.rotateKeyTimes = new float[joint.rotateKeyCount];
		joint.rotateKeyValues = new Quaternion[joint.rotateKeyCount];
		b.readInto(joint.rotateKeyTimes, joint.rotateKeyCount*sizeof(float));
		b.readInto(joint.rotateKeyValues, joint.rotateKeyCount*sizeof(Quaternion));

		// Translation
		b.readInto(&joint.translateKeyCount, sizeof(joint.translateKeyCount));
		joint.translateKeyTimes = new float[joint.translateKeyCount];
		joint.translateKeyValues = new Vec3[joint.translateKeyCount];
		b.readInto(joint.translateKeyTimes, joint.translateKeyCount*sizeof(float));
		b.readInto(joint.translateKeyValues, joint.translateKeyCount*sizeof(Vec3));

		out_animation->jointAnimations[i] = joint;
	}
}

void destroySkeletonAnimation(SkeletonAnimation *animation)
{
	if (animation->jointCount == 0) {
		return;
	}
	for (unsigned int i=0; i<animation->jointCount; i++)
	{
		SkeletonAnimation::JointAnimation &joint = animation->jointAnimations[i];
		delete[] joint.scaleKeyTimes;
		delete[] joint.scaleKeyValues;
		delete[] joint.rotateKeyTimes;
		delete[] joint.rotateKeyValues;
		delete[] joint.translateKeyTimes;
		delete[] joint.translateKeyValues;
	}
	delete[] animation->jointAnimations;
	SkeletonAnimation zero={};
	*animation = zero;
}

void findAnimationKeys(unsigned int* out_firstKey, unsigned int* out_secondKey, float time, float* arrayOfTimes, unsigned int numberOfTimes)
{
	// TODO: binary search instead of linear
	for (unsigned int i=0; i<numberOfTimes; i++) {
		if (arrayOfTimes[i] > time && i>0) {
			*out_secondKey = i;
			*out_firstKey = i-1;
			return;
		}
	}
	*out_firstKey = *out_secondKey = numberOfTimes-1;
}

void sampleSkeletonAnimation(Transform* out_jointTransformsArray, float time, const SkeletonAnimation& animation)
{
	for (unsigned int i=0; i<animation.jointCount; i++)
	{
		Transform jointTransform = Transform::identity;

		// Rotation
		if (animation.jointAnimations[i].rotateKeyCount > 0)
		{
			// search for the keys we are currently between
			unsigned int firstKey, secondKey;
			findAnimationKeys(&firstKey, &secondKey, time, animation.jointAnimations[i].rotateKeyTimes, animation.jointAnimations[i].rotateKeyCount);

			if (firstKey == secondKey) {
				// Use one key in the animation
				jointTransform.rotation = animation.jointAnimations[i].rotateKeyValues[firstKey];
			}
			else {
				// Interpolate between two keys
				// Get our normalized time between the first and second keys
				float lerpTime = inverseLerp(animation.jointAnimations[i].rotateKeyTimes[firstKey], animation.jointAnimations[i].rotateKeyTimes[secondKey], time);
				// Interpolate between the keys nearest to the current time in the animaiton
				jointTransform.rotation = lerp(animation.jointAnimations[i].rotateKeyValues[firstKey], animation.jointAnimations[i].rotateKeyValues[secondKey], lerpTime);
			}
		}

		// Position
		if (animation.jointAnimations[i].translateKeyCount > 0) {
			unsigned int firstKey, secondKey;
			findAnimationKeys(&firstKey, &secondKey, time, animation.jointAnimations[i].translateKeyTimes, animation.jointAnimations[i].translateKeyCount);

			if (firstKey == secondKey) {
				jointTransform.position = animation.jointAnimations[i].translateKeyValues[firstKey];
			}
			else {
				float lerpTime = inverseLerp(animation.jointAnimations[i].translateKeyTimes[firstKey], animation.jointAnimations[i].translateKeyTimes[secondKey], time);
				jointTransform.position = lerp(animation.jointAnimations[i].translateKeyValues[firstKey], animation.jointAnimations[i].translateKeyValues[secondKey], lerpTime);
			}
		}

		// Scale
		if (animation.jointAnimations[i].scaleKeyCount > 0) {
			unsigned int firstKey, secondKey;
			findAnimationKeys(&firstKey, &secondKey, time, animation.jointAnimations[i].scaleKeyTimes, animation.jointAnimations[i].scaleKeyCount);

			if (firstKey == secondKey) {
				jointTransform.scale = animation.jointAnimations[i].scaleKeyValues[firstKey];
			}
			else {
				float lerpTime = inverseLerp(animation.jointAnimations[i].scaleKeyTimes[firstKey], animation.jointAnimations[i].scaleKeyTimes[secondKey], time);
				jointTransform.scale = lerp(animation.jointAnimations[i].scaleKeyValues[firstKey], animation.jointAnimations[i].scaleKeyValues[secondKey], lerpTime);
			}
		}

		out_jointTransformsArray[i] = jointTransform;
	}
}

void buildDifferenceSkeletonPose(Transform *out_differenceJointPoses, Transform *referenceJointPoses, Transform *targetJointPoses, unsigned int jointCount)
{
	for (unsigned int i=0; i<jointCount; i++)
	{
		Transform t = Transform::identity;
		t.rotation = targetJointPoses[i].rotation * inverse(referenceJointPoses[i].rotation);
		t.position = targetJointPoses[i].position - referenceJointPoses[i].position;
		t.scale = targetJointPoses[i].scale - referenceJointPoses[i].scale;
		out_differenceJointPoses[i] = t;
	}
}

void additiveBlendSkeletonPoses(Transform *out_blendedJointPoses, Transform *baseJointPoses, Transform *addedJointPoses, unsigned int jointCount, float addWeight)
{
	for (unsigned int i=0; i<jointCount; i++)
	{
		Transform t = Transform::identity;
		t.position = baseJointPoses[i].position + addedJointPoses[i].position*addWeight;
		t.rotation = lerp(Quaternion::identity, addedJointPoses[i].rotation, addWeight) * baseJointPoses[i].rotation;
		t.scale = baseJointPoses[i].scale + addedJointPoses[i].scale*addWeight;
		out_blendedJointPoses[i] = t;
	}
}

void lerpBlendSkeletonPoses(Transform *out_blendedJointPoses, Transform *jointPosesA, Transform *jointPosesB, unsigned int jointCount, float t)
{
	for (unsigned int i=0; i<jointCount; i++)
	{
		out_blendedJointPoses[i] = lerp(jointPosesA[i], jointPosesB[i], t);
	}
}

void blendSkeletonAnimation(Transform *out_blendedJointPoses, SkeletonAnimation* animation, float animationTime, float weight, SkeletonAnimation *referencePose, float referencePoseTime)
{
	unsigned int jointCount = animation->jointCount;
	std::vector<Transform> jointTransforms(jointCount);
	sampleSkeletonAnimation(&jointTransforms[0], animationTime, *animation);
	if (referencePose) {
		std::vector<Transform> jointTransformsReferencePose(jointCount);
		sampleSkeletonAnimation(&jointTransformsReferencePose[0], referencePoseTime, *referencePose);
		buildDifferenceSkeletonPose(&jointTransforms[0], &jointTransformsReferencePose[0], &jointTransforms[0], jointCount);
	}
	additiveBlendSkeletonPoses(out_blendedJointPoses, out_blendedJointPoses, &jointTransforms[0], jointCount, weight);
}


void jointPosesToModelSpace(Transform *out_modelSpaceJoints, Transform *jointPoses, Skeleton& skeleton)
{
	// i=1 to skip the root joint
	for (unsigned int i=1; i<skeleton.jointCount; ++i)
	{
		Transform parentTransform = jointPoses[skeleton.joints[i].parentIndex];
		out_modelSpaceJoints[i] = concatenateTransforms(parentTransform, jointPoses[i]);
	}
}

// Version that takes joints in model space
void buildSkinningMatrix(Matrix4x4* out_matrixArray, Transform* modelSpaceJoints, Skeleton& skeleton)
{
	for (unsigned int i=0; i<skeleton.jointCount; i++) {
		out_matrixArray[i] = transformToMatrix4x4(modelSpaceJoints[i]) * skeleton.joints[i].modelSpaceBindPoseInverse;
	}
}

} // namespace
#endif // header include guard