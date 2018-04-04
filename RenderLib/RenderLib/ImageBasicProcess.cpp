#include "ImageBasicProcess.h"

#pragma pack(1)
// WinGDI structure -------------------------------------------------------------
typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
typedef struct tagBITMAPFILEHEADER {
	UINT16	bfType;
	ULONG	bfSize;
	UINT16	bfReserved1;
	UINT16	bfReserved2;
	ULONG	bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER{
	ULONG    biSize;
	LONG     biWidth;
	LONG     biHeight;
	UINT16   biPlanes;
	UINT16   biBitCount;
	ULONG    biCompression;
	ULONG    biSizeImage;
	LONG     biXPelsPerMeter;
	LONG     biYPelsPerMeter;
	ULONG    biClrUsed;
	ULONG    biClrImportant;
} BITMAPINFOHEADER;
// ------------------------------------------------------------------------------

// struct of BMP file
typedef struct CBmpInfo
{
	const char * m_Input;
	const char * mOutput;

	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;

	UINT32 m_BmpMapWidth;
	UINT32 m_BmpMapHeight;

	float * m_pBuff;
	ULONG m_offset;
}_CBmpInfo;

#pragma pack()

inline UINT16 GetRound256(float a)
{
	int res = (int)a;

	if (float(a - res) > 0.5) res++;
	if (res > 255) return 255;
	if (res < 0) return 0;

	return res;
}

UINT16 WriteBmp(struct CBmpInfo * BmpInfo)
{
	int i, j, k;

	FILE * BinFile = NULL;
	try
	{
		if (fopen_s(&BinFile, BmpInfo->mOutput, "wb") != 0)
			return 0;

		BYTE * Buff_BYTE = new BYTE[(BmpInfo->m_BmpMapWidth)*(BmpInfo->m_BmpMapHeight)];
		BYTE * newBuffer = Buff_BYTE;
		for (i = BmpInfo->m_BmpMapHeight - 1; i >= 0; i--)
		{
			for (j = 0; j < BmpInfo->m_BmpMapWidth; j++)
			{
				k = i*BmpInfo->m_BmpMapWidth + j;
				*newBuffer = (BYTE)GetRound256(BmpInfo->m_pBuff[k] * 255);
				newBuffer++;
			}
		}

		RGBQUAD rgbQuad[256];
		for (UINT16 i = 0; i < 256; i++)
		{
			rgbQuad[i].rgbBlue = (BYTE)i;
			rgbQuad[i].rgbGreen = (BYTE)i;
			rgbQuad[i].rgbRed = (BYTE)i;
			rgbQuad[i].rgbReserved = 0;
		}

		BmpInfo->m_offset = 0;

		BmpInfo->FileHeader.bfSize = (ULONG)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256 + BmpInfo->m_BmpMapWidth*BmpInfo->m_BmpMapHeight);
		BmpInfo->FileHeader.bfType = (UINT16)0x4D42;
		BmpInfo->FileHeader.bfOffBits = (ULONG)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);

		BmpInfo->FileHeader.bfReserved1 = 0;
		BmpInfo->FileHeader.bfReserved2 = 0;

		BmpInfo->BmpHeader.biBitCount = 8;
		BmpInfo->BmpHeader.biClrImportant = 0;
		BmpInfo->BmpHeader.biClrUsed = 0;
		BmpInfo->BmpHeader.biCompression = 0;   //BI_RGB
		BmpInfo->BmpHeader.biHeight = BmpInfo->m_BmpMapHeight;
		BmpInfo->BmpHeader.biWidth = BmpInfo->m_BmpMapWidth;
		BmpInfo->BmpHeader.biPlanes = 1;
		BmpInfo->BmpHeader.biSize = 40;
		BmpInfo->BmpHeader.biSizeImage = 0;
		BmpInfo->BmpHeader.biXPelsPerMeter = 0;
		BmpInfo->BmpHeader.biYPelsPerMeter = 0;

		fwrite((void *)&(BmpInfo->FileHeader), 1, sizeof(BmpInfo->FileHeader), BinFile);
		fwrite((void *)&(BmpInfo->BmpHeader), 1, sizeof(BmpInfo->BmpHeader), BinFile);
		fwrite((char*)rgbQuad, 1, sizeof(RGBQUAD) * 256, BinFile);
		fwrite((void*)Buff_BYTE, 1, ((BmpInfo->BmpHeader).biHeight)*(BmpInfo->BmpHeader).biWidth, BinFile);
		fclose(BinFile);

		delete[] Buff_BYTE;
		Buff_BYTE = NULL;

		return 1;
	}
	catch (int&  value)
	{
		fclose(BinFile);
		//std::cout<<"Error in raw data reading process: "<<value<<endl;

		return 0;
	}
}


