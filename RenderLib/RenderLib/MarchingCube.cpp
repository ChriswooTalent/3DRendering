// MarchingCube:  
// 3D Surface Construction Algorithm, choose a iso value in the volume data and reconstruct the 3D Image.  
//    
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
// 
#include "MarchingCube.h"
#include "ScanApple.h" 
#include "MCTables.h"
#include "OpenGLBase.h"
#include "ImageBasicProcess.h"
/*#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>*/

using namespace openglRendering;
using namespace std;
using namespace RenderingDataTypes;

// shader for displaying floating-point texture
static const char *shader_code =
"!!ARBfp1.0\n"
"TEX result.color, fragment.texcoord, texture[0], 2D; \n"
"END";

//kernel index
enum MCKernelMethods
{
	CLASSIFYVOXEL = 0,
	COMPACTVOXELS = 1,
	VERTCOLOR = 2,
	CLEARPARAMBUF = 3,
	MESHFILTERING = 4,
	GENERATENEWVERTS = 5
};

//kernel names
static const char* MCKernelNames[] =
{
	"classifyVoxelMem",
	"compactVoxels",
	"generateTriangles2WithColor",
	"ClearReferenceBuf",
	"MeshFiltering",
	"GenerateNewVertexAndNorm",
};

MarchingCube::MarchingCube()
{
	m_KernelCount = sizeof(MCKernelNames) / sizeof(char *);
	m_kernelFile = MCHKernelFile;
	m_Sliceobj = new Rotateslice();
	string filenames = m_Sliceobj->GetKernelFileNames();
	g_glInterop = false;
	d_voxelVerts = 0;
	d_voxelVertsScan = 0;
	d_voxelOccupied = 0;
	d_voxelOccupiedScan = 0;
	d_compVoxelArray = 0;
	d_numVertsTablebuf = 0;
	d_triTablebuf = 0;
	IsoValue = 0.0f;
	dIsoValue = 0.0f;
	IsoUp = 0.0f;
	IsoBot = 0.0f;
	//vbo
	posVbo = 0;
	normalVbo = 0;
	colorVbo = 0;
	posVboBone = 0;
	normalVboBone = 0;
	colorVboBone = 0;
	posVboMask = 0;
	normalVboMask = 0;
	colorVboMask = 0;

	//data on device
	d_posbuf = 0;
	d_normbuf = 0;
	d_pos = 0;
	d_normal = 0;
	d_color = 0;
	d_posBone = 0;
	d_normalBone = 0;
	d_colorBone = 0;
	d_posMask = 0;
	d_normalMask = 0;
	d_colorMask = 0;
	d_voxelVerts = 0;
	d_voxelVertsScan = 0;
	d_voxelOccupied = 0;
	d_voxelOccupiedScan = 0;
	d_compVoxelArray = 0;
	// tables
	d_numVertsTablebuf = 0;
	d_triTablebuf = 0;

	//vertex neighbor gpu
	d_vertexUD = 0;
	d_vertexFiltered = 0;
	d_face = 0;
	d_neighborarray = 0;
	d_neighborarraynum = 0;
	for (int i = 0; i < MAXITERTIMES + 1; i++)
	{
		d_filterNextInput[i] = 0;
	}

	//vertex neighbor
	m_facex = NULL;
	m_facey = NULL;
	m_facez = NULL;
	m_verticex = NULL;
	m_verticey = NULL;
	m_verticez = NULL;
	m_NU = NULL;
	m_NU_length = NULL;
	m_neighborarray = NULL;
	m_neighborarraynum = NULL;
	m_UDVertsnum = 0;
	m_FaceCount = 0;
	maxVerts = 0;
}

MarchingCube::~MarchingCube()
{
	MCOpenCLRelease();
	for (int i = 0; i < (int)maxVerts; i++)
	{
		free(m_NU[i]);
	}
	free(m_NU);
	free(m_NU_length);
	free(m_neighborarray);
	free(m_neighborarraynum);
	free(m_verticex);
	free(m_verticey);
	free(m_verticez);
	free(m_facex);
	free(m_facey);
	free(m_facez);
	free(m_vertexstructure);
	free(m_facestructure);
	free(m_zerobuf);
}

void MarchingCube::MCOpenCLRelease()
{
	deleteVBO(&posVbo, d_pos);
	deleteVBO(&normalVbo, d_normal);
	deleteVBO(&colorVbo, d_color);
	deleteVBO(&posVboBone, d_posBone);
	deleteVBO(&normalVboBone, d_normalBone);
	deleteVBO(&colorVboBone, d_colorBone);
	deleteVBO(&posVboMask, d_posMask);
	deleteVBO(&normalVboMask, d_normalMask);
	deleteVBO(&colorVboMask, d_colorMask);

	if (d_triTablebuf) clReleaseMemObject(d_triTablebuf);
	if (d_numVertsTablebuf) clReleaseMemObject(d_numVertsTablebuf);

	if (d_voxelVerts) clReleaseMemObject(d_voxelVerts);
	if (d_voxelVertsScan) clReleaseMemObject(d_voxelVertsScan);
	if (d_voxelOccupied) clReleaseMemObject(d_voxelOccupied);
	if (d_voxelOccupiedScan) clReleaseMemObject(d_voxelOccupiedScan);
	if (d_compVoxelArray) clReleaseMemObject(d_compVoxelArray);
	if (d_vertexUD) clReleaseMemObject(d_vertexUD);
	if (d_face) clReleaseMemObject(d_face);
	if (d_neighborarray) clReleaseMemObject(d_neighborarray);
	if (d_neighborarraynum) clReleaseMemObject(d_neighborarraynum);
	if (d_vertexFiltered) clReleaseMemObject(d_vertexFiltered);
	if (d_posbuf) clReleaseMemObject(d_posbuf);
	if (d_normbuf) clReleaseMemObject(d_normbuf);

	for (int i = 0; i < m_MAXIterTimes + 1; i++)
	{
		if (d_filterNextInput[i]) clReleaseMemObject(d_filterNextInput[i]);
	}

	closeScanAPPLE();

	cout << "memory release done" << endl;
}

void MarchingCube::ClearVertexNeighborBuf()
{
	memset(m_verticex, 0, m_UDVertsnum*sizeof(float));
	memset(m_verticex, 0, m_UDVertsnum*sizeof(float));
	memset(m_verticex, 0, m_UDVertsnum*sizeof(float));
	memset(m_vertexstructure, 0, m_UDVertsnum * 4 * sizeof(float));
	memset(m_facex, 0, m_FaceCount *sizeof(int));
	memset(m_facex, 0, m_FaceCount *sizeof(int));
	memset(m_facex, 0, m_FaceCount *sizeof(int));
	memset(m_facestructure, 0, m_FaceCount * 4 * sizeof(int));
	memset(m_neighborarray, 0, m_UDVertsnum * MAXNEIGHBORNUMUD * sizeof(int));
	memset(m_neighborarraynum, 0, m_UDVertsnum * sizeof(int));
	for (int i = 0; i < (int)m_UDVertsnum; i++) {
		m_NU_length[i] = 0;
		m_NU[i] = (int *)malloc(MAXNEIGHBORNUM * sizeof(int));
		memset(m_NU[i], 0, MAXNEIGHBORNUM * sizeof(int));
	}
	for (int i = 0; i < m_MAXIterTimes + 1; i++)
	{
		error = clEnqueueWriteBuffer(m_Queue, d_filterNextInput[i], CL_TRUE, 0, 4 * m_UDVertsnum * sizeof(float), m_vertexstructure, 0, 0, NULL);
		check_CL_error(error, "d_filterNextInput clear failed");
	}
	error = clEnqueueWriteBuffer(m_Queue, d_vertexUD, CL_TRUE, 0, 4 * m_UDVertsnum * sizeof(float), m_vertexstructure, 0, 0, NULL);
	check_CL_error(error, "d_vertexUD clear failed");
	error = clEnqueueWriteBuffer(m_Queue, d_face, CL_TRUE, 0, m_FaceCount * 4 * sizeof(int), m_facestructure, 0, 0, NULL);
	check_CL_error(error, "d_face clear failed");
	error = clEnqueueWriteBuffer(m_Queue, d_neighborarray, CL_TRUE, 0, m_UDVertsnum * MAXNEIGHBORNUMUD * sizeof(int), m_neighborarray, 0, 0, NULL);
	check_CL_error(error, "d_neighborarray clear failed");
	error = clEnqueueWriteBuffer(m_Queue, d_neighborarraynum, CL_TRUE, 0, m_UDVertsnum * sizeof(int), m_neighborarraynum, 0, 0, NULL);
	check_CL_error(error, "d_neighborarraynum clear failed");
	error = clEnqueueWriteBuffer(m_Queue, d_vertexFiltered, CL_TRUE, 0, 4 * m_UDVertsnum * sizeof(float), m_vertexstructure, 0, 0, NULL);
	check_CL_error(error, "d_vertexFiltered clear failed");
	error = clEnqueueWriteBuffer(m_Queue, d_posbuf, CL_TRUE, 0, maxVerts * sizeof(float), m_zerobuf, 0, 0, NULL);
	check_CL_error(error, "d_posbuf clear failed");
	error = clEnqueueWriteBuffer(m_Queue, d_normbuf, CL_TRUE, 0, maxVerts * sizeof(float), m_zerobuf, 0, 0, NULL);
	check_CL_error(error, "d_normbuf clear failed");
}

