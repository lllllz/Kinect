// MachineVision.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MachineVision.h"
#include "time.h"

//using namespace std;
//using namespace cv;

static const float c_JointThickness = 3.0f;
static const float c_TrackedBoneThickness = 6.0f;
static const float c_InferredBoneThickness = 1.0f;
static const float c_HandSize = 30.0f;

static const DWORD c_FaceFrameFeatures =
FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
| FaceFrameFeatures::FaceFrameFeatures_Happy
| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
| FaceFrameFeatures::FaceFrameFeatures_LookingAway
| FaceFrameFeatures::FaceFrameFeatures_Glasses
| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;
/*
int main(int argc , _TCHAR* argv[])
{
MyMachineVision mv;
mv.InitializeDefaultSensor();
//	cout << "init finish" << endl;
mv.Update();
return 0;
}
*/
MyMachineVision::MyMachineVision() :
// 初始化表
m_hWnd(NULL),
m_pKinectSensor(nullptr),
m_pCoordinateMapper(nullptr),
m_pColorFrameReader(nullptr),
m_pBodyFrameReader(nullptr),
m_pBodyFrameSources(nullptr)
{
	srand(time(NULL));
	for (int i = 0; i < BODY_COUNT; i++)
	{
		m_pFaceFrameSources[i] = nullptr; // 初始化FaceFrame资源
		m_pFaceFrameReaders[i] = nullptr; // 初始化FaceFrame阅读器
	}

	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	m_pDepthRGBX = new RGBQUAD[cDepthWidth * cDepthHeight];

	m_ColorImg = Mat::zeros(cColorHeight, cColorWidth, CV_8UC3);

	m_Rect.left = 810;
	m_Rect.top = 420;
	m_Rect.right = 1110;
	m_Rect.bottom = 660;

	m_ClothesBox.x = 0;
	m_ClothesBox.y = 0;
	m_ClothesBox.width = 0;
	m_ClothesBox.height = 0;

	m_pFaceBox = { 0 };

}

MyMachineVision::~MyMachineVision()
{
	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = nullptr;
	}

	// 操作资源和阅读器
	for (int i = 0; i < BODY_COUNT; i++)
	{
		SafeRelease(m_pFaceFrameSources[i]);
		SafeRelease(m_pFaceFrameReaders[i]);
	}

	// 释放了body的阅读器
	SafeRelease(m_pBodyFrameReader);

	// 释放了color的阅读器 
	SafeRelease(m_pColorFrameReader);

	// 释放了坐标映射
	SafeRelease(m_pCoordinateMapper);

	// 关闭Kinect对象
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	SafeRelease(m_pKinectSensor);

	m_ColorImg.release();
}

// 初始化Kinect
HRESULT MyMachineVision::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// 初始化Kinect和颜色,身体和脸的阅读器
		IColorFrameSource* pColorFrameSource = nullptr;
		IBodyFrameSource* pBodyFrameSource = nullptr;   // 读取骨骼信息
		IDepthFrameSource* pDepthFrameSource = nullptr; // 读取深度信息

		hr = m_pKinectSensor->Open();
		BOOLEAN bOpen = FALSE;
		// 一直等待直到Kinect打开完成  
		while (!bOpen)
		{
			m_pKinectSensor->get_IsOpen(&bOpen);
			Sleep(200);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}

		if (SUCCEEDED(hr)){
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr)){
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		if (SUCCEEDED(hr))
		{
			// 创建一个脸部帧源+读者跟踪视场中的每个身体
			for (int i = 0; i < BODY_COUNT; i++)
			{
				if (SUCCEEDED(hr))
				{
					// create the face frame source by specifying the required face frame features
					hr = CreateFaceFrameSource(m_pKinectSensor, 0, c_FaceFrameFeatures, &m_pFaceFrameSources[i]);
				}
				if (SUCCEEDED(hr))
				{
					// open the corresponding reader
					hr = m_pFaceFrameSources[i]->OpenReader(&m_pFaceFrameReaders[i]);
				}
			}
		}

		SafeRelease(pColorFrameSource);
		SafeRelease(pBodyFrameSource);
		SafeRelease(pDepthFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		printf("Kinect Not Find\n");
		return E_FAIL;
	}

	return hr;
}