extern "C" void SaveDicomDataToBMP(const short *DicomIn_put, const char *filename, int width, int height)
{
	int i = 0, j = 0;
	INT16 _maxv = -5000, _minv = 5000;
	for (i = 0; i < height; i += 2){
		for (j = 0; j<width; j += 2){
			if (DicomIn_put[i * width + j] > _maxv)
				_maxv = DicomIn_put[i * width + j];
			if (DicomIn_put[i * width + j] < _minv)
				_minv = DicomIn_put[i * width + j];
		}
	}

	CBmpInfo * mCBmpInfo = new CBmpInfo();
	size_t msize = width*height;
	mCBmpInfo->m_pBuff = new float[msize];
	memset(mCBmpInfo->m_pBuff, 0, msize*sizeof(float));
	mCBmpInfo->m_BmpMapHeight = height;
	mCBmpInfo->m_BmpMapWidth =  width;
	mCBmpInfo->m_offset = 0;
	mCBmpInfo->m_Input = "";
	mCBmpInfo->mOutput = filename;
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
			mCBmpInfo->m_pBuff[i * 512 + j] = (float)(DicomIn_put[i * 512 + j] - _minv) / (float)(_maxv - _minv);

	int ret = WriteBmp(mCBmpInfo);

	INT16* _ImageBuffer = new INT16[512 * 512];
	memset(_ImageBuffer, 0, 512 * 512 * sizeof(INT16));
}

extern "C" void LoadImageFromBMP(const char *filename, UINT8 *data, int &channels)
{
	FILE *fp = fopen(filename, "rb");//二进制读方式打开指定的图像文件

	if (fp == 0)
		return;

	//跳过位图文件头结构BITMAPFILEHEADER

	fseek(fp, sizeof(BITMAPFILEHEADER), 0);

	//定义位图信息头结构变量，读取位图信息头进内存，存放在变量head中

	BITMAPINFOHEADER head;

	fread(&head, sizeof(BITMAPINFOHEADER), 1, fp); //获取图像宽、高、每像素所占位数等信息

	int bmpWidth = head.biWidth;

	int bmpHeight = head.biHeight;

	int biBitCount = head.biBitCount;//定义变量，计算图像每行像素所占的字节数（必须是4的倍数）

	channels = biBitCount / 8;

	int lineByte = (bmpWidth * biBitCount / 8 + 3) / 4 * 4;//灰度图像有颜色表，且颜色表表项为256

	if (biBitCount == 8)
	{

		//申请颜色表所需要的空间，读颜色表进内存

		RGBQUAD *pColorTable = new RGBQUAD[256];

		fread(pColorTable, sizeof(RGBQUAD), 256, fp);

	}
	//申请位图数据所需要的空间，读位图数据进内存
	fread(data, 1, lineByte * bmpHeight, fp);


	fclose(fp);//关闭文件
}

extern "C" void LoadImageFromDat(const char *filename, UINT8 *data, int width, int height, int channels)
{
	FILE *fp = 0;
	fopen_s(&fp, filename, "rb");
	fread(data, 1, width*height * channels, fp);
	fclose(fp);
}