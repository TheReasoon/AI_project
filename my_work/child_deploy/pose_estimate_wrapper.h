#ifndef __POSE_ESTIMATE_WRAPPER_H__
#define __POSE_ESTIMATE_WRAPPER_H__

#ifdef __cplusplus

extern "C" {

#endif
typedef struct poseinfo
{
    float normal_score;
    float lean_score;
	float down_score;
    float lopside_score;
    float chin_score;
} PoseInfo;

typedef struct RET_DET{
	float score;
	int coords[4];
}RET_DET_T;


// 初始化侦测网络，返回值为0表示初始化成功,返回值为1表示失败
extern int InitPoseEstimate(int w,int h);
// 返回值为一个结构体，当score为0时，表示没有检测到人或者读取失败。 输入参数为图片在内存里的物理地址。
extern RET_DET_T predict_person(void* virtual_size);

// 返回值为坐姿的置信度，分别表示 正常 斜靠 趴下 托腮 高低肩，取其中最大置信度做为
extern PoseInfo predict_pose(void* mem_size,int crop_box[4]);
// 释放空间
extern int ReleaseHandle(void);


#ifdef __cplusplus
}
#endif

#endif
