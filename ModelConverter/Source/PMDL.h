#pragma once

#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

// ---- PMDL ----
// A simple model format for my engine
// It supports PBR out of the box
// I like GLTF but wanted my own format that I could use
// I take some inspiration from gltf
// Unlike GLTF I don't any extension functionality and would never implement anything
// Just different versions

// Customizing
// define PMDL_MALLOC(x) to override allocations
// Same goes for PMDL_FREE(x)
// Override index format to uint16 for exports using PMDL_EXPORT_UINT16_INDICES

// All functions and most structs have a suffix of the version they support
// Some functions work for future version and will be marked as such 

// Current Features: 
// Version 1: 
// - Binary File format
// - Multiple mesh support
// - Mutiple Material support
// - PBR materials

// min and max version supported by this importer
#define PMDL_MIN_VERSION_SUPPORT 1
#define PMDL_MAX_VERSION_SUPPORT 1


// Version the exporter exports
#define PMDL_EXPORT_VERSION 1

#ifndef PMDL_MALLOC(x)
#define PMDL_MALLOC(x) malloc(x)
#endif

#ifndef PMDL_FREE(x)
#define PMDL_FREE(x) free(x)
#endif

#ifndef PMDL_ASSERT(x)
#include <assert.h>
#define PMDL_ASSERT(x) assert(x)
#endif

namespace pmdl
{
#ifndef PMDL_EXPORT_UINT16_INDICES
	using IndexTypeExport = uint32_t;
#else 
	using IndexTypeExport = uint16_t;
#endif

#ifdef PMDL_VEC_TYPE_DOUBLE
	using VecType = double;
#else 
	using VecType = float;
#endif


	struct Vector4
	{
		Vector4() : x(0), y(0), z(0), w(0) { }

		VecType x, y, z, w;
	};

	struct Vector3
	{
		VecType x, y, z;
	};

	struct Vector2
	{
		VecType x, y;
	};

	struct Vertex
	{
		Vector3 position;
		Vector2 texCoord;
		Vector3 normal;
		Vector3 tangent;
	};

	enum class IndexType
	{
		Uint16, 
		Uint32
	};

	enum class VectorType
	{
		Float, 
		Double
	};

	struct Mesh
	{
		uint32_t firstIndex, indexCount, materialIndex;
	};

	struct Material1
	{
		// indices into textures
		// if its -1 it doesn't have a texture in that slot

		int32_t albedoIndex = -1;
		int32_t normalIndex = -1;
		int32_t roughnessIndex = -1;
		int32_t metallicIndex = -1;
		int32_t aoIndex = -1;

		Vector4 albedo;
	};


	struct Texture1
	{
		uint32_t pathSize;
		char* path;
	};

	// Header for version 1 pmdl files
	struct Header1
	{
		// One byte for the version number
		uint8_t version = 0;

		// Model information

		uint16_t meshCount = 0;

		// usually the material count will match the mesh count but some meshes could share material

		uint16_t materialCount = 0;

		uint16_t textureCount = 0;

		// In version 1 external textures are forced

		uint8_t externalTextures = 1;


		// Vertex stride? Constant for now. Kept here just in case it changes

		uint16_t vertexStride = sizeof(Vertex);

		// what types the indices are

		IndexType indexType = IndexType::Uint32;
		VectorType vecType = VectorType::Float;

		// Number of vertices the file contains
		// To get memory usage in bytes: vertexCount * vertexStride 

		uint32_t vertexCount = 0;

		uint32_t indexCount = 0;

		// These are offsets into the file where the data is located

		uint32_t vertexOffset = 0;

		uint32_t indicesOffset = 0;

		uint32_t meshDataOffset = 0;

		uint32_t materialOffset = 0;

		uint32_t textureDataOffset = 0;
	};

	// ---- Init functions ---- 

	void InitHeader(Header1* header);

	// Returns the total size of the file
	size_t InitHeaderOffsets1(Header1* header, uint32_t vertexCount, uint32_t indexCount, uint32_t meshCount, uint32_t materialCount, uint32_t textureCount);


	// ---- Reading Functions ----

	Header1 ReadHeader1(FILE* file);

	Mesh ReadMesh1(FILE* file, Header1* header, uint32_t meshIndex);

	// The returned vertices must be freed with PMDL_FREE
	Vertex* ReadVertices(FILE* file, Header1* header);

	// Must call the matching function for the type of indices
	// This can be queried from the header struct
	uint32_t* ReadIndices32bit(FILE* file, Header1* header);
	uint16_t* ReadIndices16bit(FILE* file, Header1* header);

	// ---- Writing Functions -----

	void WriteHeader1(FILE* file, Header1* header);

	void WriteMesh1(uint32_t meshIndex, Mesh mesh, Header1* header, FILE* file);

	void WriteVertices(FILE* file, Header1* header, Vertex* vertices, uint32_t vertexCount);

	void WriteIndices(FILE* file, Header1* header, IndexTypeExport* indices, uint32_t indexCount);

}

#ifdef PMDL_IMPLEMENTATION