void MarchingCube::GLDataRendering(cl_float translate[4], cl_float clrotate[4], int orderflag)
{
	// Common display code path
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set view matrix
	glMatrixMode(GL_MODELVIEW);
	//将当前变换矩阵(单位阵)压入堆栈
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(translate[0], translate[1], translate[2]);
	glRotatef(clrotate[0], 1.0, 0.0, 0.0);
	glRotatef(clrotate[1], 0.0, 1.0, 0.0);

	glScalef(m_ScaleParam, m_ScaleParam, m_ScaleParam);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_MULTISAMPLE);
	bool regflag = GetRegFinshedFlag();
	bool regrenderfinishflag = m_RegFinishRenderFlag;
	// render
	if ((regflag) && (!regrenderfinishflag) && m_SliceRenderFlag)
	{
		GetRealTimeNDIRecord();
		m_Sliceobj->ClearPboBuffer();
		m_Sliceobj->GetSlicePlane(d_volume);
		if (m_NDIConnectedFlag)
		{
			if (m_Sliceobj->SliceBorderConfine())
			{
				if (!m_PboSWflag)
				{
					RenderTexSlice(m_Texture[1], 1);
				}
				else
				{
					RenderTexSlicePBO(m_Texture[0]);
				}

				if (m_PunctureLineRenderFlag)
				{
					// render punc
					//RenderingPunctureLine();
					RenderingProbesensor(m_ProbeTexture, orderflag);
				}
			}
		}
		else
		{
			if (!m_PboSWflag)
			{
				RenderTexSlice(m_Texture[1], 1);
			}
			else
			{
				RenderTexSlicePBO(m_Texture[0]);
			}

			if (m_PunctureLineRenderFlag[0])
			{
				// render punc
				RenderingProbesensor(m_ProbeTexture, orderflag);
			}
			//RenderingPunctureLine();
		}
	}

	RenderProcess();
	glPopMatrix();//绘制完成后，恢复单位阵状态

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);// 画完后disable掉  
	glDisable(GL_MULTISAMPLE);

	glutSwapBuffers();
	glutReportErrors();
}

void MarchingCube::WGLDataRendering(int orderflag)
{
	gluLookAt(m_EyeCenterPt.x, m_EyeCenterPt.y, m_EyeCenterPt.z, m_EyeFoucusCenterPt.x, m_EyeFoucusCenterPt.y, m_EyeFoucusCenterPt.z,
		m_HeadDirection.x, m_HeadDirection.y, m_HeadDirection.z);
	// render
	glScalef(m_ScaleParam, m_ScaleParam, m_ScaleParam);
	bool regflag = GetRegFinshedFlag();
	bool regrenderfinishflag = m_RegFinishRenderFlag;
	//regflag = true;
	if ((regflag) && (!regrenderfinishflag)&&m_SliceRenderFlag)
	{
		GetRealTimeNDIRecord();
		m_Sliceobj->ClearPboBuffer();
		m_Sliceobj->GetSlicePlane(d_volume);
		if (m_NDIConnectedFlag)
		{
			if (m_Sliceobj->SliceBorderConfine())
			{
				if (!m_PboSWflag)
				{
					RenderTexSlice(m_Texture[1], 1);
				}
				else
				{
					RenderTexSlicePBO(m_Texture[0]);
				}
				if (m_PunctureLineRenderFlag[0])
				{
					//RenderingProbesensor(m_ProbeTexture, orderflag);
				}
				// render punc
				RenderingPunctureLine(0);
			}
			for (int k = 1; k < m_ValidSensorCount; k++)
			{
				RenderingPunctureLine(k);
			}
		}
		else
		{
			if (!m_PboSWflag)
			{
				RenderTexSlice(m_Texture[1], 1);
			}
			else
			{
				RenderTexSlicePBO(m_Texture[0]);
			}
			if (m_PunctureLineRenderFlag[0])
			{
				RenderingProbesensor(m_ProbeTexture, orderflag);
			}
			// render punc
			RenderingPunctureLine(0);
		}
	}
	if ((regflag) && (!regrenderfinishflag))
	{
		if (!m_NDIConnectedFlag)
		{
			for (int k = 1; k < m_ValidSensorCount; k++)
			{
				RenderingPunctureLine(k);
			}
		}
	}
	RenderProcess();
}

void MarchingCube::WGLSliceRendering(int orderflag)
{
	gluLookAt(m_EyeCenterPt.x, m_EyeCenterPt.y, m_EyeCenterPt.z, m_EyeFoucusCenterPt.x, m_EyeFoucusCenterPt.y, m_EyeFoucusCenterPt.z,
		m_HeadDirection.x, m_HeadDirection.y, m_HeadDirection.z);
	// render
	glScalef(m_ScaleParam, m_ScaleParam, m_ScaleParam);
	GLfloat modelView[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
	Render2DSlicePBO(m_Texture[3]);
	//RenderTexSlice(m_Texture[1], 1);
	//RenderTexSlice(m_Texture[1], 1);
	SetLightsForRendering();
}

void MarchingCube::KeyBoard()
{
}

void MarchingCube::TextureInit()
{
}

void MarchingCube::KernelInit()
{
	BuildKernel(MCKernelNames);

	// Setup ScanApple
	int scanflag = initScanAPPLE(m_Context, m_Queue, m_Device);
	if (scanflag < 0)
	{
		check_CL_error(-1, "Error: Failed to init Scan");
	}
	string filenames = m_Sliceobj->GetKernelFileNames();
	m_Sliceobj->SliceOpenCLini(m_Context, m_Queue, m_Device);

	m_VoFilterObj->VolumeFilterOpenCLini(m_Context, m_Queue, m_Device);
}

void MarchingCube::openglMaterialInit()
{
	// default initialization
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	gl_Shader = compileASMShader(GL_FRAGMENT_PROGRAM_ARB, shader_code);
}

int MarchingCube::OpenGLBufferInit()
{
	openglMaterialInit();
	maxVerts = GridSize[0] * GridSize[1] * GridSize[2]*4;
	m_NumVoxels = GridSize[0] * GridSize[1] * GridSize[2];
	InitScanAPPLEMem(m_NumVoxels);
	//table device mem
	unsigned int tritablesize = sizeof(uint)* 16 * 256;
	unsigned int nvertsablesize = sizeof(uint)* 256;
	d_triTablebuf = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, tritablesize, 0, &error);
	check_CL_error(error, "d_triTablebuf created failed");
	d_numVertsTablebuf = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, nvertsablesize, 0, &error);
	check_CL_error(error, "d_numVertsTablebuf created failed");
	error = clEnqueueWriteBuffer(m_Queue, d_triTablebuf, CL_TRUE, 0, tritablesize, mctriTable, 0, 0, NULL);
	check_CL_error(error, "d_triTablebuf init failed");
	error = clEnqueueWriteBuffer(m_Queue, d_numVertsTablebuf, CL_TRUE, 0, nvertsablesize, mcnumVertsTable, 0, 0, NULL);
	check_CL_error(error, "d_numVertsTablebuf init failed");

	// allocate device memory
	unsigned int memSize = sizeof(float)* m_NumVoxels;
	d_voxelVerts = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, memSize, 0, &error);
	check_CL_error(error, "d_voxelVerts created failed");
	d_voxelVertsScan = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, memSize, 0, &error);
	check_CL_error(error, "d_voxelVertsScan created failed");
	d_voxelOccupied = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, memSize, 0, &error);
	check_CL_error(error, "d_voxelOccupied created failed");
	d_voxelOccupiedScan = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, memSize, 0, &error);
	check_CL_error(error, "d_voxelOccupiedScan created failed");
	d_compVoxelArray = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, memSize, 0, &error);
	check_CL_error(error, "d_compVoxelArray created failed");

	d_posbuf = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, maxVerts*sizeof(float), 0, &error);
	d_normbuf = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, maxVerts*sizeof(float), 0, &error);

	// create VBOs,skin use less points
	createVBO(&posVbo, maxVerts*sizeof(float), d_pos);
	createVBO(&normalVbo, maxVerts*sizeof(float), d_normal);
	//createVBO(&colorVbo, maxVerts*sizeof(float) * 2, d_color);

	createVBO(&posVboBone, maxVerts*sizeof(float), d_posBone);
	createVBO(&normalVboBone, maxVerts*sizeof(float), d_normalBone);
	//createVBO(&colorVbo, maxVerts*sizeof(float) * 2, d_color);

	int maskvertsnum = GridSize[0] * GridSize[1] * m_InterpSliceNum;
	createVBO(&posVboMask, maskvertsnum*sizeof(float) / 4, d_posMask);
	createVBO(&normalVboMask, maskvertsnum*sizeof(float) / 4, d_normalMask);
	//createVBO(&colorVbo, maskvertsnum*sizeof(float) / 4, d_color);

	//vertex neighbor
	m_UDVertsnum = maxVerts / 30;
	m_FaceCount = maxVerts / 10;
	m_zerobuf = (float *)malloc(maxVerts * sizeof(float));
	m_facex = (int *)malloc(m_FaceCount *sizeof(int));
	m_facey = (int *)malloc(m_FaceCount *sizeof(int));
	m_facez = (int *)malloc(m_FaceCount *sizeof(int));
	m_facestructure = (int *)malloc(4 * m_FaceCount *sizeof(int));
	m_verticex = (float *)malloc(m_UDVertsnum*sizeof(float));
	m_verticey = (float *)malloc(m_UDVertsnum*sizeof(float));
	m_verticez = (float *)malloc(m_UDVertsnum*sizeof(float));
	m_vertexstructure = (float *)malloc(4 * m_UDVertsnum*sizeof(float));
	memset(m_zerobuf, 0, maxVerts * sizeof(float));
	memset(m_verticex, 0, m_UDVertsnum*sizeof(float));
	memset(m_verticex, 0, m_UDVertsnum*sizeof(float));
	memset(m_verticex, 0, m_UDVertsnum*sizeof(float));
	memset(m_vertexstructure, 0, m_UDVertsnum * 4 * sizeof(float));
	memset(m_facex, 0, m_FaceCount *sizeof(int));
	memset(m_facex, 0, m_FaceCount *sizeof(int));
	memset(m_facex, 0, m_FaceCount *sizeof(int));
	memset(m_facestructure, 0, m_FaceCount * 4 * sizeof(int));

	m_neighborarray = (int *)malloc(m_UDVertsnum * MAXNEIGHBORNUMUD * sizeof(int));
	m_neighborarraynum = (int *)malloc(m_UDVertsnum * sizeof(int));
	memset(m_neighborarray, 0, m_UDVertsnum * MAXNEIGHBORNUMUD * sizeof(int));
	memset(m_neighborarraynum, 0, m_UDVertsnum * sizeof(int));

	m_NU = (int **)malloc(m_UDVertsnum* sizeof(int*));
	m_NU_length = (int *)malloc(m_UDVertsnum* sizeof(int));
	for (int i = 0; i < (int)m_UDVertsnum; i++) {
		m_NU_length[i] = 0;
		m_NU[i] = (int *)malloc(MAXNEIGHBORNUM * sizeof(int));
		memset(m_NU[i], 0, MAXNEIGHBORNUM * sizeof(int));
	}

	for (int i = 0; i < m_MAXIterTimes + 1; i++)
	{
		d_filterNextInput[i] = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, 4 * m_UDVertsnum * sizeof(float), 0, &error);
		check_CL_error(error, "d_filterNextInput created failed");
	}
	d_vertexUD = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, 4 * m_UDVertsnum*sizeof(float), 0, &error);
	check_CL_error(error, "d_vertexUD created failed");
	d_vertexFiltered = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, 4 * m_UDVertsnum*sizeof(float), 0, &error);
	check_CL_error(error, "d_vertexFiltered created failed");
	d_face = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, 4 * m_FaceCount * sizeof(int), 0, &error);
	check_CL_error(error, "d_face created failed");
	d_neighborarray = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, m_UDVertsnum * MAXNEIGHBORNUMUD * sizeof(int), 0, &error);
	check_CL_error(error, "d_neighborarray created failed");
	d_neighborarraynum = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, m_UDVertsnum * sizeof(int), 0, &error);
	check_CL_error(error, "d_neighborarraynum created failed");

	return 1;
}

