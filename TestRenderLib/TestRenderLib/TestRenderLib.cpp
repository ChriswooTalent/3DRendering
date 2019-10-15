#include <iostream>
#include <stdlib.h> 
#include <tchar.h>
#include <string>
#include <windows.h> 
using namespace std;
extern "C" __declspec(dllimport) int Initial3DReconResource(float *dicominfo, float *dicomdata, int framecount, HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfow, int *uswindowinfo);
extern "C" __declspec(dllimport) void Release3DReconResource();
extern "C" __declspec(dllimport) void Call3DRecon(float *maskdata, float *maskinfo);
extern "C" __declspec(dllimport) void RenderingInit(float *maskdata, float *maskinfo);
extern "C" __declspec(dllimport) void Showing3D();
extern "C" __declspec(dllimport) bool Get3DWindowActiveFlag();
extern "C" __declspec(dllimport) void ShowingSlice();
extern "C" __declspec(dllimport) bool GetSliceWindowActiveFlag();
extern "C" __declspec(dllimport) void ShowingUltrasound();
extern "C" __declspec(dllimport) bool GetUSWindowActiveFlag();
extern "C" __declspec(dllimport) void UIGetSliceData(float *buf, int len);
extern "C" __declspec(dllimport) void UISetUltraSoundBuf(float *buf, int len);
#define MAXSLICENUM 512
typedef struct DICOMInfo
{
	DICOMInfo() :
		// 				ImageModality('\0'),
		// 				InstituteName('\0'),
		// 				PatientName('\0'),
		// 				PatientID('\0'),
		// 				PatientBirthday('\0'),
		// 				PatientSex('\0'),
		// 				PatientAge('\0'),
		// 				BodyPartExamined('\0'),
		SliceThickness(0),
		UpperLeft_X(0),
		UpperLeft_Y(0),
		UpperLeft_Z(0),
		SliceLocation(0),
		SamplePerPixel(0),
		ImageWidth(512),
		ImageHeight(512),
		PixelSpacing_X(0),
		PixelSpacing_Y(0),
		BitsAllocated(0),
		BitsStored(0),
		WindowWidth(0),
		WindowCenter(0),
		RescaleIntercept(0),
		RescaleSlope(0),
		PixelPaddingValue(0),
		GPixelMaxValue(4095),
		GPixelMinValue(0) {}

	char ImageModality[16];

	char InstituteName[64];
	char PatientName[64];
	char PatientID[64];
	char PatientBirthday[9];

	char PatientSex[16];

	char PatientAge[5];

	char BodyPartExamined[16];

	float SliceThickness;

	// image position (patient)
	float UpperLeft_X;
	float UpperLeft_Y;
	float UpperLeft_Z;

	float SliceLocation;

	unsigned short SamplePerPixel;

	unsigned short ImageWidth;
	unsigned short ImageHeight;

	// Pixel Spacing
	float PixelSpacing_X;
	float PixelSpacing_Y;

	unsigned short BitsAllocated;
	unsigned short BitsStored;

	float WindowWidth;
	float WindowCenter;

	float RescaleIntercept;
	float RescaleSlope;

	unsigned short PixelPaddingValue;

	unsigned short GPixelMaxValue;
	unsigned short GPixelMinValue;
}_DICOMInfo;

////////////////////////////////////////////////////////////////////////////////
// Load raw data from disk
////////////////////////////////////////////////////////////////////////////////
void GetVolumeDataFromFile(const char *filename, short *volume, int totalsize, int gridx)
{
	FILE *fpbintest = 0;
	fopen_s(&fpbintest, filename, "rb");
	if (!fpbintest)
	{
		fprintf(stderr, "Error opening file '%s'\n", filename);
		return;
	}
	int flag = fread(volume, sizeof(short), totalsize / sizeof(short), fpbintest);

	fclose(fpbintest);
}

////////////////////////////////////////////////////////////////////////////////
// Load raw data from disk
////////////////////////////////////////////////////////////////////////////////
void GetfloatVolumeDataFromFile(const char *filename, float *volume, int totalsize, int gridx)
{
	FILE *fpbintest = 0;
	fopen_s(&fpbintest, filename, "rb");
	if (!fpbintest)
	{
		fprintf(stderr, "Error opening file '%s'\n", filename);
		return;
	}
	int flag = fread(volume, sizeof(float), totalsize / sizeof(float), fpbintest);

	fclose(fpbintest);
}