// 更新
void MyMachineVision::Update()
{
	if (!m_pColorFrameReader || !m_pBodyFrameReader || !m_pDepthFrameReader)
	{
		return;
	}
	char key = 0;
	//for( int i = 0; i < 200; i++ )
	//{
	while (true)
	{
		IColorFrame* pColorFrame = nullptr;
		IDepthFrame* pDepthFrame = nullptr; // 深度信息

		// 得到最近的彩色帧
		HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

		if (SUCCEEDED(hr))
		{
			IFrameDescription* pFrameDescription = nullptr;
			int nWidth = 0;
			int nHeight = 0;
			ColorImageFormat imageFormat = ColorImageFormat_None;
			UINT nBufferSize = 0;
			RGBQUAD* pColorBuffer = nullptr;

			if (SUCCEEDED(hr))
			{
				// 得到最近的彩色帧的描述
				hr = pColorFrame->get_FrameDescription(&pFrameDescription);
			}

			if (SUCCEEDED(hr))
			{
				hr = pFrameDescription->get_Width(&nWidth);
			}

			if (SUCCEEDED(hr))
			{
				hr = pFrameDescription->get_Height(&nHeight);
			}

			if (SUCCEEDED(hr))
			{
				// 得到最近的彩色帧数据的格式
				hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
			}

			if (SUCCEEDED(hr))
			{
				if (imageFormat == ColorImageFormat_Bgra)
				{
					// 得到最近的彩色帧数据的指针
					hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
				}
				else if (m_pColorRGBX)
				{
					pColorBuffer = m_pColorRGBX;
					nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
					// 复制转换格式后的彩色帧数据到数组
					hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
				}
				else
				{
					hr = E_FAIL;
				}
				m_ColorImg = ConvertMat(pColorBuffer, cColorWidth, cColorHeight);
			}

			if (SUCCEEDED(hr))
			{
				try{
					Process();
				}
				catch (...){
					cout << "人脸分析函数Process异常,已忽略!" << endl;
				}

				for (int i = 0; i < 6; i++)
				{
					if (m_PersonInfo[i].Valid)
					{
						// 人物信息有效 取得人物信息
						break;
					}
				}
			}
			SafeRelease(pFrameDescription);
		}
		SafeRelease(pColorFrame);

		hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

		if (SUCCEEDED(hr))
		{
			IFrameDescription* pFrameDescription = NULL;
			int nWidth = 0;
			int nHeight = 0;
			USHORT nDepthMinReliableDistance = 0;
			USHORT nDepthMaxDistance = 0;
			UINT nBufferSize = 0;
			UINT16 *pBuffer = NULL;

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
			}

			if (SUCCEEDED(hr))
			{
				hr = pFrameDescription->get_Width(&nWidth);
			}

			if (SUCCEEDED(hr))
			{
				hr = pFrameDescription->get_Height(&nHeight);
			}

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
			}

			if (SUCCEEDED(hr))
			{
				nDepthMaxDistance = USHRT_MAX;
			}

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
			}

			if (SUCCEEDED(hr))
			{
				DepthSpacePoint pDepthSpacePoint[5][5] = {
					{ { 236, 92 }, { 276, 92 }, { 256, 112 }, { 236, 132 }, { 276, 132 } },
					{ { 136, 192 }, { 176, 192 }, { 156, 212 }, { 136, 232 }, { 176, 232 } },
					{ { 236, 192 }, { 376, 192 }, { 256, 212 }, { 236, 232 }, { 276, 232 } },
					{ { 336, 192 }, { 376, 192 }, { 356, 212 }, { 336, 232 }, { 376, 232 } },
					{ { 236, 292 }, { 276, 292 }, { 256, 312 }, { 236, 332 }, { 276, 332 } }
				};
				CameraSpacePoint pCameraSpacePoint[5][5] = { 0 };
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 5; j++)
					{
						int place = pDepthSpacePoint[i][j].Y * cDepthWidth + pDepthSpacePoint[i][j].X;
						m_pCoordinateMapper->MapDepthPointToCameraSpace(pDepthSpacePoint[i][j], pBuffer[place], &pCameraSpacePoint[i][j]);
					}
				}
				for (int i = 0; i < 5; i++)
				{
					m_TableA.Distance[i] = GetDistance(pCameraSpacePoint[i], 5);
				}
			}
			SafeRelease(pFrameDescription);
		}

		SafeRelease(pDepthFrame);

		cv::imshow("color", m_ColorImg);

		//cout << "RunTime:" << c << endl;
		key = waitKey(1);
		if (key == 27)
		{
			break;
		}
	}
}

