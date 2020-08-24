#include "testVTKRender.h"

#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkPolygon.h>
#include <vtkLineSource.h>
#include <vtkAppendPolyData.h>

#include <drw_entities.h>
using namespace DRW;
RenderDXFData::RenderDXFData()
{
    _typeRecoord.clear();
}

RenderDXFData::~RenderDXFData()
{

}

void RenderDXFData::SetDXFData(dx_data *data)
{
    _data = data;
}
void buildLookupTable(vtkLookupTable* table)
{
    table->SetNumberOfColors(256);
    for(vtkIdType i=0;i<256;i++)
        table->SetTableValue(i,dxfColors[i][0]/256.0,dxfColors[i][1]/256.0,dxfColors[i][2]/256.0);
    table->Build();
}
void DXFPoint2Point( DRW_Coord& coord,double pt[3])
{
    pt[0] = coord.x;
    pt[1] = coord.y;
    pt[2] = coord.z;
    return;
}
void Face2VTKPts(DRW_3Dface *face,vtkPoints* pts)
{
    double a[3],b[3],c[3],d[3];
    DXFPoint2Point(face->basePoint,a);
    DXFPoint2Point(face->secPoint,b);
    DXFPoint2Point(face->thirdPoint,c);
    DXFPoint2Point(face->fourPoint,d);
    pts->InsertNextPoint(a);
    pts->InsertNextPoint(b);
    pts->InsertNextPoint(c);
    pts->InsertNextPoint(d);
    return;
}
void RenderDXFData::InitDisplay()
{
    if(nullptr == _data)
    {
        cout<<"nullptr"<<endl;
        return;
    }
    auto blocks = _data->mBlock->ent;
    int num = 0;
    vtkNew<vtkAppendPolyData> append;
    int index4Color = -1;
    for (auto iter = blocks.begin(); iter != blocks.end(); iter++)
    {
        recoordType((*iter)->eType);
        index4Color = findIndexOfColorByEntity(*iter);
        if(DRW::LWPOLYLINE == (*iter)->eType)
        {
            DRW_LWPolyline *polyline = dynamic_cast<DRW_LWPolyline*>( *iter);
            vtkNew<vtkLineSource> vtkLine;
            vtkNew<vtkPoints> pts;
            for (size_t i=0;i<polyline->vertlist.size();i++)
                pts->InsertNextPoint(polyline->vertlist.at(i)->x,polyline->vertlist.at(i)->y,0);
            vtkLine->SetPoints(pts);
            vtkLine->Update();
            vtkNew<vtkIntArray> scalars;
            scalars->SetNumberOfTuples(vtkIdType(polyline->vertlist.size()));
            scalars->SetNumberOfComponents(1);
            scalars->FillComponent(0,index4Color);
            vtkLine->GetOutput()->GetCellData()->SetScalars(scalars);
            append->AddInputData(vtkLine->GetOutput());
        }
        if(DRW::LINE == (*iter)->eType)
        {
            DRW_Line *line = dynamic_cast<DRW_Line*>(*iter);
            vtkNew<vtkLineSource> vtkLine;
            vtkLine->SetPoint1(line->basePoint.x,line->basePoint.y,line->basePoint.z);
            vtkLine->SetPoint2(line->secPoint.x,line->secPoint.y,line->secPoint.z);
            vtkLine->Update();
            vtkNew<vtkIntArray> scalars;
            scalars->SetNumberOfTuples(2);
            scalars->SetNumberOfComponents(1);
            scalars->FillComponent(0,index4Color);
            vtkLine->GetOutput()->GetCellData()->SetScalars(scalars);
            append->AddInputData(vtkLine->GetOutput());
        }
        if(DRW::E3DFACE == (*iter)->eType)
        {
            DRW_3Dface *face = dynamic_cast<DRW_3Dface*>(*iter);
            vtkNew<vtkPoints> vtkPts;
            Face2VTKPts(face,vtkPts);
            vtkNew<vtkCellArray> vtkPolys;
            vtkNew<vtkPolygon> vtkPoly;
            vtkPoly->GetPointIds()->SetNumberOfIds(4);
            for(vtkIdType k=0;k<4;k++)
                vtkPoly->GetPointIds()->SetId(k, k);
            vtkPolys->InsertNextCell(vtkPoly);
            vtkNew<vtkPolyData> polydata;
            polydata->SetPoints(vtkPts);
            polydata->SetPolys(vtkPolys);
            vtkNew<vtkIntArray> scalars;
            scalars->SetNumberOfTuples(1);
            scalars->SetNumberOfComponents(1);
            scalars->FillComponent(0,index4Color);
            polydata->GetCellData()->SetScalars(scalars);
            append->AddInputData(polydata);
        }
        num++;
    }
//    for(size_t i=0;i<_typeRecoord.size();i++)
//        cout<<"type num is "<<_typeRecoord[i]<<endl;
    append->Update();
    vtkNew<vtkLookupTable> table;
    buildLookupTable(table);
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetLookupTable(table);
    mapper->SetScalarRange(0,table->GetNumberOfColors());
    mapper->SetInputData(append->GetOutput());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    vtkNew<vtkRenderer> render;
    render->AddActor(actor);
    vtkNew<vtkRenderWindow> renWin;
    vtkNew<vtkRenderWindowInteractor> Iren;
    renWin->AddRenderer(render);
    renWin->SetInteractor(Iren);
    renWin->Render();
    Iren->Start();
}
//record how many types it have
void RenderDXFData::recoordType(int type)
{
    if(_typeRecoord.empty())
        _typeRecoord.push_back(type);
    for(size_t i=0;i<_typeRecoord.size();i++)
    {
        if(type == _typeRecoord[i])
            return;
    }
    _typeRecoord.push_back(type);
    return;
}

int RenderDXFData::findIndexOfColorByEntity(DRW_Entity* ent)
{
    //entity contains color
    if(256 != ent->color)
        return ent->color;
    //color by layer
    for (auto iter = _data->layers.begin();iter != _data->layers.end(); iter++)
    {
        if(ent->layer == iter->name)
            return iter->color;
    }
    //default white
    return 255;
}
