#include "ScanApple.h" 

////////////////////////////////////////////////////////////////////////////////////////////////////
cl_context              ScanContext;
cl_command_queue        ComputeCommands;
cl_program              ComputeProgram;
cl_kernel*              ComputeKernels;
cl_mem*                 ScanPartialSums = 0;
unsigned int            ElementsAllocated = 0;
unsigned int            LevelsAllocated = 0;
int                     GROUP_SIZE = 256;
////////////////////////////////////////////////////////////////////////////////////////////////////

enum KernelMethods
{
	PRESCAN = 0,
	PRESCAN_STORE_SUM = 1,
	PRESCAN_STORE_SUM_NON_POWER_OF_TWO = 2, 
	PRESCAN_NON_POWER_OF_TWO = 3,
	UNIFORM_ADD = 4
};

static const char* KernelNames[] =
{
	"PreScanKernel",
	"PreScanStoreSumKernel",
	"PreScanStoreSumNonPowerOfTwoKernel",
	"PreScanNonPowerOfTwoKernel",
	"UniformAddKernel"
};

static const unsigned int KernelCount = sizeof(KernelNames) / sizeof(char *);

bool IsPowerOfTwo(int n)
{
	return ((n&(n - 1)) == 0);
}

int floorPow2(int n)
{
	int exp;
	frexp((float)n, &exp);
	return 1 << (exp - 1);
}

static char *
LoadProgramSourceFromFile(const char *filename)
{
	struct stat statbuf;
	FILE        *fh = 0;
	char        *source;

	fopen_s(&fh, filename, "r");
	if (fh == 0)
		return 0;

	stat(filename, &statbuf);
	source = (char *)malloc(statbuf.st_size + 1);
	fread(source, statbuf.st_size, 1, fh);
	source[statbuf.st_size] = '\0';

	return source;
}

int CreatePartialSumBuffers(unsigned int count)
{
	ElementsAllocated = count;

	unsigned int group_size = GROUP_SIZE;
	unsigned int element_count = count;

	int level = 0;

	do
	{
		unsigned int group_count = (int)fmax(1, (int)ceil((float)element_count / (2.0f * group_size)));
		if (group_count > 1)
		{
			level++;
		}
		element_count = group_count;

	} while (element_count > 1);

	ScanPartialSums = (cl_mem*)malloc(level * sizeof(cl_mem));
	LevelsAllocated = level;
	memset(ScanPartialSums, 0, sizeof(cl_mem) * level);

	element_count = count;
	level = 0;
	float memsuminside = 0.0f;
	do
	{
		unsigned int group_count = (int)fmax(1, (int)ceil((float)element_count / (2.0f * group_size)));
		
		if (group_count > 1)
		{
			size_t buffer_size = group_count * sizeof(float);
			memsuminside += (float)buffer_size / (1024.0f*1024.0f);
			ScanPartialSums[level++] = clCreateBuffer(ScanContext, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
		}
		
		element_count = group_count;

	} while (element_count > 1);

	printf("memsuminside is %f\n", memsuminside);
	return CL_SUCCESS;
}

void InitScanAPPLEMem(int Ccount)
{
	CreatePartialSumBuffers(Ccount);
}

void
ReleasePartialSums(void)
{
	unsigned int i;
	for (i = 0; i < LevelsAllocated; i++)
	{
		clReleaseMemObject(ScanPartialSums[i]);
	}

	free(ScanPartialSums);
	ScanPartialSums = 0;
	ElementsAllocated = 0;
	LevelsAllocated = 0;
}

int
PreScan(
size_t *global,
size_t *local,
size_t shared,
cl_mem output_data,
cl_mem input_data,
unsigned int n,
int group_index,
int base_index)
{
#if DEBUG_INFO
	printf("PreScan: Global[%4d] Local[%4d] Shared[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n",
		(int)global[0], (int)local[0], (int)shared, group_index, base_index, n);
#endif

	unsigned int k = PRESCAN;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &output_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &input_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &group_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &base_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &n);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	return CL_SUCCESS;
}

int
PreScanStoreSum(
size_t *global,
size_t *local,
size_t shared,
cl_mem output_data,
cl_mem input_data,
cl_mem partial_sums,
unsigned int n,
int group_index,
int base_index)
{
#if DEBUG_INFO
	printf("PreScan: Global[%4d] Local[%4d] Shared[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n",
		(int)global[0], (int)local[0], (int)shared, group_index, base_index, n);
#endif

	unsigned int k = PRESCAN_STORE_SUM;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &output_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &input_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &partial_sums);
	err |= clSetKernelArg(ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &group_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &base_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &n);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	return CL_SUCCESS;
}