// 特征分析
void MyMachineVision::Process()
{
	HRESULT hr;
	IBody* ppBodies[BODY_COUNT] = { 0 };
	bool bHaveBodyData = SUCCEEDED(UpdateBodyData(ppBodies));
	// 遍历每一个阅读器
	for (int iFace = 0; iFace < BODY_COUNT; ++iFace)
	{
		// 从这个阅读器检索最新的脸部帧
		IFaceFrame* pFaceFrame = nullptr;
		hr = m_pFaceFrameReaders[iFace]->AcquireLatestFrame(&pFaceFrame);

		BOOLEAN bFaceTracked = false;
		if (SUCCEEDED(hr) && nullptr != pFaceFrame)
		{
			// 获取一个布尔值表示如果追踪ID是有效的
			hr = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
		}
		if (SUCCEEDED(hr))
		{
			if (bFaceTracked)
			{
				IFaceFrameResult* pFaceFrameResult = nullptr;
				DetectionResult faceProperties[FaceProperty::FaceProperty_Count];

				// 获取脸部帧的结果
				hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);

				// 需要验证之前如果pFaceFrameResult包含数据试图访问它
				if (SUCCEEDED(hr) && pFaceFrameResult != nullptr)
				{
					// 在彩色空间获取脸部边框
					hr = pFaceFrameResult->get_FaceBoundingBoxInColorSpace(&m_pFaceBox);
					if (SUCCEEDED(hr))
					{
						// 获取脸部属性
						hr = pFaceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
					}

					if ((m_pFaceBox.Left != 0) && (m_pFaceBox.Right != 0))
					{

						Rect rec;
						rec.x = m_pFaceBox.Left;
						rec.y = m_pFaceBox.Top;
						rec.width = m_pFaceBox.Right - m_pFaceBox.Left;
						rec.height = m_pFaceBox.Bottom - m_pFaceBox.Top;
						cv::rectangle(m_ColorImg, rec, Scalar(255, 255, 255));

						//		cout << "process" << endl;
						m_ColorImg(Rect(m_pFaceBox.Left, m_pFaceBox.Top, m_pFaceBox.Right - m_pFaceBox.Left, m_pFaceBox.Bottom - m_pFaceBox.Top)).copyTo(m_PersonInfo[iFace].FaceImg);

						m_PersonInfo[iFace].HaveFace = TRUE;

						for (int iProperty = 0; iProperty < FaceProperty::FaceProperty_Count; iProperty++)
						{
							int ret = 0;
							switch (faceProperties[iProperty])
							{
							case DetectionResult::DetectionResult_Unknown:
								ret = 0;
								break;
							case DetectionResult::DetectionResult_Yes:
								ret = 1;
								break;

							case DetectionResult::DetectionResult_No:
							case DetectionResult::DetectionResult_Maybe:
								ret = 2;
								break;
							default:
								break;
							}

							switch (iProperty)
							{
							case FaceProperty::FaceProperty_Happy:
								m_PersonInfo[iFace].Happy = ret;
								break;
							case FaceProperty::FaceProperty_Engaged:
								m_PersonInfo[iFace].Engaged = ret;
								break;
							case FaceProperty::FaceProperty_LeftEyeClosed:
								m_PersonInfo[iFace].LeftEyeClosed = ret;
								break;
							case FaceProperty::FaceProperty_RightEyeClosed:
								m_PersonInfo[iFace].RightEyeClosed = ret;
								break;
							case FaceProperty::FaceProperty_LookingAway:
								m_PersonInfo[iFace].LookingAway = ret;
								break;
							case FaceProperty::FaceProperty_MouthMoved:
								m_PersonInfo[iFace].MouthMoved = ret;
								break;
							case FaceProperty::FaceProperty_MouthOpen:
								m_PersonInfo[iFace].MouthOpen = ret;
								break;
							case FaceProperty::FaceProperty_WearingGlasses:
								m_PersonInfo[iFace].WearingGlasses = ret;
								break;
							default:
								break;
							}

						}
					}
					else
					{
						m_PersonInfo[iFace].HaveFace = FALSE;
					}
				}

				SafeRelease(pFaceFrameResult);
			}
			else
			{
				m_PersonInfo[iFace].HaveFace = FALSE;
				// 脸跟踪无效,试图解决这个问题
				// 一个有效的身体是需要执行这个步骤

				if (bHaveBodyData)
				{
					// 检查相应的身体被跟踪
					// 如果这是真的,那么更新帧源追踪这个身体
					IBody* pBody = ppBodies[iFace];
					if (pBody != nullptr)
					{
						BOOLEAN bTracked = false;
						hr = pBody->get_IsTracked(&bTracked);

						UINT64 bodyTId;
						if (SUCCEEDED(hr) && bTracked)
						{
							// 这个身体的跟踪ID
							hr = pBody->get_TrackingId(&bodyTId);
							if (SUCCEEDED(hr))
							{
								// 更新帧与跟踪源ID
								m_pFaceFrameSources[iFace]->put_TrackingId(bodyTId);
							}
						}
					}
				}

			}
		}
		SafeRelease(pFaceFrame);


		IBody* pBody = ppBodies[iFace];

		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			m_PersonInfo[iFace].Valid = bTracked;
			if (m_PersonInfo[iFace].Valid)
			{
				if (SUCCEEDED(hr) && bTracked)
				{
					pBody->get_TrackingId(&m_PersonInfo[iFace].TrackId);
					Joint joints[JointType_Count]; // 存储关节点类

					// 存储深度坐标系中的关节点位置

					ColorSpacePoint* pColorSpacePosition = new ColorSpacePoint[_countof(joints)];
					//	DepthSpacePoint* pDepthSpacePosition = new DepthSpacePoint[_countof(joints)];
					// 获得关节点类
					hr = pBody->GetJoints(_countof(joints), joints);
					if (SUCCEEDED(hr))
					{
						for (int j = 0; j < _countof(joints); ++j)
						{
							// 将关节点坐标从摄像机坐标系（-1~1）转到彩色坐标系
							m_pCoordinateMapper->MapCameraPointToColorSpace(joints[j].Position, &pColorSpacePosition[j]);
							//  m_pCoordinateMapper->MapCameraPointToDepthSpace(joints[j].Position , &pDepthSpacePosition[j]);
						}
						//pDepthSpacePosition[JointType_SpineShoulder].
						//	m_PersonInfo[iFace].Distance = joints[JointType_SpineMid].Position.Z;
						m_PersonInfo[iFace].PeopleDistance = GetDistance(&joints[JointType_SpineMid].Position);
						//cout << "People Distance:" << m_PersonInfo[iFace].PeopleDistance << endl;

						CvPoint
							p1 = cvPoint(pColorSpacePosition[JointType_SpineShoulder].X, pColorSpacePosition[JointType_SpineShoulder].Y + 30),
							p2 = cvPoint(pColorSpacePosition[JointType_SpineMid].X, pColorSpacePosition[JointType_SpineMid].Y);

						p1.x = p1.x - ((p2.y - p1.y) / 2);
						p2.x = p2.x + ((p2.y - p1.y) / 2);

						//	line(m_ColorImg , p1 , p2 , cvScalar(0 , 0 , 255));

						if (((p2.x - p1.x) > 10) && ((p2.y - p1.y) > 10))
						{
							cv::rectangle(m_ColorImg, Rect(p1, p2), Scalar(0, 0, 255));
							try
							{
								m_Clothes[iFace] = m_ColorImg(Rect(p1, p2));
							}
							catch (...)
							{
								cout << "分析图片模糊,致出现异常,异常已忽略!" << endl;
							}

							if (m_PersonInfo[iFace].HaveFace)
							{
								GetColor(m_Clothes[iFace], m_PersonInfo[iFace].FrontColor);
							}
							else
							{
								GetColor(m_Clothes[iFace], m_PersonInfo[iFace].BehindColor);
							}
						}

					}
					delete[] pColorSpacePosition;
				}
			}
		}
		SafeRelease(ppBodies[iFace]); // 释放身体	

	}

}

