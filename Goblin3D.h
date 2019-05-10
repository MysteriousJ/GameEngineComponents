#ifndef GOBLIN3D_HEADER_INCLUDED
#define GOBLIN3D_HEADER_INCLUDED

// ==================== Settings ================================
#define GOBLIN_ENABLE_GL
//#define GOBLIN_ENABLE_D3D
#define GOBLIN_ENABLE_DEBUG_LOGGING
//#define VISUALSTUDIO
// ==================== End of settings =========================

#ifdef GOBLIN_ENABLE_GL
	#ifdef WIN32
		#include <Windows.h>
		#include "glad/glad_wgl.h"
	#endif
	#include "glad/glad.h"
	#include <GL/glu.h>
#endif
#ifdef GOBLIN_ENABLE_D3D
	#include <d3d11.h>
	#include <d3dcompiler.h>
	#include <d3d11shader.h>
#endif

#include "Algebra.h"
#include <string.h>
#include <string>

namespace goblin {

static const int maxShaderVariableNameLength = 64;

void goblinDebugLog(const char* message);
void goblinDebugLog(const char* fileName, const char* functionName, int lineNumber, GLenum errorCode);

class BinaryReader
{
public:
	BinaryReader(char* data, size_t byteCount, size_t startingOffset=0);
	// Copy to destination variable, and advance read position
	void readInto(void* destination, size_t numberOfBytes);
	// Returns a pointer to the data at the current read position, and advaces the read position
	void* get(size_t numberOfBytes);
	bool atEnd();
private:
	char* _bytes;
	size_t _byteCount;
	size_t _readPosition;
};

enum CullingMode {
	CullingMode_unchanged,
	CullingMode_none,
	CullingMode_frontFace,
	CullingMode_backFace
};

enum PolygonMode {
	PolygonMode_unchanged,
	PolygonMode_fill,
	PolygonMode_wireframe,
	PolygonMode_point
};

enum DepthTestMode {
	DepthTestMode_unchanged,
	DepthTestMode_none,
	DepthTestMode_neverPass,
	DepthTestMode_alwaysPass,
	DepthTestMode_less,
	DepthTestMode_lessOrEqual,
	DepthTestMode_greater,
	DepthTestMode_greaterOrEqual,
	DepthTestMode_equal,
	DepthTestMode_notEqual
};

#ifdef GOBLIN_ENABLE_GL
// Write-only struct for examining OpenGL state when debugging
struct OpenGLState
{
	enum Setting {unchanged, enabled, disabled};

	GLuint boundVertexArrayObject;
	GLuint boundShaderProgram;
	Setting depthTest;
	Setting framebufferSRGB;
	Setting blend;
	Setting stencil;
	CullingMode cullingMode;
	PolygonMode polygonMode;
	DepthTestMode depthTestMode;
};
#endif

struct RenderState
{
	enum Backend {
		backend_GL,
		backend_VK,
		backend_D3D
	};

	unsigned int boundMeshTriangleCount;
	Backend backend;

	#ifdef GOBLIN_ENABLE_GL
		OpenGLState glState;
		GLuint boundShader;
		GLenum boundMeshIndexBufferType;
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		ID3D11Device* device;
		ID3D11DeviceContext* deviceContext;
		IDXGISwapChain* swapChain;
		ID3D11RenderTargetView* screenRenderTarget;
		ID3D11RasterizerState* rasterizerObject;
		D3D11_RASTERIZER_DESC rasterizer;
	#endif
};

#ifdef GOBLIN_ENABLE_GL
void createRenderStateGL(RenderState* rs);
#endif
#ifdef GOBLIN_ENABLE_D3D
bool createRenderStateD3D(RenderState* rs, ID3D11Device* device, ID3D11DeviceContext* deviceContext, IDXGISwapChain* swapChain);
#endif

void render(RenderState* rs);
void renderRange(RenderState* rs, unsigned int firstTriangleIndex, unsigned int trianglesToRender);
void renderInstanced(RenderState* rs, int instances);
void waitForCompletion(RenderState* rs);

void setPolygonMode(RenderState* rs, PolygonMode mode);
void setCullingMode(RenderState* rs, CullingMode mode);
void setDepthTestMode(RenderState* rs, DepthTestMode mode);

struct VertexDataType
{
	enum Format {
		positions_2floats,
		positions_3floats,
		uvs_2floats,
		normals_3floats,
		tangents_4floats,
		colors_4ubytes,
		jointIndices_4ints,
		jointWeights_4floats
	};

	char nameInShader[maxShaderVariableNameLength];
	Format type;
};

struct VertexLayout
{
	VertexDataType* dataTypes;
	unsigned int dataTypeCount;
};

void createVertexLayout(VertexLayout* out_layout, VertexDataType* dataTypes, unsigned int dataTypeCount);
void createBasicVertexLayout(VertexLayout* out_layout);

struct IndexedTriangle
{
	unsigned int vertexIndex[3];
};

struct Mesh
{
	unsigned int triangleCount;
	unsigned int vertexBufferCount;

	#ifdef GOBLIN_ENABLE_GL
		GLuint glVertexArrayObjectHandle;
		GLuint glIndexBufferHandle;
		GLuint* glVertexBufferHandles;
		GLenum glIndexBufferType;
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		ID3D11Buffer* d3dIndexBuffer;
		ID3D11Buffer** d3dVertexBuffers;
		unsigned int* d3dVertexBufferStrides;
		unsigned int* d3dVertexBufferOffsets;
	#endif
};

void createMesh(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int faceCount, unsigned int vertexCount, unsigned short *faces, void* interleavedVertexData);
void createMesh(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int faceCount, unsigned int vertexCount, IndexedTriangle *faces, Vec3* positions, Vec2* uvs=0, Vec3* normals=0, Vec4 *tangents=0, unsigned int* boneIndices=0, float* boneWeights=0);
void fillVertexTangentArray(Vec4 *out_tangents, unsigned int faceCount, unsigned int vertexCount, IndexedTriangle *faces, Vec3 *positions, Vec2 *uvs, Vec3 *normals);
void createMeshPrimativeCube(RenderState* rs, Mesh* out_mesh, VertexLayout layout);
void createMeshPrimativeCylinder(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int sides, bool capEnds);
void createMeshPrimativeCone(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int sides, bool capEnd);
bool createMeshFromGOBMESH(RenderState* rs, Mesh* out_mesh, VertexLayout layout, char* bytes, size_t byteCount);
void destroyMesh(Mesh* mesh);
void bindMesh(RenderState* rs, Mesh& mesh);

struct UVSphere
{
	unsigned int triangleCount;
	unsigned int vertexCount;
	IndexedTriangle* triangles;
	Vec3* vertexPositions;
	Vec2* vertexUVs;
	Vec3* vertexNormals;
};
void createUVSphere(UVSphere* out_uvSphere, unsigned int segments, unsigned int rings, bool generateUVs, bool generateNormals);
void createMeshPrimativeUVSphere(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int segments, unsigned int rings);

struct ShaderProgram
{
	#ifdef GOBLIN_ENABLE_GL
		GLuint glProgram;
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		ID3DBlob* vertexShaderBlob;
		ID3DBlob* pixelShaderBlob;
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3D11InputLayout* inputLayout;
		ID3DBlob* errorMessages;
	#endif
};

bool createShaderProgram(RenderState* rs, ShaderProgram* out_shader, VertexLayout layout, const char vertexShaderData[], int vertexShaderByteCount, const char fragmentShaderData[], int fragmentShaderByteCount);
void createBasicShaderProgram(RenderState* rs, ShaderProgram* out_shader);
std::string getShaderProgramErrors(RenderState* rs, ShaderProgram& shader);
void bindShaderProgram(RenderState* rs, ShaderProgram shader);

struct UniformBuffer
{
	unsigned int byteCount;

