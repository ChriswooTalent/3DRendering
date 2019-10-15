// MarchingCube:  
// 3D Surface Construction Algorithm, choose a iso value in the volume data and reconstruct the 3D Image.  
//    
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
// 
#ifndef _MARCHINGCUBE_H_
#define _MARCHINGCUBE_H_
#include "RenderBase.h"
#include "DataTypes.h"
#include "Rotateslice.h"
#include <map>
#include <algorithm>
//#include <OpenMesh/Core/IO/MeshIO.hh>
//#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//#include <OpenMesh/Core/Geometry/VectorT.hh>

using namespace std;
 
#ifdef RENDERLIB_EXPORTS  
#define MARCHINCUBELIB_API __declspec(dllexport)  
#else  
#define MARCHINCUBELIB_API __declspec(dllimport)  
#endif  

#pragma pack(1)

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG

const char MCHKernelFile[256] = "D:/CarbonMed/Config/Local/CL/marchingCubes_kernel.cl";

class MARCHINCUBELIB_API MarchingCube : public RenderBase
{
public:
	MarchingCube();
	~MarchingCube();

	//opencl
	void KernelInit();

	//opengl buffer
	int OpenGLBufferInit();

	//texture
	void TextureInit();

	//rendering
	void RenderProcess();

	//Rendering Feature:ISO value
	void SetRenderingFeature(RenderParam param);
	void GetRenderingFeature(RenderParam &param);
	float GetNormIso(float stdiso);

	void GLDataRendering(cl_float translate[4], cl_float clrotate[4], int orderflag);
	void WGLDataRendering(int orderflag);
	void WGLSliceRendering(int orderflag);

	//Rendering TexSlice
	void RenderTexSlice(GLuint tex2d, int orderflag);
	void Render2DSlicePBO(GLuint tex2d);
	void RenderTexSlicePBO(GLuint tex2d);
	void RenderRegHintPBO(GLuint tex2d, int bufindex, int orderflag);
	void RenderingPunctureLine(int index);
	void RenderingProbesensor(GLuint tex2d, int orderflag);

	//animation
	void animation();

	//keyboard
	void KeyBoard();

	//OpenGL
	//VBO
	void createVBO(GLuint* vbo, unsigned int size, cl_mem &vbo_cl);
	void deleteVBO(GLuint* vbo, cl_mem vbo_cl);
	void openglMaterialInit();
	void renderIsosurface(GLuint pvbo, GLuint nvbo, GLuint cvbo);
	//set rendering feature
	void SetRenderFeature();
	void SetRenderFeatureBone();
	void SetRenderFeatureMask();
	void SetRenderFeatureSlice();
	void SetRenderFeatureNeedle();

	void GetNeiborInfo(int *FacesA, int *FacesB, int *FacesC, int **NU, int *NU_length, int FacesN, int VertexN, int maxnbs, int *NeArray, int *NeArrayNums);
	void LaunchMeshFiltering(cl_mem vtxfiltered, cl_mem pos, int factiveVoxels, int ftotalverts, int *vertscount, int *facecount, int maxverts, int itertimes);
	void RemoveDuplicateVerts(vector<Point3> pt_in, int ftotalverts, vector<Point3> &pt_out, vector<int> &indxout);
	//processing
	void computeIsosurface(cl_mem de_volume, cl_mem &de_pos, cl_mem &de_normal, cl_mem &de_color, UINT32 xgridoffset, UINT32 ygridoffset, UINT32 zgridoffset, cl_uint gridSize[4],
		                   cl_uint offsetx, cl_uint offsety, cl_uint offsetz, cl_uint endx, cl_uint endy, cl_uint endz,
		                   float xstartoff, float ystartoff, float zstartoff, float voxelSize[4], float isoval, bool meshfiltering_flag, int itertimes);

	//scan
	void openclScanApple(cl_mem d_voxelOccupiedScan, cl_mem d_voxelOccupied, int numVoxels);

	void GetPosVbo(GLuint &pvbo);

	void GetNormalVbo(GLuint &nvbo);

	void GetColorVbo(GLuint &cvbo);

	int GetTotalVerts()
	{
		return totalVerts;
	}

	void MCOpenCLRelease();

