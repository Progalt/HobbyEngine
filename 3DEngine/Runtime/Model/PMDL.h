#pragma once

#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

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


// TODO: 
//	- Animations system: This includes bones and animation playback
//	- Better Compression and shrinking of file size.

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
		Vector4(VecType x, VecType y, VecType z, VecType w) : x(x), y(y), z(z), w(w) { }

		VecType x, y, z, w;
	};

	struct Vertex
	{
		float x, y, z;
		float u, v;
		float nx, ny, nz;
		float tx, ty, tz;
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
		uint32_t firstIndex, indexCount, materialIndex, firstVertex, firstLOD, LODCount;
	};

	enum BlendMode
	{
		BLEND_MODE_OPAQUE,
		BLEND_MODE_MASK,
		BLEND_MODE_BLEND
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

		float roughness;
		float metallic;

		BlendMode alphaMode = BLEND_MODE_OPAQUE;
		float alphaCutoff = 1.0f;
	};


	struct Texture1
	{
		uint16_t pathSize;
		// Could this be done better? 
		char path[256];
	};

	// Header for version 1 pmdl files
	struct Header1
	{
		// One byte for the version number
		uint8_t version = 0;

		// Model information

		uint16_t meshCount = 0;

		uint16_t lodCount = 0;

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

		// First is the mesh data then LODs are stored immediately after
		uint32_t meshDataOffset = 0;

		uint32_t materialOffset = 0;

		uint32_t textureDataOffset = 0;
	};

	/*
		struct to hold a whole PMDL in one place
	*/
	struct PMDL
	{
		uint32_t vertexCount = 0;
		Vertex* vertices;

		uint32_t indexCount = 0;
		IndexTypeExport* indices;

		uint32_t meshCount = 0;
		Mesh* meshes;

		uint32_t materialCount = 0;
		Material1* materials;

		uint32_t textureCount = 0;
		Texture1* textures;
	};

	// This frees all allocated memory associated with a PMDL 
	void FreePMDL(PMDL* pmdl);


	uint32_t GetTextureSize(Texture1* tex);

	// NOTE: Alot of these functions will probably be wrapped up in a nicer interface later on

	// ---- Init functions ---- 

	void InitHeader(Header1* header);

	// Returns the total size of the file
	size_t InitHeaderOffsets1(Header1* header, uint32_t vertexCount, uint32_t indexCount, uint32_t meshCount, uint32_t materialCount, uint32_t textureCount, uint32_t lodCount);


	// ---- Reading Functions ----

	Header1 ReadHeader1(FILE* file);

	Mesh ReadMesh1(FILE* file, Header1* header, uint32_t meshIndex);

	Mesh ReadLOD1(FILE* file, Header1* header, uint32_t lodIndex);

	// The returned vertices must be freed with PMDL_FREE
	Vertex* ReadVertices(FILE* file, Header1* header);

	// Must call the matching function for the type of indices
	// This can be queried from the header struct
	uint32_t* ReadIndices32bit(FILE* file, Header1* header);
	uint16_t* ReadIndices16bit(FILE* file, Header1* header);

	Material1 ReadMaterial1(FILE* file, Header1* header, uint32_t materialIndex);

	Texture1 ReadTexture(FILE* file, Header1* header, uint32_t texIndex);


	// ---- Writing Functions -----

	void WriteHeader1(FILE* file, Header1* header);

	void WriteMesh1(uint32_t meshIndex, Mesh mesh, Header1* header, FILE* file);

	void WriteLOD1(FILE* file, Header1* header, Mesh mesh, uint32_t lodIndex);

	void WriteVertices(FILE* file, Header1* header, Vertex* vertices, uint32_t vertexCount);

	void WriteIndices(FILE* file, Header1* header, IndexTypeExport* indices, uint32_t indexCount);

	void WriteMaterial1(FILE* file, Header1* header, Material1* material, uint32_t materialIndex);

	void WriteTexture1(FILE* file, Header1* header, Texture1* texture, uint32_t texIndex);

}

#ifdef PMDL_IMPLEMENTATION

namespace pmdl
{


	void FreePMDL(PMDL* pmdl)
	{
		if (pmdl->vertices)
			PMDL_FREE(pmdl->vertices);

		if (pmdl->indices)
			PMDL_FREE(pmdl->indices);

		if (pmdl->meshes)
			PMDL_FREE(pmdl->meshes);

		if (pmdl->materials)
			PMDL_FREE(pmdl->materials);

		if (pmdl->textures)
			PMDL_FREE(pmdl->textures);

	}

