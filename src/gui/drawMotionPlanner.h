#pragma once
#include "planner/planner.h"
#include "elements/wrench_field.h"
#include "elements/path_pwl.h"
#include "elements/simplicial_complex.h"
#include "gui/colors.h"

#include <KrisLibrary/GLdraw/drawMesh.h>
#include <KrisLibrary/robotics/IK.h>
#include <KrisLibrary/robotics/IKFunctions.h>
#include <KrisLibrary/camera/viewport.h>
#include <KrisLibrary/math3d/rotation.h>
#include <View/ViewRobot.h>
#include <View/ViewIK.h>
#include <Simulation/WorldSimulation.h>


namespace GLDraw {

  void drawIKextras(ViewRobot *viewrobot, Robot *robot, std::vector<IKGoal> &constraints, std::vector<int> linksInCollision, GLColor selectedLinkColor);
  void drawUniformForceField();
  void drawSphereAtPosition(Vector3 &pos, double r);
  void drawForceField(WrenchField &wrenchfield);
  void drawWrenchField(WrenchField &wrenchfield);
  void drawCylinderArrowAtPosition(Vector3 &pos, Vector3 &dir, GLColor &color);

  void drawGLPathKeyframes(Robot *robot, std::vector<uint> keyframe_indices, std::vector<std::vector<Matrix4> > mats, vector<GLDraw::GeometryAppearance> appearanceStack, GLColor color = GLColor(0.8,0.8,0.8,1.0), double scale = 1.0);

  void drawShortestPath( SimplicialComplex& cmplx );
  void drawSimplicialComplex( SimplicialComplex& cmplx );

  void drawRobotAtConfig(Robot *robot, const Config &q, GLColor color=GLColor(1,0,0), double scale = 1.0);
  //void drawPlannerTree(const SerializedTree &_stree, GLColor colorTree=GLColor(0.3,0.7,0.3));
  void drawAxesLabels(Camera::Viewport& viewport);
  void drawFrames(std::vector< std::vector<Vector3> > &frames, std::vector<double> frameLength);

  void drawCenterOfMassPathFromController(WorldSimulation &sim);
  void drawForceEllipsoid( const ODERobot *robot );
  void drawDistanceRobotTerrain(const ODERobot *robot, const Terrain* terrain);

  void drawWireEllipsoid(Vector3 &c, Vector3 &u, Vector3 &v, Vector3 &w, int numSteps=16);
  void drawEllipsoid(Vector3 &c, Vector3 &u, Vector3 &v, Vector3 &w, int numSteps=16);

  //void drawPath( const std::vector<Config> &path, const GLColor c, double linewidth = 10, double ptsize=10);
  void setColor(GLColor &c);

};
