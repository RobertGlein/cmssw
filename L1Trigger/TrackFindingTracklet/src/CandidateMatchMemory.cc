#include "L1Trigger/TrackFindingTracklet/interface/CandidateMatchMemory.h"
#include "L1Trigger/TrackFindingTracklet/interface/Settings.h"
#include "L1Trigger/TrackFindingTracklet/interface/Tracklet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

using namespace std;
using namespace trklet;

CandidateMatchMemory::CandidateMatchMemory(string name, const Settings* const settings, unsigned int iSector)
    : MemoryBase(name, settings, iSector) {}

void CandidateMatchMemory::addMatch(std::pair<Tracklet*, int> tracklet, std::pair<const Stub*, const L1TStub*> stub) {
  std::pair<std::pair<Tracklet*, int>, std::pair<const Stub*, const L1TStub*> > tmp(tracklet, stub);

  //Check for consistency
  for (unsigned int i = 0; i < matches_.size(); i++) {
    if (tracklet.first->TCID() < matches_[i].first.first->TCID()) {
      edm::LogPrint("Tracklet") << "In " << getName() << " adding tracklet " << tracklet.first
                                << " with lower TCID : " << tracklet.first->TCID() << " than earlier TCID "
                                << matches_[i].first.first->TCID();
      assert(0);
    }
  }
  matches_.push_back(tmp);
}

void CandidateMatchMemory::writeCM(bool first) {
  std::string fname = "../data/MemPrints/Matches/CandidateMatches_";
  fname += getName();
  fname += "_";
  if (iSector_ + 1 < 10)
    fname += "0";
  fname += std::to_string(iSector_ + 1);
  fname += ".dat";
  if (first) {
    bx_ = 0;
    event_ = 1;
    out_.open(fname.c_str());
  } else
    out_.open(fname.c_str(), std::ofstream::app);

  out_ << "BX = " << (bitset<3>)bx_ << " Event : " << event_ << endl;

  for (unsigned int j = 0; j < matches_.size(); j++) {
    string stubid = matches_[j].second.first->stubindex().str();  // stub ID
    int projindex = matches_[j].first.second;                     // Allproj index
    FPGAWord tmp;
    if (projindex >= (1 << 7)) {
      projindex = (1 << 7) - 1;
    }
    tmp.set(projindex, 7, true, __LINE__, __FILE__);
    out_ << "0x";
    if (j < 16)
      out_ << "0";
    out_ << hex << j << dec;
    out_ << " " << tmp.str() << "|" << stubid << " " << trklet::hexFormat(tmp.str() + stubid) << endl;
  }
  out_.close();

  bx_++;
  event_++;
  if (bx_ > 7)
    bx_ = 0;
}
