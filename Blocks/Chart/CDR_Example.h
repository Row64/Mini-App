#ifndef CDR_EXAMPLE_H
#define CDR_EXAMPLE_H

#include "CDR_Base.h"
#include "CLO_Example.h"

class CDR_Example : virtual public CDR_Base {

public:
    
    CDR_Example( Blocks::ChartSConfig * inConfig, CLO_Base * inCLO, AppCore::Display3D * inDisp, int inPaneIndex, float &inX, float &inY, float &inW, float &inH );
    ~CDR_Example() = default;

private:

    CLO_Example *       Example = nullptr;

    void                DrawPane() override final;
    
    void                DrawExample();

};

#endif /* CDR_EXAMPLE_H */
