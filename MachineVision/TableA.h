#pragma once

#ifndef TABLEA_H_
#define TABLEA_H_

#include "stdafx.h"

using namespace std;
using namespace cv;

// 人物信息结构体
struct PersonInfo
{
	PersonInfo();
	PersonInfo(PersonInfo const& rhs);
	PersonInfo const& operator=(PersonInfo const& rhs);
	friend ostream& operator<<(ostream& os, PersonInfo& p)
	{
		return os << " 追踪ID: " << p.TrackId << "  图片ID: " << p.FacePicId << "  衣服前面的颜色: " << p.FrontColor[0] << "  " << p.FrontColor[1] << "是否戴眼镜: " << p.WearingGlasses
			<< "临时ID: " << p.tempID << endl;
	}

	UINT64 TrackId;       				 // 身体ID       			默认为0
	int    Happy;          					// 是否高兴    	 		0为不确定,1为是,2为否 默认为0          
	int    Engaged;        				// 是否订婚     			0为不确定,1为是,2为否 默认为0 
	int    LeftEyeClosed;  		// 是否闭左眼   		0为不确定,1为是,2为否 默认为0 
	int    RightEyeClosed; 		// 是否闭右眼   		0为不确定,1为是,2为否 默认为0 
	int    LookingAway;    			// 是否四处看   		0为不确定,1为是,2为否 默认为0 
	int    MouthMoved;     			// 是否动嘴     			0为不确定,1为是,2为否 默认为0 
	int    MouthOpen;      				// 是否张嘴     			0为不确定,1为是,2为否 默认为0 
	int    WearingGlasses; 		// 是否戴眼镜   		0为不确定,1为是,2为否 默认为0 
	string FrontColor[2];  	// 衣服正面颜色	 	最显著的2种颜色 默认为""
	string BehindColor[2]; 	// 衣服背面颜色 		最显著的2种颜色 默认为""
	float  Height;         				// 身高         				cm为单位 默认为0
	int    Gender;         					// 性别         				1为男性,2为女性 默认为0 
	int    Age;            						// 年龄         				默认为0
	bool   Valid;          					// 是否有效     			true为有效,false为无效
	bool   HaveFace;
	float  PeopleDistance;
	Mat    FaceImg;
};

// 表A
struct TableA
{
	TableA();
	TableA(TableA const& rhs);
	TableA const& operator=(TableA const& rhs);

	float  Distance[5];
};

#endif //TABLEA_H_