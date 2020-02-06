#include "measure.h"



Measure::Measure(std::string FileName){
    this->MeasureCC(FileName);

}
/**
 * @brief Measure::Measure
 * @param FileName
 * @param MC Number of the channel used for the measurments
 * @param flag_normalization true if the g2 needs to be normalised
 * @param sogliaGlob if not zero, threshold intensity that the signal needs to reach to be considered
 * @param binNum number of histogram bins for each pulse
 * @param g2width number of g2 peaks to analise
 * @param altAt
 * @param intTime integration time for blinking analisys in s
 * @param sON
 */
Measure::Measure(std::string FileName, int MC, int flag_normalization, int sogliaGlob, int binNum, int g2width, double altAt, double intTime, int sON)
{
    if( (MC==1 ||MC==2)&& sogliaGlob>=0 && altAt >=0 && sON>=0)
    {
        this->MC=MC;
        this->flag_normalization=flag_normalization;
        this->sogliaGlob=sogliaGlob;
        this->binNum=binNum;
        this->altAt=altAt;
        this->sON=sON;
        this->g2width=g2width;
        this->intTime=intTime;
    }
    this->MeasureCC(FileName);

}

/**
 * @brief Measure::MeasureCC
 * @param FileName
 * Common function called by both contructors
 */
void Measure::MeasureCC(std::string FileName)
{
    this->FileName=FileName;
    fpin=fopen(FileName.c_str(),"rb");
    QFileInfo File_Name= *new QFileInfo(QString::fromStdString(FileName));
    QFileInfo File_out=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_out.dat";
    QFileInfo File_int=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_int.dat";
    QFileInfo File_life=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_life.dat";
    QFileInfo File_g2_far=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_g2_far.dat";
    QFileInfo File_g2_norm=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_g2_norm.dat";
    //    boost::filesystem::path File_name=boost::filesystem::path(FileName);
    //    QFileInfo File_out= Append_to_name(File_Name,"out");
    //    File_out= QFileInfo(File_out.canonicalFilePath()+QDir::separator()+File_out.baseName()+".dat");
    ////    boost::filesystem::path File_out=Append_to_name(File_name,"out");
    //    QFileInfo File_int=Append_to_name(File_Name,"int");
    //    File_int=File_int.canonicalFilePath()+QDir::separator()+File_int.baseName()+".dat";
    ////    QFileInfo File_life=Append_to_name(File_name,"life");
    //    File_out.replace_extension(".dat");
    //    File_int.replace_extension(".dat");
    //    File_life.replace_extension(".dat");
    //    File_g2_far.replace_extension(".dat");
    //    File_g2_norm.replace_extension(".dat");
    this->File_out=File_out;
    this->File_int=File_int;
    this->File_life=File_life;
    this->File_g2_far=File_g2_far;
    this->File_g2_norm=File_g2_norm;
    QString appoggio=File_out.absoluteFilePath();
    fpout=fopen(File_out.absoluteFilePath().toStdString().c_str(),"w");
    fint=fopen(File_int.absoluteFilePath().toStdString().c_str(),"w");
    flife=fopen(File_life.absoluteFilePath().toStdString().c_str(),"w");
    if (flag_normalization) f_norm=fopen(File_g2_norm.absoluteFilePath().toStdString().c_str(),"w");
    if(flag_normalization) f_g2_far=fopen(File_g2_far.absoluteFilePath().toStdString().c_str(),"w");
    //    std::cout<<"test";

    fc1= (fotone*) calloc(g2width,sizeof(fotone)); //fotons recently seen on ch1
    fc2= (fotone*) calloc(g2width,sizeof(fotone)); //fotons recently seen on ch2

    fc1_far= (fotone*) calloc(g2width_far,sizeof(fotone)); //fotons recently seen on ch1
    fc2_far= (fotone*) calloc(g2width_far,sizeof(fotone)); //fotons recently seen on ch2

    bin=binNum*(2*g2width+2);
    bin_far=binNum*(2*g2width_far+2);

    g2array=(long long int*)calloc(bin,sizeof(long long int));
    g2array_far=(long long int*)calloc(bin_far,sizeof(long long int));


    //command that starts the analysis
    this->readHeader();
    return;
}

Measure::~Measure(){

    print_histogram();
    fclose(fpin);
    fclose(fpout);
    fclose(flife);
    fclose(fint);
    if (flag_normalization) fclose(f_g2_far);
    if (flag_normalization) fclose(f_norm);
    // free our
    free(fc1);
    free(fc2);
    free(fc1_far);
    free(fc2_far);
    free(g2array);
    free(g2array_far);

    //    std::this_thread::sleep_for(std::chrono::seconds(1));
    //    sleep(1);
    printf("\nproportions:\n"
           "    %f%% ch1\n"
           "    %f%% ch2\n",
           Nch1/((double)(Nch1+Nch2)),
           Nch2/((double)(Nch1+Nch2))
           );
    //    printf("\n press enter to exit...\n");
    //getchar();
    //exit(0);
}

