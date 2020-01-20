#ifndef USEFULFUNCTIONS_H
#define USEFULFUNCTIONS_H

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>

inline std::string replaceFirstOccurrence(
        std::string& s,
        const std::string& toReplace,
        const std::string& replaceWith)
    {
        std::size_t pos = s.find(toReplace);
        if (pos == std::string::npos) return s;
        return s.replace(pos, toReplace.length(), replaceWith);
    }

inline std::vector< std::vector<double> > readIn2dData(const char* filename){
    /* Function takes a char* filename argument and returns a
     * 2d dynamic array containing the data
     */

    std::vector< std::vector<double> > table;
    std::fstream ifs;
    /*  open file  */
    if(!boost::filesystem::exists(filename)){
        throw "The file does not exists";
    }
    ifs.open(filename);
    while (true)
    {
        bool ok=true;
        std::string line, copy;
        double buf;
        getline(ifs, line);



        if (!ifs)
            // mainly catch EOF
            break;

        do{
          copy=line;
          line=replaceFirstOccurrence(line,"none","-1");
        }while(line!=copy);

        std::stringstream ss(line, std::ios_base::out|std::ios_base::in|std::ios_base::binary);
        if (line[0] == '#' || line.empty())
            // catch empty lines or comment lines
            continue;


        std::vector<double> row;

        while (ss >> buf)

            try {
            row.push_back(buf);
        } catch (std::bad_alloc) {
            ok=false;
        }

        if(ok){
        table.push_back(row);
        }
    }

    ifs.close();

    return table;
}

inline std::vector<std::vector<double> > trasposeTable(const std::vector<std::vector<double>> b){
    if (b.size() == 0)
        return b;

    std::vector<std::vector<double> > trans_vec(b[0].size(), std::vector<double>());
    for(unsigned i = 0; i<b.size() ;++i){
        if(b[0].size()!=b[i].size())
            throw "The number of colums is not constant";
    }

    for (unsigned i = 0; i < b.size(); i++)
    {
        for (unsigned j = 0; j < b[i].size(); j++)
        {
            trans_vec[j].push_back(b[i][j]);
        }
    }

    return trans_vec;    // <--- reassign here
}


#endif // USEFULFUNCTIONS_H
