//
// Original Author:  Stefano Magni
//         Created:  Fri Mar  9 10:52:11 CET 2007
//
//


// system include files
#include <memory>
#include <string>

// user include files
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "SimTracker/TrackAssociation/interface/TrackAssociatorBase.h"
#include "SimTracker/Records/interface/TrackAssociatorRecord.h"

#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/Utilities/interface/EDGetToken.h"



//
// class decleration
//

class TrackAssociatorEDProducer : public edm::EDProducer {
public:
  explicit TrackAssociatorEDProducer(const edm::ParameterSet&);
  ~TrackAssociatorEDProducer();
  
private:
  virtual void beginJob() override {}
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override ;
  
  edm::ESHandle<TrackAssociatorBase> theAssociator;
  bool first;
  edm::InputTag label_tr;
  edm::InputTag label_tp;
  std::string associator;
  bool  theIgnoremissingtrackcollection;

  edm::EDGetTokenT<TrackingParticleCollection> TPCollectionToken_;
  edm::EDGetTokenT<edm::View<reco::Track> > trackCollectionToken_;

};

TrackAssociatorEDProducer::TrackAssociatorEDProducer(const edm::ParameterSet& pset):
  first(true),
  label_tr(pset.getParameter< edm::InputTag >("label_tr")),
  label_tp(pset.getParameter< edm::InputTag >("label_tp")),
  associator(pset.getParameter< std::string >("associator")),
  theIgnoremissingtrackcollection(pset.getUntrackedParameter<bool>("ignoremissingtrackcollection",false))
{
  produces<reco::SimToRecoCollection>();
  produces<reco::RecoToSimCollection>();

  TPCollectionToken_    = consumes<TrackingParticleCollection>(pset.getParameter< edm::InputTag >("label_tp"));
  trackCollectionToken_ = consumes<edm::View<reco::Track> >(pset.getParameter< edm::InputTag >("label_tr")); 

}


TrackAssociatorEDProducer::~TrackAssociatorEDProducer() {
 
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
TrackAssociatorEDProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;
   if(first){
     iSetup.get<TrackAssociatorRecord>().get(associator,theAssociator);
     first = false;
   }
   Handle<TrackingParticleCollection>  TPCollection ;
   //   iEvent.getByLabel(label_tp, TPCollection);
   iEvent.getByToken(TPCollectionToken_,TPCollection);

   Handle<edm::View<reco::Track> > trackCollection;
   //   bool trackAvailable = iEvent.getByLabel (label_tr, trackCollection );
   bool trackAvailable = iEvent.getByToken(trackCollectionToken_, trackCollection );

   std::auto_ptr<reco::RecoToSimCollection> rts;
   std::auto_ptr<reco::SimToRecoCollection> str;

   if (theIgnoremissingtrackcollection && !trackAvailable){
     //the track collection is not in the event and we're being told to ignore this.
     //do not output anything to the event, other wise this would be considered as inefficiency.
   } else {
     //associate tracks
     LogTrace("TrackValidator") << "Calling associateRecoToSim method" << "\n";
     reco::RecoToSimCollection recSimColl=theAssociator->associateRecoToSim(trackCollection,
									    TPCollection,
									    &iEvent, &iSetup);
     LogTrace("TrackValidator") << "Calling associateSimToReco method" << "\n";
     reco::SimToRecoCollection simRecColl=theAssociator->associateSimToReco(trackCollection,
									    TPCollection, 
									    &iEvent, &iSetup);
     
     rts.reset(new reco::RecoToSimCollection(recSimColl));
     str.reset(new reco::SimToRecoCollection(simRecColl));

     iEvent.put(rts);
     iEvent.put(str);
   }
}

// ------------ method called once each job just before starting event loop  ------------

// ------------ method called once each job just after ending the event loop  ------------
void 
TrackAssociatorEDProducer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(TrackAssociatorEDProducer);
