/*-------------------------------------------------------------------------
    tdchip.hpp
-------------------------------------------------------------------------*/

#ifndef _TD_CHIP_HPP_
    #define _TD_CHIP_HPP_

#include <math.h>
#include <c_lib.hpp>
//#include "sequence.hpp"

class IPoint {
public:
	IPoint(uint32_t nx,uint32_t ny):ix(nx),iy(ny){
	}
	uint32_t x() const { return ix; }
	uint32_t y() const { return iy; }
protected:
	uint32_t ix;
	uint32_t iy;
};

class TDChip {
public:
    enum ChipType { CheckChip, ControlChip, GraundChip };

    TDChip( long x1, long y1, long x2, long y2, uint8_t ball, uint8_t reg, const char * quality, ChipType chipType ) :
        mX1( 0 ), mY1( 0 ), mX2( 0 ), mY2( 0 ),
        X1( x1 ), Y1( y1 ), X2( x2 ), Y2( y2 ), Ball( ball ), fRegionFiltrCloudy( reg )
    {
      fChipType[0] = chipType;
      fChipType[1] = CheckChip;

      memset( fStringQuality, 0, 255 );
      int l = strlen( quality );
      strncpy( fStringQuality, quality, l );
      for( int i=0; i < 2; i++ ){
           dX[i] = dY[i] = .0;
           std_dX[i] = std_dY[i] = .0;
           fNormStatSign[i] = .0;
           dhX[i] = dhY[i] = .0;
           fExtremAlgOptimum[i] = .0;
           Ver[i] = avgLand[i] = avgSea[i] = dyspLand[i] = dyspSea[i] = psyCryt[i] = .0;
           numberDegreeofFreedom[i] = 0;
           dXAbs[i] = .0; dYAbs[i] = .0;
           fAbsExtremAlgOptimum[i] = .0;
           fAbsNormStatSign[i] = .0;
      }

      PercCloudy = .0;
      fAngleCenterSanny = .0;
      fChannels_forCalcShifts[0] = true;
      fChannels_forCalcShifts[1] = false;
    }

    TDChip( long x1=0, long y1=0, long x2=0, long y2=0, uint8_t ball=10, uint8_t reg=0, ChipType chipType = CheckChip ) :
        mX1( 0 ), mY1( 0 ), mX2( 0 ), mY2( 0 ),
        X1( x1 ), Y1( y1 ), X2( x2 ), Y2( y2 ), Ball( ball ), fRegionFiltrCloudy( reg )
    {
      fChipType[0] = chipType;
      fChipType[1] = CheckChip;

      const char * s;
      s = "резюме по чипу";
      memset( fStringQuality, 0, 255 );
      int l = strlen( s );
      strncpy( fStringQuality, s, l );
      for( int i=0; i < 2; i++ ){
           dX[i] = dY[i] = .0;
           std_dX[i] = std_dY[i] = .0;
           fNormStatSign[i] = .0;
           dhX[i] = dhY[i] = .0;
           fExtremAlgOptimum[i] = .0;
           Ver[i] = avgLand[i] = avgSea[i] = dyspLand[i] = dyspSea[i] = psyCryt[i] = .0;
           numberDegreeofFreedom[i] = 0;
           dXAbs[i] = .0; dYAbs[i] = .0;
           fAbsExtremAlgOptimum[i] = .0;
           fAbsNormStatSign[i] = .0;
      }

      PercCloudy = .0;
      fAngleCenterSanny = .0;
      fChannels_forCalcShifts[0] = true;
      fChannels_forCalcShifts[1] = false;
    }