	#ifdef GOBLIN_ENABLE_GL
		GLuint glUniformBuffer;
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		ID3D11Buffer* d3dConstantBuffer;
	#endif
};

void createUniformBuffer(RenderState* rs, UniformBuffer* out_uniforms, unsigned int byteCount);
void bindUniformBuffer(RenderState* rs, UniformBuffer* mod_uniforms, const char* nameInShader, unsigned int bindLocation, void* data);

struct Texture {
	enum Format {
		rgb8,
		srgb8,
		rgba8,
		srgba8,
		depth,
		depthStencil
	};

	unsigned int width;
	unsigned int height;
	Format format;

	#ifdef GOBLIN_ENABLE_GL
		GLuint textureHandle;
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		ID3D11ShaderResourceView* resource;
		ID3D11SamplerState* samplerState;
	#endif
};

void createTexture(RenderState* rs, Texture* out_texture, unsigned char* pixels, unsigned int width, unsigned int height, Texture::Format pixelFormat, bool shrinkSmooth, bool enlargeSmooth, bool generateMipmaps);
void destroyTexture(Texture* texture);
void bindTextures(RenderState* rs, const char* nameInShader, Texture textures[], unsigned int textureCount);

struct FrameBuffer
{
	unsigned int width;
	unsigned int height;

	#ifdef GOBLIN_ENABLE_GL
		GLuint frameBufferHandle;
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		ID3D11RenderTargetView* renderTargetView;
	#endif
};

void createFrameBuffer(RenderState* rs, FrameBuffer* out_fb, Texture rgbaTextures[], unsigned int rgbaTextureCount, Texture* depthStencilTexture);
// Future idea: version of framebuffer that lets you specify the width and weight as parameters.
bool getScreenFrameBuffer(RenderState* rs, FrameBuffer* out_fb);
void createScreenFrameBuffer(RenderState* rs, FrameBuffer* out_fb, unsigned int width, unsigned int height);
void destroyFrameBuffer(FrameBuffer* fb);
void clearFrameBuffer(RenderState* rs, FrameBuffer* out_fb, Vec4 clearColor);
void resizeFrameBuffer(RenderState* rs, FrameBuffer* out_fb, unsigned int width, unsigned int height);
void bindFrameBuffer(RenderState* rs, FrameBuffer frameBuffer);

} // namespace


/* Implementation */
#include <vector>
#include <assert.h>
#include <sstream>
#include <math.h>

namespace goblin {
#ifdef GOBLIN_ENABLE_GL
	#define GOBLIN_PRINT_GL_ERRORS \
	{\
		GLenum error = glGetError();\
		if (error){goblinDebugLog(__FILE__, __FUNCTION__, __LINE__, error);}\
	}

	#if defined(GOBLIN_ENABLE_D3D) || defined(GOBLIN_ENABLE_VK)
		#define GOBLIN_BEGIN_GL if(rs->backend == RenderState::backend_GL) {
	#else
		#define GOBLIN_BEGIN_GL {
	#endif
	#ifdef GOBLIN_ENABLE_DEBUG_LOGGING
		#define GOBLIN_END_GL \
			{\
				GLenum error = glGetError();\
				if (error){goblinDebugLog(__FILE__, __FUNCTION__, __LINE__, error);}\
			} }
	#else 
		#define GOBLIN_END_GL }
	#endif
	#else
		#define GOBLIN_GL(code)
#endif

#ifdef GOBLIN_ENABLE_D3D
#if defined(GOBLIN_ENABLE_GL) || defined(GOBLIN_ENABLE_VK)
#define GOBLIN_BEGIN_D3D if(rs->backend == RenderState::backend_D3D) {
#define GOBLIN_END_D3D }
#define GOBLIN_D3D(...) if(rs->backend == RenderState::backend_D3D) {__VA_ARGS__}
#else
#define GOBLIN_BEGIN_D3D {
#define GOBLIN_END_D3D }
#define GOBLIN_D3D(...) {__VA_ARGS__}
#endif
#else
#define GOBLIN_D3D(code)
#endif

void goblinDebugLog(const char* message)
{
	#ifdef VISUALSTUDIO
		OutputDebugStringA(message);
	#else
		printf("%s\n", message);
	#endif
}

#ifdef GOBLIN_ENABLE_GL
void goblinDebugLog(const char* fileName, const char* functionName, int lineNumber, GLenum errorCode)
{
	std::stringstream message;
	message << "OpenGL Error in function " << functionName << " in file " << fileName << " at line " << lineNumber << ":\n" << gluErrorString(errorCode) << "\n";
	goblinDebugLog(message.str().c_str());
}
#endif

BinaryReader::BinaryReader(char* data, size_t byteCount, size_t startingOffset)
	: _bytes(data)
	, _byteCount(byteCount)
	, _readPosition(startingOffset)
{}
void BinaryReader::readInto(void* readInto, size_t numberOfBytes)
{
	if (_readPosition+numberOfBytes <= _byteCount) {
		memcpy(readInto, _bytes + _readPosition, numberOfBytes);
	}
	_readPosition += numberOfBytes;
}

void* BinaryReader::get(size_t numberOfBytes)
{
	void* address = _bytes+_readPosition;
	_readPosition += numberOfBytes;
	if (_readPosition > _byteCount) {
		return 0;
	}
	return address;
}
bool BinaryReader::atEnd()
{
	return (_readPosition==_byteCount);
}


#ifdef GOBLIN_ENABLE_GL
void createRenderStateGL(RenderState* rs)
{
	*rs ={0};
	rs->backend = RenderState::backend_GL;

	glEnable(GL_DEPTH_TEST);
	rs->glState.depthTest = OpenGLState::enabled;
	glEnable(GL_FRAMEBUFFER_SRGB);
	rs->glState.framebufferSRGB = OpenGLState::enabled;
	glEnable(GL_BLEND);
	rs->glState.blend = OpenGLState::enabled;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// TODO: record state for blend func
}
#endif

#ifdef GOBLIN_ENABLE_D3D
bool createRenderStateD3D(RenderState *rs, ID3D11Device* device, ID3D11DeviceContext* deviceContext, IDXGISwapChain* swapChain)
{
	*rs ={0};
	rs->backend = RenderState::backend_D3D;
	rs->device = device;
	rs->deviceContext = deviceContext;
	rs->swapChain = swapChain;


	// Create the rasterizer state
	rs->rasterizer.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rs->rasterizer.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rs->rasterizer.FrontCounterClockwise = true;
	rs->rasterizer.DepthBias = 0;
	rs->rasterizer.SlopeScaledDepthBias = 0;
	rs->rasterizer.DepthBiasClamp = 0;
	rs->rasterizer.DepthClipEnable = true;
	rs->rasterizer.ScissorEnable = false;
	rs->rasterizer.MultisampleEnable = false;
	rs->rasterizer.AntialiasedLineEnable = false;

	rs->device->CreateRasterizerState(&rs->rasterizer, &rs->rasterizerObject);
	rs->deviceContext->RSSetState(rs->rasterizerObject);

	return true;
}
#endif // GOBLIN_ENABLE_D3D

void destroyRenderState(RenderState* rs)
{
	// Nothing to do for GL so far
	// TODO: D3D
}

void render(RenderState* rs)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glDrawElements(GL_TRIANGLES, rs->boundMeshTriangleCount*3, rs->boundMeshIndexBufferType, 0);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		rs->deviceContext->DrawIndexed(3*rs->boundMeshTriangleCount, 0, 0);
	}GOBLIN_END_D3D
#endif
}

void renderRange(RenderState* rs, unsigned int firstTriangleIndex, unsigned int trianglesToRender)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		unsigned int bytesPerIndex = (rs->boundMeshIndexBufferType==GL_UNSIGNED_SHORT) ? 2 : 4;
		unsigned int firstElementByteOffset = 3 * firstTriangleIndex * bytesPerIndex;
		glDrawElements(GL_TRIANGLES, trianglesToRender*3, rs->boundMeshIndexBufferType, (void*)firstElementByteOffset);
	}GOBLIN_END_GL
#endif
}

void renderInstanced(RenderState* rs, int instances)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glDrawElementsInstanced(GL_TRIANGLES, rs->boundMeshTriangleCount*3, GL_UNSIGNED_INT, 0, instances);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		rs->deviceContext->DrawIndexed(3*rs->boundMeshTriangleCount, 0, 0);
	}GOBLIN_END_D3D
