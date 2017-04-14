#include "create_data.hpp"
#include <algorithm>

/// SparseTimeBase

std::string SparseTimeBase::defaultConvertTS(
    std::string row, long long &result) {

    std::stringstream ss(row);
    std::tm t = {};
    ss >> std::get_time(&t, "%Y-%b-%d %H:%M:%S");
    result = mktime(&t);
    std::string rest;
    std::getline(ss, rest);
    return rest;
}

std::vector<std::string> SparseTimeBase::readCSVLine(std::string line, char delim=',') {
    // escape \, fields separated by ",", fields can be quoted with "
    boost::escaped_list_separator<char> sep( '\\', delim, '"' ) ;
    boost::tokenizer< boost::escaped_list_separator<char> > tokenizer( line, sep );
    return std::vector<std::string>( tokenizer.begin(), tokenizer.end() ) ;
}


TFlt SparseTimeBase::readValue(std::string val) {
    if (val.compare("f") == 0) return TFlt(0);
    if (val.compare("t") == 0) return TFlt(1);
    double val = stod(str_value);
    return TFlt(val);
}

Eigen::MatrixXd SparseTimeBase::fillData(long long initialTimestamp, int duration, int granularity) {
    int size = static_cast<int>(duration/granularity);
    Eigen::MatrixXd filledData(size,signal_count);
    std::vector<struct heapData> h_data;
    for(int i = 0 ; i < signal_values.Len(); ++i){
        signal_count++;
        if(signal_values[i].Len() > 0){
            h_data.push_back({i,0});
        }
    }
    if(h_data.size() > 0){
        std::make_heap(h_data.begin(),h_data.end(),[this](struct heapData& a, struct heapData& b){return static_cast<long long>(signal_values[a.signal_index][a.signal_position].Val1.Val) > static_cast<long long>(signal_values[b.signal_index][b.signal_position].Val1.Val);});
        Eigen::MatrixXd runningVector(1,signal_count);
        filledData.setZero(size,signal_count);
        runningVector.setZero(1,signal_count);
        heapData currentMin = h_data.front();
        for(long long t = initialTimestamp ; t < initialTimestamp + duration; t += granularity){
            if (t == static_cast<long long>(signal_values[currentMin.signal_index][currentMin.signal_position].Val1.Val)) { //update values
                while(static_cast<long long>(signal_values[currentMin.signal_index][currentMin.signal_position].Val1.Val) == t && h_data.size() > 0){
                    runningVector(0,currentMin.signal_index) = signal_values[currentMin.signal_index][currentMin.signal_position].Val2.Val;
                    std::pop_heap(h_data.begin(),h_data.end(),[this](struct heapData& a, struct heapData& b){return static_cast<long long>(signal_values[a.signal_index][a.signal_position].Val1.Val) > static_cast<long long>(signal_values[b.signal_index][b.signal_position].Val1.Val);});h_data.pop_back();
                    if(currentMin.signal_position + 1 < signal_values[currentMin.signal_index].Len()){
                        h_data.push_back({currentMin.signal_index,currentMin.signal_position+1});
                        std::push_heap(h_data.begin(),h_data.end(),[this](struct heapData& a, struct heapData& b){return static_cast<long long>(signal_values[a.signal_index][a.signal_position].Val1.Val) > static_cast<long long>(signal_values[b.signal_index][b.signal_position].Val1.Val);});
                    }
                    if(h_data.size() > 0){
                        currentMin = h_data.front();
                    }
                }
            }
            int count = static_cast<int>((t-initialTimestamp)/granularity);
            filledData.block(count,0,1,signal_count) = runningVector;
        }
    }
    return filledData;
}

///SparseTimeRowForm

void SparseTimeRowForm::parseData(std::string filename, bool isHeader) {
    std::ifstream inFile(filename);
    AssertR(inFile.is_open(), "could not open file " + filename);
    IdSignalValues.AddKey(id_string);
    TVec<TSensorData> signal_values(SensorCount);
    std::string line;
    while (std::getline(infile, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }
        long long ts;
        // fastforward line to remove timestamp
        std::string ff_line = ConvertTs(line, ts);
        std::vector<std::string> row = readCSVLine(ff_line);
        AssertR(row.size() == SensorCount);
        for (int i = 0; i < row.size(); i++) {
            if (row[i] != "") {
                TFlt val = readValue(row[i]);
                // add the timestamp, val pair to the right sensor
                signal_values[i].Add(TPair<TFlt, TFlt>(timestamp, val));
            }
        }
    }
    // add under the id string for this file
    IdSignalValues.AddDat(id_string, signal_values);
}