float MarchingCube::GetNormIso(float stdiso)
{
	float result = 0.0f;
	float vmin = 0.0f;
	float vmax = 0.0f;
	vmin = (m_WindowCenter - 0.5f) - (m_WindowWidth - 1.0f) / 2.0f;
	vmax = (m_WindowCenter - 0.5f) + (m_WindowWidth - 1.0f) / 2.0f;
	result = (stdiso - vmin) / (vmax - vmin);
	return result;
}

void MarchingCube::SetRenderingFeature(RenderParam param)
{
	IsoValue = GetNormIso(param.IsoValue);
	IsoValueBone = GetNormIso(param.IsoValueBone);
	IsoValueMask = GetNormIso(param.IsoValueMask);
	dIsoValue = param.DisoValue;
	IsoUp = GetNormIso(param.IsoUp);
	IsoBot = GetNormIso(param.IsoBot);
}

void MarchingCube::GetRenderingFeature(RenderParam &param)
{
	param.IsoValue = IsoValue;
	param.IsoValueBone = IsoValueBone;
	param.IsoValueMask = IsoValueMask;
	param.DisoValue = dIsoValue;
	param.IsoUp = IsoUp;
	param.IsoBot = IsoBot;
}

//animation
void MarchingCube::animation()
{
	/*if (dIsoValue == 0.0f)
	{
		m_ComputeBMFlag = false;
		return;
	}
	if (m_AnimateFlag)
	{
		IsoValueBone += dIsoValue;
		if (IsoValueBone < IsoBot)
		{
			IsoValueBone = IsoBot;
			dIsoValue *= -1.0f;
		}
		else if (IsoValueBone > IsoUp)
		{
			IsoValueBone = IsoUp;
			dIsoValue *= -1.0f;
		}
	}
	m_ComputeBMFlag = true;*/
}

//GetVbo
void MarchingCube::GetPosVbo(GLuint &pvbo)
{
	pvbo = posVbo;
}

void MarchingCube::GetColorVbo(GLuint &cvbo)
{
	cvbo = colorVbo;
}

void MarchingCube::GetNormalVbo(GLuint &nvbo)
{
	nvbo = normalVbo;
}

////////////////////////////////////////////////////////////////////////////////
//! Create VBO
////////////////////////////////////////////////////////////////////////////////
void MarchingCube::createVBO(GLuint* vbo, unsigned int size, cl_mem &vbo_cl)
{
	// create buffer object
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	// initialize buffer object
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glutReportErrors();

	vbo_cl = clCreateFromGLBuffer(m_Context, CL_MEM_WRITE_ONLY, *vbo, &error);
	check_CL_error(error, CL_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
//! Delete VBO
////////////////////////////////////////////////////////////////////////////////
void MarchingCube::deleteVBO(GLuint* vbo, cl_mem vbo_cl)
{
	if (vbo_cl) clReleaseMemObject(vbo_cl);

	if (*vbo) {
		glBindBuffer(1, *vbo);
		glDeleteBuffers(1, vbo);

		*vbo = 0;
	}
}

void MarchingCube::SetRenderFeature()
{
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);        //启动颜色材质
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// good old-fashioned fixed function lighting
	float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);

	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
	//肤色
	glColor4f(m_SkinColor[0], m_SkinColor[1], m_SkinColor[2], m_SkinColor[3]);
}

void MarchingCube::SetRenderFeatureNeedle()
{
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);        //启动颜色材质
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position0[] = { 100.0, 100.0, 100.0, 0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); //环境光
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse); //漫射光
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular); //镜面反射
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0); //光照位置
	/*GLfloat mat_ambient[] = { 0.192250, 0.192250, 0.192250, 1.000000, };
	GLfloat mat_diffuse[] = { 0.507540, 0.507540, 0.507540, 1.000000 };
	GLfloat mat_specular[] = { 0.508273, 0.508273, 0.508273, 1.000000 };
	GLfloat mat_shininess[] = { 51.200001 }; //材质RGBA镜面指数，数值在0～128范围内*/

	GLfloat mat_ambient[] = { 0.231250, 0.231250, 0.231250, 1.000000 };
	GLfloat mat_diffuse[] = { 0.277500, 0.277500, 0.277500, 1.000000 };
	GLfloat mat_specular[] = { 0.773911, 0.773911, 0.773911, 1.000000 };
	GLfloat mat_shininess[] = { 89.599998 }; //材质RGBA镜面指数，数值在0～128范围内

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	//肤色
	glColor4f(0.0f, 0.79, 0.87, 1.0f);
}

void MarchingCube::SetRenderFeatureSlice()
{
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);        //启动颜色材质
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// good old-fashioned fixed function lighting
	float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);    //最开始颜色材质对应的是ambient的。所以要给为diffuse
	//切片白色
	glColor4f(m_BoneColor[0], m_BoneColor[1], m_BoneColor[2], m_BoneColor[3]);
}

void MarchingCube::SetRenderFeatureBone()
{
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);        //启动颜色材质
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// good old-fashioned fixed function lighting
	float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);    //最开始颜色材质对应的是ambient的。所以要给为diffuse
	//glColor4f(0, 0, 0, 1);
	//骨头画白色
	glColor4f(m_BoneColor[0], m_BoneColor[1], m_BoneColor[2], m_BoneColor[3]);
}

