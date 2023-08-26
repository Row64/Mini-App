#ifndef CLO_EXAMPLE_H
#define CLO_EXAMPLE_H

#include "CLO_Base.h"

class CLO_Example : virtual public CLO_Base {

public:
    CLO_Example( Blocks::ChartSConfig * inConfig, int inPaneIndex );
    ~CLO_Example() = default;

    // Data Variables
    ExampleData             CData;
    ExampleFormat           FData;

    vector<vec2>            Points;

    // Methods
    void                Init() override final;
    void                FullUpdate() override final;
    
    void                Reset() override final;
    void *              GetSelf() override final { return this; };

protected:


};

#endif /* CLO_EXAMPLE_H */
