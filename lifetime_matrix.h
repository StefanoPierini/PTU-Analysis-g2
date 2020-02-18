#ifndef LIFETIME_MATRIX_H
#define LIFETIME_MATRIX_H

#include <stdlib.h>
#include <vector>
#include <stdexcept>
#include <QFileInfo>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <ostream>
class Lifetime_matrix
{
private:
    unsigned int max_intensity;
    unsigned int max_time_index;
    std::vector< std::vector<int> > matrix;
    double get_bin_time_width();
    double set_bin_time_width(double bin_time_width);
public:
    Lifetime_matrix(uint max_intensity, uint max_time_index);
    std::vector<int> get_life_in_range(uint min_intensity, uint max_intensity);
    ~Lifetime_matrix();
    int resize(uint max_intensity, uint max_time_index);
    int get_value(const uint intensity, const uint time_index);
    int add_count(const uint intensity, const uint time_index);
    double bin_time_width=1.;
    char save(QFileInfo fileName);
};

#endif // LIFETIME_MATRIX_H
