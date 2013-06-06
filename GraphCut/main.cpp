#include <iostream>

#include <stdio.h>
#include "graph.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
Mat RemoveSeam(Mat &GrayImage,int Seam[] );
int *FindSeam(Mat &GrayImage);
Mat RemoveSeam(Mat &GrayImage, int Seam[])
{
    int nrows = GrayImage.rows;
    int ncols = GrayImage.cols;
    Mat ReducedGrayImage(nrows,ncols-1,CV_8UC1);
    //ReducedGrayImage.copyTo(temp);
    for(int i=0; i<nrows; i++)
    {
        for(int j=0; j<ncols-1; j++)
        {
            if(j<Seam[i])
            {
                ReducedGrayImage.at<unsigned char>(i,j) = GrayImage.at<unsigned char>(i,j);
            }
            else
            {
                ReducedGrayImage.at<unsigned char>(i,j) = GrayImage.at<unsigned char>(i,j+1);
            }
        }
    }
    return ReducedGrayImage;
}
int *FindSeam(Mat &grayImage)
{
    typedef Graph<int,int,int> GraphType;
    int rows = grayImage.rows;
    int cols = grayImage.cols;
    double inf = 100000;
    int *Seam = new int[rows];
    GraphType *g = new GraphType(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ ((rows-1)*cols + (cols-1)*rows + 2*(rows-1)*(cols-1)));


    /* LR = |I(i,j+1)-I(i,j-1)|
      *+LU = |I(i,j-1)-I(i+1,j)|
      *-LU = |I(i,j-1)-I(i-1,j)|
      *
      */
    int LR, posLU, negLU;
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
                g -> add_edge( i*cols, i*cols+1,    /* capacities */ grayImage.at<unsigned char>(i,j+1), inf );
            }
            else if(j!=cols-1)
            {
                LR = abs(grayImage.at<unsigned char>(i,j+1) - grayImage.at<unsigned char>(i,j-1));
                g -> add_edge( i*cols + j, i*cols + j +1, LR, inf );
            }

            if(i!=rows-1)
            {
                if(j==0)
                {
                    posLU = grayImage.at<unsigned char>(i,j);
                    negLU = grayImage.at<unsigned char>(i+1,j);
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
                else
                {
                    posLU = abs(grayImage.at<unsigned char>(i,j)-grayImage.at<unsigned char>(i+1,j-1));
                    negLU = abs(grayImage.at<unsigned char>(i+1,j)-grayImage.at<unsigned char>(i,j-1));
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

    cout << "Flow = %d\n" << flow << endl;
    cout<< "Minimum cut:\n"<<endl;
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
    delete g;
    return Seam;
}

int main()
{
    Mat image = imread("waterfall.png",1);
    //imshow("original", image);
    Mat grayImage;// = (Mat_<int>(4, 4) << 4,5,200,200,4,5,200,200,4,5,5,198,4,5,5,5);
    cvtColor(image,grayImage, CV_RGB2GRAY);
    int ver = 10;
    imshow("Gray Image", grayImage);
    //cvWaitKey(0);
    Mat ReducedGrayImage;
    grayImage.copyTo(ReducedGrayImage);
    for(int k =0; k<ver; k++)
    {
        int rows = ReducedGrayImage.rows;
        int *Seam = new int[rows];
        Seam = FindSeam(ReducedGrayImage);

        /*for(int i=0; i<rows; i++)
    {
        cout<<"Seam Cols = " << Seam[i]<<endl;
    }*/
        ReducedGrayImage = RemoveSeam(ReducedGrayImage, Seam);
    }
    imshow("Reduced Gray Image", ReducedGrayImage);
    cvWaitKey(0);

    return 0;
}


