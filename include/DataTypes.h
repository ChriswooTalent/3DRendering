#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

#ifdef __cplusplus //#ifdef __cplusplus ---------------------------------------------------------
	extern "C" {
#endif 

		// Portable STANDARD data types (the built-in standard C/C++ data types *shall* never be used)
#define INT8		char
#define UINT8		unsigned char
#define BYTE		unsigned char
#define INT16		short
#define UINT16	unsigned short
#define INT32		int
#define UINT32	unsigned int 
#define LONG		long
#define ULONG		unsigned long
#define DOUBLE	double
#ifdef __cplusplus
	}
#endif // #ifdef __cplusplus -------------------------------------------------------------------

#ifndef NULL
#define NULL	(0L)
#endif //#ifndef NULL

#ifndef FALSE
#define FALSE	(0)
#endif //#ifndef FALSE

#ifndef TRUE
#define TRUE	(1)
#endif //#ifndef TRUE

#define BITS_PER_INT8		(8)
#define MAX_INT8			(127)
#define MIN_INT8			( -MAX_INT8 - 1 )
#define BITS_PER_SINT8	(8)
#define MAX_SINT8			(127)
#define MIN_SINT8			( -MAX_SINT8 - 1 )
#define BITS_PER_UINT8	(8)
#define MAX_UINT8			(0xFF)
#define MIN_UINT8			(0)

#define BITS_PER_INT16	(16)
#define MAX_INT16			(32767)
#define MIN_INT16			( -MAX_INT16 - 1 )
#define BITS_PER_SINT16	(16)
#define MAX_SINT16		(32767)
#define MIN_SINT16		( -MAX_SINT16 - 1 )
#define BITS_PER_UINT16	(16)
#define MAX_UINT16		(0xFFFF)
#define MIN_UINT16		(0)

#define BITS_PER_INT32	(32)
#define MAX_INT32			(2147483647L)
#define MIN_INT32			( -MAX_INT32 - 1L )
#define BITS_PER_SINT32	(32)
#define MAX_SINT32		(2147483647L)
#define MIN_SINT32		( -MAX_SINT32 - 1L )
#define BITS_PER_UINT32	(32)
#define MAX_UINT32		(0xFFFFFFFFL)
#define MIN_UINT32		(0)

#define ATOMIC_TYPE_SZ	(4)

	// Other useful macros
#ifndef MIN
#define MIN( a, b )	((( a ) < ( b )) ? ( a ) : ( b ))	// Minimum of a and b
#endif //#ifndef MIN

#ifndef MAX
#define MAX( a, b )	((( a ) > ( b )) ? ( a ) : ( b ))	// Maximum of a and b
#endif //ifndef MAX

//ImageSize
#define TIMEPARA	1000000.0
#define HEIGHT	512
#define WIDTH	512
#define NSLICE  512

//OpenCl KernelSize
#define THREAD	256
#define NTHREADS 32
#define BIN_SIZE	256
#define MAXBLOCKX 128
#define BLOCKX 16
#define BLOCKY 16
#define NUM_BANKS 16

//MeshFiltering Params
#define  MAXNEIGHBORNUM 120
#define  MAXNEIGHBORNUMUD 30
#define  MAXITERTIMES 8
namespace RenderingDataTypes
{
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

	typedef struct DICOMImage
	{
		float * _pBuff;
		short _offset;
	}_DICOMImage;

	typedef struct vector3
	{
		float fX;
		float fY;
		float fZ;
		vector3()
		{
			fX = 0.0f;
			fY = 0.0f;
			fZ = 0.0f;
		}
		vector3(float x_as, float y_as, float z_as)
		{
			fX = x_as;
			fY = y_as;
			fZ = z_as;
		}
		vector3& operator=(vector3& vec)
		{
			fX = vec.fX;
			fY = vec.fY;
			fZ = vec.fZ;
			return *this;
		}

		vector3& operator+(vector3 &pt)//实现加法重载
		{
			fX += pt.fX;
			fY += pt.fY;
			fZ += pt.fZ;
			return *this;//返回当前对象
		}
	}vector3d;

	typedef struct Point3
	{
		float x;
		float y;
		float z;
		int indsort;

		Point3()
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
			indsort = 0;
		}
		Point3(float x_as, float y_as, float z_as, int ind)
		{
			x = x_as;
			y = y_as;
			z = z_as;
			indsort = ind;
		}

		Point3& operator=(Point3& pt)
		{
			x = pt.x;
			y = pt.y;
			z = pt.z;
			indsort = pt.indsort;
			return *this;
		}

		Point3& operator+(Point3 &pt)//实现加法重载
		{
			Point3 temppt(0.0f, 0.0f, 0.0f, 0);
			temppt.x = x + pt.x;
			temppt.y = y + pt.y;
			temppt.z = z + pt.z;
			temppt.indsort = pt.indsort;
			return temppt;//返回当前对象
		}

		Point3& operator-(Point3 &pt)
		{
			Point3 temppt(0.0f, 0.0f, 0.0f, 0);
			temppt.x = x - pt.x;
			temppt.y = y - pt.y;
			temppt.z = z - pt.z;
			temppt.indsort = indsort;
			return temppt;//返回当前对象
		}

		bool operator<(const Point3& pt) const
		{
			/*if (x == pt.x)
			return indsort < pt.indsort;
			else
			return x < pt.x;*/
			if (x < pt.x)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool operator==(const Point3& pt) const
		{
			return ((x == pt.x) && (y == pt.y) && (z == pt.z));
		}
	}Point3f;

	class dim3 {
	public:
		size_t x;
		size_t y;
		size_t z;

		dim3(size_t _x = 1, size_t _y = 1, size_t _z = 1) { x = _x; y = _y; z = _z; }
	};

	class RenderParam
	{
	public:
		float IsoValue;
		float IsoValueBone;
		float IsoValueMask;
		float IsoUp;
		float IsoBot;
		float DisoValue;
		float Density;
		float Brightness;
		float TransferOffset;
		float TransferScale;
		int MaskStartSlice;
		int MaskEndSlice;

		RenderParam(float _isoval = 0.0f, float _isovalbone = 0.0f, float _isovalmask = 0.0f, float _disoval = 0.5f, float _isoup = 500, float _isobot = -800,
			int _startno = 0, int _endno = 5, float _density = 0.05f, float _brightness = 1.0f, float _transoffset = 0.0f, float _transcale = 1.0f)
		{
			IsoValue = _isoval;
			IsoValueBone = _isovalbone;
			IsoValueMask = _isovalmask;
			DisoValue = _disoval;
			IsoUp = _isoup;
			IsoBot = _isobot;
			Density = _density;
			Brightness = _brightness;
			TransferOffset = _transoffset;
			TransferScale = _transcale;
			MaskStartSlice = _startno;
			MaskEndSlice = _endno;
		}

		RenderParam &operator=(RenderParam& rp_obj)
		{
			IsoValue = rp_obj.IsoValue;
			IsoValueBone = rp_obj.IsoValueBone;
			IsoValueMask = rp_obj.IsoValueMask;
			DisoValue = rp_obj.DisoValue;
			IsoUp = rp_obj.IsoUp;
			IsoBot = rp_obj.IsoBot;
			Density = rp_obj.Density;
			Brightness = rp_obj.Brightness;
			TransferOffset = rp_obj.TransferOffset;
			TransferScale = rp_obj.TransferScale;
			MaskStartSlice = rp_obj.MaskStartSlice;
			MaskEndSlice = rp_obj.MaskEndSlice;
		}
	};
}

#endif //#ifndef _DATA_TYPES_H_