#endif
}

void waitForCompletion(RenderState* rs)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glFinish();
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		//TODO
	}GOBLIN_END_D3D
#endif
}

void setPolygonMode(RenderState* rs, PolygonMode mode)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		assert(mode != PolygonMode_unchanged);
		switch (mode) {
			case PolygonMode_fill: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
			case PolygonMode_wireframe: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
			case PolygonMode_point: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
		}
		rs->glState.polygonMode = mode;
	}GOBLIN_END_GL
#endif
}

void setCullingMode(RenderState* rs, CullingMode mode)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		assert(mode != CullingMode_unchanged);
		switch (mode) {
			case CullingMode_frontFace: glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); break;
			case CullingMode_backFace: glEnable(GL_CULL_FACE); glCullFace(GL_BACK); break;
			case CullingMode_none: glDisable(GL_CULL_FACE); break;
		}
		rs->glState.cullingMode = mode;
	}GOBLIN_END_GL
#endif
}

void setDepthTestMode(RenderState* rs, DepthTestMode mode)
{
#ifdef GOBLIN_ENABLE_GL
	assert(mode != DepthTestMode_unchanged);
	GOBLIN_BEGIN_GL{
		switch (mode) {
			case DepthTestMode_alwaysPass: glDepthFunc(GL_ALWAYS); break;
			case DepthTestMode_neverPass: glDepthFunc(GL_NEVER); break;
			case DepthTestMode_less: glDepthFunc(GL_LESS); break;
			case DepthTestMode_lessOrEqual: glDepthFunc(GL_LEQUAL); break;
			case DepthTestMode_greater: glDepthFunc(GL_GREATER); break;
			case DepthTestMode_greaterOrEqual: glDepthFunc(GL_GEQUAL); break;
			case DepthTestMode_equal: glDepthFunc(GL_EQUAL); break;
			case DepthTestMode_notEqual: glDepthFunc(GL_NOTEQUAL); break;
		}
		(mode == DepthTestMode_none) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);
		rs->glState.depthTestMode = mode;
	}GOBLIN_END_GL
#endif
}

void createVertexLayout(VertexLayout* out_layout, VertexDataType* dataTypes, unsigned int dataTypeCount)
{
	out_layout->dataTypeCount = dataTypeCount;
	out_layout->dataTypes = new VertexDataType[dataTypeCount];
	for (unsigned int i=0; i<dataTypeCount; ++i) {
		out_layout->dataTypes[i] = dataTypes[i];
	}
}

void createBasicVertexLayout(VertexLayout* out_layout)
{
	VertexDataType dataTypes[] ={
		{"vertexPositions", VertexDataType::positions_3floats},
		{"vertexUVs", VertexDataType::uvs_2floats},
		{"vertexNormals", VertexDataType::normals_3floats},
		{"vertexTangents", VertexDataType::tangents_4floats},
		{"vertexJointIndices", VertexDataType::jointIndices_4ints},
		{"vertexJointWeights", VertexDataType::jointWeights_4floats}
	};
	unsigned int dataTypeCount = sizeof(dataTypes)/sizeof(VertexDataType);
	createVertexLayout(out_layout, dataTypes, dataTypeCount);
}

void destroyVertexLayout(VertexLayout* layout)
{
	delete[] layout->dataTypes;
	*layout = {0};
}

void createMesh(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int faceCount, unsigned int vertexCount, unsigned short *faces, void* interleavedVertexData)
{
	*out_mesh ={0};

	#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		out_mesh->glIndexBufferType = GL_UNSIGNED_SHORT;
		// Vertex array object
		glGenVertexArrays(1, &out_mesh->glVertexArrayObjectHandle);
		glBindVertexArray(out_mesh->glVertexArrayObjectHandle);

		unsigned int totalVertexSize = 0;
		for (unsigned int i=0; i<layout.dataTypeCount; ++i)
		{
			switch (layout.dataTypes[i].type)
			{
				case VertexDataType::positions_2floats:	totalVertexSize += 2*sizeof(float); break;
				case VertexDataType::positions_3floats:	totalVertexSize += 3*sizeof(float); break;
				case VertexDataType::normals_3floats: totalVertexSize += 3*sizeof(float); break;
				case VertexDataType::tangents_4floats: totalVertexSize += 4*sizeof(float); break;
				case VertexDataType::uvs_2floats: totalVertexSize += 2*sizeof(float); break;
				case VertexDataType::colors_4ubytes: totalVertexSize += 4*sizeof(char); break;
				case VertexDataType::jointIndices_4ints: totalVertexSize += 4*sizeof(int); break;
				case VertexDataType::jointWeights_4floats: totalVertexSize += 4*sizeof(float); break;
				default: assert(!"Missing a case");
			}
		}
		GOBLIN_PRINT_GL_ERRORS;

		out_mesh->glVertexBufferHandles = new GLuint;
		out_mesh->vertexBufferCount = 1;
		glGenBuffers(1, out_mesh->glVertexBufferHandles);
		glBindBuffer(GL_ARRAY_BUFFER, *out_mesh->glVertexBufferHandles);
		glBufferData(GL_ARRAY_BUFFER, vertexCount*totalVertexSize, interleavedVertexData, GL_STATIC_DRAW);
		GOBLIN_PRINT_GL_ERRORS;
		unsigned int offset = 0;

		for (unsigned int i=0; i<layout.dataTypeCount; ++i)
		{
			switch (layout.dataTypes[i].type)
			{
				case VertexDataType::positions_2floats:
					glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 2*sizeof(float);
					break;
				case VertexDataType::positions_3floats:
					glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 3*sizeof(float);
					break;
				case VertexDataType::normals_3floats:
					glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 3*sizeof(float);
					break;
				case VertexDataType::tangents_4floats:
					glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 4*sizeof(float);
					break;
				case VertexDataType::uvs_2floats:
					glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 2*sizeof(float);
					break;
				case VertexDataType::colors_4ubytes:
					glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 4*sizeof(char);
					break;
				case VertexDataType::jointIndices_4ints:
					glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 4*sizeof(int);
					break;
				case VertexDataType::jointWeights_4floats:
					glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, totalVertexSize, (void*)offset);
					glEnableVertexAttribArray(i);
					offset += 4*sizeof(float);
					break;
				default: assert(!"Missing a case");
			}
		}
		GOBLIN_PRINT_GL_ERRORS;
		
		// Index buffer
		out_mesh->triangleCount = faceCount;
		glGenBuffers(1, &out_mesh->glIndexBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh->glIndexBufferHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount*3*sizeof(unsigned short), faces, GL_STATIC_DRAW);

		// TODO: Rebind the previously bound mesh

	}GOBLIN_END_GL
	#endif
}

