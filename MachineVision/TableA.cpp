#include "stdafx.h"
#include "TableA.h"

PersonInfo::PersonInfo()
{
	TrackId = 0;         // 身体ID       默认为0
	Happy = 0;           // 是否高兴     1为是,2为否,0为不确定           
	Engaged = 0;         // 是否忙碌     1为是,2为否,0为不确定
	LeftEyeClosed = 0;   // 是否闭左眼   1为是,2为否,0为不确定
	RightEyeClosed = 0;  // 是否闭右眼   1为是,2为否,0为不确定
	LookingAway = 0;     // 是否四处看   1为是,2为否,0为不确定
	MouthMoved = 0;      // 是否动嘴     1为是,2为否,0为不确定
	MouthOpen = 0;       // 是否张嘴     1为是,2为否,0为不确定
	WearingGlasses = 0;  // 是否戴眼镜   1为是,2为否,0为不确定
	FrontColor[0] = "";  // 衣服正面颜色 最显著的2种颜色
	FrontColor[1] = "";
	BehindColor[0] = ""; // 衣服背面颜色 最显著的2种颜色
	BehindColor[1] = "";
	Height = 0;          // 身高         cm为单位,默认为0
	Gender = 0;          // 性别         1为男性,2为女性,默认为0 
	Age = 0;             // 年龄         默认为0
	Valid = false;		 // 判断数据是否有效
	HaveFace = false;    // 判断是否有脸部图像
	PeopleDistance = 0;
}

PersonInfo::PersonInfo(PersonInfo const& rhs)
{
	TrackId = rhs.TrackId;               												// 身体ID       默认为0
	Happy = rhs.Happy;                 													  	// 是否高兴     1为是,2为否,0为不确定           
	Engaged = rhs.Engaged;               												// 是否忙碌     1为是,2为否,0为不确定
	LeftEyeClosed = rhs.LeftEyeClosed;   							// 是否闭左眼   1为是,2为否,0为不确定
	RightEyeClosed = rhs.RightEyeClosed; 						// 是否闭右眼   1为是,2为否,0为不确定
	LookingAway = rhs.LookingAway;       								// 是否四处看   1为是,2为否,0为不确定
	MouthMoved = rhs.MouthMoved;         									// 是否动嘴     1为是,2为否,0为不确定
	MouthOpen = rhs.MouthOpen;          										 // 是否张嘴     1为是,2为否,0为不确定
	WearingGlasses = rhs.WearingGlasses; 						// 是否戴眼镜   1为是,2为否,0为不确定

	FrontColor[0] = rhs.FrontColor[0];   								// 衣服正面颜色 最显著的2种颜色
	FrontColor[1] = rhs.FrontColor[1];
	BehindColor[0] = rhs.BehindColor[0]; 							// 衣服背面颜色 最显著的2种颜色
	BehindColor[1] = rhs.BehindColor[1];

	Height = rhs.Height;                								 					// 身高         cm为单位,默认为0
	Gender = rhs.Gender;                 												// 性别         1为男性,2为女性,默认为0 
	Age = rhs.Age;                       															// 年龄         默认为0
	Valid = rhs.Valid;	               	 													// 判断数据是否有效
	HaveFace = rhs.HaveFace;
	FaceImg = rhs.FaceImg;

	PeopleDistance = rhs.PeopleDistance;
}

PersonInfo const& PersonInfo::operator=(PersonInfo const& rhs)
{
	if (this != &rhs)
	{
		TrackId = rhs.TrackId;               												// 身体ID       默认为0
		Happy = rhs.Happy;                 													  	// 是否高兴     1为是,2为否,0为不确定           
		Engaged = rhs.Engaged;               												// 是否忙碌     1为是,2为否,0为不确定
		LeftEyeClosed = rhs.LeftEyeClosed;   							// 是否闭左眼   1为是,2为否,0为不确定
		RightEyeClosed = rhs.RightEyeClosed; 						// 是否闭右眼   1为是,2为否,0为不确定
		LookingAway = rhs.LookingAway;       								// 是否四处看   1为是,2为否,0为不确定
		MouthMoved = rhs.MouthMoved;         									// 是否动嘴     1为是,2为否,0为不确定
		MouthOpen = rhs.MouthOpen;          										 // 是否张嘴     1为是,2为否,0为不确定
		WearingGlasses = rhs.WearingGlasses; 						// 是否戴眼镜   1为是,2为否,0为不确定

		FrontColor[0] = rhs.FrontColor[0];   								// 衣服正面颜色 最显著的2种颜色
		FrontColor[1] = rhs.FrontColor[1];
		BehindColor[0] = rhs.BehindColor[0]; 							// 衣服背面颜色 最显著的2种颜色
		BehindColor[1] = rhs.BehindColor[1];

		Height = rhs.Height;                								 					// 身高         cm为单位,默认为0
		Gender = rhs.Gender;                 												// 性别         1为男性,2为女性,默认为0 
		Age = rhs.Age;                       															// 年龄         默认为0
		Valid = rhs.Valid;	               	 													// 判断数据是否有效
		HaveFace = rhs.HaveFace;
		FaceImg = rhs.FaceImg;

		PeopleDistance = rhs.PeopleDistance;
	}
	return *this;
}

TableA::TableA()
{
	for (int i = 0; i < 5; i++)
	{
		Distance[i] = 0;
	}
}

TableA::TableA(TableA const& rhs)
{
	for (int i = 0; i < 5; i++)
	{
		Distance[i] = rhs.Distance[i];
	}
}

TableA const& TableA::operator=(TableA const& rhs)
{
	if (this != &rhs)
	{
		for (int i = 0; i < 5; i++)
		{
			Distance[i] = rhs.Distance[i];
		}
	}
	return *this;
}