int
PreScanStoreSumNonPowerOfTwo(
size_t *global,
size_t *local,
size_t shared,
cl_mem output_data,
cl_mem input_data,
cl_mem partial_sums,
unsigned int n,
int group_index,
int base_index)
{
#if DEBUG_INFO
	printf("PreScanStoreSumNonPowerOfTwo: Global[%4d] Local[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n",
		(int)global[0], (int)local[0], group_index, base_index, n);
#endif

	unsigned int k = PRESCAN_STORE_SUM_NON_POWER_OF_TWO;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &output_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &input_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &partial_sums);
	err |= clSetKernelArg(ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &group_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &base_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &n);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	return CL_SUCCESS;
}

int
PreScanNonPowerOfTwo(
size_t *global,
size_t *local,
size_t shared,
cl_mem output_data,
cl_mem input_data,
unsigned int n,
int group_index,
int base_index)
{
#if DEBUG_INFO
	printf("PreScanNonPowerOfTwo: Global[%4d] Local[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n",
		(int)global[0], (int)local[0], group_index, base_index, n);
#endif

	unsigned int k = PRESCAN_NON_POWER_OF_TWO;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &output_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &input_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &group_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &base_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &n);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}
	return CL_SUCCESS;
}

int
UniformAdd(
size_t *global,
size_t *local,
cl_mem output_data,
cl_mem partial_sums,
unsigned int n,
unsigned int group_offset,
unsigned int base_index)
{
#if DEBUG_INFO
	printf("UniformAdd: Global[%4d] Local[%4d] BlockOffset[%4d] BaseIndex[%4d] Entries[%d]\n",
		(int)global[0], (int)local[0], group_offset, base_index, n);
#endif

	unsigned int k = UNIFORM_ADD;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &output_data);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_mem), &partial_sums);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(float), 0);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &group_offset);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &base_index);
	err |= clSetKernelArg(ComputeKernels[k], a++, sizeof(cl_int), &n);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
		return EXIT_FAILURE;
	}

	return CL_SUCCESS;
}

int
PreScanBufferRecursive(cl_mem output_data, cl_mem input_data, int max_group_size, unsigned int max_work_item_count, int element_count, int level)
{
	unsigned int group_size = max_group_size;
	unsigned int group_count = (int)fmax(1.0f, (int)ceil((float)element_count / (2.0f * group_size)));//求出现有的数据能分为多少个group
	unsigned int work_item_count = 0;

	if (group_count > 1)
		work_item_count = group_size;
	else if (IsPowerOfTwo(element_count))
		work_item_count = element_count / 2;
	else
		work_item_count = floorPow2(element_count);

	work_item_count = (work_item_count > max_work_item_count) ? max_work_item_count : work_item_count;

	unsigned int element_count_per_group = work_item_count * 2;
	unsigned int last_group_element_count = element_count - (group_count - 1) * element_count_per_group;
	unsigned int remaining_work_item_count = (int)fmax(1.0f, last_group_element_count / 2);
	remaining_work_item_count = (remaining_work_item_count > max_work_item_count) ? max_work_item_count : remaining_work_item_count;
	unsigned int remainder = 0;
	size_t last_shared = 0;


	if (last_group_element_count != element_count_per_group)//判断总数是不是element_count_per_group的倍数
	{
		remainder = 1;

		if (!IsPowerOfTwo(last_group_element_count))
			remaining_work_item_count = floorPow2(last_group_element_count);

		remaining_work_item_count = (remaining_work_item_count > max_work_item_count) ? max_work_item_count : remaining_work_item_count;
		unsigned int padding = (2 * remaining_work_item_count) / NUM_BANKS;
		last_shared = sizeof(float) * (2 * remaining_work_item_count + padding);
	}

	remaining_work_item_count = (remaining_work_item_count > max_work_item_count) ? max_work_item_count : remaining_work_item_count;
	size_t global[] = { (int)fmax(1, group_count - remainder) * work_item_count, 1 };
	size_t local[] = { work_item_count, 1 };

	unsigned int padding = element_count_per_group / NUM_BANKS;
	size_t shared = sizeof(float) * (element_count_per_group + padding);

	cl_mem partial_sums = ScanPartialSums[level];
	int err = CL_SUCCESS;

	if (group_count > 1)
	{
		err = PreScanStoreSum(global, local, shared, output_data, input_data, partial_sums, work_item_count * 2, 0, 0);
		if (err != CL_SUCCESS)
			return err;

		if (remainder)
		{
			size_t last_global[] = { 1 * remaining_work_item_count, 1 };
			size_t last_local[] = { remaining_work_item_count, 1 };

			err = PreScanStoreSumNonPowerOfTwo(
				last_global, last_local, last_shared,
				output_data, input_data, partial_sums,
				last_group_element_count,
				group_count - 1,
				element_count - last_group_element_count);

			if (err != CL_SUCCESS)
				return err;

		}

		err = PreScanBufferRecursive(partial_sums, partial_sums, max_group_size, max_work_item_count, group_count, level + 1);
		if (err != CL_SUCCESS)
			return err;

		err = UniformAdd(global, local, output_data, partial_sums, element_count - last_group_element_count, 0, 0);
		if (err != CL_SUCCESS)
			return err;

		if (remainder)
		{
			size_t last_global[] = { 1 * remaining_work_item_count, 1 };
			size_t last_local[] = { remaining_work_item_count, 1 };

			err = UniformAdd(
				last_global, last_local,
				output_data, partial_sums,
				last_group_element_count,
				group_count - 1,
				element_count - last_group_element_count);

			if (err != CL_SUCCESS)
				return err;
		}
	}
	else if (IsPowerOfTwo(element_count))
	{
		err = PreScan(global, local, shared, output_data, input_data, work_item_count * 2, 0, 0);
		if (err != CL_SUCCESS)
			return err;
	}
	else
	{
		err = PreScanNonPowerOfTwo(global, local, shared, output_data, input_data, element_count, 0, 0);
		if (err != CL_SUCCESS)
			return err;
	}

	return CL_SUCCESS;
}

