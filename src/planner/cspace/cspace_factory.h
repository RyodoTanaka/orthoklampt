#pragma once
#include "planner/cspace/cspace.h"
#include "planner/cspace/cspace_geometric_se2.h"
#include "planner/cspace/cspace_geometric_r2.h"
#include "planner/cspace/cspace_geometric_r2_edge_so2.h"
#include "planner/cspace/cspace_input.h"
#include "planner/cspace/cspace_decorator.h"


class CSpaceFactory{
  private:
    const CSpaceInput& input;
  public:
    CSpaceFactory(const CSpaceInput &input_): input(input_){};
    virtual KinodynamicCSpaceOMPL* MakeKinodynamicCSpace( Robot *robot, CSpaceKlampt *kspace){
      KinodynamicCSpaceOMPL *cspace = new KinodynamicCSpaceOMPL(robot, kspace);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    virtual GeometricCSpaceOMPL* MakeGeometricCSpace( RobotWorld *world, int robot_idx){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPL(world, robot_idx);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    //CSpace for a rigid object which has rotational invariance. CSpace is R3
    virtual GeometricCSpaceOMPL* MakeGeometricCSpaceRotationalInvariance( Robot *robot, CSpaceKlampt *inner){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLRotationalInvariance(robot, inner);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }

    // CSpace which has two rotational degrees of freedom
    // [0,1] \times S^1 \times S^1
    // representing a roll-orientation invariant object along an arbitrary R^3
    // path

    virtual GeometricCSpaceOMPL* MakeGeometricCSpacePathConstraintRollInvariance( Robot *robot, CSpaceKlampt *inner, const std::vector<Config>& path){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLPathConstraintRollInvariance(robot, inner, path);
      //make pwl path from path, then add one more [0,1] dimension to cspace,
      //and make it depend on pwlpath
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    virtual GeometricCSpaceOMPL* MakeGeometricCSpacePathConstraintSO3( Robot *robot, CSpaceKlampt *inner, const std::vector<Config>& path){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLPathConstraintSO3(robot, inner, path);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    // CSpace  SE(2)
    virtual GeometricCSpaceOMPL* MakeGeometricCSpaceSE2( RobotWorld *world, int robot_idx){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLSE2(world, robot_idx);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    // CSpace  R^(2)
    virtual GeometricCSpaceOMPL* MakeGeometricCSpaceR2( RobotWorld *world, int robot_idx){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLR2(world, robot_idx);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    // CSpace  R^(2) Edge \times SO(2)
    virtual GeometricCSpaceOMPL* MakeGeometricCSpaceR2EdgeSO2( RobotWorld *world, int robot_idx, const Config &q1, const Config &q2 ){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLR2EdgeSO2(world, robot_idx, q1, q2);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }

    // CSpace {q} \times SO(3)
    virtual GeometricCSpaceOMPL* MakeGeometricCSpacePointConstraintSO3( Robot *robot, CSpaceKlampt *inner, const Config q){
      GeometricCSpaceOMPL *cspace = new GeometricCSpaceOMPLPointConstraintSO3(robot, inner, q);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    //
    //a configuration q in the innerouter cspace is feasible iff
    // inner(q) is feasible AND outer(q) is infeasible
    //
    virtual CSpaceOMPL* MakeCSpaceDecoratorInnerOuter( CSpaceOMPL* cs, CSpaceKlampt *outer){
      CSpaceOMPL *cspace = new CSpaceOMPLDecoratorInnerOuter(cs, outer);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
    virtual CSpaceOMPL* MakeCSpaceDecoratorNecessarySufficient( CSpaceOMPL* cs, CSpaceKlampt *outer){
      CSpaceOMPL *cspace = new CSpaceOMPLDecoratorNecessarySufficient(cs, outer);
      cspace->SetCSpaceInput(input);
      cspace->initSpace();
      cspace->initControlSpace();
      return cspace;
    }
};

