#include "flybird.h"
#include <iostream>

using namespace std;

flybird::flybird()
{
}
flybird::~flybird()
{
	pipeList.erase(pipeList.begin(),pipeList.end());
	bird[0].release();
	bird[1].release();
	bird[2].release();
	pipe_up.release();
	pipe_down.release();
	bg.release();
	land.release();
}

void flybird::reset ()
{
	xVel = XVel;
	yVel = 0;
	birdState = 0;
	birdX = birdXStart;
	birdY = birdYStart;
	state = 0;
	pipeList.erase(pipeList.begin(),pipeList.end());
	pipeList.push_back(cv::Point2i(320,pipeRandomBoundary+rand()%(landY-pipeGap-2*pipeRandomBoundary)));
}

void flybird::loadImg(cv::Mat atlas)
{
	bg.create(atlas(bgRect).size(),atlas.type());
	land.create(atlas(landRect).size(),atlas.type());
	bird[0].create(atlas(bird0Rect).size(),atlas.type());
	bird[1].create(atlas(bird1Rect).size(),atlas.type());
	bird[2].create(atlas(bird2Rect).size(),atlas.type());
	pipe_up.create(atlas(pipeUpRect).size(),atlas.type());
	pipe_down.create(atlas(pipeDownRect).size(),atlas.type());
	atlas(bgRect).copyTo(bg);
	atlas(landRect).copyTo(land);
	atlas(bird0Rect).copyTo(bird[0]);
	atlas(bird1Rect).copyTo(bird[1]);
	atlas(bird2Rect).copyTo(bird[2]);
	atlas(pipeUpRect).copyTo(pipe_up);
	atlas(pipeDownRect).copyTo(pipe_down);
}

void flybird::updateVel(bool upClick)
{
	if (upClick)
		yVel = jumpVel;
	else
		yVel = yVel + gravity;
	if (yVel>maxFallVel) yVel = maxFallVel;
}
void flybird::updateBird()
{
	birdY += yVel;
}
void flybird::updatePipe()
{
	for (std::vector<cv::Point2i>::iterator i = pipeList.begin(); i != pipeList.end(); ++i)
	{
		i->x -= xVel;
	}
	for (std::vector<cv::Point2i>::iterator i = pipeList.begin();i!=pipeList.end(); i++)
	{
		if (i->x<0) pipeList.erase(i);
	}
	std::vector<cv::Point2i>::iterator iter=pipeList.end()-1;
	if (!pipeList.empty())
	{
		if (width-iter->x >= pipeSpace-pipeW)
		{
			pipeList.push_back(cv::Point2i(width+pipeW,pipeRandomBoundary+rand()%(landY-pipeGap-2*pipeRandomBoundary)));
		}
	}
}
bool flybird::updateReward()
{
	int curBirdState = 0;
	for (std::vector<cv::Point2i>::iterator i = pipeList.begin(); i != pipeList.end(); ++i)
	{
		if (i->x-pipeW <= birdX-(birdW-birdXoffset))
		{
			if (i->x >= birdX-(birdW-birdXoffset))
			{
				curBirdState = 1;
			}
		}else {
			break;
		}
	}
	bool result=0;
	if (birdState == 1 && curBirdState == 0)
	{
		result = 1;
	}else result = 0;
	birdState = curBirdState;
	return result ;
}

bool flybird::updateDead(int time)
{
	// 
	if (birdY  > landY) return 1;
	if (birdY -(birdH+birdYoffset)< 0) return 1;
	int k = (time%15)/5;
	//
	for (std::vector<cv::Point2i>::iterator i = pipeList.begin(); i != pipeList.end(); ++i)
	{
		int bx,by;
		bx=birdW+birdXoffset/2-1;by=birdH+birdYoffset/2-1;
		for (int x=birdX;x>=birdX-(birdW+birdXoffset)+1;x--)
		{
			by=birdH+birdYoffset/2-1;
			for (int y=birdY;y>=birdY-(birdH+birdYoffset)+1;y--)
			{
				if ((int)bird[k].at<cv::Vec4b>(by,bx)[3]!=0)
				{
					if ( x >= i->x-pipeW+1 && x<=i->x && y>=0 && y<=i->y) return 1;
					if ( x >= i->x-pipeW+1 && x<=i->x && y>=i->y+pipeGap && y<=landY) return 1;
				}
				by--;
			}
			bx--;
		}
	}

	return 0;
}

