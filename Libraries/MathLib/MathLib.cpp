#ifndef MATHLIB_CPP
#define MATHLIB_CPP

#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <algorithm>
#include <string> 
#include <ctime>
#include <iostream>


#include "MathLib.h"

#define M_PI 3.141592653589793238462643383279502884L
#define SQR(x) ((x)*(x))
#define POW2(x) SQR(x)
#define POW3(x) ((x)*(x)*(x))
#define POW4(x) (POW2(x)*POW2(x))
#define POW7(x) (POW3(x)*POW3(x)*(x))
#define DegToRad(x) ((x)*M_PI/180)
#define RadToDeg(x) ((x)/M_PI*180)

using namespace std;

namespace MLib {

    // --------------------------------------------------------------------- //
    // Charting Functions                                                    //
    // --------------------------------------------------------------------- //

    namespace Impl {    // Internal Helper Functions

        double GetInterval( double range, int nbBins ) {
            double x = pow(10.0, floor( log10( range ) ) );
            if      ( range / x >= nbBins       ) { return x; }
            else if ( range / (x / 2) >= nbBins ) { return x / 2; }
            else if ( range / (x / 5) >= nbBins ) { return x / 5; }
            else { return x / nbBins; }
        }

        void ScaleRange(double inValMin, double inValMax, int inNbBins, double &scale, double &offset){
            double threshold = 100.0f;
            double midValue = abs(inValMax - inValMin);  //# > 0 as nonsingular is called before.
            double meanValue = (inValMax + inValMin) / 2;
            if(abs(meanValue) / midValue < threshold){offset = 0;}
            else{offset = copysign((double)(pow(10,floor(log10(abs(meanValue))))),meanValue);}
            scale = (double)pow(10, floor(log10(midValue / (double)inNbBins)));
        }
        
        bool CloseTo(double ms, double edge, double iOffset, double iStep){
            // Allow more slop when the offset is large compared to the step.
            double tol;
            if(iOffset > 0){
                double digits = log10(iOffset / iStep);
                tol = max(1e-10, pow( 10,(digits - 12) ));
                tol = min(0.4999, tol);
            }
            else{tol = (double)1e-10;}
            return abs(ms - edge) < tol;
        }

    }   // end namespace Impl

    std::vector<double> MGetTickLocations(int nbins, double valMin, double valMax) {

        int min_n_ticks = 2;
        vector<double> extended_steps = {0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 8.0, 10.0, 15.0};
        double scale; double offset;
        Impl::ScaleRange(valMin, valMax, nbins, scale, offset);
        double vMin = valMin - offset;
        double vMax = valMax - offset;
        double raw_step = (vMax - vMin) / nbins;
        vector<double> steps = {};
        for(int i=0;i<extended_steps.size();i++){steps.push_back(extended_steps[i]*scale);}

        int iStep=-1; // index of the first match in steps 
        for(int i=0;i<steps.size();i++){if(steps[i] >= raw_step){iStep=i;break;}}
        double step;double best_vmin;double delta1;double delta2;double m;
        int low;int high;
        vector<double> ticks;
        for (int i = iStep+1; i --> 0; ){
            step = steps[i];
            best_vmin = (step==0) ? 0 : (floor(vMin / step)) * step;
            delta1 = vMin - best_vmin;
            low = (step==0) ? 0 : (int)floor( delta1 / step );  // quotient - x / y
            m = ((int)floor(step)==0) ? delta1 : (double)( (int)floor(delta1) % (int)floor(step) ); // remainder - x % y
            if( Impl::CloseTo(m / step, 1, offset, step)){low += 1;}
            delta2 = vMax - best_vmin;
            high = (step==0) ? 0 : (int)floor( delta2 / step );  // quotient - x / y
            m = ((int)floor(step)==0) ? delta2 : (int)floor(delta2) % (int)floor(step) ; // remainder - x % y
            if( Impl::CloseTo(m / step, 1, offset, step)){high += 1;}
            double tickVal;
            ticks = {};
            for(int i=low;i<=high + 1;i++){
                tickVal = (double)i * step + best_vmin;
                ticks.push_back(tickVal);
            }
            //cout << "ticks: ";string dl="";for(double tk:ticks){cout<<dl<<tk;dl=",";}cout <<"\n";
            int validCount =0;
            for(int i=0;i<ticks.size();i++){
                if(ticks[i] <= vMax && ticks[i] >= vMin){validCount++;}
            }
            if(validCount >= min_n_ticks){break;}
            
        }

        // Fallback algorithm if ticks are wrong
        if ( ticks.empty() || ( ticks.front() > valMin || ticks.back() < valMax ) ) {
            double incr = Impl::GetInterval( valMax - valMin, nbins );
            if ( incr == 0 ) { return { valMin, valMax }; }
            double newMin = floor( valMin / incr ) * incr;
            double newMax = ceil( valMax / incr ) * incr;
            int nbIncr = (int) ( ( newMax - newMin ) / incr + 1 );
            ticks = { newMin };
            for ( int i = 1; i < nbIncr; i++ ) {
                if ( newMin + i*incr >= newMax ) { 
                    if ( ticks.back() < valMax ) { ticks.push_back( newMin + i*incr ); }
                    break; 
                }
                ticks.push_back( newMin + i*incr );
            }
        }

        return std::move(ticks);
    }

