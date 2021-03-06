// RGB.cpp: 定义控制台应用程序的入口点。
//

#include <windows.h>
#include <NuiApi.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void getColorImage(HANDLE & colorStreamHandle, Mat & colorImg);
void getDepthImage(HANDLE & depthStreamHandle, cv::Mat & depthImg);

int main()
{
	Mat colorImg;
	colorImg.create(480, 640, CV_8UC3);

	Mat depthImg;
	depthImg.create(240, 320, CV_8UC1);

	HANDLE colorEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE depthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE colorStreamHandle = NULL;
	HANDLE depthStreamHandle = NULL;

	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR| NUI_INITIALIZE_FLAG_USES_DEPTH);//NUI接口初始化
	if (FAILED(hr))//判断是否成功
	{
		cout << "NuiInitialize failed" << endl;
		return hr;
	}
	//创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据
	//打开彩色设备
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, colorEvent, &colorStreamHandle);
	if (FAILED(hr))
	{
		cout << "Could not open color image stream video color" << endl;
		NuiShutdown();
		return hr;
	}
	//打开深度设备
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_320x240, 0, 2, depthEvent, &depthStreamHandle);
	if (FAILED(hr))
	{
		cout << "Could not open color image stream video  depth" << endl;
		NuiShutdown();
		return hr;
	}
	namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
	namedWindow("depthImg", CV_WINDOW_AUTOSIZE);

	while (1)
	{
		if (WaitForSingleObject(colorEvent, 0) == 0)
		{
			getColorImage(colorStreamHandle, colorImg);
		}
		if (WaitForSingleObject(depthEvent, 0) == 0)
		{
			getDepthImage(depthStreamHandle, depthImg);
		}
		imshow("colorImg", colorImg);
		imshow("depthImg", depthImg);
		waitKey(30);
	}

    return 0;
}

void getColorImage(HANDLE & colorStreamHandle, Mat & colorImg)
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame);
	if (FAILED(hr))
	{
		cout << "Could not get color image" << endl;
		NuiShutdown();
		return;
	}
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0)
	{
		for (int i = 0; i < colorImg.rows; i++)
		{
			uchar *ptr = colorImg.ptr<uchar>(i);    //第i行的指针
			uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
			for (int j = 0; j < colorImg.cols; j++)
			{
				ptr[3 * j] = pBuffer[4 * j];
				ptr[3 * j + 1] = pBuffer[4 * j + 1];
				ptr[3 * j + 2] = pBuffer[4 * j + 2];
			}
		}
	}
	else
	{
		cout << "捕获彩色图像出错" << endl;
	}
	pTexture->UnlockRect(0);
	NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame);
}

void getDepthImage(HANDLE & depthStreamHandle, cv::Mat & depthImg)
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(depthStreamHandle, 0, &pImageFrame);
	if (FAILED(hr))
	{
		cout << "Could not get depth image" << endl;
		NuiShutdown();
		return;
	}
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0)
	{
		int max=0;
		for (int i = 0; i < depthImg.rows; i++)
		{
			uchar * ptr = depthImg.ptr<uchar>(i);
			uchar *pBufferRun = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
			USHORT * pBuffer = (USHORT*)pBufferRun;
			for (int j = 0; j < depthImg.cols; j++)
			{
				if (pBuffer[j] > max)max = pBuffer[j];
				ptr[j] = (uchar)(255*pBuffer[j] /0x0fff );
	
			}
		}
		printf("%d \n", max);
	}
	else
	{
		cout << "Buffer length of received texture is bogus\r\n" << endl;
	}
	pTexture->UnlockRect(0);
	NuiImageStreamReleaseFrame(depthStreamHandle, pImageFrame);
}