float MyMachineVision::GetDistance(CameraSpacePoint* pos, int size)
{
	int cnt = 0;
	float average = 0;
	for (int i = 0; i < size; i++)
	{
		//	cout << "Distance[" << i << "]:" << pos[i].Z << endl;
		if ((!_finite(pos[i].Z)) && (!_isnan(pos[i].Z)))
		{
			continue;
		}
		average += pos[i].Z;
		cnt++;
	}

	return average > 0? average / cnt: -1;
}


// 更新身体数据
HRESULT MyMachineVision::UpdateBodyData(IBody** ppBodies)
{
	HRESULT hr = E_FAIL;

	if (m_pBodyFrameReader != nullptr)
	{
		IBodyFrame* pBodyFrame = nullptr;
		hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
		}
		SafeRelease(pBodyFrame);
	}

	return hr;
}

// 将身体点转换到屏幕空间
Point MyMachineVision::BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height)
{
	// Calculate the body's position on the screen
	DepthSpacePoint depthPoint = { 0 };
	m_pCoordinateMapper->MapCameraPointToDepthSpace(bodyPoint, &depthPoint);

	Point screenPoint = { 0 };

	screenPoint.x = static_cast<float>(depthPoint.X * width) / cDepthWidth;
	screenPoint.y = static_cast<float>(depthPoint.Y * height) / cDepthHeight;

	return screenPoint;
}