void GetVolumeInfoFromFile1(const char *filename, _DICOMInfo *dicomobj, int *slicenum)
{
	int filelength = 0;
	FILE *fp = 0;
	fopen_s(&fp, filename, "r");

	if (!fp)
	{
		return;
	}
	int line_count = 0;
	char c;
	do
	{
		c = fgetc(fp);
		if (c == '\n')
			line_count++;
	} while (c != EOF);
	fclose(fp);
	*slicenum = line_count;
	float *volumeinfodata = (float *)malloc(9 * sizeof(float));
	fopen_s(&fp, filename, "r");
	for (int i = 0; i < line_count; i++)
	{
		int valueinfile = 0;
		for (int j = 0; j < 9; j++)
		{
			float value = 0;
			fscanf(fp, "%f", &value);
			volumeinfodata[j] = value;
		}
		dicomobj[i].PixelSpacing_X = volumeinfodata[0];
		dicomobj[i].PixelSpacing_Y = volumeinfodata[1];
		dicomobj[i].UpperLeft_X = volumeinfodata[2];
		dicomobj[i].UpperLeft_Y = volumeinfodata[3];
		dicomobj[i].UpperLeft_Z = volumeinfodata[4];
		dicomobj[i].WindowCenter = volumeinfodata[5];
		dicomobj[i].WindowWidth = volumeinfodata[6];
		dicomobj[i].RescaleSlope = 1.0f;
		dicomobj[i].RescaleIntercept = -1024.0f;
	}
	fclose(fp);

	free(volumeinfodata);
}

inline void PixelRescale(float * floatbuffer, short *shortbuffer, _DICOMInfo* SliceInfo, int index)
{
	float curPixel = (float)(shortbuffer[index]);
	//rescale handling
	float tempvalue = 0.0f;
	float vmin = 0.0f;
	float vmax = 0.0f;
	//tempvalue = (curPixel  - SliceInfo->RescaleIntercept)/SliceInfo->RescaleSlope;
	tempvalue = curPixel;
	vmin = (SliceInfo->WindowCenter - 0.5f) - (SliceInfo->WindowWidth - 1.0f) / 2.0f;
	vmax = (SliceInfo->WindowCenter - 0.5f) + (SliceInfo->WindowWidth - 1.0f) / 2.0f;
	/*if (tempvalue <= vmin)
	floatbuffer[index] = 0.0f;
	else if (tempvalue > vmax)
	floatbuffer[index] = 1.0f;
	else*/
	floatbuffer[index] = (tempvalue - vmin) / (vmax - vmin);
	//floatbuffer[index] = tempvalue;
}

