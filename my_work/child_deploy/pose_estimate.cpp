#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h> 
#include <log/log.h>

#include <iostream>
#include <algorithm>
#include "pose_estimate.h"
#include "lombo_malloc.h"

//#define DEBUG_POSE_ALGR
#define LOG_TAG "POSE_ALGR"

#define CHECK_STATUS( status ) { \
        vx_status status_ = (status); \
        if(status_ != VX_SUCCESS) { \
            ALOGE("ERROR: failed with status = (%d) at " __FILE__ "#%d\n", status_, __LINE__); \
            error_flag = 1; \
        } \
    }

#define ERROR_INFO(INFO) {\
        ALOGE("%s  at " __FILE__ "#%d\n", INFO, __LINE__); \
        error_flag = 1; \
	}

#define CHECK_FLAG() {\
		if(error_flag) 	return 1;		\
		}

#define CHECK_OBJECT( obj ) { \
        vx_status status_ = vxGetStatus((vx_reference)(obj)); \
        if(status_ != VX_SUCCESS) { \
            ALOGE("ERROR: failed with status = (%d) at " __FILE__ "#%d\n", status_, __LINE__); \
            error_flag = 1; \
        } \
    }



// 快速exp算法，近似等于e^x
inline float fast_exp(float x) 
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}
// sigmoid计算
inline float sigmoid(float x) 
{
    return 1.0f / (1.0f + fast_exp(-x));
}

//返回浮点数
float int8_to_float(int fiexde_point,float scale)
{
	return (float)fiexde_point/scale;
}

//template<typename _Tp> softmax
int activation_function_softmax(const vx_int8* src, float* dst, float scale,int length)
{
    const vx_int8 alpha = *std::max_element(src, src + length);
    float denominator=0.0;
	int temp_sum = 0;

    for (vx_int8 i = 0; i < length; ++i) {
		temp_sum = (int)src[i]-(int)alpha;//防止精度溢出
        dst[i] = fast_exp(int8_to_float(temp_sum,scale));
        denominator += dst[i];
    }
    for (vx_int8 i = 0; i < length; ++i) {
        dst[i] /= denominator;
    }
    return 0;
}


int EzInference::check_status() const
{
	return error_flag;
}


EzInference::EzInference(const char* model_path,int w,int h)
{
	encrypt_param encryption_param = { 1 };
	error_flag = 0;
	char addr[50] = {0};
	
	// 保证输入为32的倍数,设定输入的大小
	input_fmt = YUV420	;
	input_w = w - w%32;
	input_h = h;
	y_stride = w- w%32;
	uv_stride = w- w%32;

	// load模型
	sprintf(addr,"%s%s",model_path,".ezb");
	
	CHECK_STATUS(LoadNetModel(addr, &encryption_param, &net));
	sprintf(addr,"%s%s",model_path,".bin");
	CHECK_STATUS(LoadNetModel(addr, &encryption_param, &blobs));
	
	
	// 创建context
	ALOGD("image size %d %d\n",input_w,input_h);
	context = vxCreateContext();
	vx_size image_size = input_w * input_h + input_w * input_h / 2;

	CHECK_OBJECT(context);
	CHECK_STATUS(vxLoadKernels(context, "openvx-nn"));
	
	graph = CreateNetGraph(context, (vx_uint32*)net, blobs);
	CHECK_STATUS(vxVerifyGraph(graph));
}

EzInference::~EzInference()
{

	ALOGD("destruct model \r\n");
	error_flag = 0;
	CHECK_STATUS(vxReleaseGraph(&graph));
	CHECK_STATUS(vxReleaseContext(&context));
	UnLoadNetModel(net);
	UnLoadNetModel(blobs);	
}
DetHuman::~DetHuman()
{
	ALOGD("DetHuman destructor\n");
}

PoseEstimate::~PoseEstimate()
{
	ALOGD("PoseEstimate destructor\n");
}

