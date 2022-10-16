#ifndef _GLEDITOR_H
#define _GLEDITOR_H

#include "katanox.h"

//---------------------------------------------------------------------------
// GLCAM_* flags
//
// pass to Show() or Hide() to turn things on or off
//
// GLCAM_CAMERAPOS
//     (ON)  Draw a little camera object where the camera would be.
// GLCAM_ADJACENTPATH
//     (ON)  Draw the path that the camera would take and the path following
//           the current node.
// GLCAM_ENTIREPATH
//     (ON)  Draw the path that the camera would take for all the current
//           camera's list of nodes.
// GLCAM_NODEPOS
//     (ON)  Draw the position of the nodes, and the node velocity 
//           velocity indicators.  It will draw it for all nodes if
//           GLCAM_ENTIREPATH is enabled, and only the current node if
//           GLCAM_ADJACENTPATH is enabled.
// GLCAM_FOCUSPOINT
//     (ON)  Draws the path, node position, and camera position of the 
//           current camera's focal point camera depending upon
//           GLCAM_CAMERAPOS, GLCAM_*PATH, and GLCAM_NODEPOS
// GLCAM_ALLCAMERAS
//     (ON)  Draws the information for all defined cameras, depending upon
//           the values of GLCAM_CAMERAPOS, GLCAM_ADJACENTPATH, 
//           GLCAM_ENTIREPATH, GLCAM_NODEPOS, and GLCAM_FOCUSPOINT
//---------------------------------------------------------------------------


#define GLCAM_EVERYTHING    0xFFFFFFFF
#define GLCAM_CAMERAPOS     0x00000001
#define GLCAM_ADJACENTPATH  0x00000002
#define GLCAM_ENTIREPATH    0x00000004
#define GLCAM_NODEPOS       0x00000008
#define GLCAM_FOCUSPOINT    0x00000010
#define GLCAM_ALLCAMERAS    0x00000020
#define GLCAM_SHOWARROWS    0x00000040


typedef struct {
    int        bInfoValid;
    VEC3        NodePos, NodeVel, NodeVelAng;
    float       NodeTime;
    float       NodeAccumTime;
    char        NodeFlags[80];
    float       NodeDensity;
    float       VelRed, VelBlue;

    int         CamNum;
    int         CamFocus;
} SCRIPTINFO;


//class __declspec(dllexport) GLCAMEDIT {
class GLCAMEDIT {
public:
    GLCAMEDIT();
    ~GLCAMEDIT();

    void Show( DWORD );
    void Hide( DWORD );
    void Toggle( DWORD );

    void NextCamera();
    void PrevCamera();
    void NextNode();
    void PrevNode();
    void FirstNode();
    void LastNode();

    void InsertNode( float, DWORD, const VEC3&, const VEC3& );
    void AppendNode( float, DWORD, const VEC3&, const VEC3& );
    void DeleteNode();

    void SetInitial( const VEC3&, const VEC3& );
    void DrawPath( const CAMERA *, const CAMERA *, CAMNODE *, VEC3& col ); // cam must already be initted
    void Draw( float );

    void AttachScript( JSCRIPT * );
    void GetScriptInfo( SCRIPTINFO * );

    void ScaleDensity( float );
    void SetNodeTime( float );

    void EditNodePos( const VEC3&, int bIsRelative );
    void EditNodeVel( const VEC3&, int bIsRelative );

private:

    JSCRIPT         *Slave;
    DWORD           ulShowFlags, ulSavedFlags;
    VEC3            campos, camvel, camang;
    VEC3            focpos, focvel, focang;
    VEC3            initpos, initvel;
    float           fPathDensity;
    float           fVelColorScale;
};


#endif