QFileInfo Measure::Append_to_name(QFileInfo P, QString string)
{
    QString newname = string;
    newname=P.baseName()+string;
    QString newPath = P.canonicalPath()+QDir::separator()+newname+"."+P.suffix();

    return *new QString(newPath);
}

time_t Measure::TDateTime_TimeT(double Convertee)
{
    const int EpochDiff = 25569; // days between 30/12/1899 and 01/01/1970
    const long SecsInDay = 86400; // number of seconds in a day

    time_t Result((long)(((Convertee) - EpochDiff) * SecsInDay));
    return Result;
}


void Measure::ProcessPHT3(unsigned int TTTRRecord)
{
    const long T3WRAPAROUND = 65536;
    union
    {
        unsigned long allbits;
        struct
        {
            unsigned numsync  :16;
            unsigned dtime    :12;
            unsigned channel  :4;
        } bits;
        struct
        {
            unsigned numsync  :16;
            unsigned markers  :12;
            unsigned channel  :4;
        } special;
    } Record;

    Record.allbits = TTTRRecord;
    if(Record.bits.channel==0xF) //this means we have a special record
    {
        if(Record.special.markers==0) //not a marker means overflow
        {
            //          GotOverflow(1);
            oflcorrection += T3WRAPAROUND; // unwrap the time tag overflow
        }
        else
        {
            truensync = oflcorrection + Record.bits.numsync;
            //          m = Record.special.markers;
            //          GotMarker(truensync, m);
        }
    } else
    {
        if(
                (Record.bits.channel==0) //Should never occur in T3 Mode
                ||(Record.bits.channel>4) //Should not occur with current routers
                )
        {
            printf("\nIllegal Channel: #%1d %1u",dlen,Record.bits.channel);
            fprintf(fpout,"#\nillegal channel ");
        }

        truensync = oflcorrection + Record.bits.numsync;
        int m, c;
        m = Record.bits.dtime;
        c = Record.bits.channel;
        GotPhoton(truensync, c, m);
        dlen++;
    }
};

void Measure::GotPhoton(long long TimeTag, int Channel, int DTime)
/* Called when a photon is recorded, trigger the analysis. Only works in the T3
   mode. Counts the number of photon on each channel.
 */
{

    if(IsT2)
    {
        printf("T2 files not supported!");
        exit(-1);
        //fprintf(fpout,"#%I64u CHN %1x %I64u %8.0lf\n", RecNum, Channel, TimeTag, (TimeTag * GlobRes * 1e12));
    }
    else
    {
        //fprintf(fpout,"#%I64u CHN %1x %I64u %8.0lf %10u\n", RecNum, Channel, TimeTag, (TimeTag * GlobRes * 1e9), DTime);
        fotone f={
            .RecNum=RecNum,
            .Channel=Channel,
            .TimeTag=TimeTag,
            .Dtime=DTime,
            .tt=TimeTag*GlobRes+DTime*Resolution,
        };

        Intensity(f);
        Nl=f.TimeTag;
        if(f.Channel==1) {
            Nch1++;
        }
        else{
            Nch2++;
        }

        //
    }
}

void Measure::Intensity(fotone f1){
    /*Measure the local intensity, before sending the photon to
    the function that perfoms the G2 and the lifeTime.
    Used to apply a threshold.
   */
    if(MC!=f1.Channel) {
        vf_both.push_back(f1);
        return;
    }
    if (f1.tt<startTime+intTime){
        intC++;
        vf.push_back(f1);
        vf_both.push_back(f1);
    }else{
        fprintf(fint,"%Le %d\n", startTime+(intTime/2), intC);
        for(std::vector<fotone>::size_type i=0; i<vf_both.size();i++){
            vf_both[i].intensity=intC;
            CreateG2(vf_both[i]);
            if(flag_normalization==1) CreateFarG2(vf_both[i]);
            LifeTime(vf_both[i], sogliaGlob);
        }
        intC=1; //reset the counter
        vf.clear();
        vf_both.clear();
        vf.push_back(f1);
        vf_both.push_back(f1);
        while(startTime+intTime<f1.tt){
            startTime+=intTime;
        }
    }
}

