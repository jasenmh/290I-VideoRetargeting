#include <iostream>

#include <stdio.h>
#include "graph.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
/* Declarations */
Mat RemoveSeam(Mat image, int Seam[] );
int *FindSeam(Mat grayImage1, Mat grayImage2);
Mat ReduceVer(Mat &GrayImage1, Mat &GrayImage2, Mat image);
Mat ReduceHor(Mat &GrayImage1, Mat &GrayImage2, Mat image);
Mat RemoveSeamGray(Mat GrayImage, int Seam[]);

/* Definitions */
Mat RemoveSeam(Mat image, int Seam[])
{
    int nrows = image.rows;
    int ncols = image.cols;
    Mat ReducedImage(nrows,ncols-1,CV_8UC3);
    //ReducedGrayImage.copyTo(temp);
    //for(int k = 0;k<3;k++)
    //vector<Mat> channels = cv::split()
    /*for(int k=0; k<nrows; k++)
    {
        cout<< "Seam" << Seam[k] << endl;
    }*/
    for(int i=0; i<nrows; i++)
    {
        image.row(i).colRange(Range(0,Seam[i])).copyTo(ReducedImage.row(i).colRange(Range(0,Seam[i])));
        image.row(i).colRange(Range(Seam[i]+1, ncols)).copyTo(ReducedImage.row(i).colRange(Range(Seam[i],ncols-1)));
        /*for(int j=0; j<ncols-1; j++)
        {
            if(j<Seam[i])
            {
                ReducedImage.at<cv::Vec3b>(Point(j,i))= image.at<cv::Vec3b>(Point(j,i));
            }
            else
            {
                ReducedImage.at<cv::Vec3b>(Point(j,i)) = image.at<cv::Vec3b>(Point(j+1,i));
            }
        }*/
    }
    return ReducedImage;
}
Mat RemoveSeamGray(Mat GrayImage, int Seam[])
{
    int nrows = GrayImage.rows;
    int ncols = GrayImage.cols;
    Mat ReducedImage(nrows,ncols-1,CV_8UC1);
    //ReducedGrayImage.copyTo(temp);
    //for(int k = 0;k<3;k++)
    //vector<Mat> channels = cv::split()
    for(int i=0; i<nrows; i++)
    {
        GrayImage.row(i).colRange(Range(0,Seam[i])).copyTo(ReducedImage.row(i).colRange(Range(0,Seam[i])));
        GrayImage.row(i).colRange(Range(Seam[i]+1, ncols)).copyTo(ReducedImage.row(i).colRange(Range(Seam[i],ncols-1)));
        /*for(int j=0; j<ncols-1; j++)
        {
            if(j<Seam[i])
            {
                ReducedImage.at<cv::Vec3b>(Point(j,i))= image.at<cv::Vec3b>(Point(j,i));
            }
            else
            {
                ReducedImage.at<cv::Vec3b>(Point(j,i)) = image.at<cv::Vec3b>(Point(j+1,i));
            }
        }*/
    }
    return ReducedImage;
}