void createMesh(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int faceCount, unsigned int vertexCount, IndexedTriangle *faces, Vec3* positions, Vec2* uvs, Vec3* normals, Vec4 *tangents, unsigned int* boneIndices, float* boneWeights)
{
	*out_mesh ={0};
	out_mesh->vertexBufferCount = layout.dataTypeCount;
	out_mesh->triangleCount = faceCount;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		out_mesh->glIndexBufferType = GL_UNSIGNED_INT;
		out_mesh->glVertexBufferHandles = new GLuint[layout.dataTypeCount];
		// Vertex array object
		glGenVertexArrays(1, &out_mesh->glVertexArrayObjectHandle);
		glBindVertexArray(out_mesh->glVertexArrayObjectHandle);

		for (unsigned int i=0; i<layout.dataTypeCount; ++i)
		{
			GLuint bufferHandle = 0;
			glGenBuffers(1, &bufferHandle);
			glBindBuffer(GL_ARRAY_BUFFER, bufferHandle);
			switch (layout.dataTypes[i].type)
			{
				case VertexDataType::positions_3floats:
					glBufferData(GL_ARRAY_BUFFER, vertexCount*3*sizeof(float), positions, GL_STATIC_DRAW);
					glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, 0);
					glEnableVertexAttribArray(i);
					break;
				case VertexDataType::normals_3floats:
					glBufferData(GL_ARRAY_BUFFER, vertexCount*3*sizeof(float), normals, GL_STATIC_DRAW);
					glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, 0);
					glEnableVertexAttribArray(i);
					break;
				case VertexDataType::tangents_4floats:
					glBufferData(GL_ARRAY_BUFFER, vertexCount*4*sizeof(float), tangents, GL_STATIC_DRAW);
					glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, 0, 0);
					glEnableVertexAttribArray(i);
					break;
				case VertexDataType::uvs_2floats:
					glBufferData(GL_ARRAY_BUFFER, vertexCount*2*sizeof(float), uvs, GL_STATIC_DRAW);
					glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, 0, 0);
					glEnableVertexAttribArray(i);
					break;
				case VertexDataType::jointIndices_4ints:
					glBufferData(GL_ARRAY_BUFFER, vertexCount*4*sizeof(unsigned int), boneIndices, GL_STATIC_DRAW);
					glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, 0, 0);
					glEnableVertexAttribArray(i);
					break;
				case VertexDataType::jointWeights_4floats:
					glBufferData(GL_ARRAY_BUFFER, vertexCount*4*sizeof(float), boneWeights, GL_STATIC_DRAW);
					glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, 0, 0);
					glEnableVertexAttribArray(i);
					break;
			}
			out_mesh->glVertexBufferHandles[i] = bufferHandle;
		}

		// Index buffer
		glGenBuffers(1, &out_mesh->glIndexBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh->glIndexBufferHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount*3*sizeof(unsigned int), faces, GL_STATIC_DRAW);

		// TODO: Rebind the previously bound mesh

	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		out_mesh->d3dVertexBuffers = new ID3D11Buffer*[layout.dataTypeCount];
		out_mesh->d3dVertexBufferStrides = new unsigned int[layout.dataTypeCount];
		out_mesh->d3dVertexBufferOffsets = new unsigned int[layout.dataTypeCount];

		D3D11_BUFFER_DESC bd ={};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData ={};

		// Create vertex buffers
		for (unsigned int i=0; i<layout.dataTypeCount; ++i)
		{
			unsigned int bytesPerVertex = 0;
			switch (layout.dataTypes[i].type)
			{
				case VertexDataType::positions_3floats:
					bytesPerVertex = sizeof(Vec3);
					InitData.pSysMem = positions;
					break;
				case VertexDataType::uvs_2floats:
					bytesPerVertex = sizeof(Vec2);
					InitData.pSysMem = uvs;
					break;
				default:
					bytesPerVertex = 0;
					InitData.pSysMem = 0;
			}

			if (InitData.pSysMem) {
				bd.ByteWidth = bytesPerVertex*vertexCount;
				HRESULT hr = rs->device->CreateBuffer(&bd, &InitData, &out_mesh->d3dVertexBuffers[i]);
				assert(!FAILED(hr));
			}
			else {
				out_mesh->d3dVertexBuffers[i] = 0;
			}
			out_mesh->d3dVertexBufferStrides[i] = bytesPerVertex;
			out_mesh->d3dVertexBufferOffsets[i] = 0;
		}

		// Create index buffers
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(IndexedTriangle)*faceCount;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		InitData.pSysMem = faces;
		HRESULT hr = rs->device->CreateBuffer(&bd, &InitData, &out_mesh->d3dIndexBuffer);
		assert(!FAILED(hr));
	}GOBLIN_END_D3D
#endif
}

/* Calculates the tangents needed for tangent-space normal mapping.
The corresponding bitangent is derived using btan = tan.w*cross(normal, tan).
The w component of the tangent is either 1 or -1, and is used to correct the direction of the bitangent. */
void fillVertexTangentArray(Vec4 *out_tangents, unsigned int faceCount, unsigned int vertexCount, IndexedTriangle *faces, Vec3 *positions, Vec2 *uvs, Vec3 *normals)
{
	Vec3 zero ={0};
	std::vector<Vec3> bitangents(vertexCount, zero);
	// Initialize tangents and bitangents to 0.
	memset(out_tangents, 0, vertexCount*sizeof(*out_tangents));
	// From terathon.com/code/tangent.html
	for (unsigned int i=0; i<faceCount; i++)
	{
		// Shortcuts for triangle indeces
		unsigned int a = faces[i].vertexIndex[0];
		unsigned int b = faces[i].vertexIndex[1];
		unsigned int c = faces[i].vertexIndex[2];

		// Check if the loaded file's data is corrupt
		assert(a < vertexCount
			&& b < vertexCount
			&& c < vertexCount);

		Vec3 deltaPos1 = positions[b] - positions[a];
		Vec3 deltaPos2 = positions[c] - positions[a];
		Vec2 deltaUV1 = uvs[b] - uvs[a];
		Vec2 deltaUV2 = uvs[c] - uvs[a];

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		Vec3 Tangent = (deltaPos1*deltaUV2.y - deltaPos2*deltaUV1.y)*f;
		Vec3 Bitangent = (deltaPos2*deltaUV1.x - deltaPos1*deltaUV2.x)*f;
		out_tangents[a].xyz += Tangent;
		out_tangents[b].xyz += Tangent;
		out_tangents[c].xyz += Tangent;
		bitangents[a] += Bitangent;
		bitangents[b] += Bitangent;
		bitangents[c] += Bitangent;
	}
	for (unsigned int i=0; i<vertexCount; i++) {
		// Gram-Schmidt orthogonalize and normalize each tangent
		out_tangents[i].xyz = normalize(out_tangents[i].xyz - normals[i] * dot(normals[i], out_tangents[i].xyz));
		// Set the w for handedness of bitangent in shader
		out_tangents[i].w = 1;
		if (dot(cross(normals[i], out_tangents[i].xyz), bitangents[i]) < 0.0f) {
			out_tangents[i].w = -1;
		}
	}
}

