#include "lifetime_matrix.h"


Lifetime_matrix::Lifetime_matrix(unsigned int max_intensity, unsigned int max_time_index)
{
    matrix.reserve(max_intensity+1);
    for(std::vector<int> line : matrix){
        line.reserve(max_time_index+1);
        line.assign(max_time_index+1,0);
    }
    this->max_intensity=max_intensity;
    this->max_time_index=max_time_index;
}

Lifetime_matrix::~Lifetime_matrix(){
}

int Lifetime_matrix::resize(uint max_intensity, uint max_time_index){
    //!Only increase the size of matrix
    /*The array needs bigger respect to the max intensity value,
     * as we need to take care about the 0
     */
    max_intensity++;
    max_time_index++;
    if(max_intensity>this->max_intensity){
        std::vector<int> zeros= *new std::vector<int>(this->max_time_index,0);
        matrix.resize(max_intensity,zeros);
        this->max_intensity=max_intensity;
    }
    if(max_time_index>this->max_time_index){
//        for(std::vector<int> line: matrix){
//            line.resize(max_time_index+1,0);
//        }
        for(std::vector<std::vector<int>>::iterator it=matrix.begin(); it<matrix.end();++it ){
            it->resize(max_time_index,0);
        }
        this->max_time_index=max_time_index;
    }
    return 1;
}

double Lifetime_matrix::set_bin_time_width(double bin_time_width){
    if(bin_time_width>0){
        this->bin_time_width=bin_time_width;
        return bin_time_width;
    }
    return 0;
}

double Lifetime_matrix::get_bin_time_width(){
    return bin_time_width;
}

std::vector<int> Lifetime_matrix::get_life_in_range(const uint min_intensity, const uint max_intensity){
    /*! return a vector of the intensity summed over the range
    */
    uint min, max;
    if(min_intensity>max_intensity){
        throw std::range_error("get_life_in_range: error in passing parameters");
    }
    min=min_intensity>1?min_intensity:1;
    max=(max_intensity<=this->max_intensity)?max_intensity:this->max_intensity;

    std::vector<int> result=std::vector<int>(max_time_index,0);
    for(uint i=min_intensity-1;i<=max_intensity;i++){
        for(uint j=0;j<result.size();++j)
        {
            result.at(j)+=matrix.at(i).at(j);
        }
    }
    return result;
}

int Lifetime_matrix::add_count(const uint intensity, const uint time_index){
    //!add a count to the matrix, increasing the size if needed.
    resize(intensity,time_index);
    matrix.at(intensity).at(time_index)++;
    return matrix.at(intensity).at(time_index);
}

int Lifetime_matrix::get_value(const uint intensity, const uint time_index){
    if (intensity>max_intensity || time_index>max_time_index){
        return -1;
    }else{
        return matrix.at(max_intensity).at(time_index);
    }
}

char Lifetime_matrix::save(QFileInfo fileName){
    QFile file(fileName.filePath());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return 0;
    QTextStream out(&file);
    out<<"#the first line of this files contains the times, than one line for each intensities, starting form 0"<<"\n";
    out<<"#if the width of the bin is not specified, the first line is the bin number\n";
    out.flush();
    if (!(bin_time_width>0)) bin_time_width=1;
    for(uint i=0;i<matrix.at(1).size();i++){
        out<<i*bin_time_width<<'\t';
    }
    out<<'\n';
    for(auto line:matrix){
        for(auto data:line){
            out<<data<<'\t';
        }
        out<<'\n';
    }
    out.flush();
}