int PoseEstimate::inference(vx_size yuv_address,int crop_box[4],PoseInfo& ret_struct)
{
	std::vector<blob> input_blobs;
	GetNetInputBlobs((vx_uint32*)net, input_blobs);
	if (input_blobs.size() != 1) {
		ERROR_INFO("this mode must be one input!");
		CHECK_FLAG();
	}
	//clock_t t1=clock(); //开始时间
	vx_int32 width = input_blobs[0].dims[0];
	vx_int32 height = input_blobs[0].dims[1];
	vx_int32 channel = input_blobs[0].dims[2];
	
	//创建保存转换后图片的tensor：1、分配内存，2、创建tensor封装该内存
	vx_size scale_img_handle = AllocateTensorDeviceMemory(context, width, height, channel, 1);
	
	
	if (scale_img_handle == NULL) {
		ERROR_INFO("alloc image handle error");
		CHECK_FLAG();
	}
	vx_tensor scale_img_tensor = CreateTensorFromHandle(context, width, height, channel, 1, scale_img_handle);
	if (scale_img_tensor == NULL) {
		ERROR_INFO("create image tensor error");
		CHECK_FLAG();
	}
	
	// 虚拟地址转物理地址
	vx_size phys_base_addr = lombo_vir_to_phy((void *)yuv_address);

	ALOGD("[ndk pose] get vitual address 0x%x 0x%x\r\n",phys_base_addr,yuv_address);
	
	input_image img;
	img.width = input_w;
	img.height = input_h;
	img.fmt = input_fmt;
	img.stride1 = y_stride;
	img.stride2 = uv_stride;
	img.handle1 = phys_base_addr;
	img.handle2 = phys_base_addr + input_w * input_h;	
	
	// 裁剪图像为3:4,保证等比缩放
	int center_x = (crop_box[2]+crop_box[0]) /2;
	int center_y = (crop_box[3]+crop_box[1]) /2;
	const int min_side_w = center_x < (input_w-center_x) ? center_x:(input_w-center_x);
	const int min_side_h = center_y < (input_h-center_y) ? center_y:(input_h-center_y);
	int crop_w = crop_box[2]-crop_box[0];
	int crop_h = crop_box[3]-crop_box[1];
	
	if((float)crop_w*0.75 > crop_h)
	{
		// 填充图片的高
		crop_h = (int)((float)crop_w*0.75);
		// 防止溢出
		if((crop_h/2)>min_side_h)
		{
			crop_h = min_side_h*2;
			crop_w = (int)((float)crop_h*1.33);
		}
	}
	else
	{
		//缩小图像宽度
		crop_w = (int)((float)crop_h*1.33);

		if((crop_w/2)>min_side_w)
		{
			crop_w = min_side_w*2;
			crop_h = (int)((float)crop_w*0.75);
		}
	}
	
	img.crop_x = center_x - crop_w/2;					//crop出来
	img.crop_y = center_y - crop_h/2;
	img.crop_w = crop_w;
	img.crop_h = crop_h;
	
	ImageConvertDeviceProcess(context, &img, scale_img_tensor);

	input_blobs[0].tensor = scale_img_tensor;
	SetNetInputBlobsTensor(graph, input_blobs);

	CHECK_STATUS(vxProcessGraph(graph));
	CHECK_FLAG();
	vxFinish(context);
	//output file
	std::vector<std::string> out_blob_name;
	std::vector<vx_tensor> out_blob;
	vx_status status = GetNetOutputBlobsTensor(graph, out_blob_name, out_blob);	
	
	float dst_data[5] = {0};
	vx_int8 * output_data;
	CHECK_STATUS(vxQueryTensor(out_blob[0], VX_TENSOR_MEMORY_HANDLE, &output_data, sizeof(output_data)));
	CHECK_FLAG();
	activation_function_softmax(output_data,dst_data,fl,5);
	
	for (int i = 0; i < input_blobs.size(); i++) {
		CHECK_STATUS(vxReleaseTensor(&input_blobs[i].tensor));
		CHECK_FLAG();
	}
	input_blobs.clear();
	
	out_blob_name.clear();
	for (int i = 0; i < out_blob.size(); i++) {
		CHECK_STATUS(vxReleaseTensor(&out_blob[i]));
		CHECK_FLAG();
	}
	std::vector<vx_tensor>().swap(out_blob);
	
	CHECK_STATUS(vxFreeDeviceMemory(context, scale_img_handle));
	CHECK_FLAG();

	ret_struct.normal_score = dst_data[1]; 
	ret_struct.lean_score = dst_data[0]; 
	ret_struct.down_score = dst_data[2]; 
	ret_struct.chin_score = dst_data[3]; 
	ret_struct.lopside_score = dst_data[4]; 
	return 0;
}






