#ifndef MEASURE_H
#define MEASURE_H

#include <QMainWindow>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <QFileInfo>
#include <QDir>
//#include "gnuplot-iostream.h"

#include <iostream>
#include <QCoreApplication>

// some important Tag Idents (TTagHead.Ident) that we will need to read the most common content of a PTU file
// check the output of this program and consult the tag dictionary if you need more
#define TTTRTagTTTRRecType "TTResultFormat_TTTRRecType"
#define TTTRTagNumRecords  "TTResult_NumberOfRecords"  // Number of TTTR Records in the File;
#define TTTRTagRes         "MeasDesc_Resolution"       // Resolution for the Dtime (T3 Only)
#define TTTRTagGlobRes     "MeasDesc_GlobalResolution" // Global Resolution of TimeTag(T2) /NSync (T3)
#define FileTagEnd         "Header_End"                // Always appended as last tag (BLOCKEND)

// TagTypes  (TTagHead.Typ)
#define tyEmpty8      0xFFFF0008
#define tyBool8       0x00000008
#define tyInt8        0x10000008
#define tyBitSet64    0x11000008
#define tyColor8      0x12000008
#define tyFloat8      0x20000008
#define tyTDateTime   0x21000008
#define tyFloat8Array 0x2001FFFF
#define tyAnsiString  0x4001FFFF
#define tyWideString  0x4002FFFF
#define tyBinaryBlob  0xFFFFFFFF

// RecordTypes
#define rtPicoHarpT3     0x00010303    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $03 (T3), HW: $03 (PicoHarp)
#define rtPicoHarpT2     0x00010203    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
#define rtHydraHarpT3    0x00010304    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $03 (T3), HW: $04 (HydraHarp)
#define rtHydraHarpT2    0x00010204    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $04 (HydraHarp)
#define rtHydraHarp2T3   0x01010304    // (SubID = $01 ,RecFmt: $01) (V2), T-Mode: $03 (T3), HW: $04 (HydraHarp)
#define rtHydraHarp2T2   0x01010204    // (SubID = $01 ,RecFmt: $01) (V2), T-Mode: $02 (T2), HW: $04 (HydraHarp)
#define rtTimeHarp260NT3 0x00010305    // (SubID = $00 ,RecFmt: $01) (V2), T-Mode: $03 (T3), HW: $05 (TimeHarp260N)
#define rtTimeHarp260NT2 0x00010205    // (SubID = $00 ,RecFmt: $01) (V2), T-Mode: $02 (T2), HW: $05 (TimeHarp260N)
#define rtTimeHarp260PT3 0x00010306    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T3), HW: $06 (TimeHarp260P)
#define rtTimeHarp260PT2 0x00010206    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $06 (TimeHarp260P)

#pragma pack(8) //structure alignment to 8 byte boundaries
struct fotone{
  long long int RecNum;
  int Channel;
  long long TimeTag;
  int Dtime;
  long double tt; //this is the true time
  int intensity;
};

class Measure
{
public:
    Measure(std::string FileName);
    Measure(std::string FileName, int MC, int flag_normalization=false, int sogliaGlob=0, int binNum=50, int g2width=10, double altAt=0, double intTime=0.01, int sON=0);
    ~Measure();
    std::string FileName="";
//    boost::filesystem::path File_out="";
    long int Nch1=0;
    long int Nch2=0;
    double GlobRes = 0.0;
    double Resolution = 0.0;

    long long int Nl; //total counts in the 2 channels
    //Definitions by Stefano Pierini

private:
    FILE *fpin=nullptr;
    FILE *fpout=nullptr;
    FILE *fint, *flife, *f_g2_far, *f_norm;
    bool IsT2;
    long long RecNum=0;
    long long oflcorrection=0;
    long long truensync=0, truetime=0;
    int m, c;
    struct TgHd{
      char Ident[32];     // Identifier of the tag
      int Idx;            // Index for multiple tags or -1
      unsigned int Typ;  // Type of tag ty..... see const section
      long long TagValue; // Value of tag.
    } TagHead;
    int readHeader();
    int close();
    time_t TDateTime_TimeT(double Convertee);
    void MeasureCC(std::string FileName);
    void ProcessPHT2(unsigned int TTTRRecord);
    void ProcessHHT2(unsigned int TTTRRecord, int HHVersion);
    void ProcessHHT3(unsigned int TTTRRecord, int HHVersion);
    void ProcessPHT3(unsigned int TTTRRecord);
    void GotPhoton(long long TimeTag, int Channel, int DTime);
    void Intensity(fotone f1);
    void CreateG2(fotone f);
    void CreateFarG2(fotone f);
    std::vector <std::pair<double, long int>> get_g2_max_vector(long long int *g2array_far);
    void LifeTime(fotone f1, int soglia);
    void print_histogram();
    QFileInfo Append_to_name(QFileInfo P, QString string);


    QFileInfo File_out;
    QFileInfo File_int;
    QFileInfo File_life;
    QFileInfo File_g2_far;
    QFileInfo File_g2_norm;


    unsigned int dlen = 0;
    unsigned int cnt_0=0, cnt_1=0;
    int intC=0;
    long double startTime=0; //used to evaluate the intensity
    std::vector<fotone> vf;//used in the function Intensity
    std::vector<fotone> vf_both;//used in the function Intensity contains both channels
    fotone *fc1, *fc2; //fotons recently seen on ch1 and ch2
    fotone *fc1_far, *fc2_far;
    int ic1=0, ic2=0; //index to know which pulse overwrite in fc1 and fc2
    int ic1_far=0, ic2_far=0;
    long long int *g2array, *g2array_far; //hisotgram of them
    int g2width_far=1000000;
    int bin, bin_far;
    long int Non=0,Noff=0;
    double binw;
    std::vector<int> lifeTime_hist;
    double probT; // probability that a given pulse appen at a given time
    float T=0.5; //Transmission ratio of the BS
//    Gnuplot gp; //gnuplot istance
    //**********************
    //under this line, attributes to be initialized when the object is created
    //**********************

    int MC=1; //channel that we consider for the integration
    double intTime; //integration time to evaluate the intensity
    int flag_normalization=false; //true if have to be normalized, false in other cases
    int sogliaGlob=0;
    int g2width=10;
    int binNum=50;
    double altAt=0;
    int sON=0; //soglia per considerare lo stato ON
//    int sOFF; //soglia per considerare lo stato OFF



};

#endif // MEASURE_H
