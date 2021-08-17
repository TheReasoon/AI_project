#ifndef __POSE_ESTIMATE_H__
#define __POSE_ESTIMATE_H__


#include <VX/vx.h>
#include <vx_ext_amd.h>
#include <net_api.h>
#include <math.h>
#include <memory.h>
#include <vx_log.h>
#include "pose_estimate_wrapper.h"
#include <cstring>



typedef struct HeadInfo
{
    int out_layer;
    int stride;
	float scale;
};

typedef struct BoxInfo 
{
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
} BoxInfo;



class EzInference
{
protected:
	int error_flag ;


	vx_int8 * net;
	vx_int8 * blobs;
	vx_context context;
	vx_graph graph;
	vx_size img_handle;
	img_fmt input_fmt;

	int input_w;
    int input_h;
	int y_stride;
	int uv_stride;
public:
	EzInference(const char* model_path,int w,int h);
	~EzInference();
	int	check_status() const;
	virtual int inference(vx_size yuv_address,int crop_box[4],PoseInfo& ret_struct)
	{	}

	virtual int inference(vx_size yuv_address,RET_DET_T& ret_struct)
	{	}
};


class PoseEstimate:public EzInference
{
public:
	int inference(vx_size yuv_address,int crop_box[4],PoseInfo& ret_struct) override;
	PoseEstimate(const char* model_path,int input_w,int input_h,int fl):EzInference(model_path,input_w,input_h),fl(fl)
	{	
	}
	~PoseEstimate();
private:
	const int fl;
}	;


class DetHuman:public EzInference
{
public:
	int inference(vx_size yuv_address,RET_DET_T& ret_struct) override;
	DetHuman(const char* model_path,int input_w,int input_h):EzInference(model_path,input_w,input_h)
	{
		
	}
	~DetHuman();
private:

	
	std::vector<HeadInfo> heads_info{
        // cls_pred|dis_pred|scale
            {2,    8,  16.},//"conv_65"
            {1,   16,  16.},//"Conv_78"
            {0,   32,  32.},//"Conv_91"
    };
	std::vector<BoxInfo> detect(const std::vector<vx_tensor>& out_blob, float score_threshold, float nms_threshold);
	void decode_infer(vx_int8 * output_data,   float scale,int stride, float threshold, std::vector<std::vector<BoxInfo>>& results);
    BoxInfo disPred2Bbox(const vx_int8*& dfl_det, int label, float score, int x, int y, int stride,float scale);
    static void nms(std::vector<BoxInfo>& result, float nms_threshold);
    int input_size = 416;
    int num_class = 1;
    vx_int8 reg_max = 10;
};


#endif 










