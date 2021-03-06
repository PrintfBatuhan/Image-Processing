//Batuhan DEDEO�LU
#include "Img_Processing_Lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

Img_Processing_Lib::Img_Processing_Lib( //definition of required arguments
                                        const char *_inImgName,
                                        const char *_outImgName,
                                        int * _height,  // to store image height
                                        int * _width,   //to store image width
                                        int * _bitDepth, //to store image bitdepth
                                        unsigned char * _header, //to store image header
                                        unsigned char * _colorTable,
                                        unsigned char * _inBuffer,
                                        unsigned char * _outBuffer
                                       )
{
    inImgName  = _inImgName;
    outImgName = _outImgName;
    height     = _height;
    width      = _width;
    bitDepth   = _bitDepth;
    header     = _header;
    colorTable = _colorTable;
    inBuffer   = _inBuffer;
    outBuffer  = _outBuffer;
}
/*********************************Image Reader Function**********************************/
void Img_Processing_Lib::readImage()
{
    int i;
    FILE *Input;
    Input = fopen(inImgName,"rb");

    if(Input ==(FILE *)0) //Error control
    {
        cout<<"Unable to open file. Maybe file does not exist"<<endl;
        exit(0);
    }

    for(i=0;i<BMP_HEADER_SIZE;i++)
    {
        header[i] =  getc(Input);
    }
    *width = *(int *)&header[18];           //read the width from the image header
    *height = *(int *)&header[22];          //read the height from the image header
    *bitDepth = *(int *)&header[28];        //read the bitdepth from the image header

    //If bith depth is less than or equal to eight color table exist.
    if(*bitDepth <=8)  //reading color table.
    {

        fread(colorTable,sizeof(unsigned char),BMP_COLOR_TABLE_SIZE,Input);
        //fread(what you want to read, size of element you want to read,total number of elements, source)
    }

    fread(inBuffer,sizeof(unsigned char),_512by512_IMG_SIZE,Input); //read pixel data

    fclose(Input);
}

/*********************************Image Writer Function**********************************/
void Img_Processing_Lib::writeImage(){

    FILE  *Output = fopen(outImgName,"wb");
    fwrite(header,sizeof(unsigned char),BMP_HEADER_SIZE,Output);

    if(*bitDepth <=8)
    {
        fwrite(colorTable,sizeof(unsigned char),BMP_COLOR_TABLE_SIZE,Output);
    }

   fwrite(outBuffer, sizeof(unsigned char),_512by512_IMG_SIZE,Output);
   fclose(Output);
   //Create a new image, first image read func. reads the input image, stores it in the input buffer
   //and the the process between read and write takes what's in the input buffer manipulate its stores
   //in an output buffer and then the image right takes what's in the output buffer and then create a new
   //from that show over here.
}

/*********************************Copy Image Data Function**********************************/
void Img_Processing_Lib ::copyImgData(unsigned char *_srcBuffer, unsigned char *_destBuffer, int bufferSize)
{                                  //(image data we want to copy (input img), empty array(copy it into), size of the image)

    for(int i =0;i<bufferSize;i++)
    {
        _destBuffer[i] = _srcBuffer[i];
    }
}

/*********************************Image Brightness Up Function**********************************/
//We are adding brightness factor with pixels. Pixel value increases the brighter the image. The brightest pixel is 255.
//When we add a number to the pixel it moves the pixel towards the white color.
//It doesn't go over what the maximum data size that our image can contain.
//Our image is an 8-Bit image therefore max value range is 255. If we go over that value, we must normalized our image or truncate the pixel value to 255.
void Img_Processing_Lib::brightnessUP(unsigned char *_inputImageData,unsigned char *_outImgData, int imgSize, int brightness)
{                                   //(input is image data, empty array yo store the resort which is brighter=output, image size, brightness factor)
    for(int i=0;i<imgSize;i++)
    {
        int temp=_inputImageData[i]+brightness;            //temp is the sum of the pixel data value and the brightnessfactor
        _outImgData[i]=(temp>MAX_COLOR)? MAX_COLOR :temp;  //
                        //condition ? result1: result2     if condition is true,the entire expression evaluates to result1, and otherwise to result2.
    }
}

/*********************************Image Brightness Down Function**********************************/
void Img_Processing_Lib::brightnessDOWN(unsigned char *_inputImageData,unsigned char *_outImgData, int imgSize, int darkness)
{                                   //(input is image data, empty array yo store the resort which is darker=output, image size, darkness factor)
    for(int i=0;i<imgSize;i++)
    {
        int temp=_inputImageData[i]-darkness;            //temp is the substract of the pixel data value and the darknessfactor
        _outImgData[i]=(temp<MIN_COLOR)? MIN_COLOR :temp;  //
                        //condition ? result1: result2     if condition is true,the entire expression evaluates to result1, and otherwise to result2.
    }
}

/***************************************Histogram Function****************************************/
void Img_Processing_Lib::Histogram(unsigned char * _imgData, int imgRows, int imgCols, float hist[], const char *histFile)
{                                             //(imgInBuffer, imgHeight, imgWidth,imgHist)
    FILE *fptr;
    fptr =fopen(histFile,"w"); //open output as txt file. Write Pixels values on text file.

    int x,y,i,j;                 //some local variables
    long int ihist[255],sum;   //temporary histogram

    for(i =0;i<=255;i++)      // initializing all intensity values to zero
    {
        ihist[i] =0;
    }
    sum =0;

    for(y=0;y<imgRows;y++)
    {
        for(x=0;x<imgCols;x++)
        {
            j = *(_imgData+x+y*imgCols); // increment each value that we encounter in the array (pixels values)
            ihist[j] = ihist[j] +1;      //each intensity goes to the appropriate histogram pane
            sum = sum+1;
        }

    }

    for(i=0;i<255;i++)
        hist[i] = (float)ihist[i]/(float)sum; //normalize

    for(int i=0;i<255;i++) //Scale the histogram
    {
        fprintf(fptr,"\n%f",hist[i]);
    }
    fclose(fptr);
}