void Measure::CreateG2(fotone f){
    /* Create the histogram for the g2, after having cheked if the intensity
     when the photon f arrived was over the threshold.
   */
    ic1=ic1%g2width;//channel 1 renormalize to the array size
    ic2=ic2%g2width;//channel 2 renormalize to the array size
    int ica;
    long long int RecNum=f.RecNum;
    int Channel=f.Channel;
    long long TimeTag=f.TimeTag;
    int DTime=f.Dtime;
    fotone *oca; //other channel array
    // discarding photons out of the imposed limit
    if (altAt!=0 and f.tt>altAt ){
        return;
    }
    //couting if the intensity is above or under the threshold
    (f.intensity>sON)?Non++:Noff++;
    //printf("create g2 executed\n");
    if(Channel==1){
        fc1[ic1].RecNum=RecNum;
        fc1[ic1].Channel=Channel;
        fc1[ic1].TimeTag=TimeTag;
        fc1[ic1].Dtime=DTime;
        fc1[ic1].tt=TimeTag*GlobRes+DTime*Resolution;
        ic1++;
        oca=fc2;
        ica=ic2-1;//the last that was filled
    } else if(Channel==2){
        fc2[ic2].RecNum=RecNum;
        fc2[ic2].Channel=Channel;
        fc2[ic2].TimeTag=TimeTag;
        fc2[ic2].Dtime=DTime;
        fc2[ic2].tt=TimeTag*GlobRes+DTime*Resolution;
        ic2++;
        oca=fc1;
        ica=ic1-1;//the last that was filled
    } else{
        std::cerr<<"ERR: channel not 1 nor 2: error!";
    }

    //generico a prescindere dal canale

    double time;
    double tmax;
    int indice;
    for(int i=ica;i!=ica+1 ;i--){
        i=(i+g2width)%g2width;
        if(i<0)
            std::cout<<i<<"\n";
        if(oca[i].Channel==0) break;
        if(!(abs((TimeTag)-(oca[i].TimeTag))<=g2width)){
            break;
        }
        else
        {
            //    if(abs((TimeTag)-(oca[i].TimeTag))<=g2width){
            //TODO: se faccio il test con tt al posto di time tag viene meglio (?)

            //printf("coincidence found and added\n");
            //int t; //time elapsed, with sign, in numeber of pulses

            time=(TimeTag-oca[i].TimeTag)*GlobRes+(DTime-oca[i].Dtime)*Resolution;
            if (Channel==2) time*=-1;
            tmax=(2*g2width+2)*GlobRes; //the +2 is needed!

            binw=tmax/bin;
            //binw=2*tmax/bin;
            //TODO: l'errore credo che sia qui.
            // int test=(int)1.9;
            indice= (int) ((time+(tmax)/2)/binw);
            //int indice= (int) ((time+tmax)/binw);
            // printf("globRes=%e, binw=%e\n",GlobRes,binw);
            if(indice>=bin || indice<0) {
                printf("\n ERROR! i=%d\t bin=%d\n",indice,bin);
            } else{
                g2array[indice]++;}
        }

    }
    // printf("ciao\n");
}

