#ifndef CLO_BASE_H
#define CLO_BASE_H

#include "CoreChartTypes.h"

namespace Blocks {

    // Mini version of ChartSConfig to line up with Row64 Studio
    class ChartSConfig {
    public:
        std::string Name = "";       // Immutable identifier name for the tab (aka sheet)
        std::string Category = "";   // Viewer, Chart, Canvas
        std::string Type = "";       // Viewer subcategories: Geo Analysis, Location, Region, 3D Viewer; Chart subcategories: Pie, Bar, etc.
    };

} // end namespace Blocks

class CLO_Base {

public:
    
    CLO_Base( Blocks::ChartSConfig * inConfig, int inPaneIndex );
    virtual ~CLO_Base();

    Blocks::ChartSConfig *      Config = nullptr;

    std::string                 SheetName = "";         // Sheet name from Config->Name
    int                         PaneIndex = 0;
    std::string                 CLOType = "CLO_Base";

    ChartBase                   Details;                // The "Details" element from the .chart file JSON Data
                                                        // Note: The "Data" and "Format" elements are custom to each 
                                                        // chart type and are defined in the derived classes.

    bool                        UpdateBB = false;       // Indicates whether the Layout has been updated

    std::string                 TitleText = "";         // Title text to display (since Details.Title can be a sheet link)
    bool                        TitleUpdate = true;     // Indicates whether the Title has been updated. Set by CLO_* and unset by CDR_* classes.

    virtual void                Init() {};
    virtual void                FullUpdate() {};

    virtual void                Reset();
    virtual void *              GetSelf() { return this; };
    void                        SaveChart( bool saveNow = true ) {};

    bool                        IsLoaded() { return Loaded; };

protected:

    bool                        Loaded = false;         // Indicates whether the .chart data has been loaded
    
};

#endif /* CLO_BASE_H */