void MarchingCube::SetRenderFeatureMask()
{
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);        //启动颜色材质
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	float black[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat afAmbientWhite[] = { 0.25, 0.25, 0.25, 1.00 };   // 周围 环绕 白 
	GLfloat afAmbientRed[] = { 0.25, 0.00, 0.00, 1.00 };     // 周围 环绕 红
	GLfloat afDiffuseWhite[] = { 0.75, 0.75, 0.75, 1.00 };   // 漫射 白
	GLfloat afDiffuseRed[] = { 0.75, 0.00, 0.00, 0.00 };     // 漫射 红
	GLfloat afSpecularRed[] = { 1.00, 0.25, 0.25, 1.00 };    // 反射 红
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, afAmbientRed);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, afDiffuseRed);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
	glMaterialf(GL_FRONT, GL_SHININESS, 5.0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);    //最开始颜色材质对应的是ambient的。所以要给为diffuse
	//器官画红色
	glColor4f(m_MaskColor[0], m_MaskColor[1], m_MaskColor[2], m_MaskColor[3]);
}

////////////////////////////////////////////////////////////////////////////////
//! Rendering
////////////////////////////////////////////////////////////////////////////////
void MarchingCube::renderIsosurface(GLuint pvbo, GLuint nvbo, GLuint cvbo)
{
	glBindBuffer(GL_ARRAY_BUFFER, pvbo);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, nvbo);
	glNormalPointer(GL_FLOAT, sizeof(float)* 4, 0);
	glEnableClientState(GL_NORMAL_ARRAY);

	glDrawArrays(GL_TRIANGLES, 0, totalVerts);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MarchingCube::openclScanApple(cl_mem d_voxelOccupiedScan, cl_mem d_voxelOccupied, int numVoxels)
{
	ScanAPPLEProcess(d_voxelOccupiedScan, d_voxelOccupied, numVoxels);
}

void MarchingCube::launch_classifyVoxel(dim3 grid, dim3 threads, cl_mem voxelVerts, cl_mem voxelOccupied, cl_mem volume,
	cl_uint gridSize[4], uint numVoxels,
	cl_float voxelSize[4], float isoValue,
	cl_uint offsetx, cl_uint offsety, cl_uint offsetz,
	cl_uint endx, cl_uint endy, cl_uint endz)
{
	unsigned int k = CLASSIFYVOXEL;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelVerts);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelOccupied);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &volume);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &numVoxels);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_float), voxelSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &isoValue);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_numVertsTablebuf);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &offsetx);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &offsety);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &offsetz);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &endx);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &endy);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &endz);
	check_CL_error(error, "Setting launch_classifyVoxel failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);

	check_CL_error(error, "launch_classifyVoxel failed!");
}

void MarchingCube::launch_compactVoxels(dim3 grid, dim3 threads, cl_mem compVoxelArray, cl_mem voxelOccupied, cl_mem voxelOccupiedScan, uint numVoxels)
{
	unsigned int k = COMPACTVOXELS;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &compVoxelArray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelOccupied);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelOccupiedScan);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &numVoxels);
	check_CL_error(error, "Setting launch_compactVoxels failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_compactVoxels failed!");
}

void MarchingCube::launch_generateTriangles2WithColor(dim3 grid, dim3 threads,
	cl_mem pos, cl_mem norm, cl_mem color, cl_mem compactedVoxelArray, cl_mem numVertsScanned, cl_mem volume,
	float upperleftx, float upperlefty, float upperleftz,
	cl_uint gridSize[4], cl_float voxelSize[4], float isoValue,
	float offsetx, float offsety, float offsetz, float xcenteroffset, float ycenteroffset, float zcenteroffset,
	UINT32 activeVoxels, UINT32 maxVerts)
{
	/*voxelSize[0] = 1.0f;
	voxelSize[1] = 1.0f;
	voxelSize[2] = 1.0f;
	upperleftx = 0.0f;
	upperlefty = 0.0f;*/
	//upperleftz[0] = 0.0f;
	unsigned int k = VERTCOLOR;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &pos);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &norm);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &compactedVoxelArray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &numVertsScanned);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &volume);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_xstartoffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_ystartoffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &upperleftx);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &upperlefty);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &upperleftz);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_float), voxelSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &isoValue);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &activeVoxels);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &maxVerts);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &offsetx);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &offsety);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &offsetz);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &xcenteroffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &ycenteroffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &zcenteroffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_numVertsTablebuf);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_triTablebuf);
	check_CL_error(error, "Setting launch_generateTriangles2WithColor failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_generateTriangles2MemNTRefCor failed!");
}

void MarchingCube::launch_MeshFiltering(dim3 grid, dim3 threads, cl_mem vertexout, cl_mem nextinput, cl_mem vertexin, cl_mem neighborarray, cl_mem neighbornum,
	cl_uint maxneinum, cl_float lamdaCoef, cl_uint vertexnum, cl_uint facenum)
{
	unsigned int k = MESHFILTERING;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &vertexout);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &nextinput);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &vertexin);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &neighborarray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &neighbornum);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &maxneinum);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &lamdaCoef);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &vertexnum);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &facenum);
	check_CL_error(error, "Setting launch_MeshFiltering failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_MeshFiltering failed!");
}

void MarchingCube::launch_generateNewVertex(dim3 grid, dim3 threads, cl_mem pos, cl_mem norm, cl_mem vertexin, cl_mem facein, cl_uint facenum, cl_uint maxverts)
{
	unsigned int k = GENERATENEWVERTS;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &pos);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &norm);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &vertexin);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &facein);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &facenum);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &maxverts);
	check_CL_error(error, "Setting launch_generateNewVertex failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_generateNewVertex failed!");
}

void MarchingCube::launch_clearDeviceBuffer(dim3 grid, dim3 threads,
	cl_mem voxelVerts, cl_mem voxelOccupied, cl_mem voxelOccupiedscan,
	cl_mem compactedVoxelArray, cl_mem numVertsScanned, cl_uint numVoxels)
{
	unsigned int k = CLEARPARAMBUF;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelVerts);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelOccupied);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &voxelOccupiedscan);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &compactedVoxelArray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &numVertsScanned);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(uint), &numVoxels);
	check_CL_error(error, "Setting launch_clearDeviceBuffer failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_clearDeviceBuffer failed!");
}

/* The matlab mex function */
void MarchingCube::GetNeiborInfo(int *FacesA, int *FacesB, int *FacesC, int **NU, int *NU_length, int FacesN, int VertexN, int maxnbs, int *NeArray, int *NeArrayNums)
{
	/* Neighbour sort list of one vertex */
	int *Pneig, PneighPos, PneighStart, *Pneighf;

	/* Loop variable */
	int i, j, index1, index2;

	/* Found */
	int found, found2;

	/* face vertices int */
	int vertexa, vertexb, vertexc;

	/* Loop throuh all faces */
	for (i = 0; i<FacesN; i++) {
		/* Add the neighbors of each vertice of a face
		* to his neighbors list. */

		vertexa = (int)FacesA[i]; vertexb = (int)FacesB[i]; vertexc = (int)FacesC[i];

		/*if (NU_length[vertexa]>18) {
		NU[vertexa] = (int *)realloc(NU[vertexa], (NU_length[vertexa] + 2) * sizeof(int));
		}*/
		NU[vertexa][NU_length[vertexa]] = vertexb; NU[vertexa][NU_length[vertexa] + 1] = vertexc;
		NU_length[vertexa] += 2;

		/*if (NU_length[vertexb]>18) {
		NU[vertexb] = (int *)realloc(NU[vertexb], (NU_length[vertexb] + 2) * sizeof(int));
		}*/
		NU[vertexb][NU_length[vertexb]] = vertexc; NU[vertexb][NU_length[vertexb] + 1] = vertexa;
		NU_length[vertexb] += 2;

		/*if (NU_length[vertexc]>18) {
		NU[vertexc] = (int *)realloc(NU[vertexc], (NU_length[vertexc] + 2) * sizeof(int));
		}*/
		NU[vertexc][NU_length[vertexc]] = vertexa; NU[vertexc][NU_length[vertexc] + 1] = vertexb;
		NU_length[vertexc] += 2;
	}

	/*  Loop through all neighbor arrays and sort them (Rotation same as faces) */

	for (i = 0; i<VertexN; i++) {
		if (NU_length[i]>0) {

			/* Create Matlab array for sorted neighbours of vertex*/
			Pneig = (int *)malloc(NU_length[i] * sizeof(int));
			PneighPos = 0;
			Pneighf = NU[i];

			/* Start with the first vertex or if exist with a unique vertex */
			PneighStart = 0;
			for (index1 = 0; index1<NU_length[i]; index1 += 2) {
				found = 0;
				for (index2 = 1; index2<NU_length[i]; index2 += 2) {
					if (Pneighf[index1] == Pneighf[index2]) {
						found = 1; break;
					}
				}
				if (found == 0) {
					PneighStart = index1; break;
				}
			}

			Pneig[PneighPos] = Pneighf[PneighStart];    PneighPos++;
			Pneig[PneighPos] = Pneighf[PneighStart + 1]; PneighPos++;


			/* Add the neighbours with respect to original rotation */
			for (j = 1 + found; j<(NU_length[i] / 2); j++) {
				found = 0;
				for (index1 = 0; index1<NU_length[i]; index1 += 2) {
					if (Pneighf[index1] == Pneig[PneighPos - 1]) {
						found2 = 0;
						for (index2 = 0; index2<PneighPos; index2++) {
							if (Pneighf[index1 + 1] == Pneig[index2]) { found2 = 1; }
						}
						if (found2 == 0) {
							found = 1;
							Pneig[PneighPos] = Pneighf[index1 + 1];  PneighPos++;
						}
					}
				}
				if (found == 0) /* This only happens with weird edge vertices */
				{
					for (index1 = 0; index1<NU_length[i]; index1 += 2) {
						found2 = 0;
						for (index2 = 0; index2<PneighPos; index2++) {
							if (Pneighf[index1] == Pneig[index2]) { found2 = 1; }
						}
						if (found2 == 0) {
							Pneig[PneighPos] = Pneighf[index1];  PneighPos++;
							if (Pneighf[index1] == Pneig[PneighPos - 1]) {
								found2 = 0;
								for (index2 = 0; index2<PneighPos; index2++) {
									if (Pneighf[index1 + 1] == Pneig[index2]) { found2 = 1; }
								}
								if (found2 == 0) {
									found = 1;
									Pneig[PneighPos] = Pneighf[index1 + 1];  PneighPos++;
								}
							}
						}

					}
				}
			}

			/* Add forgotten neigbours */
			if (PneighPos<NU_length[i]) {
				for (index1 = 0; index1<NU_length[i]; index1++) {
					found2 = 0;
					for (index2 = 0; index2<PneighPos; index2++) {
						if (Pneighf[index1] == Pneig[index2]) { found2 = 1; break; }
					}
					if (found2 == 0) {
						Pneig[PneighPos] = Pneighf[index1];  PneighPos++;
					}
				}
			}

			NeArrayNums[i] = PneighPos;
			for (j = 0; j < PneighPos; j++)
			{
				NeArray[i*maxnbs + j] = Pneig[j];
			}
		}
	}
}

