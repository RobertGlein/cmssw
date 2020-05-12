#ifndef L1Trigger_TrackFindingTracklet_interface_TETableBase_h
#define L1Trigger_TrackFindingTracklet_interface_TETableBase_h

#include "L1Trigger/TrackFindingTracklet/interface/Settings.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>
#include <vector>

namespace trklet {

  class Settings;

  class TETableBase {
  public:
    TETableBase(const Settings* settings);

    virtual ~TETableBase() {}

    virtual int lookup(int, int);

    void writeVMTable(std::string name, bool positive = true);

  protected:
    const Settings* settings_;
    std::vector<int> table_;
    int nbits_;
  };

};  // namespace trklet
#endif
