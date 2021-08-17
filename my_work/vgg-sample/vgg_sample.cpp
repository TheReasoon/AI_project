#include <stdio.h>
#include <stdlib.h>
#include <VX/vx.h>
#include <vx_ext_amd.h>
#include <net_api.h>
#include <math.h>
#include <memory.h>
#include <vx_log.h>

int main()
{
    FILE * fin;
	int flen;
	vx_int8 * net;
	vx_int8 * blobs;
    int input_w;
    int input_h;
    int y_stride;
    int uv_stride;
    img_fmt input_fmt;
    
	input_w = 960;
    input_h = 640;
    y_stride = 960;
    uv_stride = 640;
    input_fmt = YUV420;
	encrypt_param encryption_param = { 1 };

	LoadNetModel("weights/VGG_ILSVRC_16_layers_deploy.ezb", &encryption_param, &net);
	LoadNetModel("weights/VGG_ILSVRC_16_layers_deploy.bin", &encryption_param, &blobs);

	//create context
	vx_context context = vxCreateContext();
	ERROR_CHECK_OBJECT(context);

	//load nn module
	ERROR_CHECK_STATUS(vxLoadKernels(context, "openvx-nn"));

	//get input blobs
	std::vector<blob> input_blobs;
	printf("hello!1\n");
	GetNetInputBlobs((vx_uint32*)net, input_blobs);
	printf("hello!2\n");
	if (input_blobs.size() != 1) {
		LOG("this face mode must be one input!\n");
		exit(-1);
	}

	vx_int32 width = input_blobs[0].dims[0];
	vx_int32 height = input_blobs[0].dims[1];
	vx_int32 channel = input_blobs[0].dims[2];

	//创建保存转换后图片的tensor：1、分配内存，2、创建tensor封装该内存
	vx_size scale_img_handle = AllocateTensorDeviceMemory(context, width, height, channel, 1);
	if (scale_img_handle == NULL) {
		LOG("alloc image handle error!\n");
		exit(-1);
	}
	vx_tensor scale_img_tensor = CreateTensorFromHandle(context, width, height, channel, 1, scale_img_handle);
	if (scale_img_tensor == NULL) {
		LOG("create image tensor error!\n");
		exit(-1);
	}

	//alloc input image device memory
    vx_size image_size = input_w * input_h + input_w * input_h / 2;
	vx_size img_handle = vxAllocateDeviceMemory(context, image_size);
	if(img_handle == 0){
		LOG("alloc memory error!\n");
        exit(-1);
	}

#if 0
	vx_size phys_base_addr = vxGetDeviceMemoryPhysicalAddress(img_handle);
	vx_size virt_base_addr = vxGetDeviceMemoryVirtualAddress(img_handle);

	//load image data	
	fin = fopen("orange.y", "rb");
	if (fin == NULL) {
		LOG("open image file fail!\n");
        return -1;
	}
	fseek(fin, 0, SEEK_END);
	flen = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	fread((vx_int8*)virt_base_addr, 1, flen, fin);
	fclose(fin);
    
	fin = fopen("orange.uv", "rb");
	if (fin == NULL) {
		LOG("open image file fail!\n");
        return -1;
	}
	fseek(fin, 0, SEEK_END);
	flen = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	fread((vx_int8*)virt_base_addr + input_w * input_h, 1, flen, fin);
	fclose(fin);

	//image convert process
	input_image img;
	img.width = input_w;
	img.height = input_h;
	img.fmt = input_fmt;
	img.stride1 = y_stride;
	img.stride2 = uv_stride;
	img.handle1 = phys_base_addr;
	img.handle2 = phys_base_addr + input_w * input_h;	
	img.crop_x = 0;
	img.crop_y = 0;
	img.crop_w = input_w;
	img.crop_h = input_h;
	ImageConvertDeviceProcess(context, &img, scale_img_tensor);
#endif
	//load image
	vx_int8 * output_data;
	int output_data_size;
	ERROR_CHECK_STATUS(vxQueryTensor(scale_img_tensor, VX_TENSOR_MEMORY_SIZE, &output_data_size, sizeof(output_data_size)));
	ERROR_CHECK_STATUS(vxQueryTensor(scale_img_tensor, VX_TENSOR_MEMORY_HANDLE, &output_data, sizeof(output_data)));
	fin = fopen("image/data.rgb", "rb");
	if (fin == NULL) {
		LOG("open image file fail!\n");
        return -1;
	}
	fread((vx_int8*)output_data, 1, output_data_size, fin);
	fclose(fin);
	//create net graph
	vx_graph graph = CreateNetGraph(context, (vx_uint32*)net, blobs, -1);
	//编译不过：undefined reference to `CreateNetDumpBlobGraph' 估计是so库里面没有CreateNetDumpBlobGraph函数的实现
	//vx_graph graph = CreateNetDumpBlobGraph(context, (vx_uint32*)net, blobs, -1);
	ERROR_CHECK_STATUS(vxVerifyGraph(graph));
	
	//set net input
	input_blobs[0].tensor = scale_img_tensor;
	SetNetInputBlobsTensor(graph, input_blobs);

	//process graph
	ERROR_CHECK_STATUS(vxProcessGraph(graph));
	vxFinish(context);

	//output file
	std::vector<std::string> out_blob_name;
	std::vector<vx_tensor> out_blob;
	vx_status status = GetNetOutputBlobsTensor(graph, out_blob_name, out_blob);	
	LOG("out_blob size: %d\n", out_blob.size());
	for(int i = 0; i < out_blob.size(); i++){	
		char out_file[1024];

		for(int j=0; j<out_blob_name[i].size();j++){
			if(out_blob_name[i][j] == '/'){
				out_blob_name[i][j]= '_';
			}
		}

		sprintf(out_file, "./result/%s.bin", out_blob_name[i].c_str());
		vx_size dims[4] = {1};
		int output_data_size;
		vx_int8 * output_data;
		ERROR_CHECK_STATUS(vxQueryTensor(out_blob[i], VX_TENSOR_DIMS, dims, sizeof(dims)));
		ERROR_CHECK_STATUS(vxQueryTensor(out_blob[i], VX_TENSOR_MEMORY_SIZE, &output_data_size, sizeof(output_data_size)));
		ERROR_CHECK_STATUS(vxQueryTensor(out_blob[i], VX_TENSOR_MEMORY_HANDLE, &output_data, sizeof(output_data)));
		LOG("%d: %s, (%d, %d, %d, %d), %d\n", i, out_blob_name[i].c_str(), dims[0], dims[1], dims[2], dims[3], output_data_size);
		fin = fopen(out_file, "wb");
		fwrite(output_data, 1, output_data_size, fin);
		fclose(fin);
	}

	//release
    ERROR_CHECK_STATUS(vxFreeDeviceMemory(context, img_handle));
	for (int i = 0; i < input_blobs.size(); i++) {
		ERROR_CHECK_STATUS(vxReleaseTensor(&input_blobs[i].tensor));
	}
	input_blobs.clear();
	ERROR_CHECK_STATUS(vxFreeDeviceMemory(context, scale_img_handle));
	out_blob_name.clear();
	for (int i = 0; i < out_blob.size(); i++) {
		ERROR_CHECK_STATUS(vxReleaseTensor(&out_blob[i]));
	}
	std::vector<vx_tensor>().swap(out_blob);
	ERROR_CHECK_STATUS(vxReleaseGraph(&graph));
    ERROR_CHECK_STATUS(vxReleaseContext(&context));
	UnLoadNetModel(net);
	UnLoadNetModel(blobs);

    return 0;
}