// RGB转Mat
Mat MyMachineVision::ConvertMat(const RGBQUAD* pBuffer, int nWidth, int nHeight)
{

	cv::Mat img(nHeight, nWidth, CV_8UC3);
	uchar* p_mat = img.data;

	const RGBQUAD* pBufferEnd = pBuffer + (nWidth * nHeight);

	while (pBuffer < pBufferEnd)
	{
		*p_mat = pBuffer->rgbBlue;
		p_mat++;
		*p_mat = pBuffer->rgbGreen;
		p_mat++;
		*p_mat = pBuffer->rgbRed;
		p_mat++;

		++pBuffer;
	}
	return img;

}

void MyMachineVision::GetColor(Mat img_in, string* color)
{
	//	cout << "1" << endl; 
	/*
	if( img_in.channels() != 1 )
	{
	cvtColor(img_in , m_hsvImg , CV_BGR2HSV);
	}
	else
	{
	img_in.copyTo(m_hsvImg);
	}
	*/
	//	cout << "error -1" << endl;
	cvtColor(img_in, m_hsvImg, CV_BGR2HSV);
	//	cout << "error 0" << endl;
	//	cout << "2" << endl;
	Mat Image_2(m_hsvImg.size(), CV_32FC3);
	//	cout << "error 1" << endl;
	m_hsvImg.convertTo(Image_2, CV_32FC3);
	//	cout << "error 2" << endl;
	Mat h(m_hsvImg.rows, m_hsvImg.cols, CV_32FC1);
	Mat s(m_hsvImg.rows, m_hsvImg.cols, CV_32FC1);
	Mat v(m_hsvImg.rows, m_hsvImg.cols, CV_32FC1);
	//	cout << "error 3" << endl;
	Mat out[] = { h, s, v };
	split(Image_2, out);
	//	cout << "error 4" << endl;
	h = out[0] / 255;
	s = out[1] / 255;
	v = out[2] / 255;

	resize(m_hsvImg, m_hsvImg, Size(m_hsvImg.rows / 5, m_hsvImg.cols / 5));
	//	cout << "error 5" << endl;
	//	imshow("color", m_hsvImg);

	ColorReduce(m_hsvImg, color);
	//	cout << "error 6" << endl;
}