template<typename T>
void WriteFileForMatlab(const char *filename, T *buf, int length)
{
	FILE *fpdata = NULL;
	fopen_s(&fpdata, filename, "wb");
	if (NULL == fpdata)
	{
		fclose(fpdata);
		return;
	}
	fwrite(buf, sizeof(T), length, fpdata);
	fclose(fpdata);
}

bool cmpfuncz(Point3 a, Point3 b)
{
	if (a.z == b.z)
		return a.indsort < b.indsort;
	else
		return a.z < b.z;
}

bool cmpfuncy(Point3 a, Point3 b)
{
	if (a.y == b.y)
		return a.indsort < b.indsort;
	else
		return a.y < b.y;
}

bool cmpfuncx(Point3 a, Point3 b)
{
	if (a.x == b.x)
		return a.indsort < b.indsort;
	else
		return a.x < b.x;
}

template < class T >
void ClearVector(vector< T >& vt)
{
	vector< T > vtTemp;
	vtTemp.swap(vt);
}

void MarchingCube::RemoveDuplicateVerts(vector<Point3> vertsvec, int ftotalverts, vector<Point3> &vertsvec2, vector<int> &fIndmaptable)
{
	Point3 zeropt(0.0f, 0.0f, 0.0f, 0);
	std::vector<Point3> vertsvec1;
	std::vector<Point3> diffvec;
	std::vector<int> indxvec;
	std::vector<int> scantable;
	std::vector<int> indexzvec;
	std::vector<int> indexzvec1;
	vertsvec1.resize(ftotalverts);
	diffvec.resize(ftotalverts);
	indxvec.resize(ftotalverts);
	scantable.resize(ftotalverts);
	indexzvec.resize(ftotalverts);
	indexzvec1.resize(ftotalverts);
	vector<Point3>::iterator pind;
	vector<Point3>::iterator pindright;
	vector<Point3>::iterator diffind;
	vector<Point3>::iterator lastdiffind;
	vector<int>::iterator inditer;
	vector<int>::iterator iterscan;
	vector<int>::iterator iterindex;
	sort(vertsvec.begin(), vertsvec.end(), cmpfuncz);
	int count = 0;
	for (pind = vertsvec.begin(); pind != vertsvec.end(); ++pind)
	{
		vertsvec1[count] = *pind;
		vertsvec1[count].indsort = count;
		indexzvec[count] = (pind->indsort);
		count++;
	}
	sort(vertsvec1.begin(), vertsvec1.end(), cmpfuncy);
	count = 0;
	for (pind = vertsvec1.begin(); pind != vertsvec1.end(); ++pind)
	{
		vertsvec2[count] = *pind;
		vertsvec2[count].indsort = count;
		indexzvec1[count] = (pind->indsort);
		count++;
	}
	sort(vertsvec2.begin(), vertsvec2.end(), cmpfuncx);
	count = 0;
	pindright = vertsvec2.begin();
	lastdiffind = diffvec.begin();
	iterscan = scantable.begin();
	++pindright;
	int sum = 0;
	for (inditer = indxvec.begin(), diffind = diffvec.begin(), pind = vertsvec2.begin(); pindright != vertsvec2.end(); ++pind, ++diffind, ++inditer, ++pindright, ++iterscan)
	{
		if (diffind == diffvec.begin())
		{
			*inditer = 1;
			(*iterscan) = 0;
			(*iterscan) = (*iterscan) + (*inditer);
			fIndmaptable[indexzvec[indexzvec1[pind->indsort]]] = (*iterscan) - 1;
			sum = (*iterscan);
			count++;
			continue;
		}
		Point3 temp1 = (*pind);
		Point3 temp2 = (*pindright);
		Point3 diff = temp2 - temp1;
		(*diffind) = diff;
		*inditer = (!(*lastdiffind == zeropt)) & 1;
		sum = sum + (*inditer);
		(*iterscan) = sum;
		++lastdiffind;
		fIndmaptable[indexzvec[indexzvec1[pind->indsort]]] = (*iterscan) - 1;
		count++;
	}
	fIndmaptable[indexzvec[indexzvec1[pind->indsort]]] = sum - 1;
	vertsvec2.erase(unique(vertsvec2.begin(), vertsvec2.end()), vertsvec2.end());
	ClearVector(vertsvec1);
	ClearVector(diffvec);
	ClearVector(indxvec);
	ClearVector(scantable);
	ClearVector(indexzvec);
	ClearVector(indexzvec1);
}

void MarchingCube::LaunchMeshFiltering(cl_mem vtxfiltered, cl_mem pos, int factiveVoxels, int ftotalverts, int *vertscountout, int *facecountout, int maxverts, int itertimes)
{
	float *h_vertsPosition = (float *)malloc(maxVerts*sizeof(float));
	error = clEnqueueReadBuffer(m_Queue, pos, CL_TRUE, 0, maxVerts * sizeof(float), h_vertsPosition, 0, 0, 0);
	clCheckErrorIP(error, CL_SUCCESS);
#if 1
	FILE *fpverts = 0;
	fopen_s(&fpverts, "fileverts.txt", "w+");
	for (int s = 0; s < totalVerts; s++)
	{
		fprintf_s(fpverts, "%f, %f, %f  \n", h_vertsPosition[s * 4], h_vertsPosition[s * 4 + 1], h_vertsPosition[s * 4 + 2]);
	}
	fclose(fpverts);
#endif
	int vercounts = 0;
	int maxneighnum = MAXNEIGHBORNUMUD;
	int vertexcount = 0;
	int facecount = ftotalverts / 3;

	std::vector<Point3> vertsvec;
	std::vector<Point3> vertsvec2;
	std::vector<int> fIndmaptable;
	vertsvec.resize(ftotalverts);
	vertsvec2.resize(ftotalverts);
	fIndmaptable.resize(ftotalverts);
	int count = 0;
	for (int s = 0; s < facecount; s++)
	{
		m_facex[s] = (s)* 3 + 1;
		m_facey[s] = (s)* 3 + 1 + 1;
		m_facez[s] = (s)* 3 + 2 + 1;
	}
	vercounts = 0;
	for (int s = 0; s < ftotalverts; s++)
	{
		Point3 temp(h_vertsPosition[s * 4], h_vertsPosition[s * 4 + 1], h_vertsPosition[s * 4 + 2], s);
		vertsvec[s] = temp;
		vercounts++;
	}
	free(h_vertsPosition);
	RemoveDuplicateVerts(vertsvec, vercounts, vertsvec2, fIndmaptable);

	vector<Point3> vtTemp;
	vtTemp.swap(vertsvec);

	//vertsvec.clear();
	vercounts = 0;
	for (std::vector<Point3>::iterator pind = vertsvec2.begin(); pind != vertsvec2.end(); ++pind)
	{
		m_verticex[vercounts] = (*pind).x;
		m_verticey[vercounts] = (*pind).y;
		m_verticez[vercounts] = (*pind).z;
		m_vertexstructure[vercounts * 4] = m_verticex[vercounts];
		m_vertexstructure[vercounts * 4 + 1] = m_verticey[vercounts];
		m_vertexstructure[vercounts * 4 + 2] = m_verticez[vercounts];
		m_vertexstructure[vercounts * 4 + 3] = 0.0f;
		vercounts++;
	}

	vector<Point3> vtTemp1;
	vtTemp1.swap(vertsvec2);
	for (int s = 0; s < facecount; s++)
	{

		m_facex[s] = fIndmaptable[(s)* 3];
		m_facey[s] = fIndmaptable[(s)* 3 + 1];
		m_facez[s] = fIndmaptable[(s)* 3 + 2];
		m_facestructure[s * 4] = m_facex[s];
		m_facestructure[s * 4 + 1] = m_facey[s];
		m_facestructure[s * 4 + 2] = m_facez[s];
		m_facestructure[s * 4 + 3] = 0;
	}
	ClearVector(vertsvec);
	ClearVector(vertsvec2);
	ClearVector(fIndmaptable);

	GetNeiborInfo(m_facex, m_facey, m_facez, m_NU, m_NU_length, facecount, vercounts, maxneighnum, m_neighborarray, m_neighborarraynum);
#if 0
	printf("vercounts is %d\n", vercounts);
	printf("facecount is %d\n", facecount);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/verticesx.dat", m_verticex, vercounts);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/verticesy.dat", m_verticey, vercounts);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/verticesz.dat", m_verticez, vercounts);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/facex.dat", m_facex, facecount);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/facey.dat", m_facey, facecount);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/facez.dat", m_facez, facecount);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/neighborarray.dat", m_neighborarray, vercounts * maxneighnum);
	WriteFileForMatlab("D:/Code_local/VTK/smoothpatch_version1b/neighborarraynum.dat", m_neighborarraynum, vercounts);
