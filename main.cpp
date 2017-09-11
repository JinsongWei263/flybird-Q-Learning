#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <ctime>

#include "opencv/cv.hpp"
#include "opencv/highgui.h"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include "flybird.h"

using namespace std;
using namespace cv;
bool chickflag;

#define QLearning
const int qlResolution = 15;
struct action
{
	double chick;
	double noChick;	
};
struct qlState
{
	Point2i state;
	bool act;
};

void onMouse(int event,int x,int y,int flags,void *ustc)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		chickflag = false;
	}else if (event == CV_EVENT_LBUTTONUP)
		chickflag = true;

}


double Q[288][512][30][2];
bool Qv[288][512][30][2];
// map Q[qlState,double];

cv::Point2i getQstate(flybird *flybird)
{
	cv::Point2i state(0,0);
	std::vector<Point2i>::iterator iter = flybird->pipeList.end();
	for (std::vector<Point2i>::iterator i = flybird->pipeList.begin(); i != flybird->pipeList.end(); ++i)
	{
		if (i->x >= flybird->birdX - (birdW-birdXoffset))
		{
			iter = i;
			break;
		}
	}
	if (iter!=flybird->pipeList.end())
	{
		state.x = (iter->x - (flybird->birdX-(birdW-birdXoffset)) )/qlResolution;
		state.y = (iter->y - (flybird->birdY-(birdH-birdYoffset) ) + 400)/qlResolution;
	}
	return state;
}

int main()
{
	srand(time(0));
	namedWindow("game");
	setMouseCallback("game",onMouse);

	Mat atlas = imread("atlas.png",CV_LOAD_IMAGE_UNCHANGED);

	flybird flybird;
	flybird.reset();
	flybird.loadImg(atlas);
	cout << atlas.type() << endl;
	bool jumpflag = 0;
	int time = 0;
	int score,maxscore;
	maxscore = score = 0;
	cv::Point2i state;
	int statecnt;
	state = getQstate(&flybird);
	int yVel = flybird.yVel+14;
	double qlAlpha = 0.6;
	double qlGamma = 0.8;
	double qlEpsilon = 0;
	double qlExploreJumpRate = 0.1;
	double qlAliveReward = 1;
	double qlDeadReward = -100;
	int controlT = 4;
	while(1)
	{
#ifdef QLearning
		int act = -1;
		double e = rand()%10000/10000.0;
		if (e < qlEpsilon )
		{
			double r = rand()%10000/10000.0;
			if (r < qlExploreJumpRate)
			{
				act = 1;
			}else
				act = 0;
		}else if (Q[state.x][state.y][yVel][1] > Q[state.x][state.y][yVel][0])
		{
			act = 1;
		}else
		{
			act = 0;
		}
		if (act == 1)
			flybird.updateVel(true);
		else flybird.updateVel(false);
//		cout << state.x << ' ' << state.y << endl;
//		cout << act << "  state : " << " " << Q[state.x][state.y][0] << ' ' << Q[state.x][state.y][1] << endl;

#else
		if (chickflag==true)
		{
			jumpflag = true;
			chickflag = false;
		}else jumpflag = false;
		if (jumpflag==true)
		{
			flybird.updateVel(true);
			jumpflag =false;
		} else
			flybird.updateVel(false);
#endif

		flybird.updateBird();
		flybird.updatePipe();
		bool reward = flybird.updateReward();
		bool dead = flybird.updateDead(time);
		Mat display = flybird.display(time);

#ifdef	QLearning
		double getReward = 0;
		if (dead) getReward = qlDeadReward;
		else getReward = qlAliveReward;
		double 	qlSmax;
		int yVelCurr = flybird.yVel+14;
		Point2i stateCurr = getQstate(&flybird);

		if (Q[stateCurr.x][stateCurr.y][yVelCurr][0] > Q[stateCurr.x][stateCurr.y][yVelCurr][1])
		{
			qlSmax = Q[stateCurr.x][stateCurr.y][yVelCurr][0];
		}else qlSmax = Q[stateCurr.x][stateCurr.y][yVelCurr][1];
//		cout << stateCurr.x << " " << stateCurr.y << endl;
//		cout << "stateCurr : " << Q[stateCurr.x][stateCurr.y][0] << " " << Q[stateCurr.x][stateCurr.y][1] << endl;
		double qlValueCurr = Q[state.x][state.y][yVel][act];
		Q[state.x][state.y][yVel][act] = qlValueCurr + qlAlpha * ( getReward + qlGamma*qlSmax - qlValueCurr);
		if (!Qv[state.x][state.y][yVel][act])
		{
			statecnt ++;
			Qv[state.x][state.y][yVel][act] = 1;
		}
		state = stateCurr;
		yVel = yVelCurr;
#endif

		if (reward)
		{
			if (++score > maxscore)
				maxscore = score;
			cout << "reward" << endl;
			cout << "maxscore" << " :  " << maxscore << endl;
			cout << "score" << " : " << score << endl;
			cout << "stateCnt" << " : "  << statecnt << endl;
		}
		time = (time+1)%15;
		imshow("game",display);
		if (dead)
		{
			score = 0;
			flybird.reset();
			cout << "maxscore" << " :  " << maxscore << endl;
			cout << "score" << " : " << score << endl;
			state = getQstate(&flybird);
			yVel = flybird.yVel+14;
			cout << "stateCnt" << " : "  << statecnt << endl;
		}
		cv::waitKey(1);
	}
	return 0;
}