inline void EnableMemLeakCheck()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}
#define MHE 1
int main(int argc, char **argv)
{
#ifdef MHE
	string volumeinfofile = "../../data/MHEInfoData.txt";
#else
	string volumeinfofile = "../../data/Luohu2InfoData.txt";
#endif
	//string volumeinfofile = "../../data/prostateinfofinal.txt";
	_DICOMInfo *dicomobj = new _DICOMInfo[MAXSLICENUM];
	int slicenum1 = 0;
	GetVolumeInfoFromFile1(volumeinfofile.c_str(), dicomobj, &slicenum1);
	float *dicominfo = (float *)malloc(7 * slicenum1 * sizeof(float));
	for (int i = 0; i < slicenum1; i++)
	{
		dicominfo[i * 7 + 0] = dicomobj[i].PixelSpacing_X;
		dicominfo[i * 7 + 1] = dicomobj[i].PixelSpacing_Y;
		dicominfo[i * 7 + 2] = dicomobj[i].UpperLeft_X;
		dicominfo[i * 7 + 3] = dicomobj[i].UpperLeft_Y;
		dicominfo[i * 7 + 4] = dicomobj[i].UpperLeft_Z;
		dicominfo[i * 7 + 5] = dicomobj[i].WindowCenter;
		dicominfo[i * 7 + 6] = dicomobj[i].WindowWidth;
	}
	
	int datanum = 512 * 512 * slicenum1;
	int size = 512 * 512 * slicenum1 * sizeof(short);
	int floatsize = 512 * 512 * slicenum1 * sizeof(float);
	short *volumedata = (short *)malloc(size);
	float *volumedataf = (float *)malloc(size * 2);
#ifdef MHE
	string file = "../../data/MHENoTable.dat";
	//string file = "../../data/ProstateVolume.dat";
	GetVolumeDataFromFile(file.c_str(), volumedata, size, 512);
	int accumulate_i = 0;
	for (int i = 0; i < datanum; i++, accumulate_i++)
	{
		PixelRescale(volumedataf, volumedata, dicomobj, i);
	}
	
#else
	string file = "../../data/CS_volume.dat";
	GetfloatVolumeDataFromFile(file.c_str(), volumedataf, floatsize, 512);
#endif
	short CIsoValue = -800;
	short CIsoValueBone = 130;
	short CIsoUp = -798;
	short CIsoBot = -802;
	float CIsoNorm = 0.0f;
	float CIsoBoneNorm = 0.0f;
	float CIsoupNorm = 0.0f;
	float CIsobotNorm = 0.0f;
	PixelRescale(&CIsoNorm, &CIsoValue, dicomobj, 0);
	PixelRescale(&CIsoBoneNorm, &CIsoValueBone, dicomobj, 0);
	PixelRescale(&CIsoupNorm, &CIsoUp, dicomobj, 0);
	PixelRescale(&CIsobotNorm, &CIsoBot, dicomobj, 0);

	/*FILE *fvolumedatanormalized = NULL;
	fopen_s(&fvolumedatanormalized, "D:/Code_local/DICOM Reader/marchingcube_matlab/volumedatanormalized.dat", "wb+");
	fwrite(volumedataf, sizeof(float), datanum, fvolumedatanormalized);
	fclose(fvolumedatanormalized);*/

	int masksize = 512 * 512 * 20 * sizeof(short);
	short *volumemaskdata = (short *)malloc(masksize);
	float *volumemaskdataf = (float *)malloc(masksize * 2);
	string VolumeMaskFileNames = "../../data/SegVolumeMask1.dat";
	//string VolumeMaskFileNames = "../../data/ProstateSegVolumeMask1.dat";
	GetVolumeDataFromFile(VolumeMaskFileNames.c_str(), volumemaskdata, masksize, 512);
	for (int i = 0; i < masksize / 2; i++)
	{
		//volumemaskdataf[i] = (float)volumemaskdata[i];
		PixelRescale(volumemaskdataf, volumemaskdata, dicomobj, i);
	}
	delete[]dicomobj;
	//CCarbonMed3DRecon *_3DRender = new CCarbonMed3DRecon();
	//slicenum1 = 20;
	int _3dWin[4] = { 0, 0, 512, 512 };
	int SliceWin[4] = { 513, 0, 512, 512 };
	int UltrasoundWin[4] = { 1023, 0, 512, 512 };
	Initial3DReconResource(dicominfo, volumedataf, slicenum1, NULL, _3dWin, SliceWin, UltrasoundWin);
	/*Point3f refpt1(260.0f, 368.0f, 82.0f, 0.0f);
	Point3f refpt2(260.0f, 368.0f, 82.0f, 0.0f);
	Point3f refpt3(260.0f, 368.0f, 82.0f, 0.0f);
	Point3f refpt4(260.0f, 368.0f, 82.0f, 0.0f);
	_3DRender->InitRegRefPts(refpt1, refpt2, refpt3, refpt4);*/
	float *inputA = NULL;
	float *inputB = new float[2];
	inputB[0] = 19;

	inputB[1] = 4.94444466;
	inputB[2] = 20;
	//Call3DRecon(volumemaskdataf, inputB);
	RenderingInit(volumemaskdataf, inputB);
	MSG msg;                                // Windowsx消息结构
	bool done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))           // 有消息在等待吗?
		{
			if (msg.message == WM_QUIT)               // 收到退出消息?
			{
				done = TRUE;                  // 是，则done=TRUE
			}
			else                            // 不是，处理窗口消息
			{
				TranslateMessage(&msg);             // 翻译消息
				DispatchMessage(&msg);              // 发送消息
			}
		}
		else
		{
			if (Get3DWindowActiveFlag())                     // 程序激活的么?
			{
				Showing3D();
			}
			if (GetSliceWindowActiveFlag())
			{
				ShowingSlice();
#ifdef TEST_NORM_OUT
				float *test_buf = (float *)malloc(512*512 * sizeof(float));
				UIGetSliceData(test_buf, 512 * 512 * sizeof(float));
				FILE *fp = fopen("D:/CarbonMed/testdata1/test_normlize.txt", "w+");
				for (int i = 0; i < 512; i++)
				{
					for (int j = 0; j < 512; j++)
					{
						fprintf(fp, "%f ", test_buf[i * 512 + j]);
					}
					fprintf(fp, "\n");
				}
				fclose(fp);
#endif
			}
			if (GetUSWindowActiveFlag())
			{
				ShowingUltrasound();
#ifdef TEST_US_RENDERING
				float *test_buf = (float *)malloc(512 * 512 * sizeof(float));
				UISetUltraSoundBuf(test_buf, 512 * 512 * sizeof(float));
#endif
			}
		}
	}
	Release3DReconResource();
	//delete _3DRender;

	//_CrtDumpMemoryLeaks();

	return 0;
}