    TDChip( const TDChip &chip ) :
        mX1( chip.mX1 ), mY1( chip.mY1 ), mX2( chip.mX2 ), mY2( chip.mY2 ),
        X1( chip.X1 ), Y1( chip.Y1 ), X2( chip.X2 ), Y2( chip.Y2 ), Ball( chip.Ball ),
        fAngleCenterSanny( chip.fAngleCenterSanny ), PercCloudy( chip.PercCloudy ), fRegionFiltrCloudy( chip.fRegionFiltrCloudy )
    {
      memset( fStringQuality, 0, 255 );
      int l = strlen( chip.fStringQuality );
      strncpy( fStringQuality, chip.fStringQuality, l );
      for( int i=0; i < 2; i++ ){
           fChipType[i] = chip.fChipType[i];
           dX[i] = chip.dX[i]; dY[i] = chip.dY[i];
           std_dX[i] = chip.std_dX[i]; std_dY[i] = chip.std_dY[i];
           dhX[i] = chip.dhX[i]; dhY[i] = chip.dhY[i];
           fExtremAlgOptimum[i] = chip.fExtremAlgOptimum[i];
           fChannels_forCalcShifts[i] = chip.fChannels_forCalcShifts[i];
           fNormStatSign[i] = chip.fNormStatSign[i];
           avgLand[i] = chip.avgLand[i]; avgSea[i] = chip.avgSea[i]; dyspLand[i] = chip.dyspLand[i]; dyspSea[i] = chip.dyspSea[i];
           psyCryt[i] = chip.psyCryt[i]; Ver[i] = chip.Ver[i]; numberDegreeofFreedom[i] = chip.numberDegreeofFreedom[i];
           dXAbs[i] = chip.dXAbs[i]; dYAbs[i] = chip.dYAbs[i];
           fAbsExtremAlgOptimum[i] = chip.fAbsExtremAlgOptimum[i];
           fAbsNormStatSign[i] = chip.fAbsNormStatSign[i];
      }
    }

    void inicil( uint8_t i, const TDChip * chip )
    {
      mX1 = chip->mX1; mY1 = chip->mY1; mX2 = chip->mX2; mY2 = chip->mY2;
      X1 = chip->X1; Y1 = chip->Y1; X2 = chip->X2; Y2 = chip->Y2; Ball = chip->Ball;
      fAngleCenterSanny = chip->fAngleCenterSanny; PercCloudy = chip->PercCloudy; fRegionFiltrCloudy = chip->fRegionFiltrCloudy;
      memset( fStringQuality, 0, 255 );
      int l = strlen( chip->fStringQuality );
      strncpy( fStringQuality, chip->fStringQuality, l );
      fChipType[i] = chip->fChipType[i];
      dX[i] = chip->dX[i]; dY[i] = chip->dY[i];
      std_dX[i] = chip->std_dX[i]; std_dY[i] = chip->std_dY[i];
      dhX[i] = chip->dhX[i]; dhY[i] = chip->dhY[i];
      fExtremAlgOptimum[i] = chip->fExtremAlgOptimum[i];
      fChannels_forCalcShifts[i] = chip->fChannels_forCalcShifts[i];
      fNormStatSign[i] = chip->fNormStatSign[i];
      avgLand[i] = chip->avgLand[i]; avgSea[i] = chip->avgSea[i]; dyspLand[i] = chip->dyspLand[i]; dyspSea[i] = chip->dyspSea[i];
      psyCryt[i] = chip->psyCryt[i]; Ver[i] = chip->Ver[i]; numberDegreeofFreedom[i] = chip->numberDegreeofFreedom[i];
      dXAbs[i] = chip->dXAbs[i]; dYAbs[i] = chip->dYAbs[i];
      fAbsExtremAlgOptimum[i] = chip->fAbsExtremAlgOptimum[i];
      fAbsNormStatSign[i] = chip->fAbsNormStatSign[i];
    }

    void nullChip()
    {
        if( fChannels_forCalcShifts[1] ){
            for( int i=0; i < 2; i++ ){
                 dX[i] = dY[i] = .0; std_dX[i] = std_dY[i] = .0;
                 fNormStatSign[i] = .0; fExtremAlgOptimum[i] = .0;
                 Ver[i] = avgLand[i] = avgSea[i] = dyspLand[i] = dyspSea[i] = psyCryt[i] = .0;
                 numberDegreeofFreedom[i] = 0; col_sea[i] = 0; col_land[i] = 0;
                 dXAbs[i] = .0; dYAbs[i] = .0; fAbsExtremAlgOptimum[i] = .0; fAbsNormStatSign[i] = .0;
            }
        }
        else {
            dX[0] = dY[0] = .0; std_dX[0] = std_dY[0] = .0;
            fNormStatSign[0] = .0; fExtremAlgOptimum[0] = .0;
            Ver[0] = avgLand[0] = avgSea[0] = dyspLand[0] = dyspSea[0] = psyCryt[0] = .0;
            numberDegreeofFreedom[0] = 0; col_sea[0] = 0; col_land[0] = 0;
            dXAbs[0] = .0; dYAbs[0] = .0; fAbsExtremAlgOptimum[0] = .0; fAbsNormStatSign[0] = .0;
        }
        PercCloudy = .0;
    }