void MyMachineVision::ColorReduce(Mat img, string* color)
{
	//	cout << "error 7" << endl;
	//int color[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };//0黑1灰2白3红4橙5黄6绿7青8蓝9紫
	for (int i = 1; i < img.cols; i++)
	{
		for (int j = 1; j < img.rows; j++)
		{
			//判断彩色
			if (((43 <= img.at<Vec3b>(j, i)[1]) && (img.at<Vec3b>(j, i)[1] <= 255)) &&
				((46 <= img.at<Vec3b>(j, i)[2]) && (img.at<Vec3b>(j, i)[2] <= 255)))
			{
				//橙色
				if ((11 <= img.at<Vec3b>(j, i)[0]) && (img.at<Vec3b>(j, i)[0] <= 25)){ m_color[4] += 1; }
				//黄色
				else if ((25 <= img.at<Vec3b>(j, i)[0]) && (img.at<Vec3b>(j, i)[0] <= 34)){ m_color[5] += 1; }
				//绿色
				else if ((34 <= img.at<Vec3b>(j, i)[0]) && (img.at<Vec3b>(j, i)[0] <= 77)){ m_color[6] += 1; }
				//青色
				else if ((78 <= img.at<Vec3b>(j, i)[0]) && (img.at<Vec3b>(j, i)[0] <= 99)){ m_color[7] += 1; }
				//蓝色
				else if ((100 <= img.at<Vec3b>(j, i)[0]) && (img.at<Vec3b>(j, i)[0] <= 124)){ m_color[8] += 1; }
				//紫色
				else if ((125 <= img.at<Vec3b>(j, i)[0]) && (img.at<Vec3b>(j, i)[0] <= 155)){ m_color[9] += 1; }
				//红色
				else { m_color[3] += 1; };
			}
			//白色
			else if ((0 <= img.at<Vec3b>(j, i)[2]) && (img.at<Vec3b>(j, i)[2] <= 46)){ m_color[0] += 1; }
			//灰色
			else if (((0 <= img.at<Vec3b>(j, i)[1]) && (img.at<Vec3b>(j, i)[1] <= 43)) &&
				((46 <= img.at<Vec3b>(j, i)[2]) && (img.at<Vec3b>(j, i)[2] <= 220)))
			{
				m_color[1] += 1;
			}
			else if (((0 <= img.at<Vec3b>(j, i)[1]) && (img.at<Vec3b>(j, i)[1] <= 30)) &&
				((221 <= img.at<Vec3b>(j, i)[2]) && (img.at<Vec3b>(j, i)[2] <= 255)))
			{
				m_color[2] += 1;
			};
		}
	}
	//	cout << "error 8" << endl;
	for (int i = 0; i < 2; i++)
	{
		int max = -1;
		max = GetMax();
		m_color[max] = -1;
		switch (max)
		{
		case 0:
			color[i] = "black";
			break;
		case 1:
			color[i] = "gray";
			break;
		case 2:
			color[i] = "white";
			break;
		case 3:
			color[i] = "red";
			break;
		case 4:
			color[i] = "orange";
			break;
		case 5:
			color[i] = "yellow";
			break;
		case 6:
			color[i] = "green";
			break;
		case 7:
			color[i] = "cyan";
			break;
		case 8:
			color[i] = "blue";
			break;
		case 9:
			color[i] = "purple";
			break;
		default:
			color[i] = "";
			break;
		}
	}
	//	cout << "error 9" << endl;
	//cout << "MaxColor:" << m_maxColor[0] << ", " << m_maxColor[1] << endl;
	//return m_maxColor;
}

int MyMachineVision::GetMax()
{
	bool ismax = false;
	int max = -1;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (i == j)
			{
				continue;
			}
			if (m_color[i] < m_color[j])
			{
				ismax = false;
				break;
			}
			ismax = true;
		}
		if (ismax)
		{
			return i;
		}
	}
	return -1;
}
