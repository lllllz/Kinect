/*******************************************************
* File:			MachineVision.h
* Date:			2016
* Author:		lllllz
* Description:	基于Kinect SDK开发的机器视觉项目
*******************************************************/
#pragma once

#ifndef MACHINEVISION_H_
#define MACHINEVISION_H_

#include "stdafx.h"
#include "TableA.h"

using namespace std;
using namespace cv;

// 机器视觉类
class MyMachineVision
{
	// 彩色图片的宽度
	static const int       cColorWidth = 1920;
	// 彩色图片的高度
	static const int       cColorHeight = 1080;

	// 深度图片的宽度
	static const int       cDepthWidth = 512;
	// 深度图片的高度
	static const int       cDepthHeight = 424;

public:
	// 构造函数
	MyMachineVision();

	// 析构函数
	~MyMachineVision();

	// 初始化Kinect
	HRESULT                InitializeDefaultSensor();

	// 更新
	void                   Update();

private:

	// 特征分析
	void                   Process();

	// 分析衣服颜色
	void                   ProcessClothes(Mat ClothesImg);

	// 更新身体数据
	HRESULT                UpdateBodyData(IBody** ppBodies);

	// 将身体点转换到屏幕空间
	Point				   BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height);

	// RGB转Mat
	Mat                    ConvertMat(const RGBQUAD* pBuffer, int nWidth, int nHeight);
           
	void                   GetColor(Mat img_in, string* color);
          
	void                   ColorReduce(Mat img, string* color);

	int                    GetMax();

	float                  GetDistance(CameraSpacePoint* pos, int size = 1);

	// 句柄
	HWND                   m_hWnd;

	// 当前Kinect
	IKinectSensor*         m_pKinectSensor;

	// 坐标映射器
	ICoordinateMapper*     m_pCoordinateMapper;

	// 彩色阅读器
	IColorFrameReader*     m_pColorFrameReader;

	// 身体帧
	IBodyFrame*			   m_pBodyFrame;
	// 身体输入源
	IBodyFrameSource*	   m_pBodyFrameSources;
	// 身体阅读器
	IBodyFrameReader*      m_pBodyFrameReader;

	// 脸部输入源
	IFaceFrameSource*	   m_pFaceFrameSources[BODY_COUNT];
	// 脸部阅读器
	IFaceFrameReader*	   m_pFaceFrameReaders[BODY_COUNT];

	IDepthFrameReader*     m_pDepthFrameReader;//用于深度数据读取

	// 脸部框
	RectI                  m_pFaceBox;

	RGBQUAD*               m_pColorRGBX;

	RGBQUAD*               m_pDepthRGBX;
	// 彩色图像数据
	Mat                    m_ColorImg;

	Mat                    m_Clothes[6];

	// 人物信息
	PersonInfo             m_PersonInfo[6];

	TableA                 m_TableA;

	// 关注某人时的脸部框 
	RECT                   m_Rect;

	Rect                   m_ClothesBox;

	// 衣服颜色识别 
	Mat                    m_rgbImg;
	Mat                    m_hsvImg;

	int                    m_color[10];
};

#endif // MACHINEVISION_H_