int *FindSeam(Mat grayImage1, Mat grayImage2)
{
    typedef Graph<int,int,int> GraphType;
    int rows = grayImage1.rows;
    int cols = grayImage1.cols;
    double inf = 100000;
    int *Seam = new int[rows];
    float alpha = 0.7;
    GraphType *g = new GraphType(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ ((rows-1)*cols + (cols-1)*rows + 2*(rows-1)*(cols-1)));


    /* LR = |I(i,j+1)-I(i,j-1)|
      *+LU = |I(i,j-1)-I(i+1,j)|
      *-LU = |I(i,j-1)-I(i-1,j)|
      *
      */
    int LR, LR1,LR2, posLU1,posLU2,posLU, negLU, negLU1, negLU2;
    for (int i = 1; i<=rows*cols; i++)
    {
        g -> add_node();
    }

    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
            if(j==0)
            {
                g -> add_tweights( i*cols,   /* capacities */  inf,0 );
            }
            else if(j==cols-1)
            {
                g -> add_tweights( ((i+1)*cols) -1,   /* capacities */ 0, inf );
            }

            if(j==0)
            {
                LR1 = grayImage1.at<unsigned char>(i,j+1);
                LR2 = grayImage2.at<unsigned char>(i,j+1);
                LR = alpha*LR1 + (1-alpha)*LR2;
                g -> add_edge( i*cols, i*cols+1,    /* capacities */ LR, inf );
            }
            else if(j!=cols-1)
            {
                LR1 = abs(grayImage1.at<unsigned char>(i,j+1) - grayImage1.at<unsigned char>(i,j-1));
                LR2 = abs(grayImage2.at<unsigned char>(i,j+1) - grayImage2.at<unsigned char>(i,j-1));
                LR = alpha*LR1 + (1-alpha)*LR2;
                g -> add_edge( i*cols + j, i*cols + j +1, LR, inf );
            }

            if(i!=rows-1)
            {
                if(j==0)
                {
                    posLU1 = grayImage1.at<unsigned char>(i,j);
                    posLU2 = grayImage2.at<unsigned char>(i,j);
                    posLU = alpha*posLU1 + (1-alpha)*posLU2;
                    negLU1 = grayImage1.at<unsigned char>(i+1,j);
                    negLU2 = grayImage2.at<unsigned char>(i+1,j);
                    negLU = alpha*negLU1 + (1-alpha)*negLU2;
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
                else
                {
                    posLU1 = abs(grayImage1.at<unsigned char>(i,j)-grayImage1.at<unsigned char>(i+1,j-1));
                    posLU2= abs(grayImage2.at<unsigned char>(i,j)-grayImage2.at<unsigned char>(i+1,j-1));
                    posLU = alpha*posLU1 + (1-alpha)*posLU2;
                    negLU1 = abs(grayImage1.at<unsigned char>(i+1,j)-grayImage1.at<unsigned char>(i,j-1));
                    negLU2 = abs(grayImage2.at<unsigned char>(i+1,j)-grayImage2.at<unsigned char>(i,j-1));
                    negLU = alpha*negLU1 + (1-alpha)*negLU2;
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
            }
            if(i!=0 && j!=0)
            {
                g -> add_edge( i*cols + j, (i-1)*cols + j-1, inf, 0 );
            }
            if(i!=rows-1 && j!=0)
            {
                g -> add_edge( i*cols + j, (i+1)*cols + j-1, inf, 0 );
            }
        }
    }


    int flow = g -> maxflow();

    //cout << "Flow = %d\n" << flow << endl;
    //cout<< "Minimum cut:\n"<<endl;
    /*for(int i=0;i<16; i++)
     {
     if (g->what_segment(i) == GraphType::SOURCE)
         cout<<"node" << i <<" is in the SOURCE set\n"<<endl;
     else
         cout<<"node" << i <<"is in the SINK set\n"<<endl;
     }*/
    for(int i=0; i<rows; i++)
    {
        for(int j=0;j<cols; j++)
        {
            if(g->what_segment(i*cols+j) == GraphType::SINK)
            {
                Seam[i] = j-1;
                break;
            }
            if(j==cols-1 && g->what_segment(i*cols+j) == GraphType::SOURCE)
            {
                Seam[i] = cols-1;
            }
        }
    }
    /*
    for(int k=0; k<rows; k++)
    {
        cout<< "Seam" << Seam[k] << endl;
    }*/
    delete g;
    return Seam;
}
Mat ReduceVer(Mat &GrayImage1, Mat &GrayImage2, Mat image)
{
    int rows = GrayImage1.rows;
    int *Seam = new int[rows];
    Seam = FindSeam(GrayImage1, GrayImage2);
    Mat ReturnImage = RemoveSeam(image, Seam);
    GrayImage1 = RemoveSeamGray(GrayImage1, Seam);
    GrayImage2 = RemoveSeamGray(GrayImage2, Seam);
    return ReturnImage;
}
Mat ReduceHor(Mat &GrayImage1, Mat &GrayImage2, Mat image)
{
    int rows = GrayImage1.rows;
    int *Seam = new int[rows];
    Seam = FindSeam(GrayImage1.t(), GrayImage2.t());
    /*for(int k=0; k<rows; k++)
    {
        cout<< "Seam Hor" << Seam[k] << endl;
    }*/
    //cout << "New Seam" << endl;
    Mat ReturnImage = RemoveSeam(image, Seam);
    Mat GrayImage1temp = RemoveSeamGray(GrayImage1.t(), Seam);
    Mat GrayImage2temp = RemoveSeamGray(GrayImage2.t(), Seam);
    GrayImage1 = GrayImage1temp.t();
    GrayImage2 = GrayImage2temp.t();
    return ReturnImage.t();
}
Mat ReduceFrame(Mat frame1, Mat frame2, int ver, int hor)
{
    //Mat image = frame1;
    Mat grayImage1, grayImage2;// = (Mat_<int>(4, 4) << 4,5,200,200,4,5,200,200,4,5,5,198,4,5,5,5);
    cvtColor(frame1,grayImage1, CV_RGB2GRAY);
    cvtColor(frame2,grayImage2, CV_RGB2GRAY);
    int minDim = 0, diffHorVer = 0;
    Mat ReducedGrayImage1, ReducedGrayImage2, ReducedImage;
    grayImage1.copyTo(ReducedGrayImage1);
    grayImage2.copyTo(ReducedGrayImage2);
    frame1.copyTo(ReducedImage);

    if(hor > ver)
    {
        diffHorVer = hor - ver;
        minDim = ver;
    }
    else
    {
        diffHorVer = ver - hor;
        minDim = hor;
    }

    // do alternating horizontal and vertical seam carves up the the min of
    // the two
    for(int i = 0; i < minDim; ++i)
    {
        ReducedImage = ReduceVer(ReducedGrayImage1, ReducedGrayImage2, ReducedImage);
        //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        ReducedImage = ReduceHor(ReducedGrayImage1, ReducedGrayImage2, ReducedImage.t());
        //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
    }

    // finish carving the larger dimension
    if(hor > ver)
    {
        for(int i = 0; i < diffHorVer; ++i)
        {
            ReducedImage = ReduceHor(ReducedGrayImage1,ReducedGrayImage2, ReducedImage.t());
            //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        }
    }
    else
    {
        for(int i = 0; i < diffHorVer; ++i)
        {
            ReducedImage = ReduceVer(ReducedGrayImage1, ReducedGrayImage2, ReducedImage);
            //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        }
    }
    return ReducedImage;
}

void printUsage()
{
    cout << "Usage: GraphCut -f <filename> -v <vertical cuts> -h <horizontal cuts>" << endl;
}

int main(int argc, char* argv[])
{
    VideoCapture cap;
    VideoWriter output;
    string inFile = "88_7_orig.mov";
    Mat frame1, frame2, NewFrame;
    int frameCount = 1;
    int ver = 2;
    int hor = 2;

    if(argc > 1)
    {
        for(int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i], "-f") == 0)
            {
                inFile = argv[++i];
            }
            else if(strcmp(argv[i], "-v") == 0)
            {
                ver = atoi(argv[++i]);
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                hor = atoi(argv[++i]);
            }
            else
            {
                cerr << "Unknown flag: " << argv[i] << endl;
                printUsage();
            }
        }
    }
    else
    {
        printUsage();
        return -1;
    }

    cap.open(inFile);
    int maxFrame = cap.get(CV_CAP_PROP_FRAME_COUNT);

    if(!cap.isOpened())
    {
        cout << "Unable to open input file " << inFile << endl;
        return -1;
    }
    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH) -ver , (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT)-hor);
    //char key = 0;
    int first = 1;
    int last = 0;
    NewFrame = Mat::zeros(S, CV_32F);
    string::size_type pAt = inFile.find_last_of('.');   // Find extension point
    const string outFile = inFile.substr(0, pAt) + "-temp2.mov";
    output.open(outFile, ex, cap.get(CV_CAP_PROP_FPS), S, true);

    //int fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
    while (/*key != 'q' &&*/ !last)
    {
        if(first ==1 )
        {
            cap >> frame1;
            if (frame1.empty())
            {
                printf("!!! cvQueryFrame failed: no frame\n");
                break;
            }
            first = 0;
            continue;
        }
        else
        {
            cap >> frame2;
            if(frame2.empty())
            {
                /* Graph cut on frame 1 */
                //cout<< "Last frame" << endl;
                frame2 = frame1;
                last = 1;
            }
            NewFrame = ReduceFrame(frame1, frame2, ver, hor);
            frame1 = frame2;
        }
        //imshow("Frames", NewFrame);
        // quit when user press 'q'
        output<<NewFrame;
        //key = cvWaitKey(1000 / 25);
        cout << "Frame " << frameCount++ << "/" << maxFrame << endl;
    }
    return 0;
}