void createMeshPrimativeCube(RenderState* rs, Mesh* out_mesh, VertexLayout layout)
{
	Vec3 topLeftBack ={-1, 1, -1};
	Vec3 topLeftFront ={-1, 1, 1};
	Vec3 topRightBack ={1, 1, -1};
	Vec3 topRightFront ={1, 1, 1};
	Vec3 bottomLeftBack ={-1, -1, -1};
	Vec3 bottomLeftFront ={-1, -1, 1};
	Vec3 bottomRightBack ={1, -1, -1};
	Vec3 bottomRightFront ={1, -1, 1};

	Vec3 unitCubeVerteces[] ={
		topLeftBack, topLeftFront, topRightFront, topRightBack, // Top
		bottomLeftFront, bottomLeftBack, bottomRightBack, bottomRightFront, // Bottom
		topRightFront, bottomRightFront, bottomRightBack, topRightBack, // Right
		topLeftBack, bottomLeftBack, bottomLeftFront, topLeftFront, // Left
		topLeftFront, bottomLeftFront, bottomRightFront, topRightFront, // Front
		topRightBack, bottomRightBack, bottomLeftBack, topLeftBack // Back
	};
	Vec2 unitCubeUVs[] ={
		{0, 1}, {0, 0}, {1, 0}, {1, 1}, // Top
		{0, 1}, {0, 0}, {1, 0}, {1, 1}, // Bottom
		{0, 1}, {0, 0}, {1, 0}, {1, 1}, // Right
		{0, 1}, {0, 0}, {1, 0}, {1, 1}, // Left
		{0, 1}, {0, 0}, {1, 0}, {1, 1}, // Front
		{0, 1}, {0, 0}, {1, 0}, {1, 1} // Back
	};
	Vec3 unitCubeNormals[] ={
		{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, // Top
		{0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, // Bottom
		{1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, // Right
		{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, // Left
		{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, // Front
		{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1} // Back
	};
	IndexedTriangle unitCubeFaces[] ={
		{0, 1, 3}, {1, 2, 3}, // Top
		{4, 5, 7}, {5, 6, 7}, // Bottom
		{8, 9, 11}, {9, 10, 11}, // Right
		{12, 13, 15}, {13, 14, 15}, // Left
		{16, 17, 19}, {17, 18, 19}, // Front
		{20, 21, 23}, {21, 22, 23}  // Back
	};
	createMesh(rs, out_mesh, layout, 6*2, 4*6, unitCubeFaces, unitCubeVerteces, unitCubeUVs, unitCubeNormals);
}

// Segments and rings must be at least 3
// Segments are the verticle slices of the sphere
// Rings are the horizontal layers
void createUVSphere(UVSphere* out_uvSphere, unsigned int segments, unsigned int rings, bool generateUVs, bool generateNormals)
{
	assert(segments >= 3 && rings >= 3);

	unsigned int rows = rings-1; // The number of horizontal rows of vertices
	unsigned int vertexCount = segments*rows + 2; // Plus two for the top and bottom verteces
	Vec3 *verteces = new Vec3[vertexCount];

	// The verteces at the very top and bottom are stored at the end of the vertex array
	unsigned int topVertexIndex = vertexCount - 1;
	Vec3 topVertex ={0, 1, 0};
	verteces[topVertexIndex] = topVertex;

	unsigned int bottomVertexIndex = vertexCount - 2;
	Vec3 bottomVertex ={0, -1, 0};
	verteces[bottomVertexIndex] = bottomVertex;

	/* The verteces for the body of the sphere are constructed as a grid,
	but stored in a single-dimensional array.
	They are built in columns, from bottom to top.*/
	unsigned int vertexIndex = 0;
	for (unsigned int s=0; s<segments; s++)
	{
		float segmentX = sinf(s*pi*2.0f/segments);
		float segmentZ = cosf(s*pi*2.0f/segments);
		for (unsigned int r=1; r<=rows; r++)
		{
			Vec3 v;
			v.y = sinf(r*pi/rings - pi/2);
			float scaleForRingRadius = cosf(r*pi/rings - pi/2);
			v.x = segmentX*scaleForRingRadius;
			v.z = segmentZ*scaleForRingRadius;
			verteces[vertexIndex++] = v;
		}
	}
	assert(vertexIndex == vertexCount-2);

	// Connect the verteces into triangle, one segment at a time
	unsigned int triangleCount = segments*(rows)*2;
	IndexedTriangle *triangles = new IndexedTriangle[triangleCount];
	unsigned int triangleIndex = 0;
	for (unsigned int s=0; s<segments; s++)
	{
		// The columns on the left and right side of the segment
		unsigned int leftSegmentFirstRing = (s*rows);
		// The grid is like a cylinder, so the right side of the last segment need to wrap around to the beginning.
		unsigned int rightSegmentFirstRing = ((s+1)*rows)%(segments*rows);

		// The triangle at the bottom of the segment that connects to the center vertex
		IndexedTriangle bottomTriangle ={bottomVertexIndex, rightSegmentFirstRing, leftSegmentFirstRing};
		triangles[triangleIndex++] = bottomTriangle;

		for (unsigned int r=0; r<rows-1; r++)
		{
			unsigned int bottomLeftVertex = leftSegmentFirstRing + r;
			unsigned int bottomRightVertex = rightSegmentFirstRing + r;
			unsigned int topLeftVertex = leftSegmentFirstRing + r + 1;
			unsigned int topRightVertex = rightSegmentFirstRing + r + 1;

			IndexedTriangle bottomRightTriangle ={bottomLeftVertex, bottomRightVertex, topRightVertex};
			triangles[triangleIndex++] = bottomRightTriangle;

			IndexedTriangle topLeftTriangle ={bottomLeftVertex, topRightVertex, topLeftVertex};
			triangles[triangleIndex++] = topLeftTriangle;
		}

		// The triangle at the top of the segment that connects to the center vertex
		IndexedTriangle topTriangle ={topVertexIndex, leftSegmentFirstRing+rows-1, rightSegmentFirstRing+rows-1};
		triangles[triangleIndex++] = topTriangle;
	}
	assert(triangleIndex == triangleCount);

	Vec2* uvs = 0;
	if (generateUVs)
	{
		uvs = new Vec2[vertexCount];
		Vec2 top ={float(segments)/2, 1};
		Vec2 bottom ={float(segments)/2, 0};
		uvs[topVertexIndex] = top;
		uvs[bottomVertexIndex] = bottom;

		for (unsigned int s=0; s<segments; s++) {
			for (unsigned int r=0; r<=rows; r++) {
				Vec2 uvCoord;
				uvCoord.x = fabs(float(s*2)-float(segments))/float(segments);
				uvCoord.y = float(r+1)/float(rows+2);
				uvs[s*rows+r] = uvCoord;
			}
		}
	}

	// Optional vertex normals
	Vec3 *normals = 0;
	if (generateNormals)
	{
		normals = new Vec3[vertexCount];
		// Initialize normal values to 0
		Vec3 zero ={};
		for (unsigned int i=0; i<vertexCount; i++) {
			normals[i] = zero;
		}

		// For each vertex, add up the surface normals of each tirangle it's connected to.
		for (unsigned int i=0; i<triangleCount; i++)
		{
			// Indeces of the verteces in the triangle
			unsigned int vA = triangles[i].vertexIndex[0];
			unsigned int vB = triangles[i].vertexIndex[1];
			unsigned int vC = triangles[i].vertexIndex[2];
			// Calculate the triangle's surface normal
			Vec3 edge1 = verteces[vB] - verteces[vA];
			Vec3 edge2 = verteces[vC] - verteces[vB];
			Vec3 triangleNormal = cross(edge1, edge2);

			normals[vA] += triangleNormal;
			normals[vB] += triangleNormal;
			normals[vC] += triangleNormal;
		}

		// Normalize the normals.
		// We can skip the last two, which are the top and bottom verteces.
		for (unsigned int i=0; i<vertexCount-2; i++) {
			normals[i] = normalize(normals[i]);
		}

		// Normals for the top and bottom of the sphere
		Vec3 topNormal ={0, 1, 0};
		normals[topVertexIndex] = topNormal;
		Vec3 bottomNormal ={0, -1, 0};
		normals[bottomVertexIndex] = bottomNormal;
	}

	out_uvSphere->triangleCount = triangleCount;
	out_uvSphere->triangles = triangles;
	out_uvSphere->vertexCount = vertexCount;
	out_uvSphere->vertexPositions = verteces;
	out_uvSphere->vertexUVs = uvs;
	out_uvSphere->vertexNormals = normals;
}

void destroyUVSphere(UVSphere* uvSphere)
{
	delete[] uvSphere->triangles;
	delete[] uvSphere->vertexPositions;
	delete[] uvSphere->vertexUVs;
	delete[] uvSphere->vertexNormals;
	*uvSphere ={0};
}

void createMeshPrimativeUVSphere(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int segments, unsigned int rings)
{
	bool generateUVs = false;
	bool generateNormals = false;
	for (unsigned int i=0; i<layout.dataTypeCount; ++i) {
		switch (layout.dataTypes[i].type) {
			case VertexDataType::Format::uvs_2floats:
				generateUVs = true;
				break; case VertexDataType::Format::normals_3floats:
					generateNormals = true;
		}
	}
	UVSphere sphere ={0};
	createUVSphere(&sphere, segments, rings, generateUVs, generateNormals);
	createMesh(rs, out_mesh, layout, sphere.triangleCount, sphere.vertexCount, sphere.triangles, sphere.vertexPositions, sphere.vertexUVs, sphere.vertexNormals);
	destroyUVSphere(&sphere);
}

// Cylinder is along y axis
void createMeshPrimativeCylinder(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int sides, bool capEnds)
{
	// Vertex order in array:
	// top ring, bottom ring, 1 top center point, 1 bottom center point
	// Face order in array:
	// all faces on the side, triangle circle on top, triangle circle on bottom
	unsigned int vertexCount = sides*2;
	unsigned int faceCount = sides*2;
	if (capEnds) {
		vertexCount += 2;
		faceCount += sides*2;
	}
	Vec3* vertices = new Vec3[vertexCount];
	IndexedTriangle* faces = new IndexedTriangle[faceCount];

	for (unsigned int i=0; i<sides; ++i) {
		float angle = i*(2*pi/sides);
		// Bottom ring
		vertices[i] ={cosf(angle), -1, sinf(angle)};
		// Top ring
		vertices[sides+i] ={cosf(angle), 1, sinf(angle)};
		// Top left face
		faces[i*2] ={i%sides, (i+1)%sides, (i%sides)+sides};
		// Bottom right face
		faces[i*2+1] ={(i%sides)+sides, (i+1)%sides, (i+1)%sides+sides};
	}

	if (capEnds) {
		unsigned int topVertexIndex = vertexCount-1;
		unsigned int bottomVertexIndex = vertexCount-2;
		vertices[topVertexIndex] ={0, 1, 0};
		vertices[bottomVertexIndex] ={0, -1, 0};
		for (unsigned int i=0; i<sides; ++i) {
			faces[sides*2+i] ={i%sides, (i+1)%sides, bottomVertexIndex};
			faces[sides*3+i] ={i%sides+sides, (i+1)%sides+sides, topVertexIndex};
		}
	}

	createMesh(rs, out_mesh, layout, faceCount, vertexCount, faces, vertices);
	delete[] vertices;
	delete[] faces;
}

// Cone is along y axis with base ring at 0
void createMeshPrimativeCone(RenderState* rs, Mesh* out_mesh, VertexLayout layout, unsigned int sides, bool capEnd)
{
	// Vertex order in array: bottom ring, 1 bottom center point if capping end, top point
	// Face order in array: all faces on the side, triangle circle on bottom
	unsigned int vertexCount = sides + 1;
	unsigned int faceCount = sides;
	if (capEnd) {
		vertexCount += 1;
		faceCount += sides;
	}
	Vec3* vertices = new Vec3[vertexCount];
	IndexedTriangle* faces = new IndexedTriangle[faceCount];

	unsigned int topVertexIndex = vertexCount - 1;
	vertices[topVertexIndex] = {0,1,0};

	for (unsigned int i = 0; i<sides; ++i) {
		float angle = i * (2 * pi / sides);
		// Bottom ring
		vertices[i] = {cosf(angle), 0, sinf(angle)};
		// side face
		faces[i] = {i, topVertexIndex, (i+1)%sides};
	}

	if (capEnd) {
		unsigned int bottomVertexIndex = vertexCount - 2;
		vertices[bottomVertexIndex] = {0, 0, 0};
		for (unsigned int i = 0; i<sides; ++i) {
			faces[sides + i] = {i, (i+1)%sides, bottomVertexIndex};
		}
	}

	createMesh(rs, out_mesh, layout, faceCount, vertexCount, faces, vertices);
	delete[] vertices;
	delete[] faces;
}

bool createMeshFromGOBMESH(RenderState* rs, Mesh* out_mesh, VertexLayout layout, char* bytes, size_t byteCount)
{
	/* .gobmesh File format:
	Header
	{
	uint32 number of faces
	uint32 number of verteces
	bool8 whether the mesh has uvs
	bool8 whether the mesh has normals
	bool8 whether mesh has joint indeces and weights
	}
	faces (3 uints per face)
	vertex positions (3 floats per vertex)
	vertex UVs (2 floats per vertex)
	vertex normals (3 floats per vertex)
	vertex bone indeces (x uint8s per vertex; optional)
	vertex bone weights (x floats per vertex; optional)
	*/

	bool success = true;
	BinaryReader b(bytes, byteCount);
	unsigned int faceCount,
		vertexCount;
	bool hasUVs;
	bool hasNormals;
	bool hasSkeletonBindings;
	static const int jointsPerVertex = 4;

	b.readInto(&faceCount, sizeof(unsigned int));
	b.readInto(&vertexCount, sizeof(unsigned int));

	b.readInto(&hasUVs, sizeof(bool));
	b.readInto(&hasNormals, sizeof(bool));
	b.readInto(&hasSkeletonBindings, sizeof(bool));

	IndexedTriangle* faces = (IndexedTriangle*)b.get(faceCount*sizeof(IndexedTriangle));
	Vec3* positions = (Vec3*)b.get(vertexCount*sizeof(Vec3));
	Vec2* uvs = 0;
	Vec3* normals = 0;
	Vec4* tangents = 0;
	unsigned int* jointIndices = 0;
	float* jointWeights = 0;

	if (hasUVs) {
		uvs = (Vec2*)b.get(vertexCount*sizeof(Vec2));
	}
	if (hasNormals) {
		normals = (Vec3*)b.get(vertexCount*sizeof(Vec3));
		// Tangents
		tangents = new Vec4[vertexCount];
		fillVertexTangentArray(tangents, faceCount, vertexCount, faces, positions, uvs, normals);
	}
	if (hasSkeletonBindings) {
		jointIndices = (unsigned int*)b.get(vertexCount*jointsPerVertex*sizeof(float));
		jointWeights = (float*)b.get(vertexCount*jointsPerVertex*sizeof(float));
	}

	if (b.atEnd()) {
		createMesh(rs, out_mesh, layout, faceCount, vertexCount, faces, positions, uvs, normals, tangents, jointIndices, jointWeights);
	}
	else {
		success = false;
	}

	if (tangents) {
		delete[] tangents;
	}

	return success;
}

void destroyMesh(Mesh* mesh)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glDeleteBuffers(mesh->vertexBufferCount, mesh->glVertexBufferHandles);
		delete[] mesh->glVertexBufferHandles;
		glDeleteBuffers(1, &mesh->glIndexBufferHandle);
		glDeleteVertexArrays(1, &mesh->glVertexArrayObjectHandle);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{

	}GOBLIN_END_D3D;
#endif

	*mesh ={0};
}

void bindMesh(RenderState* rs, Mesh& mesh)
{
	rs->boundMeshTriangleCount = mesh.triangleCount;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glBindVertexArray(mesh.glVertexArrayObjectHandle);
		rs->boundMeshIndexBufferType = mesh.glIndexBufferType;
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		rs->deviceContext->IASetVertexBuffers(0, mesh.vertexBufferCount, &mesh.d3dVertexBuffers[0], mesh.d3dVertexBufferStrides, mesh.d3dVertexBufferOffsets);
		rs->deviceContext->IASetIndexBuffer(mesh.d3dIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		rs->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}GOBLIN_END_D3D;
#endif
}

std::string getShaderProgramErrors(RenderState* rs, ShaderProgram& shader)
{
	std::string message;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		int errorStringLength = 0;
		glGetProgramiv(shader.glProgram, GL_INFO_LOG_LENGTH, &errorStringLength);
		// Length will always be at least 1 for the null terminator
		message.resize(errorStringLength);
		glGetProgramInfoLog(shader.glProgram, errorStringLength, &errorStringLength, &message[0]);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		if (shader.errorMessages) {
			message = (char*)shader.errorMessages->GetBufferPointer();
		}
	}GOBLIN_END_D3D
#endif

		return message;
}

bool createShaderProgram(RenderState* rs, ShaderProgram* out_shader, VertexLayout layout, const char vertexShaderData[], int vertexShaderByteCount, const char fragmentShaderData[], int fragmentShaderByteCount)
{
	*out_shader ={};

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		GLuint vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShaderHandle, 1, &vertexShaderData, &vertexShaderByteCount);
		glCompileShader(vertexShaderHandle);

		GLuint fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShaderHandle, 1, &fragmentShaderData, &fragmentShaderByteCount);
		glCompileShader(fragmentShaderHandle);

		GLuint shaderProgramHandle = glCreateProgram();
		glAttachShader(shaderProgramHandle, vertexShaderHandle);
		glAttachShader(shaderProgramHandle, fragmentShaderHandle);
		glLinkProgram(shaderProgramHandle);
		out_shader->glProgram = shaderProgramHandle;

		// Free vertex and fragment shaders
		glDetachShader(out_shader->glProgram, vertexShaderHandle);
		glDeleteShader(vertexShaderHandle);
		glDetachShader(out_shader->glProgram, fragmentShaderHandle);
		glDeleteShader(fragmentShaderHandle);

		GLint success;
		glGetProgramiv(shaderProgramHandle, GL_LINK_STATUS, &success);
		if (!success) {
#ifdef GOBLIN_ENABLE_DEBUG_LOGGING
			goblinDebugLog(getShaderProgramErrors(rs, *out_shader).c_str());
#endif
			return false;
		}
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		char vertexShaderEntryPoint[] = "main";
		char fragmentShaderEntryPoint[] = "main";
		HRESULT hr;
		hr = D3DCompile(vertexShaderData, vertexShaderByteCount, NULL, NULL, NULL, vertexShaderEntryPoint, "vs_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &out_shader->vertexShaderBlob, &out_shader->errorMessages);
		if (FAILED(hr)) {
			return false;
		}

		hr = D3DCompile(fragmentShaderData, fragmentShaderByteCount, NULL, NULL, NULL, fragmentShaderEntryPoint, "ps_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &out_shader->pixelShaderBlob, &out_shader->errorMessages);
		if (FAILED(hr)) {
			return false;
		}

		rs->device->CreateVertexShader(out_shader->vertexShaderBlob->GetBufferPointer(), out_shader->vertexShaderBlob->GetBufferSize(), NULL, &out_shader->vertexShader);
		rs->device->CreatePixelShader(out_shader->pixelShaderBlob->GetBufferPointer(), out_shader->pixelShaderBlob->GetBufferSize(), NULL, &out_shader->pixelShader);

		D3D11_INPUT_ELEMENT_DESC* inputLayout = new D3D11_INPUT_ELEMENT_DESC[layout.dataTypeCount];
		for (unsigned int i=0; i<layout.dataTypeCount; ++i)
		{
			inputLayout[i] ={0};
			inputLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayout[i].InputSlot = i;
			inputLayout[i].SemanticName = layout.dataTypes[i].nameInShader;
			switch (layout.dataTypes[i].type)
			{
				case VertexDataType::positions_3floats:
				case VertexDataType::normals_3floats:
					inputLayout[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
					break;
				case VertexDataType::uvs_2floats:
					inputLayout[i].Format = DXGI_FORMAT_R32G32_FLOAT;
					break;
				case VertexDataType::tangents_4floats:
				case VertexDataType::jointWeights_4floats:
					inputLayout[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					break;
				case VertexDataType::jointIndices_4ints:
					inputLayout[i].Format = DXGI_FORMAT_R32G32B32A32_UINT;
					break;
				default:
					assert(false); // This type needs to be implemented
			}
		}

		rs->device->CreateInputLayout(inputLayout, layout.dataTypeCount, out_shader->vertexShaderBlob->GetBufferPointer(), out_shader->vertexShaderBlob->GetBufferSize(), &out_shader->inputLayout);
	} GOBLIN_END_D3D;
#endif

	return true;
}

void createBasicShaderProgram(RenderState* rs, ShaderProgram* out_shader)
{
	*out_shader ={};

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		char vsCode[] = R"(
			#version 330 core
			layout (std140) uniform uniforms {
				mat4 mvp;
			};
			layout(location = 0) in vec4 vertexPosition;
			layout(location = 1) in vec2 vertexUVs;
			out vec2 texCoords;
			void main() {
				texCoords.x = vertexUVs;
				texCoords.y = -vertexUVs.y;
				gl_Position = vertexPosition*mvp;
			}
		)";
		unsigned int vsLength = sizeof(vsCode);

		char fsCode[] = R"(
			#version 330 core
			uniform sampler2D textures[1];
			in vec2 texCoords;
			layout (location = 0) out vec4 outColor;
			void main(){
				outColor = texture(textures[0], texCoords);
			}
		)";
		unsigned int fsLength = sizeof(fsCode);

		VertexLayout layout;
		createBasicVertexLayout(&layout);
		createShaderProgram(rs, out_shader, layout, vsCode, vsLength, fsCode, fsLength);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		char vsCode[] = R"(
			cbuffer uniforms : register( b0 )
			{
				matrix mvp;
			}
			struct VS_INPUT
			{
				float4 Pos : vertexPositions;
				float2 uvs : vertexUVs;
				float4 normals : vertexNormals;
			};
			struct PS_INPUT
			{
				float4 Pos : SV_POSITION;
				float2 uvs : uv;
			};
			PS_INPUT main( VS_INPUT input )
			{
				PS_INPUT output;
				output.Pos = mul(input.Pos, mvp);
				output.uvs = input.uvs;
				return output;
			}
		)";
		unsigned int vsLength = sizeof(vsCode);

		char fsCode[] = R"(
			Texture2D txDiffuse : register( t0 );
			SamplerState samLinear : register( s0 );
			struct PS_INPUT
			{
				float4 Pos : SV_POSITION;
				float2 uvs : uv;
			};

			float4 main( PS_INPUT input ) : SV_Target
			{
				return txDiffuse.Sample( samLinear, input.uvs );
			}

		)";
		unsigned int fsLength = sizeof(fsCode);

		VertexLayout layout;
		createBasicVertexLayout(&layout);
		createShaderProgram(rs, out_shader, layout, vsCode, vsLength, fsCode, fsLength);
	}GOBLIN_END_D3D