void
PreScanBuffer(
cl_mem output_data,
cl_mem input_data,
unsigned int max_group_size,
unsigned int max_work_item_count,
unsigned int element_count)
{
	PreScanBufferRecursive(output_data, input_data, max_group_size, max_work_item_count, element_count, 0);
}

extern "C" int initScanAPPLE(cl_context cxGPUContext, cl_command_queue cqParamCommandQue, cl_device_id device)
{
	ScanContext = cxGPUContext;
	ComputeCommands = cqParamCommandQue;
	cl_int err;
	const char filename[256] = "D:/CarbonMed/Config/Local/CL/scan_kernel.cl";
	printf("Loading program %s ...\n", filename);
	printf(SEPARATOR);

	char *source = LoadProgramSourceFromFile(filename);
	if (!source)
	{
		printf("Error: Failed to load compute program from file!\n");
		return EXIT_FAILURE;
	}

	// Create the compute program from the source buffer
	// load CL file
	std::ifstream kernelFile(filename, std::ios::in);
	if (!kernelFile.is_open()){ cout << "Opening CL file failed" << endl;	exit(0); }
	ostringstream oss;
	oss << kernelFile.rdbuf();
	string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	size_t src_size = srcStdStr.length();
	ComputeProgram = clCreateProgramWithSource(ScanContext, 1, &srcStr, &src_size, &err);
	if (!ComputeProgram || err != CL_SUCCESS)
	{
		printf("%s\n", source);
		printf("Error: Failed to create compute program!\n");
		return EXIT_FAILURE;
	}

	// Build the program executable
	//
	err = clBuildProgram(ComputeProgram, 1, &device, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t length;
		char build_log[2048];
		printf("%s\n", source);
		printf("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(ComputeProgram, device, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
		printf("%s\n", build_log);
		return -1;
	}

	ComputeKernels = (cl_kernel*)malloc(KernelCount * sizeof(cl_kernel));
	for (int i = 0; i < KernelCount; i++)
	{
		// Create each compute kernel from within the program
		//
		ComputeKernels[i] = clCreateKernel(ComputeProgram, KernelNames[i], &err);
		if (!ComputeKernels[i] || err != CL_SUCCESS)
		{
			printf("Error: Failed to create compute kernel!\n");
			return -1;
		}

		size_t wgSize;
		err = clGetKernelWorkGroupInfo(ComputeKernels[i], device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgSize, NULL);
		if (err)
		{
			printf("Error: Failed to get kernel work group size\n");
			return EXIT_FAILURE;
		}
		GROUP_SIZE = min(GROUP_SIZE, wgSize);
	}
	free(source);
	return 1;
}

extern "C" void closeScanAPPLE(void)
{
	cl_int ciErrNum = 0;
	ReleasePartialSums();
	for (int i = 0; i < KernelCount; i++)
		ciErrNum |= clReleaseKernel(ComputeKernels[i]);
	free(ComputeKernels);
	clCheckErrorIP(ciErrNum, CL_SUCCESS);
}

extern "C" void ScanAPPLEProcess(cl_mem d_Dst, cl_mem d_Src, int Ccount)
{
	//CreatePartialSumBuffers(Ccount);
	PreScanBuffer(d_Dst, d_Src, GROUP_SIZE, GROUP_SIZE, Ccount);
}