    std::vector<double> MSplitRange(int nbins, double valMin, double valMax) {
        std::vector<double> ticks = {};
        double incr = ( valMax - valMin ) / nbins;
        for ( int i = 0; i <= nbins; i++ ) {
            ticks.push_back( valMin + i* incr );
        }
        return std::move(ticks);
    }

    // --------------------------------------------------------------------- //
    // Geometry Functions                                                    //
    // --------------------------------------------------------------------- //

    bool MLineIntersect( const glm::vec2 &inP0, const glm::vec2 &inP1, const glm::vec2 &inP2, const glm::vec2 &inP3, glm::vec2 &pOut ) {
		// Calculates the intersection of 2 lines.  Returns true if intersection beyond the segments.
		float a1 = inP1.y - inP0.y; 
		float b1 = inP0.x - inP1.x; 
		float c1 = a1*(inP0.x) + b1*(inP0.y); 
		float a2 = inP3.y - inP2.y;
		float b2 = inP2.x - inP3.x; 
		float c2 = a2*(inP2.x)+ b2*(inP2.y); 
		float determinant = a1*b2 - a2*b1; 
                
		if (determinant<.00001 && determinant>-.00001){return false;}  // parallel
		else{
			pOut.x = (b2*c1 - b1*c2)/determinant;;
			pOut.y = (a1*c2 - a2*c1)/determinant;;
			return true;
		} 
	}

    bool MLineSegIntersect( const glm::vec2 &inP0, const glm::vec2 &inP1, const glm::vec2 &inP2, const glm::vec2 &inP3, glm::vec2 &pOut ) {
        // Calculates the intersection of 2 line segments.  Returns false if intersection beyond the segments.
        float s1_x, s1_y, s2_x, s2_y, s, t;
        s1_x = inP1.x - inP0.x;  s1_y = inP1.y - inP0.y;
        s2_x = inP3.x - inP2.x;  s2_y = inP3.y - inP2.y;
        s = (-s1_y * (inP0.x - inP2.x) + s1_x * (inP0.y - inP2.y)) / (-s2_x * s1_y + s1_x * s2_y);
        t = ( s2_x * (inP0.y - inP2.y) - s2_y * (inP0.x - inP2.x)) / (-s2_x * s1_y + s1_x * s2_y);
        if (s >= 0 && s <= 1 && t >= 0 && t <= 1){ // Collision within line segments
            pOut.x = inP0.x + (t * s1_x);
            pOut.y = inP0.y + (t * s1_y);
            return true;
        }
        return false; // No collision
    }


    // --------------------------------------------------------------------- //
    // Blending Function: Math for Rigging Type Operations                   //
    // --------------------------------------------------------------------- //

    float MSharpBlend(float inVal){
        if(inVal==0){return 0;}
        return (float)(asin(inVal) / (M_PI * 0.5));
    }

    float MSigmoid(float inX){
        return inX / (1 + abs(inX));
    }
    float MSharpRamp(float inX){ // slow then sharp ramp up from 0-1 - like an asymtoptic curve between 0-1
        float nVal = (inX -1) * 6;
        return 1 - (nVal / (1 + abs(nVal))) / -0.857143f;
    }

    // --------------------------------------------------------------------- //
    // Date Time Functions                                                   //
    // --------------------------------------------------------------------- //
    std::string CurrentDate(std::string inSep){
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&now);
        string tFormat = "%m" + inSep + "%d" + inSep + "%Y";
        strftime(buf, sizeof(buf), tFormat.c_str(), &tstruct);
        return buf;
    }

    std::string CurrentDateTime(std::string inSep){ // 24 hour time
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&now);
        string tFormat = "%m" + inSep + "%d" + inSep + "%Y %R %p";
        strftime(buf, sizeof(buf), tFormat.c_str(), &tstruct);
        return buf;
    }


} // end namespace MLib

#endif /* MATHLIB_CPP */
