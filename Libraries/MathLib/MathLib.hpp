#ifndef MATLIB_HPP
#define MATLIB_HPP

#include "MathLib.h"

#include <iostream>
#include <numeric>
#include <charconv>     // for std::from_chars
#include "fast_float.h"

namespace MLib {

    // --------------------------------------------------------------------- //
    // Statistical Functions                                                 //
    // --------------------------------------------------------------------- //

    template <typename T> 
    void MLinearReg( const std::vector<T> & yData, const std::vector<T> & xData, double & b0, double & b1 ) {
        // Simple Linear Regression estimated using Ordinary Least Squares (OLS).

        if ( xData.size() != yData.size() ) { 
            printf("X and Y data are different sizes! (X size: %lld, Y size: %lld)\n", xData.size(), yData.size() ); 
            return;
        }

        size_t N = xData.size();
        T * X = ( T* ) xData.data();
        T * Y = ( T* ) yData.data();

        T xSum = 0, ySum = 0, xxSum = 0, xySum = 0;

        for ( size_t i = 0; i < N; i++ ) {
            xSum += X[i];
            ySum += Y[i];
            xxSum += X[i]*X[i];
            xySum += X[i]*Y[i];
        }

        b1 = (double) ( N*xySum - xSum*ySum ) / (double) ( N*xxSum - xSum*xSum );
        b0 =  ( (double) ySum - b1 * (double) xSum ) / (double) N;

    }

    // --------------------------------------------------------------------- //
    // Sort Permutations Functions                                           //
    // --------------------------------------------------------------------- //

    template <typename T, typename Compare>
    std::vector<std::size_t> MSortPermutation( const std::vector<T>& vec, Compare& compare) {
        std::vector<std::size_t> p(vec.size());
        std::iota(p.begin(), p.end(), 0);
        std::sort(p.begin(), p.end(),
            [&](std::size_t i, std::size_t j){ return compare(vec[i], vec[j]); });
        return p;
    }

    template <typename T>
    std::vector<T> MApplyPermutation( const std::vector<T>& vec, const std::vector<std::size_t>& p) {
        std::vector<T> sorted_vec(vec.size());
        std::transform(p.begin(), p.end(), sorted_vec.begin(),
            [&](std::size_t i){ return vec[i]; });
        return sorted_vec;
    }

    template <typename T>
    void MApplyPermutationInPlace(std::vector<T>& vec,const std::vector<std::size_t>& p){
        std::vector<bool> done(vec.size());
        for (std::size_t i = 0; i < vec.size(); ++i){
            if (done[i]){continue;}
            done[i] = true;
            std::size_t prev_j = i;
            std::size_t j = p[i];
            while (i != j){
                std::swap(vec[prev_j], vec[j]);
                done[j] = true;
                prev_j = j;
                j = p[j];
            }
        }
    }

    // --------------------------------------------------------------------- //
    // String to Number Conversion Functions                                 //
    // --------------------------------------------------------------------- //

    // NOTE: GCC from:chars is only available for integer types
    //       See: https://stackoverflow.com/a/63964295
    //       This implementation DOES NOT handle scientific notation.
    
    template <typename outT>
    bool MConvertCharsToINT( outT * outVal, const char * c, size_t size ) {
        outT result;
        auto [ptr, ec] { std::from_chars( c, c + size, result) };
        if ( ec == std::errc() ) {
            if ( ptr-c == size ) {
                std::memcpy( outVal, &result, sizeof( outT ) );    // Successfully parsed!
                return true;
            }
        }
        return false;
    };

    template <typename outT>
    void MConvertCharsToINT( outT * outVal, const char * c, size_t size, const outT & outNULL ) {
        outT result;
        auto [ptr, ec] { std::from_chars( c, c + size, result) };
        if ( ec == std::errc() ) {
            if ( ptr-c != size ) {
                std::memcpy( outVal, &outNULL, sizeof( outT ) );   // Invalid number
            }
            else {
                std::memcpy( outVal, &result, sizeof( outT ) );    // Successfully parsed!
            }
        }
        else {
            std::memcpy( outVal, &outNULL, sizeof( outT ) );       // Invalid number
        } 
    };

    // NOTE: Using the fast_float implementation of from_chars
    //      See: https://github.com/fastfloat/fast_float

    template <typename outT>
    bool MConvertCharsToFLT( outT * outVal, const char * c, size_t size ) {
        outT result;
        auto [ptr, ec] { fast_float::from_chars( c, c + size, result) };
        if ( ec == std::errc() ) {
            if ( ptr-c == size ) {
                memcpy( outVal, &result, sizeof( outT ) );    // Successfully parsed!
                return true;
            }
        }
        return false;
    };

    template <typename outT>
    void MConvertCharsToFLT( outT * outVal, const char * c, size_t size, const outT & outNULL ) {
        outT result;
        auto [ptr, ec] { fast_float::from_chars( c, c + size, result) };
        if ( ec == std::errc() ) {
            if ( ptr-c != size ) {
                memcpy( outVal, &outNULL, sizeof( outT ) );   // Invalid number
            }
            else {
                memcpy( outVal, &result, sizeof( outT ) );    // Successfully parsed!
            }
        }
        else {
            memcpy( outVal, &outNULL, sizeof( outT ) );       // Invalid number
        }
    };

    // --------------------------------------------------------------------- //
    // Logging Functions                                                     //
    // --------------------------------------------------------------------- //

    template<typename T> void MLogVector( const std::vector<T> & inVector, bool addEndl ) {
        std::string dlm = "";
        std::cout << "{ ";
        for ( auto v : inVector ) {
            std::cout << dlm << v;
            dlm = ", ";
        }
        std::cout << "}";
        if ( addEndl ) { std::cout << std::endl; }
    }

} // end namespace MLib

#endif /* MATLIB_HPP */