/***************************************Equalize Function****************************************/
void Img_Processing_Lib::EqualizeHistogram(unsigned char *_inputImgData, unsigned char *_outputImgData, int imgRows, int imgCols)
{
    int x,y,i,j;
    int hist_eq[256];
    float hist[256];
    float sum;
    const char initHist[]="init_hist.txt";
    const char finalHist[]="final_hist.txt";

    Histogram(&_inputImgData[0],imgRows,imgCols,&hist[0],initHist);

    for(i=0;i<=255;i++)
        {
        sum =0.0;
        for(j=0;j<=i;j++){
            sum = sum+hist[j];
        }
        hist_eq[i] =  (int)(255*sum+0.5);

    }
    for(y =0;y<imgRows;y++)
    {
        for(x=0;x<imgCols;x++)
        {
            *(_outputImgData+x+y*imgCols) = hist_eq[*(_inputImgData+x+y*imgCols)];
        }
    }
    Histogram(&_outputImgData[0], imgRows,imgCols,&hist[0],finalHist);
}

/***************************************Negative Function****************************************/
void Img_Processing_Lib::ImageNegative(unsigned char *_InimgData, unsigned char*_outImgData, int imgWidth, int imgHeight)
{
    for(int i=0;i<imgHeight;i++)
    {
        for(int j=0;j<imgWidth;j++)
        {
         _outImgData[i*imgWidth+j]=255-_InimgData[i*imgWidth+j];
         //computes the negative of the image
        }
    }
}

/***************************************2D Convolution Function****************************************/
void Img_Processing_Lib::Convolve2D(int imgRows,int imgCols, struct Mask *myMask, unsigned char *inputBuffer, unsigned char *OutputBuffer)
{
long i,j,m,n,idx,jdx;
int ms,im,val;
unsigned char *tmp;

    for(i=0;i<imgRows;i++)
        for(j=0;j<imgCols;j++)
            {
                val=0;
                for(m=0;m<myMask->Rows;++m)
                for(n=0;n<myMask->Cols;++n)
                    {
                        ms=(signed char)*(myMask->Data+ m*myMask->Rows+n);
                        idx=i-m;
                        jdx=j-n;
                        if(idx>=0&&jdx>=0)
                            im=*(inputBuffer+idx*imgRows+jdx);
                        val+=ms*im;
                    }
                    if(val>255) val=255;
                    if(val<0)val=0;
                    tmp=outBuffer+i*imgRows+j;
                    *tmp=(unsigned char)val;
            }
}

/***************************************Line Detect with Line Detector Mask Function****************************************/
void Img_Processing_Lib::DetectLine(unsigned char *_inImgData, unsigned char *_outImgData,int imgCols, int imgRows, const int MASK[][3])
{
    int x,y,i,j,sum;
    for(y=1;y<=imgRows-1;y++)
    {
        for(x=1;x<=imgCols;x++)
        {
            sum=0;
            for(i=-1;i<=1;i++)
            {
                for(j=-1;j<=1;j++)
                {
                    sum = sum + *(_inImgData+x+i+(long)(y+j)*imgCols)*MASK[i+1][j+1];
                }
            }
            if(sum>255) sum=255;
            if(sum<0)sum=0;
            *(_outImgData+x+(long)y*imgCols)=sum;
        }
    }
}

/***************************************Line Detect with Another Masks Function****************************************/
void Img_Processing_Lib::setMask(int MskRows, int MskCols, const int MskData[]){

    signed char *tmp;
    int requiredSize;

    myMask.Rows = MskRows;
    myMask.Cols = MskCols;
    requiredSize = myMask.Rows * myMask.Cols;
    myMask.Data= (unsigned char *)malloc(requiredSize);

    tmp=(signed char*)myMask.Data;

    for(int i=0;i<requiredSize;i++)
    {
        *tmp=MskData[i];
        ++tmp;
    }
}

void Img_Processing_Lib::GaussNoise(unsigned char*_inImgData, int imgCols,int imgRows, float var,float mean)
{
    int x,y;
    float theta,noise;
    for(y=0;y<imgRows;y++)
        for(x=0;x<imgCols;x++)
    {
        noise = sqrt(-2*var*log(1.0-(float)rand()/32767.1));
        theta = (float)112*1.9175345e-4 - 3.14159265;
        noise = noise * cos(theta);
        noise=noise+mean;
        if(noise>255) noise=255;
        if(noise<0) noise =0;
        *(_inImgData+x+(long)y*imgCols)=(unsigned char)(noise+0.5);
    }
}

void Img_Processing_Lib::SaltandPepper(unsigned char*_intImgData, int imgCols,int imgRows, float prob)
{
    int x,y,data1,data2,data;
    data=(int)(prob*32768/2);
    data1=data+16384;
    data2=16384-data;

    for(y=0;y<imgRows;y++)
        for(x=0;x<imgCols;x++)
    {
        data=rand();
        if(data>=16384 && data<data1)
            *(_intImgData+x+(long)y*imgCols)=0;
        if(data>=data2&&data<16384)
            *(_intImgData+x+(long)y*imgCols)=255;
    }

}
Img_Processing_Lib::~Img_Processing_Lib()
{
    //dtor
}
