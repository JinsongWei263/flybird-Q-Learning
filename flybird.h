#ifndef FLYBIRD
#define FLYBIRD	 
#include <vector>
#include <random>

#include "opencv/cv.hpp"
#include "opencv/highgui.h"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

const cv::Rect bgRect(0,0,288,512);
const cv::Rect landRect(584,0,288,112);
const cv::Rect bird0Rect(0,970,48,48);
const cv::Rect bird1Rect(56,970,48,48);
const cv::Rect bird2Rect(112,970,48,48);
const cv::Rect pipeDownRect(112,646,52,320);
const cv::Rect pipeUpRect(168,646,52,320);

const int width = 288;
const int height = 512;

const int XVel = 4;
const double gravity = 1.5;
const int jumpVel = -10;
const int maxFallVel = 15;

const int birdXoffset = -11;
const int birdYoffset = -18;
const int birdW = 48;
const int birdH = 48;
const int birdXStart = 69;
const int birdYStart = 236;

const int pipeW = 52;
const int pipeH = 320;
const int pipeSpace = 172;
const int pipeGap = 100;
const int pipeStartX = 320;
const int pipeRandomBoundary = 50;

const int landStartX = 0;
const int landWidth = 288;
const int landHeight = 112;
const int landY = 400;

const int swingT = 5;

class flybird
{
public:
	flybird();
	~flybird();
	int birdX, birdY;
	std::vector<cv::Point2i> pipeList;
	int state;

	int birdState;

	int xVel;
	double yVel;

	// 
	cv::Mat bird[3];
	cv::Mat pipe_up,pipe_down;
	cv::Mat bg,land;
//
	void reset();
	void updateState();
	void updateVel(bool );
	void updateBird();
	void updatePipe();
	bool updateReward();
	bool updateDead(int );
//
	void loadImg(cv::Mat atlas);
	cv::Mat display(int );
private:
	int max(int a,int b);
	int min(int a,int b);

};

void cvStack(cv::Mat A, const cv::Mat B);

#endif