void Measure::CreateFarG2(fotone f){
    /* Create the histogram for nomalizing the g2, after having cheked if the
     intensity when the photon f arrived was over the threshold. This does not
     consider any peak but only one peak over some.
   */
    //  int g2width_far=10000;
//    QCoreApplication::processEvents();
    int g2red_far=5; //only one peak over 10 is used
    ic1_far=ic1_far%g2width_far;//channel 1 renormalize to the array size
    ic2_far=ic2_far%g2width_far;//channel 2 renormalize to the array size
    int ica;
    long long int RecNum=f.RecNum;
    int Channel=f.Channel;
    long long TimeTag=f.TimeTag;
    int DTime=f.Dtime;
    fotone *oca; //other channel array
    // discarding photons out of the imposed limit
    if (altAt!=0 and f.tt>altAt ){
        return;
    }
    //couting if the intensity is above or under the threshold
    (f.intensity>sON)?Non++:Noff++;
    //printf("create g2 executed\n");
    if(Channel==1){
        fc1_far[ic1_far].RecNum=RecNum;
        fc1_far[ic1_far].Channel=Channel;
        fc1_far[ic1_far].TimeTag=TimeTag;
        fc1_far[ic1_far].Dtime=DTime;
        fc1_far[ic1_far].tt=TimeTag*GlobRes+DTime*Resolution;
        ic1_far++;
        oca=fc2_far;
        ica=ic2_far-1;//the last that was filled
    }

    if(Channel==2){
        fc2_far[ic2_far].RecNum=RecNum;
        fc2_far[ic2_far].Channel=Channel;
        fc2_far[ic2_far].TimeTag=TimeTag;
        fc2_far[ic2_far].Dtime=DTime;
        fc2_far[ic2_far].tt=TimeTag*GlobRes+DTime*Resolution;
        ic2_far++;
        oca=fc1_far;
        ica=ic1_far-1;//the last that was filled
    }

    //generico a prescindere dal canale

    double time;
    double tmax;
    int indice;
    //  static double maxrel=0;
    for(int i=ica;i!=ica+1 ;i--){
        i=(i+g2width_far)%g2width_far;
        if(i<0)
            std::cout<<i<<"\n";
        if(oca[i].Channel==0) break;
        if((abs((TimeTag)-(oca[i].TimeTag))%g2red_far)>1) {
            continue;
        } //sto in pratica considerando due intervalli
        if(!(abs((TimeTag)-(oca[i].TimeTag))<=g2width_far)){
            break;
        }
        else
        {
            //if(abs((TimeTag)-(oca[i].TimeTag))<=g2width){
            //TODO: se faccio il test con tt al posto di time tag viene meglio (?)

            //printf("coincidence found and added\n");
            //int t; //time elapsed, with sign, in numeber of pulses

            time=(TimeTag-oca[i].TimeTag)*GlobRes+(DTime-oca[i].Dtime)*Resolution;
            if (Channel==2) time*=-1;
            tmax=(2*g2width_far+2)*GlobRes; //the +2 is needed!

            binw=tmax/bin_far;

            indice= (int) ((time+(tmax)/2)/binw);

            if(indice>=bin_far || indice<0) {
                printf("\n ERROR! i=%d\t bin=%d\n",indice,bin_far);
            } else{
                g2array_far[indice]++;}
        }
    }

}

std::vector <std::pair<double, long int>> Measure::get_g2_max_vector(long long int *g2array_far)
{
    /*this functions returns the g2_max_vector,with the time on x and the
    intensity on y.
  */
    std::vector <std::pair<double, long int>> g2_max_vector;
    g2_max_vector.reserve(g2width*2+2);
    int j=0;
    int zero_counter=-1;
    std::pair<double, long int> maxrel=std::make_pair(0.,0);
    double t; //time
    for(int i=0;i<bin_far;i++){
        t=(i-bin_far/2.)*binw;
        if(g2array_far[i]>maxrel.second){
            if(zero_counter<0) zero_counter=0;
            maxrel=std::make_pair(t,g2array_far[i]);
        }
        else if(g2array_far[i]==0 && zero_counter!=-1){
            //zero_counter=-1;
            zero_counter++;
            if (zero_counter>=binNum){
                zero_counter=-1;
                g2_max_vector.push_back(maxrel);
                j++;
                maxrel=std::make_pair(0,0);
            }
        }
    }
    return g2_max_vector;
}

void Measure::LifeTime(fotone f1, int soglia){
    /* Create the istogram for the lifetime.
     Add the right arrival time of f1, after had check that
     the intensity is over the threshold "soglia"
   */
    int sign= (soglia==0)?1:abs(soglia)/soglia;
    if (altAt!=0 and f1.tt>altAt ){
        return;
    }
    if(((f1.intensity*sign) >= soglia) and (f1.Channel==MC)){
        lifeTime_hist.at(f1.Dtime)++;
    }
}