int DetHuman::inference(vx_size yuv_address,RET_DET_T& ret_struct)
{

	std::vector<blob> input_blobs;
	GetNetInputBlobs((vx_uint32*)net, input_blobs);
	if (input_blobs.size() != 1) {
		ERROR_INFO("this mode must be one input");
		CHECK_FLAG();
	}

	vx_int32 width = input_blobs[0].dims[0];
	vx_int32 height = input_blobs[0].dims[1];
	vx_int32 channel = input_blobs[0].dims[2];
	
	//创建保存转换后图片的tensor：1、分配内存，2、创建tensor封装该内存
	vx_size scale_img_handle = AllocateTensorDeviceMemory(context, width, height, channel, 1);
	
	
	if (scale_img_handle == NULL) {
		ERROR_INFO("alloc image handle error");
		CHECK_FLAG();

	}
	vx_tensor scale_img_tensor = CreateTensorFromHandle(context, width, height, channel, 1, scale_img_handle);
	if (scale_img_tensor == NULL) {
		ERROR_INFO("create image tensor error!");
		CHECK_FLAG();

	}
	// 虚拟地址转物理地址
	vx_size phys_base_addr = lombo_vir_to_phy((void *)yuv_address);
	ALOGD("[ndk people] get vitual address 0x%x 0x%x\r\n",phys_base_addr,yuv_address);
	input_image img;
	img.width = input_w;
	img.height = input_h;
	img.fmt = input_fmt;
	img.stride1 = y_stride;
	img.stride2 = uv_stride;

	img.handle1 = phys_base_addr;
	img.handle2 = phys_base_addr + input_w * input_h;	
	int minlen = input_w < input_h ? input_w :input_h;
	img.crop_x = (input_w-minlen)/2;  //取中间部分的地址，这里是为了防止左右两侧畸变较大的部分被拍进去
	img.crop_y = (input_h-minlen)/2;
	img.crop_w = minlen;
	img.crop_h = minlen;
	// image转换
	ImageConvertDeviceProcess(context, &img, scale_img_tensor);
	// 设置tensor
	input_blobs[0].tensor = scale_img_tensor;
	SetNetInputBlobsTensor(graph, input_blobs);

	//process graph
	CHECK_STATUS(vxProcessGraph(graph));
	CHECK_FLAG();
	vxFinish(context);
	//output file
	std::vector<std::string> out_blob_name;
	std::vector<vx_tensor> out_blob;
	GetNetOutputBlobsTensor(graph, out_blob_name, out_blob);

	
	//int output_data_size;
	std::vector<BoxInfo> dets;
	
	// 后处理
	dets = detect(out_blob,0.05,0.6);
	ALOGD("det person over\n");

	//ret_struct.score = dets[0].score > 0.6 ? dets[0].score : 0;
	ret_struct.score = dets[0].score ;
	float w_scale = (float)minlen/(float)input_blobs[0].dims[0];
	float h_scale = (float)minlen/(float)input_blobs[0].dims[1];
	
	// 做边界处理
	ret_struct.coords[0] = int(dets[0].x1*w_scale) + (input_w-minlen)/2;  
	ret_struct.coords[1] = int(dets[0].y1*h_scale) + (input_h-minlen)/2; 
	ret_struct.coords[2] = int(dets[0].x2*w_scale) + (input_w-minlen)/2;
	ret_struct.coords[3] = int(dets[0].y2*h_scale) + (input_h-minlen)/2; 


	// 释放内存开辟的内存空间
	for (int i = 0; i < input_blobs.size(); i++) {
		CHECK_STATUS(vxReleaseTensor(&input_blobs[i].tensor));
		CHECK_FLAG();
	}
	input_blobs.clear();
	CHECK_STATUS(vxFreeDeviceMemory(context, scale_img_handle));
	CHECK_FLAG();
	out_blob_name.clear();

	for (int i = 0; i < out_blob.size(); i++) {
		CHECK_STATUS(vxReleaseTensor(&out_blob[i]));
		CHECK_FLAG();
	}
	std::vector<vx_tensor>().swap(out_blob);

	return 0;
}
std::vector<BoxInfo> DetHuman::detect(const std::vector<vx_tensor>& out_blob, float score_threshold, float nms_threshold)
{
	
	std::vector<std::vector<BoxInfo>> results;
	
    results.resize(this->num_class);
	//std::cout<< this->heads_info.size()<<std::endl;
	
	vx_int8 * output_data = NULL;	
	// 反算bbox
    for (const auto& head_info : this->heads_info)
    { 	
    	int output_data_size = 0;
		CHECK_STATUS(vxQueryTensor(out_blob[head_info.out_layer], VX_TENSOR_MEMORY_SIZE, &output_data_size, sizeof(output_data_size)));
		CHECK_STATUS(vxQueryTensor(out_blob[head_info.out_layer], VX_TENSOR_MEMORY_HANDLE, &output_data, sizeof(output_data)));
        this->decode_infer(output_data, head_info.scale,head_info.stride, score_threshold, results);
    }

    std::vector<BoxInfo> dets;
	// nms
    for (int i = 0; i < (int)results.size(); i++)
    {
        this->nms(results[i], nms_threshold);        
        for (auto box : results[i])
        {
            dets.push_back(box);
        }
    }
	
    return dets;
}

