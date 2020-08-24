#ifndef TESTVTKRENDER_H
#define TESTVTKRENDER_H
#include "vtkSmartPointer.h"
#include "dx_data.h"
#include "vtkRenderer.h"
#include "vector"

class RenderDXFData{
public:
    RenderDXFData();
    ~RenderDXFData();
    void SetDXFData(dx_data *data);
    void InitDisplay();
protected:
    void recoordType(int type);
    int findIndexOfColorByEntity(DRW_Entity* ent);
private:
    dx_data *_data;
    std::vector<int> _typeRecoord;
};

#endif