#endif
	int threads = THREAD;
	int gridnum = vercounts / threads;
	if (vercounts%threads != 0)
	{
		gridnum = gridnum + 1;
	}
	dim3 grid(gridnum, 1, 1);

	error = clEnqueueWriteBuffer(m_Queue, d_filterNextInput[0], CL_TRUE, 0, vercounts * 4 * sizeof(float), m_vertexstructure, 0, 0, NULL);
	check_CL_error(error, "d_filterNextInput[0] init failed");
	error = clEnqueueWriteBuffer(m_Queue, d_vertexUD, CL_TRUE, 0, vercounts * 4 * sizeof(float), m_vertexstructure, 0, 0, NULL);
	check_CL_error(error, "d_vertexUD init failed");
	error = clEnqueueWriteBuffer(m_Queue, d_face, CL_TRUE, 0, facecount * 4 * sizeof(int), m_facestructure, 0, 0, NULL);
	check_CL_error(error, "d_face init failed");
	error = clEnqueueWriteBuffer(m_Queue, d_neighborarraynum, CL_TRUE, 0, vercounts * sizeof(int), m_neighborarraynum, 0, 0, NULL);
	check_CL_error(error, "d_neighborarraynum init failed");
	error = clEnqueueWriteBuffer(m_Queue, d_neighborarray, CL_TRUE, 0, vercounts * maxneighnum * sizeof(int), m_neighborarray, 0, 0, NULL);
	check_CL_error(error, "d_neighborarray init failed");

	for (int i = 0; i < itertimes; i++)
	{
		launch_MeshFiltering(grid, threads, vtxfiltered, d_filterNextInput[i + 1], d_filterNextInput[i], d_neighborarray, d_neighborarraynum, maxneighnum, 1.0, vercounts, facecount);
		check_CL_error(error, "launch_MeshFiltering failed");
	}


#if 0
	float *h_vertsfiltered = (float *)malloc(4 * vercounts*sizeof(float));
	error = clEnqueueReadBuffer(m_Queue, vtxfiltered, CL_TRUE, 0, 4 * vercounts * sizeof(float), h_vertsfiltered, 0, 0, 0);
	clCheckErrorIP(error, CL_SUCCESS);
	for (int s = 0; s < vercounts; s++)
	{
		m_verticex[s] = h_vertsfiltered[s * 4];
		m_verticey[s] = h_vertsfiltered[s * 4 + 1];
		m_verticez[s] = h_vertsfiltered[s * 4 + 2];
	}
	WriteFileForMatlab("D:/Code_local/BoneSkinShared/verticesxfiltered.dat", m_verticex, vercounts);
	WriteFileForMatlab("D:/Code_local/BoneSkinShared/verticesyfiltered.dat", m_verticey, vercounts);
	WriteFileForMatlab("D:/Code_local/BoneSkinShared/verticeszfiltered.dat", m_verticez, vercounts);

	/*FILE *fpvertsfiltered = 0;
	fopen_s(&fpvertsfiltered, "fpvertsfiltered.txt", "w+");
	for (int s = 0; s < vercounts; s++)
	{
		fprintf_s(fpvertsfiltered, "%f, %f, %f  \n", h_vertsfiltered[s * 4], h_vertsfiltered[s * 4 + 1], h_vertsfiltered[s * 4 + 2]);
	}
	fclose(fpvertsfiltered);*/
	free(h_vertsfiltered);
#endif

	*vertscountout = vercounts;
	*facecountout = facecount;
}