int flybird::max(int a,int b)
{
	if (a>b) return a;
	else return b;
}
int flybird::min(int a,int b)
{
	if (a<b) return a;
	else return b;
}

cv::Mat flybird::display(int time)
{
	cv::Mat dispImg;
	dispImg.create(cv::Size(width,height),bg.type());
	// display background & land
	bg.copyTo(dispImg);
	land.copyTo(dispImg(cv::Rect(0,landY,width,landHeight)));
	// display pipes
	for (std::vector<cv::Point2i>::iterator i = pipeList.begin(); i != pipeList.end(); ++i)
	{
		cv::Rect bgRect;
		cv::Rect pipeRect;
		cv::Point2i a,b;
		bgRect.x = max(0,i->x-pipeW+1);
		bgRect.y = 0;
		bgRect.width = min(width - bgRect.x, i->x - bgRect.x+1);
		bgRect.height = i->y;

		pipeRect.x = max(0, -(i->x-pipeW+1));
		pipeRect.y = pipeH - bgRect.height;
		pipeRect.width = bgRect.width;
		pipeRect.height = bgRect.height;

		if (bgRect.x < width)
		{
//			pipe_down(pipeRect).copyTo(dispImg(bgRect));
			cvStack(dispImg(bgRect),pipe_down(pipeRect));
		}
//		bgRect.x = bgRect.x;
		bgRect.y = bgRect.y + bgRect.height + pipeGap;
		bgRect.height = landY - bgRect.y;
//		bgRect.width = bgRect.width;

//		pipeRect.x = pipeRect.x;
		pipeRect.y = 0;
		pipeRect.height = bgRect.height;
//		pipeRect.width = pipeRect.width;

		if (bgRect.x < width)
		{
//			pipe_up(pipeRect).copyTo(dispImg(bgRect));
			cvStack(dispImg(bgRect),pipe_up(pipeRect));
		}		
	}
	// display bird
	int k = (time%15)/5;

	if (birdY-(birdH+birdYoffset)>=0)
		cvStack(dispImg(cv::Rect(birdX-(birdW+birdXoffset)+1,birdY-(birdH+birdYoffset)+1,birdW+birdXoffset,birdH+birdYoffset)),bird[k](cv::Rect(-birdXoffset/2,-birdYoffset/2,birdW+birdXoffset,birdH+birdYoffset)));
//		bird[k](cv::Rect(-birdXoffset/2,-birdYoffset/2,birdW+birdXoffset,birdH+birdYoffset)).copyTo(dispImg(cv::Rect(birdX-(birdW+birdXoffset)+1,birdY-(birdH+birdYoffset)+1,birdW+birdXoffset,birdH+birdYoffset)));
	return dispImg;
}

void cvStack(cv::Mat A, const cv::Mat B)
{
	if (A.size()!=B.size())
	{
		cout << " A.size != B.size" << endl;
		return ;
	}
	for (int i=0;i<A.rows;i++)
		for (int j=0;j<A.cols;j++)
		{
			double alpha = B.at<cv::Vec4b>(i,j)[3]/255.0;
			A.at<cv::Vec4b>(i,j)[0] = A.at<cv::Vec4b>(i,j)[0]*(1-alpha)+B.at<cv::Vec4b>(i,j)[0]*alpha;
			A.at<cv::Vec4b>(i,j)[1] = A.at<cv::Vec4b>(i,j)[1]*(1-alpha)+B.at<cv::Vec4b>(i,j)[1]*alpha;
			A.at<cv::Vec4b>(i,j)[2] = A.at<cv::Vec4b>(i,j)[2]*(1-alpha)+B.at<cv::Vec4b>(i,j)[2]*alpha;
//			A.at<cv::Vec4b>(i,j)[3] = A.at<cv::Vec4b>(i,j)[3]*(1-alpha)+B.at<cv::Vec4b>(i,j)[3]*alpha;
		}
}