void Measure::print_histogram(){
    /* Fuction that prints all the results on different files
   */
    printf("printing in files\n");
    //  double normFactor, normFactorON;
    long int Ntot=Nch1+Nch2;
    double Nl_on;
    probT=Nch1/Nl;
    Nl_on=Nl*(Non/(double) Noff);
    //  normFactor=(T/(1-T))*(Nl)/(Nch1*Nch1*GlobRes);
    fprintf(fpout,"#g2 histogram\n");
    fprintf(fpout,"# T=%f, Nch1=%ld, Nch2=%ld\n",
            T,Nch1, Nch2);
    fprintf(fpout,"#Ntot=%ld\n",Nch1+Nch2);
    fprintf(fpout,"#Nl=%lld\t total number of laser pulses\n",Nl);
    fprintf(fpout,"#binw=%e\n",binw);

    //fprintf(fpout"#\n",);

    fprintf(fpout,"#progressive time corr_time counts normCounts\n");
    //  normFactor=((1-T)/T)*(Nch1*Nch1/Nl)*binw*1e6;
    //  normFactorON=((1-T)/T)*(Nch1*Nch1/Nl_on)*binw*1e6;
    double mean=1.;
    if(flag_normalization==1){
        mean=0;
        std::vector<std::pair<double, long int>> g2_max_vector;
        g2_max_vector=get_g2_max_vector(g2array_far);
        for(size_t i=0;i<g2_max_vector.size() and i<30 ;i++){
            // fprintf(f_g2_far,"%zu %le %ld\n",
            //         i,
            //         g2_max_vector.at(i).first,
            //         g2_max_vector.at(i).second);
            if(i>1 and i<30){
                mean+=(g2_max_vector.at(i).second/29.);
            }
            std::cout<<"\n the mean is"<<mean<<"\n";
        }


        //prints g2 only with maxima
        for(size_t i=0;i<g2_max_vector.size();i++){
            fprintf(f_g2_far,"%zu %le %ld %le\n",
                    i,
                    g2_max_vector.at(i).first,
                    g2_max_vector.at(i).second,
                    g2_max_vector.at(i).second/mean);
        }
        // for(int i=0;i<bin_far;i++){
        //   //   std::cout<<"i="<<i<<"\n";
        //   fprintf(f_g2_far,"%d %le %lld %lf %lf\n", i,
        //           (i-bin_far/2.)*binw, g2array_far[i], g2array_far[i] / normFactor,
        //           g2array_far[i] / normFactorON);
        // }
    }

    //print the g2
    //double time;

    for(int i=0;i<bin;i++){
        double corrTime=0;
        int toleave;
        if(abs((i-bin/2.)*binw)<GlobRes/2.){
            toleave=0;
        }else{
            toleave=1;
            corrTime=((i-bin/2.)*binw);
            corrTime=(corrTime<GlobRes/2.)?(corrTime+GlobRes):corrTime;
            //      toleave=((i-bin)<0)?-1:1;
            //      corrTime=(i-bin/2.)*binw-toleave*GlobRes/2;
        }

        if(toleave){
            fprintf(fpout,"%d %le %le %lld %lf\n", i,
                    (i-bin/2.)*binw, corrTime,
                    g2array[i], g2array[i] / mean);
            if(flag_normalization) fprintf(f_norm,"%d %le %le\n", i, corrTime, g2array[i]/mean);
        }else{
            fprintf(fpout,"%d %le none %lld %lf\n", i,
                    (i-bin/2.)*binw,
                    g2array[i], g2array[i] / mean);
        }

        //    fprintf(fpout,"%d %le %lld %lf\n", i,
        //            (i-bin/2.)*binw, g2array[i], g2array[i] / mean);
    }

    fprintf(flife,"#lifetime histogram");
    fprintf(flife,"#Channel=%d",MC);
    //<<<<<<< HEAD
    // for(std::vector<int>::size_type i=0;i<lifeTime_hist.size();i++){
    //   fprintf(flife,"%lu %le %d\n",
    //           i, i*Resolution, lifeTime_hist[i]);
    //=======
    std::vector<double> tempo;
    for(std::vector<int>::size_type i=0;i<lifeTime_hist.size();i++){
        if(lifeTime_hist[i]>=0){
            fprintf(flife,"%lu %le %d\n",
                    i, i*Resolution, lifeTime_hist[i]);
        }
        fflush(flife);
        tempo.push_back(i*Resolution*1e6);
        //>>>>>>> parent of 526d7db... Revert "first adding gnuplot functionalities"
    }



//    gp<<"file1='" << std::string(File_out.c_str())<<"'\n";
//    gp<<"file2='" << std::string(File_int.c_str())<<"'\n";
//    gp<<"file3='" << std::string(File_life.c_str())<<"'\n";


//    gp<<"l '/home/pierinis/Documenti/Dottorato/DatiParigi/Codici/Qt_analisi/Analisi/script_fit2.gpt'\n";
//    gp<<"\n";
//    gp.flush();


}


void Measure::ProcessPHT2(unsigned int TTTRRecord)
{
    const int T2WRAPAROUND = 210698240;
    union
    {
        unsigned int allbits;
        struct
        {
            unsigned time   :28;
            unsigned channel  :4;
        } bits;

    } Record;
    unsigned int markers;
    Record.allbits = TTTRRecord;
    if(Record.bits.channel==0xF) //this means we have a special record
    {
        //in a special record the lower 4 bits of time are marker bits
        markers=Record.bits.time&0xF;
        if(markers==0) //this means we have an overflow record
        {
            oflcorrection += T2WRAPAROUND; // unwrap the time tag overflow
            //          GotOverflow(1);
        }
        else //a marker
        {
            //Strictly, in case of a marker, the lower 4 bits of time are invalid
            //because they carry the marker bits. So one could zero them out.
            //However, the marker resolution is only a few tens of nanoseconds anyway,
            //so we can just ignore the few picoseconds of error.
            truetime = oflcorrection + Record.bits.time;
            //          GotMarker(truetime, markers);
        }
    }
    else
    {
        if((int)Record.bits.channel > 4) //Should not occur
        {
            printf(" Illegal Chan: #%I64lld %1u\n",RecNum,Record.bits.channel);
            fprintf(fpout,"# illegal chan.\n");
        }
        else
        {
            if(Record.bits.channel==0) cnt_0++;
            if(Record.bits.channel>=1) cnt_1++;

            truetime = oflcorrection + Record.bits.time;
            m = Record.bits.time;
            c = Record.bits.channel;
            GotPhoton(truetime, c, m);
        }
    }
}

