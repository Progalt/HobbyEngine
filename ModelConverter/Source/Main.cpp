
#define PMDL_IMPLEMENTATION
#include "PMDL.h"

int main(int argc, char* argv[])
{
	{
		pmdl::Header1 header{};
		pmdl::InitHeader(&header);
		pmdl::InitHeaderOffsets1(&header, 6, 12, 2, 2, 0);

		pmdl::Mesh mesh1, mesh2;
		mesh1.firstIndex = 0;
		mesh1.indexCount = 6;

		mesh2.firstIndex = 6;
		mesh2.indexCount = 6;

		FILE* file = fopen("TestModel.pmdl", "wb");

		pmdl::WriteHeader1(file, &header);

		pmdl::WriteMesh1(0, mesh1, &header, file);
		//pmdl::WriteMesh1(1, mesh2, &header, file);

		fclose(file);
	}

	{
		pmdl::Header1 header{};

		FILE* file = fopen("TestModel.pmdl", "rb");

		header = pmdl::ReadHeader1(file);

		pmdl::Mesh mesh1 = pmdl::ReadMesh1(file, &header, 0);

		printf("Mesh Count: %d\n", header.meshCount);


		fclose(file);
	}
}