////////////////////////////////////////////////////////////////////////////////
//! Run the OPENCL part of the computation
////////////////////////////////////////////////////////////////////////////////
void MarchingCube::computeIsosurface(cl_mem de_volume, cl_mem &de_pos, cl_mem &de_normal, cl_mem &de_color, UINT32 xgridoffset, UINT32 ygridoffset, UINT32 zgridoffset, cl_uint gridSize[4], 
	                                 cl_uint offsetx, cl_uint offsety, cl_uint offsetz, cl_uint endx, cl_uint endy, cl_uint endz,
	                                 float xstartoff, float ystartoff, float zstartoff, float cvoxelSize[4], float isoval, bool meshfiltering_flag, int itertimes)
{
	int threads = THREAD;
	dim3 grid(m_NumVoxels / threads, 1, 1);
	// get around maximum grid size of 65535 in each dimension
	//if (grid.x > 65535) {
	//	grid.y = grid.x / 32768;
	//	grid.x = 32768;
	//}

	// calculate number of vertices need per voxel
	launch_classifyVoxel(grid, threads,
		d_voxelVerts, d_voxelOccupied, de_volume,
		gridSize, m_NumVoxels, VoxelSize, isoval, offsetx, offsety, offsetz, endx, endy, endz);

#if 0
	float* h_voxelverts = (float *)malloc(m_NumVoxels* sizeof(float));
	int err = clEnqueueReadBuffer(m_Queue, d_voxelOccupied, CL_TRUE, 0, m_NumVoxels* sizeof(float), h_voxelverts, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	FILE *fp = NULL;
	fopen_s(&fp, "voxelOccupied.txt", "w+");
	for (int i = 0; i < m_NumVoxels / GridSize[0]; i++)
	{
		for (int j = 0; j < GridSize[0]; j++)
		{
			fprintf_s(fp, "%f ", h_voxelverts[i*GridSize[0] + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	free(h_voxelverts);
#endif

	// scan voxel occupied array

	openclScanApple(d_voxelOccupiedScan, d_voxelOccupied, m_NumVoxels);

#if 0
	float* h_voxelverts1 = (float *)malloc(m_NumVoxels* sizeof(float));
	err = clEnqueueReadBuffer(m_Queue, d_voxelOccupiedScan, CL_TRUE, 0, m_NumVoxels* sizeof(float), h_voxelverts1, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	fp = NULL;
	fopen_s(&fp, "voxelOccupiedscan.txt", "w+");
	for (int i = 0; i < m_NumVoxels / GridSize[0]; i++)
	{
		for (int j = 0; j < GridSize[0]; j++)
		{
			fprintf_s(fp, "%f ", h_voxelverts1[i*GridSize[0] + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	free(h_voxelverts1);
#endif


	// read back values to calculate total number of non-empty voxels
	// since we are using an exclusive scan, the total is the last value of
	// the scan result plus the last value in the input array

	{
		float lastElement, lastScanElement;

		clEnqueueReadBuffer(m_Queue, d_voxelOccupied, CL_TRUE, (m_NumVoxels - 1) * sizeof(float), sizeof(float), &lastElement, 0, 0, 0);
		clEnqueueReadBuffer(m_Queue, d_voxelOccupiedScan, CL_TRUE, (m_NumVoxels - 1) * sizeof(float), sizeof(float), &lastScanElement, 0, 0, 0);

		activeVoxels = (uint)(lastElement + lastScanElement);
	}

	if (activeVoxels == 0) {
		// return if there are no full voxels
		totalVerts = 0;
		return;
	}

	// compact voxel index array
	launch_compactVoxels(grid, threads, d_compVoxelArray, d_voxelOccupied, d_voxelOccupiedScan, m_NumVoxels);


	// scan voxel vertex count array
	openclScanApple(d_voxelVertsScan, d_voxelVerts, m_NumVoxels);

	// readback total number of vertices
	{
		float lastElement, lastScanElement;
		clEnqueueReadBuffer(m_Queue, d_voxelVerts, CL_TRUE, (m_NumVoxels - 1) * sizeof(float), sizeof(float), &lastElement, 0, 0, 0);
		clEnqueueReadBuffer(m_Queue, d_voxelVertsScan, CL_TRUE, (m_NumVoxels - 1) * sizeof(float), sizeof(float), &lastScanElement, 0, 0, 0);

		totalVerts = (uint)(lastElement + lastScanElement);
	}


	cl_mem interopBuffers[] = { de_pos, de_normal };

	dim3 grid2((int)ceil(activeVoxels / (float)NTHREADS), 1, 1);

	int vertscount = 0;
	int facecount = 0;
	// generate triangles, writing to vertex buffers
	if (g_glInterop) {
		// Acquire PBO for OpenCL writing
		glFlush();
		error = clEnqueueAcquireGLObjects(m_Queue, 2, interopBuffers, 0, 0, 0);
		check_CL_error(error, "clEnqueueAcquireGLObjects failed");
	}
	if (meshfiltering_flag)
	{
		//write vbo
		launch_generateTriangles2WithColor(grid2, NTHREADS, d_posbuf, d_normbuf, de_color, d_compVoxelArray, d_voxelVertsScan, de_volume,
			xstartoff, ystartoff, zstartoff,
			GridSize, cvoxelSize, isoval, xgridoffset, ygridoffset, zgridoffset, m_XCenterOffset, m_YCenterOffset, m_ZCenterOffset,
			activeVoxels, maxVerts);
		LaunchMeshFiltering(d_vertexFiltered, d_posbuf, activeVoxels, totalVerts, &vertscount, &facecount, maxVerts, itertimes);

		int gridnum = facecount / threads;
		if (facecount%threads != 0)
		{
			gridnum = gridnum + 1;
		}
		dim3 grid_face(gridnum, 1, 1);
		launch_generateNewVertex(grid_face, threads, de_pos, de_normal, d_vertexFiltered, d_face, facecount, maxVerts);
		ClearVertexNeighborBuf();
	}
	else
	{
		launch_generateTriangles2WithColor(grid2, NTHREADS, de_pos, de_normal, de_color, d_compVoxelArray, d_voxelVertsScan, de_volume,
			xstartoff, ystartoff, zstartoff,
			GridSize, cvoxelSize, isoval, xgridoffset, ygridoffset, zgridoffset, m_XCenterOffset, m_YCenterOffset, m_ZCenterOffset,
			activeVoxels, maxVerts);
	}
#if 0
	float *h_posfiltered = (float *)malloc(maxVerts*sizeof(float) / 8);
	error = clEnqueueReadBuffer(m_Queue, de_pos, CL_TRUE, 0, maxVerts * sizeof(float) / 8, h_posfiltered, 0, 0, 0);
	clCheckErrorIP(error, CL_SUCCESS);

	float *h_normfiltered = (float *)malloc(maxVerts*sizeof(float) / 8);
	error = clEnqueueReadBuffer(m_Queue, de_normal, CL_TRUE, 0, maxVerts * sizeof(float) / 8, h_normfiltered, 0, 0, 0);
	clCheckErrorIP(error, CL_SUCCESS);
	FILE *fposfiltered = 0;
	FILE *fnormfiltered = 0;
	fopen_s(&fposfiltered, "fpos_origin.txt", "w+");
	fopen_s(&fnormfiltered, "fnorm_origin.txt", "w+");
	//fopen_s(&fposfiltered, "fnorm.txt", "w+");
	for (int s = 0; s < totalVerts; s++)
	{
		fprintf_s(fposfiltered, "%f, %f, %f  \n", h_posfiltered[s * 4], h_posfiltered[s * 4 + 1], h_posfiltered[s * 4 + 2]);
		fprintf_s(fnormfiltered, "%f, %f, %f  \n", h_normfiltered[s * 4], h_normfiltered[s * 4 + 1], h_normfiltered[s * 4 + 2]);
	}
	fclose(fposfiltered);
	fclose(fnormfiltered);
	free(h_posfiltered);
	free(h_normfiltered);
#endif
	//release lock
	if (g_glInterop)
	{
		// Transfer ownership of buffer back from CL to GL    
		error = clEnqueueReleaseGLObjects(m_Queue, 2, interopBuffers, 0, 0, 0);
		check_CL_error(error, "clEnqueueReleaseGLObjects failed");
		clFinish(m_Queue);
	}
	launch_clearDeviceBuffer(grid, threads, d_voxelVerts, d_voxelOccupied, d_voxelOccupiedScan, d_compVoxelArray, d_voxelVertsScan, m_NumVoxels);
}

//Reshape func
void MarchingCube::RenderTexSlice(GLuint tex2d, int orderflag)
{
	Point3f pt_tl;
	Point3f pt_tr;
	Point3f pt_bl;
	Point3f pt_br;
	Point3f pt_offset;
	pt_offset.x = m_XCenterOffset;
	pt_offset.y = m_YCenterOffset;
	pt_offset.z = m_ZCenterOffset;
	GetRealBoundCorOfSlice(pt_tl, pt_tr, pt_bl, pt_br, 2);
	float width = m_Sliceobj->CalcDistance(pt_tl, pt_tr);
	float height = m_Sliceobj->CalcDistance(pt_bl, pt_br);
	if ((width == 0) || (height == 0))
	{
		return;
	}
	else
	{
		Init2DTexture();
		//绘制切面的纹理
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glBindTexture(GL_TEXTURE_2D, tex2d);
		pt_tl = pt_tl + pt_offset;
		pt_tr = pt_tr + pt_offset;
		pt_bl = pt_bl + pt_offset;
		pt_br = pt_br + pt_offset;

		glColor3f(1.0, 1.0, 1.0);

		glBegin(GL_QUADS);//绘制场景，提供纹理坐标和几何坐标   
		//纹理坐标：是指使用纹理的哪部分，每个绘制顶点前面跟个纹理坐标，表示整个图元都会被贴图   
		/*glTexCoord2f(0.0f, 0.0f); glVertex3f(-width / 2, -height / 2, -256);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(width / 2, -height / 2, -256);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(width / 2, height / 2, -256);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(width / 2, -height / 2, -256);*/
		glTexCoord2f(0.0f, 0.0f); glVertex3f(pt_tl.x, pt_tl.y, pt_tl.z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(pt_tr.x, pt_tr.y, pt_tr.z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(pt_br.x, pt_br.y, pt_br.z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(pt_bl.x, pt_bl.y, pt_bl.z);

		/*glTexCoord2f(0.0f, 0.0f); glVertex2f(0.2f, 0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(0.8f, 0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(0.8f, 0.8f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.2f, 0.8f);*/


		glEnd();
		glDisable(GL_TEXTURE_2D);    //禁止使用纹理
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void MarchingCube::Render2DSlicePBO(GLuint tex2d)
{
	bool regflag = GetRegFinshedFlag();
	bool regrenderfinishflag = m_RegFinishRenderFlag;
	if ((regflag) && (!regrenderfinishflag))
	{
		GetOriginSlice(m_HostSliceBuf);
		Point3f pt_tl;
		Point3f pt_tr;
		Point3f pt_bl;
		Point3f pt_br;
		Point3f pt_offset;
		pt_offset.x = m_XCenterOffset;
		pt_offset.y = m_YCenterOffset;
		pt_offset.z = m_ZCenterOffset;
		GetRealBoundCorOfSlice(pt_tl, pt_tr, pt_bl, pt_br, 2);
		pt_tl = pt_tl + pt_offset;
		pt_tr = pt_tr + pt_offset;
		pt_bl = pt_bl + pt_offset;
		pt_br = pt_br + pt_offset;
		float width = m_Sliceobj->CalcDistance(pt_tl, pt_tr);
		float height = m_Sliceobj->CalcDistance(pt_tl, pt_bl);
		float scalex = width / WIDTH;
		float scaley = height / HEIGHT;
		SetPixelScale(scalex, scaley);
		if ((width == 0) || (height == 0))
		{
			return;
		}
		else
		{
			glEnable(GL_TEXTURE_2D);

			float base_size = WIDTH /2;
			float base_size1 = HEIGHT/2;
			glBindTexture(GL_TEXTURE_2D, tex2d);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_LUMINANCE, GL_FLOAT, m_HostSliceBuf);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glColor3f(255, 255, 255);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-base_size, -base_size1, -80);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(base_size, -base_size1, -80);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(base_size, base_size1, -80);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-base_size, base_size1, -80);
			glBindTexture(GL_TEXTURE_2D, 0);
			glEnd();

			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		//free(h_pbo);
	}
}

void MarchingCube::RenderTexSlicePBO(GLuint tex2d)
{
	bool regflag = GetRegFinshedFlag();
	bool regrenderfinishflag = m_RegFinishRenderFlag;
	if ((regflag) && (!regrenderfinishflag))
	{
		GetPBO();
		Point3f pt_tl;
		Point3f pt_tr;
		Point3f pt_bl;
		Point3f pt_br;
		Point3f pt_offset;
		pt_offset.x = m_XCenterOffset;
		pt_offset.y = m_YCenterOffset;
		pt_offset.z = m_ZCenterOffset;
		GetRealBoundCorOfSlice(pt_tl, pt_tr, pt_bl, pt_br, 2);
		pt_tl = pt_tl + pt_offset;
		pt_tr = pt_tr + pt_offset;
		pt_bl = pt_bl + pt_offset;
		pt_br = pt_br + pt_offset;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glRasterPos2i(0, 0);
		glClearColor(0.0, 0.0, 0.0, 0.0f);
		glBindTexture(GL_TEXTURE_2D, tex2d);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, slicepbo);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		// draw textured quad
		SetRenderFeatureSlice();
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(pt_tl.x, pt_tl.y, pt_tl.z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(pt_tr.x, pt_tr.y, pt_tr.z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(pt_br.x, pt_br.y, pt_br.z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(pt_bl.x, pt_bl.y, pt_bl.z);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void MarchingCube::RenderRegHintPBO(GLuint tex2d, int bufindex, int orderflag)
{
	if (bufindex == 1)
	{ 
		GetRegProcessFinishedHintPBO();
	}
	else
	{
		GetRegProcessHintPBO();
	}
	Point3f pt_tl;
	Point3f pt_tr;
	Point3f pt_bl;
	Point3f pt_br;
	GetHintOriginFlagCor(pt_tl, pt_tr, pt_bl, pt_br);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos2i(0, 0);
	glClearColor(0.0, 0.0, 0.0, 0.0f);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, regpbo);
	glBindTexture(GL_TEXTURE_2D, tex2d);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, REGDIRECIMWIDTH, REGDIRECIMHEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	// draw textured quad
	glColor4f(1.0, 1.0, 1.0, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, orderflag - 0.0f); glVertex3f(pt_tl.x, pt_tl.y, pt_tl.z);
	glTexCoord2f(1.0f, orderflag - 0.0f); glVertex3f(pt_tr.x, pt_tr.y, pt_tr.z);
	glTexCoord2f(1.0f, orderflag - 1.0f); glVertex3f(pt_br.x, pt_br.y, pt_br.z);
	glTexCoord2f(0.0f, orderflag - 1.0f); glVertex3f(pt_bl.x, pt_bl.y, pt_bl.z);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void MarchingCube::RenderingPunctureLine(int index)
{
	GLfloat puncvector[3] = { 0.0f };
	GLfloat cylindercenter[3] = { 0.0f };
	GLfloat puncneedletop[3] = { 0.0f };
	GLfloat toptemparray[DEFAULTCYLINDERSLICE * 3] = { 0.0f };
	GLfloat bottemparray[DEFAULTCYLINDERSLICE * 3] = { 0.0f };
	GLfloat conecirclearray[DEFAULTCYLINDERSLICE * 3] = { 0.0f };
	int i = index;
	//for (int i = 0; i < m_ValidSensorCount; i++)
	{
		if (m_PunctureLineRenderFlag[i])
		{
			Point3f pt_lineStart;
			Point3f pt_lineEnd;
			//m_RegObj[i]->CalcingPunctureLineStart(pt_lineStart);
			//m_RegObj[i]->CalcingPunctureLineEnd(pt_lineEnd);
			//m_RegObj[i]->CalcingCylinderPtArray();
			//m_RegObj[i]->GetPuncNeedlePtArray(toptemparray, bottemparray, cylindercenter, conecirclearray, puncneedletop);

			m_RegObj[i]->CalcingPunctureLineStartSpace(pt_lineStart);
			m_RegObj[i]->CalcingPunctureLineEndSpace(pt_lineEnd);
			m_RegObj[i]->CalcingCylinderPtArraySpace();
			m_RegObj[i]->GetPuncNeedlePtArray(toptemparray, bottemparray, cylindercenter, conecirclearray, puncneedletop);
			if (i == 0)
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glLineStipple(1, 0x00ff);
				glEnable(GL_LINE_STIPPLE);
				glColor4f(m_PuncLineColor[i][0], m_PuncLineColor[i][1], m_PuncLineColor[i][2], m_PuncLineColor[i][3]);
				glBegin(GL_LINES);
				glVertex3f(pt_lineStart.x, pt_lineStart.y, pt_lineStart.z);
				glVertex3f(pt_lineEnd.x, pt_lineEnd.y, pt_lineEnd.z);
				glEnd();
				glDisable(GL_LINE_STIPPLE);
			}
			else
			{
				SetRenderFeatureNeedle();
				DrawingPuncNeedle(toptemparray, bottemparray, cylindercenter, conecirclearray, puncneedletop, DEFAULTCYLINDERSLICE);
			}
		}
	}
}

void MarchingCube::RenderingProbesensor(GLuint tex2d, int orderflag)
{
	SetLightsForRendering();
	int texwidth = PROBEICONWIDTH;
	int texheight = PROBEICONHEIGHT;
	InitImageBufAlpha(m_ProbeIconImBuf, texwidth, texheight);
	Point3f pt_sensorul = m_Sliceobj->GetWorldSensorUlPt();
	Point3f pt_sensorur = m_Sliceobj->GetWorldSensorUrPt();
	Point3f pt_sensordl = m_Sliceobj->GetWorldSensorDlPt();
	Point3f pt_sensordr = m_Sliceobj->GetWorldSensorDrPt();
	pt_sensorul.x = pt_sensorul.x + m_UpperLeftX + m_XCenterOffset;
	pt_sensorul.y = pt_sensorul.y + m_UpperLeftY + m_YCenterOffset;
	pt_sensorul.z = pt_sensorul.z + m_UpperLeftZ[0] +m_ZCenterOffset;
	pt_sensorur.x = pt_sensorur.x + m_UpperLeftX + m_XCenterOffset;
	pt_sensorur.y = pt_sensorur.y + m_UpperLeftY + m_YCenterOffset;
	pt_sensorur.z = pt_sensorur.z + m_UpperLeftZ[0] + m_ZCenterOffset;
	pt_sensordl.x = pt_sensordl.x + m_UpperLeftX + m_XCenterOffset;
	pt_sensordl.y = pt_sensordl.y + m_UpperLeftY + m_YCenterOffset;
	pt_sensordl.z = pt_sensordl.z + m_UpperLeftZ[0] + m_ZCenterOffset;
	pt_sensordr.x = pt_sensordr.x + m_UpperLeftX + m_XCenterOffset;
	pt_sensordr.y = pt_sensordr.y + m_UpperLeftY + m_YCenterOffset;
	pt_sensordr.z = pt_sensordr.z + m_UpperLeftZ[0] + m_ZCenterOffset;

	if (m_WorkBuf)
	{
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glGenTextures(1, &tex2d);
		glBindTexture(GL_TEXTURE_2D, tex2d);
		// Step2 设定wrap参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Step3 设定filter参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_ProbeIconImBuf);

		glColor4f(1.0, 1.0, 1.0, 1.0f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, orderflag-0.0f);  glVertex3f(pt_sensorul.x, pt_sensorul.y, pt_sensorul.z);
		glTexCoord2f(1.0f, orderflag-0.0f);  glVertex3f(pt_sensorur.x, pt_sensorur.y, pt_sensorur.z);
		glTexCoord2f(1.0f, orderflag-1.0f);  glVertex3f(pt_sensordr.x, pt_sensordr.y, pt_sensordr.z);
		glTexCoord2f(0.0f, orderflag-1.0f);  glVertex3f(pt_sensordl.x, pt_sensordl.y, pt_sensordl.z);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

//rendering
void MarchingCube::RenderProcess()
{
	SetLightsForRendering();
	//Rendering Organ
	for (int i = 0; i < m_OrganNum; i++)
	{
		if (m_OrganRenderFlag[i])
		{
			if (m_ComputeMaskFlag)
			{
				//IsoValueMask = 0.06;
				computeIsosurface(d_volumemask, d_posMask, d_normalMask, d_colorMask, m_XGridoffset, m_YGridoffset, m_ZGridoffset, MaskGridSize, MaskOffsetx, MaskOffsety, MaskOffsetz, MaskEndx, MaskEndy, MaskEndz, 
					              m_UpperLeftX, m_UpperLeftY, m_InterpZstart, MaskVoxelSize, IsoValueMask, m_MaskMeshFilteringFlag, m_MaskFilteringIterTimes);
				m_ComputeMaskFlag = false;
			}
			SetRenderFeatureMask();
			renderIsosurface(posVboMask, normalVboMask, colorVboMask);
		}
	}
	//Rendering Bone
	if (m_BoneRenderFlag)
	{
		if (m_ComputeBMFlag)
		{
			computeIsosurface(d_volume, d_posBone, d_normalBone, d_colorBone, 0, 0, 0, GridSize, Offsetx, Offsety, Offsetz, Endx, Endy, Endz,
				              m_UpperLeftX, m_UpperLeftY, m_UpperLeftZ[0], VoxelSize, IsoValueBone, m_BoneMeshFilteringFlag, m_BoneFilteringIterTimes);
			m_ComputeBMFlag = false;
		}
		SetRenderFeatureBone();
		renderIsosurface(posVboBone, normalVboBone, colorVboBone);
	}
	//Rendering Skin
	if (m_SkinRenderFlag)
	{
		if (m_ComputeFlag)
		{
			//IsoValue = 0.15;
			computeIsosurface(d_volume, d_pos, d_normal, d_color, 0, 0, 0, GridSize, Offsetx, Offsety, Offsetz, Endx, Endy, Endz,
				m_UpperLeftX, m_UpperLeftY, m_UpperLeftZ[0], VoxelSize, IsoValue, m_SkinMeshFilteringFlag, m_SkinFilteringIterTimes);
			m_ComputeFlag = false;
		}
		SetRenderFeature();
		renderIsosurface(posVbo, normalVbo, colorVbo);
	}
}


