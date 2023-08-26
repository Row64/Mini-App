#ifndef CORECHARTTYPES_CPP
#define CORECHARTTYPES_CPP

#include "CoreChartTypes.h"
#include "ByteStream.h"

startBTO( FillOptions )
    ABR( Int32, Color )
    ABR( Float, Alpha )
endBTO

startBFROM( FillOptions )
    GBR( Int32, Color )
    GBR( Float, Alpha )
endBFROM

startBTO( LineOptions )
    ABR( Int32, Color )
    ABR( Float, Size )
    ABR( Float, Alpha )
endBTO

startBFROM( LineOptions )
    GBR( Int32, Color )
    GBR( Float, Size )
    GBR( Float, Alpha )
endBFROM

startBTO( FontOptions )
    ABR( Int32, Font )
    ABR( Float, Size )
    ABR( Int32, Color )
    ABR( Bool, Bold )
    ABR( Bool, Italic )
    ABR( Bool, Underline )
    ABR( Float, Alpha )
endBTO

startBFROM( FontOptions )
    GBR( Int32, Font )
    GBR( Float, Size )
    GBR( Int32, Color )
    GBR( Bool, Bold )
    GBR( Bool, Italic )
    GBR( Bool, Underline )
    GBR( Float, Alpha )
endBFROM

startBTO( LayoutOptions )
    ABR( Float, RPadding )
    ABR( Float, LPadding )
    ABR( Float, TPadding )
    ABR( Float, BPadding )
    ABR( Float, Width )
    ABR( Float, Height )
    ABR( Bool, HasChartBorder )
    ABRS( ChartBorder )
    ABR( Bool, HasChartFill )
    ABRS( ChartFill )
    ABR( Bool, HasBkgdFill )
    ABRS( BkgdFill )
endBTO

startBFROM( LayoutOptions )
    GBR( Float, RPadding )
    GBR( Float, LPadding )
    GBR( Float, TPadding )
    GBR( Float, BPadding )
    GBR( Float, Width )
    GBR( Float, Height )
    GBR( Bool, HasChartBorder )
    GBRS( ChartBorder )
    GBR( Bool, HasChartFill )
    GBRS( ChartFill )
    GBR( Bool, HasBkgdFill )
    GBRS( BkgdFill )
endBFROM

startBTO( ChartBase )
    ABR( String, Type )
    ABR( String, Title )
    ABR( Bool, ShowTitle )
    ABRS( TitleFont )
    ABR( FloatVector, TitlePos )
    ABRS( Layout )
endBTO

startBFROM( ChartBase )
    GBR( String, Type )
    GBR( String, Title )
    GBR( Bool, ShowTitle )
    GBRS( TitleFont )
    GBR( FloatVector, TitlePos )
    GBRS( Layout )
endBFROM

startBTO( ExampleData )
    ABR( FloatVector, XValues )
    ABR( FloatVector, YValues )
endBTO

startBFROM( ExampleData )
    GBR( FloatVector, XValues )
    GBR( FloatVector, YValues )
endBFROM

startBTO( ExampleFormat )
    ABR( Int32, Color )
endBTO

startBFROM( ExampleFormat )
    GBR( Int32, Color )
endBFROM


#endif /* CORECHARTTYPES_CPP */