namespace pmdl
{
	void InitHeader(Header1* header)
	{
		PMDL_ASSERT(header);

		header->version = PMDL_EXPORT_VERSION;
	}

	size_t InitHeaderOffsets1(Header1* header, uint32_t vertexCount, uint32_t indexCount, uint32_t meshCount, uint32_t materialCount, uint32_t textureCount)
	{
		PMDL_ASSERT(header);

		// This inits most of the header data 

		header->vertexCount = vertexCount;
		header->indexCount = indexCount;

		uint32_t cursor = sizeof(Header1);

		header->vertexOffset = cursor;

		cursor += header->vertexCount * header->vertexStride;

		header->indicesOffset = cursor;

		cursor += header->indexCount * sizeof(IndexTypeExport);
		
		header->indexType = (sizeof(IndexTypeExport) == sizeof(uint32_t)) ? IndexType::Uint32 : IndexType::Uint16;

		header->meshDataOffset = cursor;

		cursor += meshCount * sizeof(Mesh);

		header->meshCount = meshCount;

		header->materialCount = materialCount;
		header->materialOffset = cursor;
		
		cursor += materialCount * sizeof(Material1);

		header->textureCount = textureCount;
		header->textureDataOffset = cursor;

		cursor += textureCount * sizeof(Texture1);

		return cursor;
	}

	// Reading implementation

	Header1 ReadHeader1(FILE* file)
	{
		Header1 header;
		fread(&header, sizeof(Header1), 1, file);

		// Assert here if the version is not supported because other functions could fail later on if not
		PMDL_ASSERT((header.version >= PMDL_MIN_VERSION_SUPPORT) && (header.version <= PMDL_MAX_VERSION_SUPPORT) &&
			"PMDL version not supported by importer");

		return header;
	}

	Mesh ReadMesh1(FILE* file, Header1* header, uint32_t meshIndex)
	{
		PMDL_ASSERT(file && header);

		uint32_t offset = header->meshDataOffset + (sizeof(Mesh) * meshIndex);

		Mesh mesh;

		fseek(file, offset, SEEK_SET);
		fread(&mesh, sizeof(Mesh), 1, file);

		return mesh;
	}

	Vertex* ReadVertices(FILE* file, Header1* header)
	{
		PMDL_ASSERT(file && header);

		Vertex* verts = (Vertex*)PMDL_MALLOC(sizeof(Vertex) * header->vertexCount);

		PMDL_ASSERT(verts);

		fseek(file, header->vertexOffset, SEEK_SET);
		fread(verts, sizeof(Vertex), header->vertexCount, file);

		return verts;
	}

	uint32_t* ReadIndices32bit(FILE* file, Header1* header)
	{
		PMDL_ASSERT(file && header);
		PMDL_ASSERT(header->indexType == IndexType::Uint32);

		uint32_t* indices = (uint32_t*)PMDL_MALLOC(sizeof(uint32_t) * header->indexCount);

		PMDL_ASSERT(indices);

		fseek(file, header->indicesOffset, SEEK_SET);
		fread(indices, sizeof(uint32_t), header->indexCount, file);

		return indices;
	}

	uint16_t* ReadIndices16bit(FILE* file, Header1* header)
	{
		PMDL_ASSERT(file && header);
		PMDL_ASSERT(header->indexType == IndexType::Uint16);

		uint16_t* indices = (uint16_t*)PMDL_MALLOC(sizeof(uint16_t) * header->indexCount);

		PMDL_ASSERT(indices);

		fseek(file, header->indicesOffset, SEEK_SET);
		fread(indices, sizeof(uint16_t), header->indexCount, file);

		return indices;
	}

	// Writing implementation

	void WriteHeader1(FILE* file, Header1* header)
	{
		PMDL_ASSERT(file && header);

		fwrite(header, sizeof(Header1), 1, file);
	}

	void WriteMesh1(uint32_t meshIndex, Mesh mesh, Header1* header, FILE* file)
	{
		PMDL_ASSERT(file && header);
		PMDL_ASSERT(meshIndex < header->meshCount);

		uint32_t offset = header->meshDataOffset + (sizeof(Mesh) * meshIndex);

		fseek(file, offset, SEEK_SET);
		fwrite(&mesh, sizeof(Mesh), 1, file);
	}

	void WriteVertices(FILE* file, Header1* header, Vertex* vertices, uint32_t vertexCount)
	{
		PMDL_ASSERT(file && header);
		PMDL_ASSERT(header->vertexCount == vertexCount);

		uint32_t offset = header->vertexOffset;

		fseek(file, offset, SEEK_SET);
		fwrite(vertices, sizeof(Vertex), vertexCount, file);
	}

	void WriteIndices(FILE* file, Header1* header, IndexTypeExport* indices, uint32_t indexCount)
	{
		PMDL_ASSERT(file && header);
		PMDL_ASSERT(header->indexCount == indexCount);

		uint32_t offset = header->indicesOffset;

		fseek(file, offset, SEEK_SET);
		fwrite(indices, sizeof(IndexTypeExport), indexCount, file);
	}

}

#endif