	void InitHeader(Header1* header)
	{
		PMDL_ASSERT(header);

		header->version = PMDL_EXPORT_VERSION;
	}

	size_t InitHeaderOffsets1(Header1* header, uint32_t vertexCount, uint32_t indexCount, uint32_t meshCount, uint32_t materialCount, uint32_t textureCount, uint32_t lodCount)
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

		cursor += (meshCount + lodCount) * sizeof(Mesh);

		header->meshCount = meshCount;
		header->lodCount = lodCount;

		header->materialCount = materialCount;
		header->materialOffset = cursor;

		cursor += materialCount * sizeof(Material1);

		header->textureCount = textureCount;
		header->textureDataOffset = cursor;

		// This doesn't actually work
		// because of the string in Texture1
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

	Mesh ReadLOD1(FILE* file, Header1* header, uint32_t lodIndex)
	{
		PMDL_ASSERT(file && header);

		uint32_t offset = header->meshDataOffset + (sizeof(Mesh) * header->meshCount) + (sizeof(Mesh) * lodIndex);

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
		//PMDL_ASSERT(header->indexType != IndexType::Uint32);

		uint32_t* indices = (uint32_t*)PMDL_MALLOC(sizeof(uint32_t) * header->indexCount);

		PMDL_ASSERT(indices);

		fseek(file, header->indicesOffset, SEEK_SET);
		fread(indices, sizeof(uint32_t), header->indexCount, file);

		return indices;
	}

	uint16_t* ReadIndices16bit(FILE* file, Header1* header)
	{
		PMDL_ASSERT(file && header);
		//PMDL_ASSERT(header->indexType != IndexType::Uint16);

		uint16_t* indices = (uint16_t*)PMDL_MALLOC(sizeof(uint16_t) * header->indexCount);

		PMDL_ASSERT(indices);

		fseek(file, header->indicesOffset, SEEK_SET);
		fread(indices, sizeof(uint16_t), header->indexCount, file);

		return indices;
	}

	Material1 ReadMaterial1(FILE* file, Header1* header, uint32_t materialIndex)
	{
		PMDL_ASSERT(file && header);

		// This function should be checked before being called
		PMDL_ASSERT(header->materialCount != 0);
		PMDL_ASSERT(header->materialCount > materialIndex);

		Material1 out;

		uint32_t offset = header->materialOffset + sizeof(Material1) * materialIndex;

		fseek(file, offset, SEEK_SET);
		fread(&out, sizeof(Material1), 1, file);

		return out;

	}

	Texture1 ReadTexture(FILE* file, Header1* header, uint32_t texIndex)
	{
		PMDL_ASSERT(file && header);
		PMDL_ASSERT(header->textureCount != 0);
		PMDL_ASSERT(header->textureCount > texIndex);

		Texture1 out;

		uint32_t offset = header->textureDataOffset + sizeof(Texture1) * texIndex;

		fseek(file, offset, SEEK_SET);
		fread(&out, sizeof(Texture1), 1, file);

		return out;
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

	void WriteLOD1(FILE* file, Header1* header, Mesh mesh, uint32_t lodIndex)
	{
		PMDL_ASSERT(file && header);
		//PMDL_ASSERT(lodIndex < header->lodCount);

		uint32_t offset = header->meshDataOffset + (header->meshCount * sizeof(Mesh)) + (sizeof(Mesh) * lodIndex);

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

	void WriteMaterial1(FILE* file, Header1* header, Material1* material, uint32_t materialIndex)
	{
		PMDL_ASSERT(file && header && material);
		PMDL_ASSERT(materialIndex < header->materialCount);

		uint32_t offset = header->materialOffset + sizeof(Material1) * materialIndex;

		fseek(file, offset, SEEK_SET);
		fwrite(material, sizeof(Material1), 1, file);
	}

	void WriteTexture1(FILE* file, Header1* header, Texture1* texture, uint32_t texIndex)
	{
		PMDL_ASSERT(file && header && texture);
		PMDL_ASSERT(texIndex < header->textureCount);

		uint32_t offset = header->textureDataOffset + sizeof(Texture1) * texIndex;
		fseek(file, offset, SEEK_SET);
		fwrite(texture, sizeof(Texture1), 1, file);
	}

}

#endif