	void ClearVertexNeighborBuf();

private:
	//kernel function
	void launch_classifyVoxel(dim3 grid, dim3 threads, cl_mem voxelVerts, cl_mem voxelOccupied, cl_mem volume,
		cl_uint gridSize[4], UINT32 numVoxels,
		cl_float voxelSize[4], float isoValue, cl_uint offsetx, cl_uint offsety, cl_uint offsetz, cl_uint endx, cl_uint endy, cl_uint endz);
	void launch_compactVoxels(dim3 grid, dim3 threads, cl_mem compVoxelArray, cl_mem voxelOccupied, cl_mem voxelOccupiedScan, UINT32 numVoxels);

	void launch_generateTriangles2WithColor(dim3 grid, dim3 threads,
		cl_mem pos, cl_mem norm, cl_mem color, cl_mem compactedVoxelArray, cl_mem numVertsScanned, cl_mem volume,
		float upperleftx, float upperlefty, float upperleftz,
		cl_uint gridSize[4], cl_float voxelSize[4], float isoValue,
		float offsetx, float offsety, float offsetz, float xcenteroffset, float ycenteroffset, float zcenteroffset,
		UINT32 activeVoxels, UINT32 maxVerts);

	void launch_clearDeviceBuffer(dim3 grid, dim3 threads, cl_mem voxelVerts, cl_mem voxelOccupied, cl_mem voxelOccupiedscan,
		cl_mem compactedVoxelArray, cl_mem numVertsScanned, cl_uint numVoxels);

	void launch_MeshFiltering(dim3 grid, dim3 threads, cl_mem vertexout, cl_mem nextinput, cl_mem vertexin, cl_mem neighborarray, cl_mem neighbornum,
		cl_uint maxneinum, cl_float lamdaCoef, cl_uint vertexnum, cl_uint facenum);

	void launch_generateNewVertex(dim3 grid, dim3 threads, cl_mem pos, cl_mem norm, cl_mem vertexin, cl_mem facein, cl_uint facenum, cl_uint maxverts);
private:
	//voxelnums
	UINT32 m_NumVoxels;
	UINT32 maxVerts;
	UINT32 m_UDVertsnum;
	UINT32 m_FaceCount;
	UINT32 activeVoxels;
	UINT32 totalVerts;

	//isovalue
	float IsoValue;
	float IsoValueBone;
	float IsoValueMask;
	float dIsoValue;
	float IsoUp;
	float IsoBot;

	////////////////MarchingCubes data on device declarations //////////////////////	
	GLuint posVbo;
	GLuint normalVbo;
	GLuint colorVbo;

	GLuint posVboBone;
	GLuint normalVboBone;
	GLuint colorVboBone;

	GLuint posVboMask;
	GLuint normalVboMask;
	GLuint colorVboMask;

	GLint  gl_Shader;

	//data on device
	cl_mem d_posbuf;
	cl_mem d_normbuf;
	cl_mem d_pos;
	cl_mem d_normal;
	cl_mem d_color;

	cl_mem d_posBone;
	cl_mem d_normalBone;
	cl_mem d_colorBone;

	cl_mem d_posMask;
	cl_mem d_normalMask;
	cl_mem d_colorMask;

	cl_mem d_voxelVerts;
	cl_mem d_voxelVertsScan;
	cl_mem d_voxelOccupied;
	cl_mem d_voxelOccupiedScan;
	cl_mem d_compVoxelArray;

	// tables
	cl_mem d_numVertsTablebuf;
	cl_mem d_triTablebuf;

	//vertex neighbor
	float *m_zerobuf;
	int *m_facestructure;
	int *m_facex;
	int *m_facey;
	int *m_facez;
	float *m_vertexstructure;
	float *m_verticex;
	float *m_verticey;
	float *m_verticez;
	int **m_NU;
	int *m_NU_length;
	int *m_neighborarray;
	int *m_neighborarraynum;

	//vertex neighbor GPU
	cl_mem d_filterNextInput[MAXITERTIMES];
	cl_mem d_vertexUD;
	cl_mem d_vertexFiltered;
	cl_mem d_face;
	cl_mem d_neighborarray;
	cl_mem d_neighborarraynum;
};


#endif // !_MARCHINGCUBE_H_