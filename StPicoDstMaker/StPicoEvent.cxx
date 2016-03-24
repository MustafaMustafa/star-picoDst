#include <algorithm>
#include <limits>
#include "StPicoEvent.h"
#include "StPicoConstants.h"
#include "StEvent/StEventTypes.h"
#include "St_base/StTree.h"
#include "StEventUtilities/StuRefMult.hh"
#include "TVector2.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StMuDSTMaker/COMMON/StMuEvent.h"
#include "StMuDSTMaker/COMMON/StMuPrimaryVertex.h"
#include "StMuDSTMaker/COMMON/StMuMtdHeader.h"
#include "StEvent/StBTofHeader.h"
#include "St_base/StMessMgr.h"
#include "StPicoUtilities.h"

ClassImp(StPicoEvent)

StPicoEvent::StPicoEvent():
 mRunId(0), mEventId(0), mFillId(0), mBField(0),
 mPrimaryVertex{-999.,-999.,-999.}, mPrimaryVertexError{-999.,-999.,-999},
 mRanking(-999), mNBEMCMatch(0), mNBTOFMatch(0),
 mTriggerIds{},
 mRefMultFtpcEast(0), mRefMultFtpcWest(0),
 mRefMultNeg(0), mRefMultPos(0),
 mRefMult2NegEast(0), mRefMult2PosEast(0), mRefMult2NegWest(0), mRefMult2PosWest(0),
 mRefMultHalfNegEast(0), mRefMultHalfPosEast(0), mRefMultHalfNegWest(0), mRefMultHalfPosWest(0),
 mGRefMult(0), mNumberOfGlobalTracks(0), mbTofTrayMultiplicity(0), mNHitsHFT{},
 mNVpdHitsEast(0), mNVpdHitsWest(0), mNT0(0), mVzVpd(std::numeric_limits<short>::max()),
 mZDCx(0), mBBCx(0), mBackgroundRate(0), mBbcBlueBackgroundRate(0), mBbcYellowBackgroundRate(0),
 mBbcEastRate(0), mBbcWestRate(0), mZdcEastRate(0), mZdcWestRate(0),
 mVpd{}, mZdcSumAdcEast(0), mZdcSumAdcWest(0),
 mZdcSmdEastHorizontal{}, mZdcSmdEastVertical{}, mZdcSmdWestHorizontal{}, mZdcSmdWestVertical{},
 mBbcAdcEast{}, mBbcAdcWest{},
 mSpaceCharge(0),
 mHT_Th{}, mJP_Th{}
{}

