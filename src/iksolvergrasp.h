#pragma once
#include <Contact/Grasp.h> //class Grasp
#include <Planning/ContactCSpace.h>
#include <KrisLibrary/geometry/AnyGeometry.h>
#include "iksolver.h"

class IKSolverGrasp: public IKSolver
{
  public:

    IKSolverGrasp(RobotWorld *world):
      IKSolver(world)
    {
    }

  protected:
    int objectid;
    vector<Real> fixedDofValues;
    vector<int> fixedDofs;


    virtual string GetRobotName() = 0;
    virtual vector<IKGoal> GetConstraints() = 0;
    virtual vector<int> GetFixedDofs() = 0;
    virtual vector<Real> GetFixedDofValues() = 0;
    virtual void ComputeFixedDofs() = 0;

    ///*
    virtual bool solveIKconstraints(){



      this->ComputeFixedDofs();
      //###########################################################################
      //Grasping OBJECT
      //
      //  ContactCSpace is a description of a specific contact submanifold, where
      //  certain points are in contact. Sampling this Cspace corresponds to
      //  sampling IK confs
      //
      //  Grasp worldGrasp: we need to define the IK constraints (i.e. which frames
      //  are in contact at which position. Also the object id is necessary to
      //  disable collision checking between links in contact and object)
      //
      //  Having done that, we can freely sample around the IK space to find a
      //  feasible configuration. This corresponds to cspace.Sample(Config&) and
      //  cspace.Isfeasible(Config)
      //
      //###########################################################################

      Grasp worldGrasp();
      WorldPlannerSettings settings;
      settings.InitializeDefault(*_world);
      settings.robotSettings[0].contactEpsilon = 1e-2;
      settings.robotSettings[0].contactIKMaxIters = 100;
      SingleRobotCSpace freeSpace = SingleRobotCSpace(*_world,_irobot,&settings);

      // The contact cspace needs to be initiziliazed with a free CSpace, and also
      // with the constraints of contacts + which dofs we like to fix. This defines
      // the contact submanifold on which we like to sample for IK solutions)
      ContactCSpace cspace(freeSpace);
      this->_constraints = this->GetConstraints();

      //#######################################################################
      //(0) add margin
      //#######################################################################
      //Real margin = 0.01;
      //for(size_t i=0;i<_robot->geometry.size();i++)
      //{
      //  std::cout << i << "/" << _robot->geometry.size() << std::endl;
      //  //SmartPointer<CollisionGeometry> cc = _robot->geometry[i];
      //  //std::cout << _robot->geometry[i]->margin << std::endl;
      //  AnyCollisionGeometry3D *cc = _robot->geometry[i];
      //  std::cout << cc->margin << std::endl;
      //    //std::vector<SmartPointer<CollisionGeometry> > geometry;

      //}
      //#######################################################################
      //(1) add the constraints of IK here
      //#######################################################################
      for(size_t i=0;i<_constraints.size();i++) {
        std::cout << "constraint " <<i << " link" << _constraints[i].link << std::endl;
        cspace.AddContact(_constraints[i]);
      }

      //#######################################################################
      //(2) which dofs should be fixed
      //#######################################################################
      cspace.fixedDofs = this->GetFixedDofs();
      cspace.fixedValues = this->GetFixedDofValues();

      //#######################################################################
      //(3) ignore collisions between fixed dofs and object
      //and between contact links and object
      //#######################################################################
      int objectid = _world->RigidObjectID(0);
      string oname = _world->rigidObjects[0]->name;
      std::cout << "object" << objectid << " " << oname << std::endl;
      for(size_t i=0;i<_constraints.size();i++) 
      {
        int k=_constraints[i].link;
        cspace.ignoreCollisions.push_back(pair<int,int>(objectid,_world->RobotLinkID(_irobot,k)));
      }
      for(size_t i=0;i<cspace.fixedDofs.size();i++) 
      {
        int k=cspace.fixedDofs[i];
        cspace.ignoreCollisions.push_back(pair<int,int>(objectid,_world->RobotLinkID(_irobot,k)));
      }
      for(size_t i=0;i<cspace.fixedDofs.size();i++) 
      {
        for(size_t j=0;j<cspace.fixedDofs.size();j++) 
        {
          int ilhs=cspace.fixedDofs[i];
          int irhs=cspace.fixedDofs[j];
          cspace.ignoreCollisions.push_back(pair<int,int>(_world->RobotLinkID(_irobot,ilhs),_world->RobotLinkID(_irobot,irhs)));
        }
      }

      //#######################################################################
      //(4) draw #iters samples from the contact submanifold using IK solver
      //#######################################################################
      Config out;
      _isSolved=false;
      std::cout << cspace.fixedDofs << std::endl;
      int iters = 200;

      //cspace.SolveContact();
      //Config out = this->_robot->q;
      //if(cspace.IsFeasible(out)){
      //  return true;
      //}
      //else {
      //  printf("Infeasible:\n");
      //  cspace.PrintInfeasibleNames(out);
      //  getchar();
      //} 
      vector<Real> cdist;
      Config bestConfig = this->_robot->q;
      double bestDist = 1000;

      while(iters-- > 0 ){
        cspace.Sample(out);

        std::cout << "DIST TO CMANIF" << cspace.ContactDistance() << std::endl;
        double d = cspace.ContactDistance();

        if(cspace.IsFeasible(out)){
          std::cout << std::string(80, '-') << std::endl;
          std::cout << "found feasible grasp" << std::endl;
          std::cout << std::string(80, '-') << std::endl;
          _isSolved=true;
          bestDist = d;
          bestConfig = out;
          break;
        }else{
          if(d<bestDist)
          {
            bestDist = d;
            bestConfig = out;
          }
          cspace.PrintInfeasibleNames(out);
          //getchar();
        }
      }

      std::cout << std::string(80, '-') << std::endl;

      if(!cspace.IsFeasible(bestConfig)){
        cspace.PrintInfeasibleNames(out);
      }
      this->_robot->q = bestConfig;
      std::cout << this->_robot->q << std::endl;
      std::cout << std::string(80, '-') << std::endl;

      Vector distV(cdist);
      std::cout << "min dist:" << bestDist << std::endl;
      std::cout << std::string(80, '-') << std::endl;
      return _isSolved;

    }
    //*/

};
