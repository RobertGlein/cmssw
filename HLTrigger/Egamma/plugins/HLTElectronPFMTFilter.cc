#include "HLTElectronPFMTFilter.h"
#include "HLTrigger/HLTcore/interface/defaultModuleLabel.h"

template <typename T>
HLTElectronPFMTFilter<T>::HLTElectronPFMTFilter(const edm::ParameterSet& iConfig) : HLTFilter(iConfig) {
  // MHT parameters
  inputMetTag_ = iConfig.getParameter<edm::InputTag>("inputMetTag");
  minMht_ = iConfig.getParameter<double>("minMht");

  // Electron parameters
  inputEleTag_ = iConfig.getParameter<edm::InputTag>("inputEleTag");
  lowerMTCut_ = iConfig.getParameter<double>("lowerMTCut");
  upperMTCut_ = iConfig.getParameter<double>("upperMTCut");
  minN_ = iConfig.getParameter<int>("minN");
  l1EGTag_ = iConfig.getParameter<edm::InputTag>("l1EGCand");

  inputMetToken_ = consumes<reco::METCollection>(inputMetTag_);
  inputEleToken_ = consumes<trigger::TriggerFilterObjectWithRefs>(inputEleTag_);
}

template <typename T>
HLTElectronPFMTFilter<T>::~HLTElectronPFMTFilter() = default;

template <typename T>
void HLTElectronPFMTFilter<T>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  desc.add<edm::InputTag>("inputMetTag", edm::InputTag("hltPFMHT"));
  desc.add<edm::InputTag>("inputEleTag", edm::InputTag("hltEle25CaloIdVTTrkIdTCaloIsoTTrkIsoTTrackIsolFilter"));
  desc.add<edm::InputTag>("l1EGCand", edm::InputTag("hltL1IsoRecoEcalCandidate"));
  desc.add<int>("minN", 0);
  desc.add<double>("minMht", 0.0);
  desc.add<double>("lowerMTCut", 0.0);
  desc.add<double>("upperMTCut", 9999.0);
  descriptions.add(defaultModuleLabel<HLTElectronPFMTFilter<T>>(), desc);
}

template <typename T>
bool HLTElectronPFMTFilter<T>::hltFilter(edm::Event& iEvent,
                                         const edm::EventSetup& iSetup,
                                         trigger::TriggerFilterObjectWithRefs& filterproduct) const {
  using namespace std;
  using namespace edm;
  using namespace reco;
  using namespace trigger;
  // The filter object
  if (saveTags()) {
    filterproduct.addCollectionTag(inputMetTag_);
    filterproduct.addCollectionTag(l1EGTag_);
  }

  // Get the Met collection
  edm::Handle<reco::METCollection> pfMHT;
  iEvent.getByToken(inputMetToken_, pfMHT);

  // Sanity check:
  if (!pfMHT.isValid()) {
    edm::LogError("HLTElectronPFMTFilter") << "missing input Met collection!";
  }

  const METCollection* metcol = pfMHT.product();
  const MET* met;
  met = &(metcol->front());

  edm::Handle<trigger::TriggerFilterObjectWithRefs> PrevFilterOutput;
  iEvent.getByToken(inputEleToken_, PrevFilterOutput);

  int nW = 0;

  vector<Ref<vector<T>>> refEleCollection;

  PrevFilterOutput->getObjects(TriggerElectron, refEleCollection);
  int trigger_type = trigger::TriggerElectron;
  if (refEleCollection.empty()) {
    PrevFilterOutput->getObjects(TriggerCluster, refEleCollection);
    trigger_type = trigger::TriggerCluster;
    if (refEleCollection.empty()) {
      PrevFilterOutput->getObjects(TriggerPhoton, refEleCollection);
      trigger_type = trigger::TriggerPhoton;
    }
  }

  TLorentzVector pMET(met->px(), met->py(), 0.0, sqrt(met->px() * met->px() + met->py() * met->py()));

  for (unsigned int i = 0; i < refEleCollection.size(); i++) {
    TLorentzVector pThisEle(
        refEleCollection.at(i)->px(), refEleCollection.at(i)->py(), 0.0, refEleCollection.at(i)->et());
    TLorentzVector pTot = pMET + pThisEle;
    double mass = pTot.M();

    if (mass >= lowerMTCut_ && mass <= upperMTCut_ && pMET.E() >= minMht_) {
      nW++;
      filterproduct.addObject(trigger_type, refEleCollection.at(i));
    }
  }

  // filter decision
  const bool accept(nW >= minN_);
  return accept;
}

// declare the specialisations as framework plugins
#include "FWCore/Framework/interface/MakerMacros.h"

using HLTGsfElectronPFMTFilter = HLTElectronPFMTFilter<reco::Electron>;
DEFINE_FWK_MODULE(HLTGsfElectronPFMTFilter);

using HLTEcalCandidatePFMTFilter = HLTElectronPFMTFilter<reco::RecoEcalCandidate>;
DEFINE_FWK_MODULE(HLTEcalCandidatePFMTFilter);
