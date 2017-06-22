#include "tangentbundle.h"
#include "ompl_space.h"

Matrix3 GetTotalInertiaAtPoint(const Robot *robot, const Vector3 &p)
{
  Matrix3 I,temp,ccross;
  I.setZero();
  Vector3 ci;
  for(size_t i=0;i<robot->links.size();i++) {
    robot->links[i].GetWorldInertia(temp);
    I += temp;
    //get the inertia matrix associated with the center of mass
    robot->links[i].T_World.mul(robot->links[i].com,ci);
    ci -= p;
    Real x,y,z;
    ci.get(x,y,z);
    ccross(0,0)=y*y+z*z; ccross(0,1)=-x*y; ccross(0,2)=-x*z;
    ccross(1,0)=-x*y; ccross(1,1)=x*x+z*z; ccross(1,2)=-y*z;
    ccross(2,0)=-x*z; ccross(2,1)=-y*z; ccross(2,2)=x*x+y*y;
    ccross.inplaceMul(robot->links[i].mass);
    I += ccross;
  }
  return I;
}

void TangentBundleIntegrator::propagate(const ob::State *state, const oc::Control* control, const double duration, ob::State *result) const 
{

  //
  // Configuration space is Q = G x M whereby G is the position space and M the
  // shape space. We write \hat{q} = (x,r) \in Q for an element in Q.
  //
  // State Space is TQ with an element represented as q = (\hat{q},\dot{\hat{q}}) or
  // ((x,r),(\dot{x},\dot{r}))
  //
  // The local chart on which q resides has dimensionality 6 + N + 6 + N = 12+2N
  // with G=SE(3), M=R^N, TG \times TM = R^{6+N}
  //
  // Control is applied on the tangent space TTQ of the tangent bundle TQ.
  // The control dimensionality is R^{6+N} \times R whereby the last dimension
  // is the step size parameter (some algorithms are sensitive to the stepsize,
  // so it is advantageous to change it, adapt it on the fly or even include it
  // into some optimization routine)
  //

  const ob::StateSpacePtr s = si_->getStateSpace();
  Config qstate = ompl_space->OMPLStateToConfig(state);
  const double *ucontrol = control->as<oc::RealVectorControlSpace::ControlType>()->values;

  //###########################################################################
  // extract step size parameter
  //###########################################################################

  uint N = 0.5*s->getDimension() - 6;
  assert( 2*N + 12 == s->getDimension());

  uint Nduration = N+6;
  Real dt = ucontrol[Nduration];
  if(dt<0){
    std::cout << "propagation step size is negative:"<<dt << std::endl;
    exit(0);
  }
  Real dt2 = 0.5*dt*dt;

  Vector q0,dq0;q0.resize(6+N);dq0.resize(6+N);
  for(int i = 0; i < 6+N; i++){
    q0(i) = qstate(i);
    dq0(i) = qstate(i+6+N);
  }

  robot->dq = dq0;
  robot->UpdateConfig(q0);
  robot->UpdateDynamics();

  uint lidx = 5;
  RobotLink3D *link  = &robot->links.at(lidx);
  Vector3 com = link->com;
  Matrix3 R = link->T_World.R;

  //R.mul(com,com);

  Config q1; q1.resize(12+2*N); q1.setZero();
  Config dq1;dq1.resize(6+N); dq1.setZero();
  Config ddq0; ddq0.resize(6+N); ddq0.setZero();
  //###########################################################################
  // update based on real dynamics
  //###########################################################################
  Vector fext; fext.resize(6+N);
  fext.setZero();

  //fext = S^T * torque, whereby S is the selection matrix
  for(int k = 6; k < N+6; k++){
    fext(k) = ucontrol[k];
  }

  Vector3 torque,tmp,force;
  force[0]=ucontrol[0];
  force[1]=ucontrol[1];
  force[2]=ucontrol[2];

  tmp[0]=ucontrol[5];
  tmp[1]=ucontrol[4];
  tmp[2]=ucontrol[3];

  R.mul(force, force);
  R.mul(tmp, tmp);
  torque[0] = tmp[2];
  torque[1] = tmp[1];
  torque[2] = tmp[0];

  Vector Fq;
  robot->GetWrenchTorques(tmp, force, 5, Fq);
  fext += Fq;



  // //Matrix3 inertia = GetTotalInertiaAtPoint(robot, link->T_World*com);
  Matrix3 inertia = robot->GetTotalInertia();
  Matrix3 inertia_inv;
  inertia.getInverse(inertia_inv);

  Vector3 ddqTorque;
  inertia_inv.mul(torque, ddqTorque);

  fext[3]=ddqTorque[0];
  fext[4]=ddqTorque[1];
  fext[5]=ddqTorque[2];

  robot->CalcAcceleration(ddq0, fext);
  //ddq0(0)=0;
  //ddq0(1)=0;
  //ddq0(6)=0;

  ddq0(6)=0;
  std::cout << std::string(80, '-') << std::endl;
  std::cout << fext << std::endl;
  std::cout << ddq0 << std::endl;

  ////

  //Vector wrench;wrench.resize(6);
  ////row 0-2 of jacobian is angular, 3-5 translational
  ////for(int i = 0; i < 3; i++) wrench(i)=ddqTorque[i];
  //wrench(0) = ddqTorque[2];
  //wrench(1) = ddqTorque[1];
  //wrench(2) = ddqTorque[0];

  //for(int i = 3; i < 6; i++) wrench(i)=ddqForce[i-3];
  //fext[0]+=ddqTorque[0];
  //fext[1]+=ddqTorque[1];
  //fext[2]+=ddqTorque[2];
  //fext[3]+=ddqForce[0];
  //fext[4]+=ddqForce[1];
  //fext[5]+=ddqForce[2];

  //Vector Fq;
  //J.mulTranspose(wrench, Fq);
  //fext += Fq;

  // exit(0);
  /*
   LieGroupIntegrator integrator;

   Config x0; x0.resize(6);
   Config dx0; dx0.resize(6);
   Config ddx0; ddx0.resize(6);
   for(int i = 0; i < 6; i++){
     x0(i) = q0(i);
     dx0(i) = dq0(i);
     ddx0(i) = ddq0(i);
   }

   Matrix4 x0_SE3 = integrator.StateToSE3(x0);

   Matrix4 dx0_SE3 = integrator.SE3Derivative(dx0);

   Matrix4 ddp = integrator.SE3Derivative(ddx0);

   Matrix4 dp = ddp*dt*0.5 + dx0_SE3;

   Matrix4 x1_SE3 = integrator.Integrate(x0_SE3,dp,dt);

   State x1;x1.resize(6);
   integrator.SE3ToState(x1, x1_SE3);

   State dx1 = ddx0*dt + dx0;

   for(int i = 0; i < 6; i++){
     q1(i) = x1(i);
     q1(i+6+N) = dx1(i);
     dq1(i) = dx1(i);
   }

  // std::cout << std::string(80, '-') << std::endl; 
  //*/
  //*
  for(int i = 0; i < (N+6); i++){
    q1(i) = q0(i) + dt*dq0(i) + dt2*ddq0(i);
    q1(i+N+6) = dq0(i) + dt*ddq0(i);
    dq1(i) = q1(i+N+6);
  }
  q1(6) = 0;
  q1(6+N+6) = 0;

  if(q1(3)<-M_PI) q1(3)+=2*M_PI;
  if(q1(3)>M_PI) q1(3)-=2*M_PI;

  if(q1(4)<-M_PI/2) q1(4)+=M_PI;
  if(q1(4)>M_PI/2) q1(4)-=M_PI;

  if(q1(5)<-M_PI) q1(5)+=2*M_PI;
  if(q1(5)>M_PI) q1(5)-=2*M_PI;

  // std::cout << std::string(80, '-') << std::endl;
  // std::cout << "torque:" << torque << std::endl;
  // std::cout << "ddq0 :" << ddq0 << std::endl;
  // std::cout << "dq0 :" << dq0 << std::endl;
  // std::cout << "q0 :" << q0 << std::endl;
  // std::cout << "dt :" << dt << std::endl;
  // std::cout << "dq1 :" << dq1 << std::endl;
  // std::cout << "q1 :" << q1 << std::endl;
  // static uint xk = 0;
  // if(xk++ > 10) exit(0);
  //*/


  //###########################################################################
  // Forward Simulate R^N component
  //###########################################################################
  /*
  for(int i = 0; i < N; i++){
    q1[i+6] = q0[i+6] + dt*dq0[i+6] + dt2*ddq0[i+6];
    q1[i+N+6+6] = dq0[i+6] + dt*ddq0[i+6];
  }
  //*/

  //###########################################################################
  // Config to OMPL
  //###########################################################################

  ob::ScopedState<> ssr = ompl_space->ConfigToOMPLState(q1);

  ob::SE3StateSpace::StateType *ssrSE3 = ssr->as<ob::CompoundState>()->as<ob::SE3StateSpace::StateType>(0);
  ob::SO3StateSpace::StateType *ssrSO3 = &ssrSE3->rotation();
  ob::RealVectorStateSpace::StateType *ssrRn = ssr->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(1);
  ob::RealVectorStateSpace::StateType *ssrTM = ssr->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(2);

  ob::SE3StateSpace::StateType *resultSE3 = result->as<ob::CompoundState>()->as<ob::SE3StateSpace::StateType>(0);
  ob::SO3StateSpace::StateType *resultSO3 = &resultSE3->rotation();
  ob::RealVectorStateSpace::StateType *resultRn = result->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(1);
  ob::RealVectorStateSpace::StateType *resultTM = result->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(2);

  resultSE3->setXYZ(ssrSE3->getX(),ssrSE3->getY(),ssrSE3->getZ());
  resultSO3->x = ssrSO3->x;
  resultSO3->y = ssrSO3->y;
  resultSO3->z = ssrSO3->z;
  resultSO3->w = ssrSO3->w;

  //###########################################################################
  // R^N Control
  //###########################################################################
  for(int i = 0; i < N; i++){
    resultRn->values[i] = ssrRn->values[i];
  }
  for(int i = 0; i < N+6; i++){
    resultTM->values[i] = ssrTM->values[i];
  }

}

TangentBundleOMPLValidityChecker::TangentBundleOMPLValidityChecker(const ob::SpaceInformationPtr &si, CSpace* space, CSpaceOMPL* ompl_space_):
  ob::StateValidityChecker(si),cspace_(space),ompl_space(ompl_space_)
{
}

bool TangentBundleOMPLValidityChecker::isValid(const ob::State* state) const
{
  const ob::StateSpacePtr ssp = si_->getStateSpace();
  Config q = ompl_space->OMPLStateToConfig(state);
  return cspace_->IsFeasible(q) && si_->satisfiesBounds(state);
}