void DetHuman::decode_infer(vx_int8 * output_data, float scale ,int stride, float threshold, std::vector<std::vector<BoxInfo>>& results)
{
	
    int feature_h = this->input_size / stride;
    int feature_w = this->input_size / stride;
	vx_int8 *p_out_data = &output_data[0];
	int map_side = 416/scale;

    for (int idx = 0; idx < feature_h * feature_w; idx++)
    {
    	int row = idx % feature_w;
        int col  = idx / feature_w;
    	p_out_data =  &output_data[(row*feature_h+col)*64];// HW形状，要做一次转换 ；ez在转换时，会自动做32位补齐，nanodet的输出 是1+4*11，所以取64位
        const float score = sigmoid(int8_to_float(p_out_data[0],scale)); //算score
        	
        int cur_label = 0;
        
		// 算bbox
        if (score > threshold)
        {
        	const vx_int8 * bbox_pred =(p_out_data+1);
        	results[0].push_back(this->disPred2Bbox(bbox_pred, cur_label, score, row, col,  stride,scale));
		}
    }
	
}

BoxInfo DetHuman::disPred2Bbox(const vx_int8*& dfl_det, int label, float score, int x, int y, int stride,float scale)
{
    float ct_x = (x + 0.5) * stride;
    float ct_y = (y + 0.5) * stride;
    std::vector<float> dis_pred;
    dis_pred.resize(4);
    for (vx_int8 i = 0; i < 4; i++)
    {
        float dis = 0;
        float* dis_after_sm = new float[this->reg_max + 1];

		// coord = 1*dfl_det[0] + 2*dfl_det[1] + ... + n*dfl_det[n-1] 算法来源于nanodet
        activation_function_softmax(dfl_det + i * (this->reg_max + 1), dis_after_sm, scale,(int)(this->reg_max + 1));
		
        for (int j = 0; j < this->reg_max + 1; j++)
        {
            dis += j * dis_after_sm[j];
			
        }
        dis *= stride;
        
        dis_pred[i] = dis;
        delete[] dis_after_sm;
    }
	
	float xmin = (std::max)(ct_x - dis_pred[0], .0f);
    float ymin = (std::max)(ct_y - dis_pred[1], .0f);
    float xmax = (std::min)(ct_x + dis_pred[2], (float)this->input_size);
    float ymax = (std::min)(ct_y + dis_pred[3], (float)this->input_size);

    return BoxInfo { xmin, ymin, xmax, ymax, score, label };
}

void DetHuman::nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
	std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        for (int j = i + 1; j < int(input_boxes.size());) {
            float xx1 = (std::max)(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = (std::max)(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = (std::min)(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = (std::min)(input_boxes[i].y2, input_boxes[j].y2);
            float w = (std::max)(float(0), xx2 - xx1 + 1);
            float h = (std::max)(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else {
                j++;
            }
        }
    }
}