#endif
}

void destroyShaderProgram(ShaderProgram* shader)
{

}

void bindShaderProgram(RenderState* rs, ShaderProgram shader)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		GLint success;
		glGetProgramiv(shader.glProgram, GL_LINK_STATUS, &success);
		if (success) {
			glUseProgram(shader.glProgram);
			rs->boundShader = shader.glProgram;
		}
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		rs->deviceContext->VSSetShader(shader.vertexShader, NULL, 0);
		rs->deviceContext->IASetInputLayout(shader.inputLayout);
		rs->deviceContext->PSSetShader(shader.pixelShader, NULL, 0);
	}GOBLIN_END_D3D
#endif
}

void createUniformBuffer(RenderState* rs, UniformBuffer* out_uniforms, unsigned int byteCount)
{
	*out_uniforms ={0};
	out_uniforms->byteCount = byteCount;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glGenBuffers(1, &out_uniforms->glUniformBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, out_uniforms->glUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, byteCount, 0, GL_STATIC_DRAW); // allocate 150 bytes of memory
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC bufferDesc ={0};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = byteCount;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		rs->device->CreateBuffer(&bufferDesc, 0, &out_uniforms->d3dConstantBuffer);
	}GOBLIN_END_D3D
#endif
}

void destroyUniformBuffer(UniformBuffer* uniforms)
{

}

