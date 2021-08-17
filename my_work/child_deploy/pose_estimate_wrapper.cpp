#include "pose_estimate.h"
#include "pose_estimate_wrapper.h"
#include <vector>
#include <iostream>
#include <stdio.h>
#include <log/log.h>


static EzInference*  pdet_model =NULL;
static EzInference*  ppose_estim_model =NULL;
#define LOG_TAG "POSE_ALGR"
// 获取推理句柄
int InitPoseEstimate(int w,int h)
{
	pdet_model = new DetHuman("/usr/bin/det",w,h);
	if((pdet_model == NULL)&&(pdet_model->check_status()!= 0))
	{
		return 1;
	}

	ppose_estim_model = new PoseEstimate("/usr/bin/pose",w,h,8);

	if((ppose_estim_model == NULL)&&(ppose_estim_model->check_status()!= 0))
	{
		return 1;
	}
	return 0;
}


// 推理单帧图片.
RET_DET_T predict_person(void* mem_size)
{
	printf("predict_person\n");
	RET_DET_T info;
	if(pdet_model->inference((vx_size)mem_size,info))
	{
		ALOGE("predict person error\r\n");
	}

	return info;
}

# if 1
// 推理单帧图片.
PoseInfo predict_pose(void* mem_size,int crop_box[4])
{
	PoseInfo info;
	if(ppose_estim_model->inference((vx_size)mem_size,crop_box,info))
	{
		ALOGE("predict pose error\r\n");
	}
	return info;
}
# endif
int ReleaseHandle(void)
{
//	printf("release the handle\r\n");
	delete pdet_model;
	delete ppose_estim_model;
	return 0;
}