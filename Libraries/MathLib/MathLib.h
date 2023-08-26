#ifndef MATHLIB_H
#define MATHLIB_H

// *** NOTE *** 
// To include the Math Library, use:    #include "MathLib.hpp" 

#include <vector>
#include <glm/glm.hpp>

namespace MLib {

    // --------------------------------------------------------------------- //
    // Declarations                                                          //
    // --------------------------------------------------------------------- //

    // Statistical Functions ----------------------------------------------- //

    template<typename T> void MLinearReg( const std::vector<T> & yData, const std::vector<T> & xData, double & b0, double & b1 );


    // Charting Functions -------------------------------------------------- //

    namespace Impl {            // Internal hepler functions for charting
        void ScaleRange(double inValMin, double inValMax, int inNbBins, double &scale, double &offset);
        bool CloseTo(double ms, double edge, double iOffset, double iStep);
    }

    std::vector<double> MGetTickLocations(int nbins, double valMin, double valMax);
    std::vector<double> MSplitRange(int nbins, double valMin, double valMax);

    // Geometry Functions -------------------------------------------------- //

    bool MLineIntersect( const glm::vec2 &inP0, const glm::vec2 &inP1, const glm::vec2 &inP2, const glm::vec2 &inP3, glm::vec2 &pOut );
    bool MLineSegIntersect( const glm::vec2 &inP0, const glm::vec2 &inP1, const glm::vec2 &inP2, const glm::vec2 &inP3, glm::vec2 &pOut );
    

    // TO DO: Move bezier functions into here ------------------------------- //

    
    // Sort Permuation Functions -------------------------------------------- //
    template <typename T, typename Compare> std::vector<std::size_t> MSortPermutation( const std::vector<T>& vec, Compare& compare);
    template <typename T> std::vector<T> MApplyPermutation( const std::vector<T>& vec, const std::vector<std::size_t>& p);
    template <typename T> void MApplyPermutationInPlace(std::vector<T>& vec,const std::vector<std::size_t>& p);

    // String to Numeric Conversion Functions ------------------------------- //
    
    template <typename outT>
    static bool MConvertCharsToINT( outT * outVal, const char * c, size_t size );
    template <typename outT>
    static void MConvertCharsToINT( outT * outVal, const char * c, size_t size, const outT & outNULL );
    template <typename outT>
    static bool MConvertCharsToFLT( outT * outVal, const char * c, size_t size );
    template <typename outT>
    static void MConvertCharsToFLT( outT * outVal, const char * c, size_t size, const outT & outNULL );

    // Logging Functions

    template<typename T> void MLogVector( const std::vector<T> & inVector, bool addEndl = true );

    // --------------------------------------------------------------------- //
    // Blending Function: Math for Rigging Type Operations                   //
    // --------------------------------------------------------------------- //

    float MSharpBlend(float inVal);
    float MSigmoid(float inVal);
    float MSharpRamp(float inVal);

    // --------------------------------------------------------------------- //
    // Date Time Functions                                                   //
    // --------------------------------------------------------------------- //

    std::string CurrentDate(std::string inSep);
    std::string CurrentDateTime(std::string inSep);

} // end namespace MLib

#endif /* MATHLIB_H */