void bindUniformBuffer(RenderState* rs, UniformBuffer* mod_uniforms, const char* nameInShader, unsigned int bindLocation, void* data)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		// Update buffer data
		glBindBuffer(GL_UNIFORM_BUFFER, mod_uniforms->glUniformBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, mod_uniforms->byteCount, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Bind buffer
		GLuint uniformIndex = glGetUniformBlockIndex(rs->boundShader, nameInShader);
		glUniformBlockBinding(rs->boundShader, uniformIndex, bindLocation);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindLocation, mod_uniforms->glUniformBuffer);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		rs->deviceContext->Map(mod_uniforms->d3dConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, data, mod_uniforms->byteCount);
		rs->deviceContext->Unmap(mod_uniforms->d3dConstantBuffer, 0);
		//rs->deviceContext->UpdateSubresource(mod_uniforms->d3dConstantBuffer, 0, NULL, data, 0, 0);
		rs->deviceContext->VSSetConstantBuffers(0, 1, &mod_uniforms->d3dConstantBuffer);
	}GOBLIN_END_D3D
#endif
}

void createTexture(RenderState* rs, Texture* out_texture, unsigned char* pixels, unsigned int width, unsigned int height, Texture::Format pixelFormat, bool shrinkSmooth, bool enlargeSmooth, bool generateMipmaps)
{
	*out_texture ={0};
	out_texture->width = width;
	out_texture->height = height;
	out_texture->format = pixelFormat;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		GLint internalFormat;
		GLint format;
		switch (pixelFormat) {
			case Texture::Format::rgb8: {
				internalFormat = format = GL_RGB;
			} break;
			case Texture::Format::srgb8: {
				format = GL_RGB;
				internalFormat = GL_SRGB8;
			} break;
			case Texture::Format::rgba8: {
				internalFormat = format = GL_RGBA;
			} break;
			case Texture::Format::srgba8: {
				format = GL_RGBA;
				internalFormat = GL_SRGB8_ALPHA8;
			} break;
			case Texture::Format::depth: {
				internalFormat = format = GL_DEPTH_COMPONENT;
			} break;
			case Texture::Format::depthStencil: {
				internalFormat = GL_DEPTH32F_STENCIL8;
				format = GL_DEPTH_STENCIL;
			} break;
			default: assert(0);
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

		// Interpolation and mipmaping
		GLint minFilter;
		if (generateMipmaps) {
			minFilter = shrinkSmooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			minFilter = shrinkSmooth ? GL_LINEAR : GL_NEAREST;
		}
		GLint magFilter = enlargeSmooth ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		out_texture->textureHandle = textureID;
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		unsigned char* formattedPixels = pixels;
		D3D11_TEXTURE2D_DESC texInfo ={0};
		switch (pixelFormat) {
			case Texture::Format::r8g8b8a8: {
				texInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			} break;
			case Texture::Format::r8g8b8: {
				// Convert the pixels to RGBA
				formattedPixels = new unsigned char[width*height*4];
				unsigned int srcIndex = 0;
				unsigned int dstIndex = 0;
				while (dstIndex<width*height*4) {
					formattedPixels[dstIndex]   = pixels[srcIndex];
					formattedPixels[dstIndex+1] = pixels[srcIndex+1];
					formattedPixels[dstIndex+2] = pixels[srcIndex+2];
					formattedPixels[dstIndex+3] = 0xFF;
					srcIndex += 3;
					dstIndex += 4;
				}
				texInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			} break;
		}
		texInfo.Width = width;
		texInfo.Height = height;
		texInfo.MipLevels = 1;
		texInfo.ArraySize = 1;
		texInfo.SampleDesc ={1, 0};
		texInfo.Usage = D3D11_USAGE_DEFAULT;
		texInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA pixelData ={0};
		pixelData.pSysMem = formattedPixels;
		pixelData.SysMemPitch = 4*width;
		ID3D11Texture2D* texture;
		rs->device->CreateTexture2D(&texInfo, &pixelData, &texture);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		srvd.Format = texInfo.Format;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MipLevels = 1;
		srvd.Texture2D.MostDetailedMip = 0;
		rs->device->CreateShaderResourceView(texture, &srvd, &out_texture->resource);

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		rs->device->CreateSamplerState(&sampDesc, &out_texture->samplerState);

		rs->deviceContext->PSSetShaderResources(0, 1, &out_texture->resource);
		rs->deviceContext->PSSetSamplers(0, 1, &out_texture->samplerState);

		if (formattedPixels != pixels) {
			delete formattedPixels;
		}
	} GOBLIN_END_D3D
#endif
}