void Measure::ProcessHHT2(unsigned int TTTRRecord, int HHVersion)
{
    const int T2WRAPAROUND_V1 = 33552000;
    const int T2WRAPAROUND_V2 = 33554432;
    union{
        unsigned int   allbits;
        struct{ unsigned timetag  :25;
                unsigned channel  :6;
                    unsigned special  :1; // or sync, if channel==0
              } bits;
    } T2Rec;
    T2Rec.allbits = TTTRRecord;

    if(T2Rec.bits.special==1)
    {
        if(T2Rec.bits.channel==0x3F) //an overflow record
        {
            if(HHVersion == 1)
            {
                oflcorrection += (uint64_t)T2WRAPAROUND_V1;
                //              GotOverflow(1);
            }
            else
            {
                //number of overflows is stored in timetag
                if(T2Rec.bits.timetag==0) //if it is zero it is an old style single overflow
                {
                    //                  GotOverflow(1);
                    oflcorrection += (uint64_t)T2WRAPAROUND_V2;  //should never happen with new Firmware!
                }
                else
                {
                    oflcorrection += (uint64_t)T2WRAPAROUND_V2 * T2Rec.bits.timetag;
                    //                  GotOverflow(T2Rec.bits.timetag);
                }
            }
        }

        if((T2Rec.bits.channel>=1)&&(T2Rec.bits.channel<=15)) //markers
        {
            truetime = oflcorrection + T2Rec.bits.timetag;
            //Note that actual marker tagging accuracy is only some ns.
            m = T2Rec.bits.channel;
            //          GotMarker(truetime, m);
        }

        if(T2Rec.bits.channel==0) //sync
        {
            truetime = oflcorrection + T2Rec.bits.timetag;
            GotPhoton(truetime, 0, 0);
        }
    }
    else //regular input channel
    {
        truetime = oflcorrection + T2Rec.bits.timetag;
        c = T2Rec.bits.channel + 1;
        GotPhoton(truetime, c, 0);
    }

}


void Measure::ProcessHHT3(unsigned int TTTRRecord, int HHVersion)
{
    const int T3WRAPAROUND = 1024;
    union {
        unsigned int allbits;
        struct  {
            unsigned nsync    :10;  // numer of sync period
            unsigned dtime    :15;    // delay from last sync in units of chosen resolution
            unsigned channel  :6;
            unsigned special  :1;
        } bits;
    } T3Rec;
    T3Rec.allbits = TTTRRecord;
    if(T3Rec.bits.special==1)
    {
        if(T3Rec.bits.channel==0x3F) //overflow
        {
            //number of overflows is stored in nsync
            if((T3Rec.bits.nsync==0) || (HHVersion==1)) //if it is zero or old version it is an old style single oferflow
            {
                oflcorrection += (uint64_t)T3WRAPAROUND;
                //              GotOverflow(1); //should never happen with new Firmware!
            }
            else
            {
                oflcorrection += (uint64_t)T3WRAPAROUND * T3Rec.bits.nsync;
                //              GotOverflow(T3Rec.bits.nsync);
            }
        }
        if((T3Rec.bits.channel>=1)&&(T3Rec.bits.channel<=15)) //markers
        {
            truensync = oflcorrection + T3Rec.bits.nsync;
            //the time unit depends on sync period which can be obtained from the file header
            c = T3Rec.bits.channel;
            //          GotMarker(truensync, c);
        }
    }
    else //regular input channel
    {
        truensync = oflcorrection + T3Rec.bits.nsync;
        //the nsync time unit depends on sync period which can be obtained from the file header
        //the dtime unit depends on the resolution and can also be obtained from the file header
        c = T3Rec.bits.channel;
        m = T3Rec.bits.dtime;
        GotPhoton(truensync, c, m);
    }
}