    const IPoint ChipSize() const { return IPoint( abs(X2-X1+1), abs(Y2-Y1+1) ); }

    void autoDisplacements( uint8_t i, double dx, double dy ) {  if( fChannels_forCalcShifts[i] ){ dX[i] = dx; std_dX[i] = dx; dY[i] = dy; std_dY[i] = dy; } }

    void autoDisplacementsAbs( uint8_t i, double maxAlg, double dx, double dy ) {  if( fChannels_forCalcShifts[i] ){ fAbsExtremAlgOptimum[i] = maxAlg; dXAbs[i] = dx; dYAbs[i] = dy; } }

    void handDisplacements( uint8_t i, double dx, double dy ) {  dhX[i] = dx; dhY[i] = dy; }

    double Delta( uint8_t i ) const { return sqrt( pow( dX[i] - dhX[i], 2. ) + pow( dY[i] - dhY[i], 2. ) ); }

    double Modulus( bool flag_std, uint8_t i ) const { if( !flag_std ) return sqrt( pow( dX[i], 2. ) + pow( dY[i], 2. ) ); else return sqrt( pow( std_dX[i], 2. ) + pow( std_dY[i], 2. ) ); }

    void setDisplacems( uint8_t i, double dx, double dy )
         {
            if( fChannels_forCalcShifts[i] ){
                dXAbs[i] = std_dX[i]; std_dX[i] += dx; dX[i] = std_dX[i];
                dYAbs[i] = std_dY[i]; std_dY[i] += dy; dY[i] = std_dY[i];
            }
         }

    void unsetDisplacems( uint8_t i )
         {
            if( fChannels_forCalcShifts[i] ){
                std_dX[i] = dXAbs[i]; dX[i] = dXAbs[i];
                std_dY[i] = dYAbs[i]; dY[i] = dYAbs[i];
            }
         }

    long X1, Y1;
    long X2, Y2;

    long mX1, mY1, mX2, mY2;

    double dX[2], dY[2], std_dX[2], std_dY[2];

    double fExtremAlgOptimum[2];

    double dhX[2], dhY[2], std_dhX[2], std_dhY[2];

    double PercCloudy;

    double fNormStatSign[2];

    double avgLand[2], avgSea[2], dyspLand[2], dyspSea[2], psyCryt[2], Ver[2];
    long numberDegreeofFreedom[2];
    long col_sea[2], col_land[2];

    ChipType fChipType[2];

    uint8_t Ball, fRegionFiltrCloudy;

    bool fChannels_forCalcShifts[2]; // По каким каналам идёт расчёт смещений.

    double fAngleCenterSanny;

    double dXAbs[2], dYAbs[2];
    double fAbsExtremAlgOptimum[2];
    double fAbsNormStatSign[2];

    char fStringQuality[255];
};

//class TCLCursor : public TSimpleListCursor<TDChip>;
//class TCLCursor;
#define TCLCursor TSimpleListCursor<TDChip>

class TChipList : public TSimpleList<TDChip> {
public:
    TChipList() : TSimpleList<TDChip> () {}
    ~TChipList() {}
//	TCLCursor createTCLCursor(){
//		return TCLCursor(this);
//		return 0;
//	}
};

//class TCLCursor: public TSimpleListCursor<TDChip> {
//public:
//    ~TCLCursor() {}
//protected:
//	friend class TChipList;

//    TCLCursor(TChipList* cl) : TSimpleListCursor<TDChip> (cl) {}
//};

#endif