void destroyTexture(Texture* texture)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glDeleteTextures(1, &texture->textureHandle);
	}GOBLIN_END_GL
#endif

		*texture ={0};
}

void bindTextures(RenderState* rs, const char* nameInShader, Texture textures[], unsigned int textureCount)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		GLuint uniformLocation = glGetUniformLocation(rs->boundShader, nameInShader);
		for (unsigned int i=0; i<textureCount; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i].textureHandle);
			glUniform1i(uniformLocation + i, i);
		}
	}GOBLIN_END_GL
#endif
}

void createFrameBuffer(RenderState* rs, FrameBuffer* out_fb, Texture rgbaTextures[], unsigned int rgbaTextureCount, Texture* depthStencilTexture)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glGenFramebuffers(1, &out_fb->frameBufferHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, out_fb->frameBufferHandle);
		unsigned int maxWidth = 0;
		unsigned int maxHeight = 0;
		for (unsigned int i=0; i<rgbaTextureCount; ++i) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, rgbaTextures[i].textureHandle, 0);
			maxWidth = (maxWidth > rgbaTextures[i].width) ? maxWidth : rgbaTextures[i].width;
			maxHeight = (maxHeight > rgbaTextures[i].height) ? maxHeight : rgbaTextures[i].height;
		}
		if (depthStencilTexture) {
			GLenum attachmentType = depthStencilTexture->format==Texture::Format::depth? GL_DEPTH_ATTACHMENT : GL_DEPTH_STENCIL_ATTACHMENT;
			glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, depthStencilTexture->textureHandle, 0);
			maxWidth = (maxWidth > depthStencilTexture->width) ? maxWidth : depthStencilTexture->width;
			maxHeight = (maxHeight > depthStencilTexture->height) ? maxHeight : depthStencilTexture->height;
		}
		out_fb->width = maxWidth;
		out_fb->height = maxHeight;
	}GOBLIN_END_GL
#endif
}

bool getScreenFrameBuffer(RenderState* rs, FrameBuffer* out_fb)
{
	*out_fb ={0};

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		out_fb->frameBufferHandle = 0;
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		rs->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		HRESULT hr = rs->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr)) {
			return false;
		}
		hr = rs->device->CreateRenderTargetView(pBackBuffer, NULL, &out_fb->renderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr)) {
			return false;
		}
		rs->deviceContext->OMSetRenderTargets(1, &out_fb->renderTargetView, NULL);

		// Setup the viewport
		D3D11_TEXTURE2D_DESC textureInfo ={0};
		pBackBuffer->GetDesc(&textureInfo);
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)textureInfo.Width;
		vp.Height = (FLOAT)textureInfo.Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		rs->deviceContext->RSSetViewports(1, &vp);
	}GOBLIN_END_D3D
#endif

		return true;
}

void createScreenFrameBuffer(RenderState* rs, FrameBuffer* out_fb, unsigned int width, unsigned int height)
{
	*out_fb ={0};
	out_fb->width = width;
	out_fb->height = height;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		out_fb->frameBufferHandle = 0;
	}GOBLIN_END_GL
#endif
}

void destroyFrameBuffer(FrameBuffer* fb)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		if (fb->frameBufferHandle != 0) {
			glDeleteFramebuffers(1, &fb->frameBufferHandle);
		}
	}GOBLIN_END_GL
#endif

		*fb ={0};
}

// We need to pass in the framebuffer in case there are multiple textures.
void clearFrameBuffer(RenderState* rs, FrameBuffer* out_fb, Vec4 clearColor)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		glBindFramebuffer(GL_FRAMEBUFFER, out_fb->frameBufferHandle);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
		rs->deviceContext->ClearRenderTargetView(out_fb->renderTargetView, (float*)&clearColor);
	}GOBLIN_END_D3D
#endif
}

void resizeFrameBuffer(RenderState* rs, FrameBuffer* out_fb, unsigned int width, unsigned int height)
{
	out_fb->width = width;
	out_fb->height = height;

#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{

	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
	GOBLIN_BEGIN_D3D{
		rs->deviceContext->OMSetRenderTargets(0, 0, 0);
		rs->screenRenderTarget->Release();
		getScreenFrameBuffer(rs, out_fb);
	}GOBLIN_END_D3D
#endif
}

void bindFrameBuffer(RenderState* rs, FrameBuffer frameBuffer)
{
#ifdef GOBLIN_ENABLE_GL
	GOBLIN_BEGIN_GL{
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.frameBufferHandle);
			glViewport(0, 0, frameBuffer.width, frameBuffer.height);
		}
	}GOBLIN_END_GL
#endif

#ifdef GOBLIN_ENABLE_D3D
		GOBLIN_BEGIN_D3D{
	}GOBLIN_END_D3D
#endif
}

} // namespace
#endif // include guard
