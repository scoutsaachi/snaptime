#ifndef CREATE_DATA_H_
#define CREATE_DATA_H_
#include <iostream>
#include <fstream>
#include "Eigen/Core"
#include "Eigen/Eigen"
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include "Snap.h"

typedef TVec<TPair<TFlt,TFlt> > TSensorData;

/// Virtual class encapsulating TSV conversion to sparse SNAP format
class SparseTimeBase {
protected:
    /// id_str: [sensor1 data, sensor2 data, ...]
    THash<TStr, TVec<TSensorData> > IdSignalValues;
    // number of sensors
    TInt SensorCount;
    struct heapData
    {
        int signal_index;
        int signal_position;
    };
    /* Read in the timestamp from the row and remove it. Place
     * the timestamp in epoch form into result */
    std::string(*ConvertTS)(std::string row, long long &);

protected:
    /// Default implementation for ConvertTS.
    /// The timestamp looks like Year-Day-Month Hour:Minute:Second
    std::string defaultConvertTS(std::string row, long long &result);
    // interpret the value as a bool or float
    TFlt readValue(std::string val);
    // return vector of strings split by ,
    std::vector<std::string> readCSVLine(std::string line);

public:
    SparseTimeSensorBase(int n_sensors) : SensorCount(n_sensors), ConvertTS(defaultConvertTS) {}
    /// can pass in a method to convert the timestamp to long long
    SparseTimeSensorBase(int n_sensors, int(*ConvertTSfn)(std::string row, long long &)) : SensorCount(n_sensors), ConvertTS(ConvertTSfn) {}

    virtual void parseData(std::string filename, bool hasHeader) = 0;
    /// Save SNAP format
    void Save(TSOut &Sout) {
        IdSignalValues.Save(Sout);
        SensorCount.Save(Sout);
    }
    /// Load from SNAP binary
    void Load(TSIn &Sin) {
        IdSignalValues.Load(Sout);
        SensorCount.Load(Sout);
    }
    Eigen::MatrixXd fillData(long long,int,int);
};

/// Read TSV in Row Form: Each file is named with id_str
/// Each row of file contains timestamp, sensor values
class SparseTimeRowForm : SparseTimeBase {
public:
    void parseData(std::string filename, bool hasHeader)

};

#endif