int Measure::readHeader()
{
    int Result;
    char Magic[8];
    char Version[8];
    char Buffer[40];
    char* AnsiBuffer;
//    wchar_t* WideBuffer;
    char32_t* WideBuffer;
    long long NumRecords = -1;
    long long RecordType = 0;

    Result = fread( &Magic, 1, sizeof(Magic) ,fpin);
    if (Result!= sizeof(Magic))
    {
        printf("\nerror reading header, aborted.");
        goto close;
    }
    Result = fread(&Version, 1, sizeof(Version) ,fpin);
    if (Result!= sizeof(Version))
    {
        printf("\nerror reading header, aborted.");
        goto close;
    }
    if (strncmp(Magic, "PQTTTR", 6))
    {
        printf("\nWrong Magic, this is not a PTU file.");
        goto close;
    }
    fprintf(fpout, "#Tag Version: %s \n", Version);

    do
    {
        // This loop is very generic. It reads all header items and displays the identifier and the
        // associated value, quite independent of what they mean in detail.
        // Only some selected items are explicitly retrieved and kept in memory because they are
        // needed to subsequently interpret the TTTR record data.

        Result = fread( &TagHead, 1, sizeof(TagHead) ,fpin);
        if (Result!= sizeof(TagHead))
        {
            printf("\nIncomplete File.");
            goto close;
            //            close();
        }

        strcpy(Buffer, TagHead.Ident);
        if (TagHead.Idx > -1)
        {
            sprintf(Buffer, "%s(%d)", TagHead.Ident,TagHead.Idx);
        }
        fprintf(fpout, "\n#%-40s", Buffer);
        switch (TagHead.Typ)
        {
        case tyEmpty8:
            fprintf(fpout, "#<empty Tag>");
            break;
        case tyBool8:
            fprintf(fpout, "#%s", bool(TagHead.TagValue)?"True":"False");
            break;
        case tyInt8:
            fprintf(fpout, "#%lld", TagHead.TagValue);
            // get some Values we need to analyse records
            if (strcmp(TagHead.Ident, TTTRTagNumRecords)==0) // Number of records
                NumRecords = TagHead.TagValue;
            if (strcmp(TagHead.Ident, TTTRTagTTTRRecType)==0) // TTTR RecordType
                RecordType = TagHead.TagValue;
            break;
        case tyBitSet64:
            fprintf(fpout, "#0x%16.16lld", TagHead.TagValue);
            break;
        case tyColor8:
            fprintf(fpout, "#0x%16.16lld", TagHead.TagValue);
            break;
        case tyFloat8:
            fprintf(fpout, "#%E", *(double*)&(TagHead.TagValue));
            if (strcmp(TagHead.Ident, TTTRTagRes)==0) {// Resolution for TCSPC-Decay
                Resolution = *(double*)&(TagHead.TagValue);
                printf("\n *** Resolution=%e s  ***\n", Resolution);
            }
            if (strcmp(TagHead.Ident, TTTRTagGlobRes)==0) {// Global resolution for timetag
                GlobRes = *(double*)&(TagHead.TagValue); // in ns
                printf("\n *** GlobRes=%e s  ***\n", GlobRes);
            }
            break;
        case tyFloat8Array:
            fprintf(fpout, "#<Float Array with %llu Entries>", TagHead.TagValue / sizeof(double));
            // only seek the Data, if one needs the data, it can be loaded here
            fseek(fpin, (long)TagHead.TagValue, SEEK_CUR);
            break;
        case tyTDateTime:
            time_t CreateTime;
            CreateTime = TDateTime_TimeT(*((double*)&(TagHead.TagValue)));
            // fprintf(fpout, "#%s", asctime(gmtime(&CreateTime)), "\0");
            fprintf(fpout, "#%s", asctime(gmtime(&CreateTime)));
            break;
        case tyAnsiString:
            AnsiBuffer = (char*)calloc((size_t)TagHead.TagValue,1);
            Result = fread(AnsiBuffer, 1, (size_t)TagHead.TagValue, fpin);
            if (Result!= TagHead.TagValue)
            {
                printf("\nIncomplete File.");
                free(AnsiBuffer);
                goto close;
            }
            fprintf(fpout, "#%s", AnsiBuffer);
            free(AnsiBuffer);
            break;
        case tyWideString:
            WideBuffer = (char32_t*)calloc((size_t)TagHead.TagValue,1);
            Result = fread(WideBuffer, 1, (size_t)TagHead.TagValue, fpin);
            if (Result!= TagHead.TagValue)
            {
                printf("\nIncomplete File.");
                free(WideBuffer);
                goto close;
            }
            fwprintf(fpout, L"%s", WideBuffer);
            free(WideBuffer);
            break;
        case tyBinaryBlob:
            fprintf(fpout, "#<Binary Blob contains %lld Bytes>", TagHead.TagValue);
            // only seek the Data, if one needs the data, it can be loaded here
            fseek(fpin, (long)TagHead.TagValue, SEEK_CUR);
            break;
        default:
            printf("Illegal Type identifier found! Broken file?");
            goto close;
        }

    }
    while((strncmp(TagHead.Ident, FileTagEnd, sizeof(FileTagEnd))));

    // TTTR Record type
    switch (RecordType)
    {
    case rtPicoHarpT2:
        fprintf(fpout, "#PicoHarp T2 data\n");
        //      fprintf(fpout,"\n#record# chan   nsync truetime/ps\n");
        break;
    case rtPicoHarpT3:
        fprintf(fpout, "#PicoHarp T3 data\n");
        //      fprintf(fpout,"\n#record# chan   nsync truetime/ns dtime\n");
        break;
    case rtHydraHarpT2:
        fprintf(fpout, "#HydraHarp V1 T2 data\n");
        //     fprintf(fpout,"\n#record# chan   nsync truetime/ps\n");
        break;
    case rtHydraHarpT3:
        fprintf(fpout, "#HydraHarp V1 T3 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ns dtime\n");
        break;
    case rtHydraHarp2T2:
        fprintf(fpout, "#HydraHarp V2 T2 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ps\n");
        break;
    case rtHydraHarp2T3:
        fprintf(fpout, "#HydraHarp V2 T3 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ns dtime\n");
        break;
    case rtTimeHarp260NT3:
        fprintf(fpout, "#TimeHarp260N T3 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ns dtime\n");
        break;
    case rtTimeHarp260NT2:
        fprintf(fpout, "#TimeHarp260N T2 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ps\n");
        break;
    case rtTimeHarp260PT3:
        fprintf(fpout, "#TimeHarp260P T3 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ns dtime\n");
        break;
    case rtTimeHarp260PT2:
        fprintf(fpout, "#TimeHarp260P T2 data\n");
        // fprintf(fpout,"\n#record# chan   nsync truetime/ps\n");
        break;
    default:
        // fprintf(fpout, "#Unknown record type: 0x%X\n 0x%X\n ", RecordType);
        goto close;
    }

    //ugly to put it here
    lifeTime_hist.reserve(static_cast<unsigned long>(GlobRes/Resolution)+100);
    lifeTime_hist.assign(static_cast<unsigned long>(GlobRes/Resolution)+100,0);

//    unsigned int TTTRRecord;
    uint32_t TTTRRecord;
//    std::cout<<"unsigned int size: " << sizeof (uint32_t)<<"\n";

    for(RecNum=0;RecNum<NumRecords;RecNum++)
    {
        Result = fread(&TTTRRecord, 1, sizeof(TTTRRecord) ,fpin);
        if (Result!= sizeof(TTTRRecord))
        {
            printf("\nUnexpected end of input file!");
            break;
        }
        switch (RecordType)
        {
        case rtPicoHarpT2:
            IsT2 = true;
            ProcessPHT2(TTTRRecord);
            break;
        case rtPicoHarpT3:
            IsT2 = false;
            ProcessPHT3(TTTRRecord);
            break;
        case rtHydraHarpT2:
            IsT2 = true;
            ProcessHHT2(TTTRRecord, 1);
            break;
        case rtHydraHarpT3:
            IsT2 = false;
            ProcessHHT3(TTTRRecord, 1);
            break;
        case rtHydraHarp2T2:
        case rtTimeHarp260NT2:
        case rtTimeHarp260PT2:
            IsT2 = true;
            ProcessHHT2(TTTRRecord, 2);
            break;
        case rtHydraHarp2T3:
        case rtTimeHarp260NT3:
        case rtTimeHarp260PT3:
            IsT2 = false;
            ProcessHHT3(TTTRRecord, 2);
            break;
        default:

            goto close;
        }
    }

    fprintf(fpout, "#\n#-----------------------\n");

    return 0;
close:
    close();
    return 0;
}


int Measure::close(){
close:
    //    print_histogram();
    fclose(fpin);
    fclose(fpout);
    fclose(flife);
    fclose(fint);
    if (flag_normalization) fclose(f_g2_far);
ex:
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //    sleep(1);
    printf("warning: close function used");
    printf("\nproportions:\n"
           "    %f%% ch1\n"
           "    %f%% ch2\n",
           Nch1/((double)(Nch1+Nch2)),
           Nch2/((double)(Nch1+Nch2))
           );
    //    printf("\n press enter to exit...\n");
    //    getchar();
    //exit(0);
    return(0);
}