StPicoEvent::StPicoEvent(const StMuDst& muDst) : StPicoEvent()
{
  StMuEvent* ev = muDst.event() ;

  mRunId = ev->runNumber();
  mEventId = ev->eventNumber();
  mFillId = ev->runInfo().beamFillNumber(blue);
  mBField=ev->magneticField();

  mPrimaryVertex = ev->primaryVertexPosition();
  mPrimaryVertexError = ev->primaryVertexErrors();
  if( mPrimaryVertex.x()==mPrimaryVertex.y() && mPrimaryVertex.y()==mPrimaryVertex.z() )
  {
    mPrimaryVertex.set(-999.,-999.,-999.);
    mPrimaryVertexError.set(0.,0.,0);
  }

  if(StMuPrimaryVertex* pv = muDst.primaryVertex())
  {
    mRanking = pv->ranking();
    mNBEMCMatch = pv->nBEMCMatch();
    mNBTOFMatch = pv->nBTOFMatch();
  }

  mTriggerIds = ev->triggerIdCollection().nominal().triggerIds();

  mRefMultFtpcEast = (UShort_t)(ev->refMultFtpcEast());
  mRefMultFtpcWest = (UShort_t)(ev->refMultFtpcWest());
  mRefMultNeg = (UShort_t)(ev->refMultNeg());
  mRefMultPos = (UShort_t)(ev->refMultPos());
  mRefMult2NegEast = (UShort_t)StPicoUtilities::refMult2(0, 0, muDst);
  mRefMult2PosEast = (UShort_t)StPicoUtilities::refMult2(1, 0, muDst);
  mRefMult2NegWest = (UShort_t)StPicoUtilities::refMult2(0, 1, muDst);
  mRefMult2PosWest = (UShort_t)StPicoUtilities::refMult2(1, 1, muDst);
  mRefMultHalfNegEast = (UShort_t)StPicoUtilities::refMultHalf(0, 0, muDst);
  mRefMultHalfPosEast = (UShort_t)StPicoUtilities::refMultHalf(1, 0, muDst);
  mRefMultHalfNegWest = (UShort_t)StPicoUtilities::refMultHalf(0, 1, muDst);
  mRefMultHalfPosWest = (UShort_t)StPicoUtilities::refMultHalf(1, 1, muDst);

  mGRefMult = (UShort_t)ev->grefmult();
  mNumberOfGlobalTracks = muDst.numberOfGlobalTracks();
  mbTofTrayMultiplicity = ev->btofTrayMultiplicity() ;
  mNHitsHFT[0] = (UShort_t)ev->numberOfPxlInnerHits();
  mNHitsHFT[1] = (UShort_t)ev->numberOfPxlOuterHits();
  mNHitsHFT[2] = (UShort_t)ev->numberOfIstHits();
  mNHitsHFT[3] = (UShort_t)ev->numberOfSsdHits();

  if(StBTofHeader* header = muDst.btofHeader())
  {
    mNVpdHitsEast = (UChar_t)(header->numberOfVpdHits(east));
    mNVpdHitsWest = (UChar_t)(header->numberOfVpdHits(west));
    mNT0 = (UShort_t)(header->nTzero());
    mVzVpd = (fabs(header->vpdVz()*100.)>Pico::SHORTMAX) ? Pico::SHORTMAX : (UShort_t)(TMath::Nint(header->vpdVz()*100.));
  }

  //Nov.10, 2008, Na
  StZdcTriggerDetector& ZDC = ev->zdcTriggerDetector();
  mZdcSumAdcEast = (UShort_t)ZDC.adcSum(east);
  mZdcSumAdcWest = (UShort_t)ZDC.adcSum(west);
  for (int strip=1;strip<9;++strip)
  {
    if (ZDC.zdcSmd(east,1,strip))
      mZdcSmdEastHorizontal[strip-1] = (UShort_t)ZDC.zdcSmd(east,1,strip);
    if (ZDC.zdcSmd(east,0,strip))
      mZdcSmdEastVertical[strip-1] = (UShort_t)ZDC.zdcSmd(east,0,strip);
    if (ZDC.zdcSmd(west,1,strip))
      mZdcSmdWestHorizontal[strip-1] = (UShort_t)ZDC.zdcSmd(west,1,strip);
    if (ZDC.zdcSmd(west,0,strip))
      mZdcSmdWestVertical[strip-1] = (UShort_t)ZDC.zdcSmd(west,0,strip);
  }

  StVpdTriggerDetector& VPD = ev->vpdTriggerDetector();

  for(int i=0; i<16; ++i)
  {
    //event.VPD[i]= 1.0*theVPD.adc(i);
    if(i>=0 && i<8) {
      mVpd[i]=(UShort_t)VPD.ADC(east,8-i);
      mVpd[i+8]=(UShort_t)VPD.TDC(east,8-i);
      mVpd[i+32]=(UShort_t)VPD.ADC(west,8-i);
      mVpd[i+40]=(UShort_t)VPD.TDC(west,8-i);
    }
    if(i>=8 && i<16) {
      mVpd[i+8]=(UShort_t)VPD.ADC(east,32-(i+8));
      mVpd[i+16]=(UShort_t)VPD.TDC(east,32-(i+8));
      mVpd[i+40]=(UShort_t)VPD.ADC(west,32-(i+8));
      mVpd[i+48]=(UShort_t)VPD.TDC(west,32-(i+8));
      //cout<<"VPD-------  "<<VPD.ADC(east,32-(i+8))<<endl;
    }
  }

  mZDCx = (UInt_t)ev->runInfo().zdcCoincidenceRate();
  mBBCx = (UInt_t)ev->runInfo().bbcCoincidenceRate();
  mBackgroundRate = ev->runInfo().backgroundRate();
  mBbcBlueBackgroundRate = ev->runInfo().bbcBlueBackgroundRate();
  mBbcYellowBackgroundRate = ev->runInfo().bbcYellowBackgroundRate();
  mBbcEastRate = ev->runInfo().bbcEastRate();
  mBbcWestRate = ev->runInfo().bbcWestRate();
  mZdcEastRate = ev->runInfo().zdcEastRate();
  mZdcWestRate = ev->runInfo().zdcWestRate();


  // BBC ADC (Hiroshi)
  StBbcTriggerDetector bbc = ev->bbcTriggerDetector() ;
  for(UInt_t i=0;i<bbc.numberOfPMTs();++i)
  {
    const UInt_t eastWest = (i<24) ? 0 : 1 ; // East:0-23, West:24-47
    const UInt_t pmtId    = i%24 ;           // pmtId:0-23

    if( eastWest == 0 ) mBbcAdcEast[pmtId] = bbc.adc(i) ;
    else                mBbcAdcWest[pmtId] = bbc.adc(i) ;
  }

  mSpaceCharge = ev->runInfo().spaceCharge();
}

StPicoEvent::~StPicoEvent()
{ }

int StPicoEvent::year() const
{
  return mRunId/1000000 - 1 + 2000;
}

int StPicoEvent::day() const
{
  return (mRunId%1000000)/1000;
}

bool StPicoEvent::isTrigger(unsigned int id) const
{
  return std::find(mTriggerIds.begin(), mTriggerIds.end(), id) != mTriggerIds.end();
}
