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
    /// id_str: {id_sensor: sensor data} ...]
    THash<TStr, THash<TInt, TSensorData> > IdSignalValues;
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
    static std::string defaultConvertTS(std::string row, long long &result);
    // interpret the value as a bool or float
    TFlt readValue(std::string val);
    // return vector of strings split by ,
    std::vector<std::string> readCSVLine(std::string line);

public:
    SparseTimeBase() {ConvertTS = defaultConvertTS;}
    /// can pass in a method to convert the timestamp to long long
    SparseTimeBase(std::string(*ConvertTSfn)(std::string row, long long &)) {ConvertTS = ConvertTSfn;}

    virtual void parseData(std::string filename, bool hasHeader) = 0;
    /// Save SNAP format
    void Save(TSOut &Sout) {
        IdSignalValues.Save(Sout);
    }
    /// Load from SNAP binary
    void Load(TSIn &Sin) {
        IdSignalValues.Load(Sin);
    }
    Eigen::MatrixXd fillData(long long,int,int);
};

/// Read TSV in Row Form: Each file is named with id_str
/// Each row of file contains timestamp, sensor values
class SparseTimeRowForm : SparseTimeBase {
public:
    void parseData(std::string filename, bool hasHeader);
};

/// Read TSV in Col Form: Each file is named with the sensor name
/// Each row of file contains ID, timestamp, ID, value
class SparseTimeColForm : SparseTimeBase {
protected:
    THash<TStr, TInt> SensorIndex; //sensor string to index in IdSignalValues
    TInt SensorCount; //next index for sensor
public:
    void Save(TSOut &Sout) {
        SparseTimeBase::Save(Sout);
        SensorIndex.Save(Sout);
        SensorCount.Save(Sout);
    }
    /// Load from SNAP binary
    void Load(TSIn &Sin) {
        SparseTimeBase::Load(Sin);
        SensorIndex.Load(Sin);
        SensorCount.Load(Sin);
    }
    void parseData(std::string filename, bool hasHeader);
};

#endif
