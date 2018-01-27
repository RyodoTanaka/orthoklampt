#include "quotient.h"
#include "planner/cspace/cspace.h"

#include <ompl/base/goals/GoalSampleableRegion.h>
#include <ompl/base/spaces/SO2StateSpace.h>
#include <ompl/base/spaces/SO3StateSpace.h>
#include <ompl/base/spaces/SE2StateSpace.h>
#include <ompl/base/spaces/SE3StateSpace.h>

using namespace ompl::geometric;
using namespace ompl::base;

uint Quotient::counter = 0;
void Quotient::resetCounter()
{
  Quotient::counter = 0;
}

Quotient::Quotient(const ob::SpaceInformationPtr &si, Quotient *previous_):
  ob::Planner(si,"QuotientSpace"), previous(previous_), M1(si)
{
  const StateSpacePtr M1_space = M1->getStateSpace();

  id = counter++;
  std::cout << "--- Level " << id << " " << getName() << std::endl;
  setName("Quotient"+std::to_string(id));
  if(previous == nullptr){
    std::cout << "M1 dimension : " << M1_space->getDimension() << std::endl;
  }else{
    const StateSpacePtr M0_space = previous->getSpaceInformation()->getStateSpace();
    //C1 = M1 / M0
    const StateSpacePtr C1_space = ComputeQuotientSpace(M1_space, M0_space);

    C1 = std::make_shared<SpaceInformation>(C1_space);
    C1_sampler = C1->allocStateSampler();
    if(M0_space->getDimension()+C1_space->getDimension() != M1_space->getDimension()){
      std::cout << "quotient of state spaces got dimensions wrong." << std::endl;
      exit(0);
    }
    std::cout << "M0 dimension : " << M0_space->getDimension() << " measure: " << M0_space->getMeasure() << std::endl;
    std::cout << "C1 dimension : " << C1_space->getDimension() << " measure: " << C1_space->getMeasure() << std::endl;
    std::cout << "M1 dimension : " << M1_space->getDimension() << " measure: " << M1_space->getMeasure() << std::endl;
  }
  if (!sampler_){
    sampler_ = si_->allocValidStateSampler();
  }
  //if (!simpleSampler_){
  //  simpleSampler_ = si_->allocStateSampler();
  //}

}


const StateSpacePtr Quotient::ComputeQuotientSpace(const StateSpacePtr M1, const StateSpacePtr M0)
{
    //STATE_SPACE_UNKNOWN = 0,
    //STATE_SPACE_REAL_VECTOR = 1,
    //STATE_SPACE_SO2 = 2,
    //STATE_SPACE_SO3 = 3,
    //STATE_SPACE_SE2 = 4,
    //STATE_SPACE_SE3 = 5,
    //STATE_SPACE_TIME = 6,
    //STATE_SPACE_DISCRETE = 7,

    //Cases we can handle:
    // ---- non-compound:
    // (1) M1 Rn       , M0 Rm       , 0<m<n   => C1 = R(n-m)
    // ---- compound:
    // (2) M1 SE2      , M0 R2                 => C1 = SO2
    // (3) M1 SE3      , M0 R3                 => C1 = SO3
    // (4) M1 SE3xRn   , M0 SE3                => C1 = Rn
    // (5) M1 SE3xRn   , M0 SE3xRm   , 0<m<n   => C1 = R(n-m)
    ///// M1 SE3      , M0 R3xSO2xSO2         =>C1 = SO2
    ///// M1 R3xS1xS1 , M0 R3                 =>C1 = SO2xSO2

    if(!M1->isCompound()){
      //------------------ non-compound cases:
      //
      //------------------ (1) M1 = Rn, M0 = Rm, 0<m<n, C1 = R(n-m)
      if( M1->getType() == base::STATE_SPACE_REAL_VECTOR ){
        uint n = M1->getDimension();
        if( M0->getType() == base::STATE_SPACE_REAL_VECTOR ){
          uint m = M0->getDimension();
          if(n>m && m>0){
            type = RN_RM;
          }else{
            std::cout << "not allowed: we need n>m>0, but have " << n << ">" << m << ">0" << std::endl;
            exit(0);
          }
        }else{
          std::cout << "M1 is R^"<<n <<" but state " << M1->getType() << " is not handled." << std::endl;
          exit(0);
        }
      }else{
        std::cout << "M1 is non-compound state, but state " << M1->getType() << " is not handled." << std::endl;
        exit(0);
      }
    }else{
      //------------------ compound cases:
      //
      //------------------ (2) M1 = SE2, M0 = R2, C1 = SO2
      if( M1->getType() == base::STATE_SPACE_SE2 ){
        if( M0->getType() == base::STATE_SPACE_REAL_VECTOR){
          if( M0->getDimension() == 2){
            type = SE2_R2;
          }else{
            std::cout << "M1 is SE2 but state " << M0->getType() << " is of dimension " << M0->getDimension() << std::endl;
            exit(0);
          }
        }else{
          std::cout << "M1 is SE2 but state " << M0->getType() << " is not handled." << std::endl;
          exit(0);
        }
        exit(0);
      }
      //------------------ (3) M1 = SE3, M0 = R3, C1 = SO3
      else if( M1->getType() == base::STATE_SPACE_SE3 ){
        if( M0->getType() == base::STATE_SPACE_REAL_VECTOR){
          if( M0->getDimension() == 3){
            type = SE3_R3;
          }else{
            std::cout << "M1 is SE3 but state " << M0->getType() << " is of dimension " << M0->getDimension() << std::endl;
            exit(0);
          }
        }else{
          std::cout << "M1 is SE3 but state " << M0->getType() << " is not handled." << std::endl;
          exit(0);
        }
      }
      //------------------ M1 = COMPOSED
      else{
        ob::CompoundStateSpace *M1_compound = M1->as<ob::CompoundStateSpace>();
        const std::vector<StateSpacePtr> M1_decomposed = M1_compound->getSubspaces();
        uint M1_subspaces = M1_decomposed.size();
        if(M1_subspaces == 2){
          if(M1_decomposed.at(0)->getType() == base::STATE_SPACE_SE3
              && M1_decomposed.at(1)->getType() == base::STATE_SPACE_REAL_VECTOR){

            uint n = M1_decomposed.at(1)->getDimension();
            //------------------ (4) M1 = SE3xRn, M0 = SE3, C1 = Rn
            if( M0->getType() == base::STATE_SPACE_SE3 ){
              type = SE3RN_SE3;
            }else{
            //------------------ (5) M1 = SE3xRn, M0 = SE3xRm, C1 = R(n-m)
              ob::CompoundStateSpace *M0_compound = M0->as<ob::CompoundStateSpace>();
              const std::vector<StateSpacePtr> M0_decomposed = M0_compound->getSubspaces();
              uint M0_subspaces = M0_decomposed.size();
              if(M0_subspaces==2){
                if(M1_decomposed.at(0)->getType() == base::STATE_SPACE_SE3
                    && M1_decomposed.at(1)->getType() == base::STATE_SPACE_REAL_VECTOR){
                  uint m = M0_decomposed.at(1)->getDimension();
                  if(m<n && m>0){
                    type = SE3RN_SE3RM;
                  }else{
                    std::cout << "not allowed: we need n>m>0, but have " << n << ">" << m << ">0" << std::endl;
                    exit(0);
                  }
                }
              }else{
                std::cout << "State compound with " << M0_subspaces << " not handled."<< std::endl;
                exit(0);
              }
            }
          }else{
            std::cout << "State compound " <<  M1_decomposed.at(0)->getType() << " and " <<  M1_decomposed.at(1)->getType() << " not recognized."<< std::endl;
            exit(0);
          }
        }
      }

    }

    StateSpacePtr C1;
    M1_dimension = M1->getDimension();
    M0_dimension = M0->getDimension();

    switch (type) {
      case RN_RM:
        {
          C1 = std::make_shared<ob::RealVectorStateSpace>(M1_dimension-M0_dimension);
          break;
        }
      case SE2_R2:
        {
          C1 = std::make_shared<ob::SO2StateSpace>();
          break;
        }
      case SE3_R3:
        {
          C1 = std::make_shared<ob::SO3StateSpace>();
          break;
        }
      case SE3RN_SE3:
        {
          ob::CompoundStateSpace *M1_compound = M1->as<ob::CompoundStateSpace>();
          const std::vector<StateSpacePtr> M1_decomposed = M1_compound->getSubspaces();

          C1_dimension = M1_decomposed.at(1)->getDimension();
          C1 = std::make_shared<ob::RealVectorStateSpace>(C1_dimension);
          break;
        }
      case SE3RN_SE3RM:
        {
          ob::CompoundStateSpace *M1_compound = M1->as<ob::CompoundStateSpace>();
          const std::vector<StateSpacePtr> M1_decomposed = M1_compound->getSubspaces();
          ob::CompoundStateSpace *M0_compound = M0->as<ob::CompoundStateSpace>();
          const std::vector<StateSpacePtr> M0_decomposed = M0_compound->getSubspaces();

          C1_dimension = M1_decomposed.at(1)->getDimension()-M0_decomposed.at(1)->getDimension();
          C1 = std::make_shared<ob::RealVectorStateSpace>(C1_dimension);
          break;
        }
      default:
        {
          std::cout << "unknown type: " << type << std::endl;
          exit(0);
        }

    }
    return C1;

}

void Quotient::mergeStates(const ob::State *qM0, const ob::State *qC1, ob::State *qM1)
{
  ////input : qM0 \in M0, qC1 \in C1
  ////output: qM1 = qM0 \circ qC1 \in M1
  const StateSpacePtr M1_space = M1->getStateSpace();
  const StateSpacePtr C1_space = C1->getStateSpace();
  const StateSpacePtr M0_space = previous->getSpaceInformation()->getStateSpace();

  switch (type) {
    case RN_RM:
      {
        ob::RealVectorStateSpace::StateType *sM1 = qM1->as<RealVectorStateSpace::StateType>();
        const ob::RealVectorStateSpace::StateType *sM0 = qM0->as<RealVectorStateSpace::StateType>();
        const ob::RealVectorStateSpace::StateType *sC1 = qC1->as<RealVectorStateSpace::StateType>();

        for(uint k = 0; k < M0_dimension; k++){
          sM1->values[k] = sM0->values[k];
        }
        for(uint k = M0_dimension; k < M1_dimension; k++){
          sM1->values[k] = sC1->values[k-M0_dimension];
        }
        break;
      }
    case SE2_R2:
      {
        ob::SE2StateSpace::StateType *sM1 = qM1->as<SE2StateSpace::StateType>();
        const ob::RealVectorStateSpace::StateType *sM0 = qM0->as<RealVectorStateSpace::StateType>();
        const ob::SO2StateSpace::StateType *sC1 = qC1->as<SO2StateSpace::StateType>();

        sM1->setXY( sM0->values[0], sM0->values[1] );
        sM1->setYaw( sC1->value );

        break;
      }
    case SE3_R3:
      {
        ob::SE3StateSpace::StateType *sM1 = qM1->as<SE3StateSpace::StateType>();
        ob::SO3StateSpace::StateType *sM1_rotation = &sM1->rotation();

        const ob::RealVectorStateSpace::StateType *sM0 = qM0->as<RealVectorStateSpace::StateType>();
        const ob::SO3StateSpace::StateType *sC1 = qC1->as<SO3StateSpace::StateType>();

        sM1->setXYZ( sM0->values[0], sM0->values[1], sM0->values[2]);

        sM1_rotation->x = sC1->x;
        sM1_rotation->y = sC1->y;
        sM1_rotation->z = sC1->z;
        sM1_rotation->w = sC1->w;

        break;
      }
    case SE3RN_SE3:
      {
        ob::SE3StateSpace::StateType *sM1_SE3 = qM1->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
        ob::SO3StateSpace::StateType *sM1_SE3_rotation = &sM1_SE3->rotation();
        ob::RealVectorStateSpace::StateType *sM1_RN = qM1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

        const ob::SE3StateSpace::StateType *sM0 = qM0->as<SE3StateSpace::StateType>();
        const ob::SO3StateSpace::StateType *sM0_rotation = &sM0->rotation();
        const ob::RealVectorStateSpace::StateType *sC1 = qC1->as<RealVectorStateSpace::StateType>();

        sM1_SE3->setXYZ( sM0->getX(), sM0->getY(), sM0->getZ());
        sM1_SE3_rotation->x = sM0_rotation->x;
        sM1_SE3_rotation->y = sM0_rotation->y;
        sM1_SE3_rotation->z = sM0_rotation->z;
        sM1_SE3_rotation->w = sM0_rotation->w;

        for(uint k = 0; k < C1_dimension; k++){
          sM1_RN->values[k] = sC1->values[k];
        }

        break;
      }
    case SE3RN_SE3RM:{
        ob::SE3StateSpace::StateType *sM1_SE3 = qM1->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
        ob::SO3StateSpace::StateType *sM1_SE3_rotation = &sM1_SE3->rotation();
        ob::RealVectorStateSpace::StateType *sM1_RN = qM1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

        const ob::SE3StateSpace::StateType *sM0_SE3 = qM0->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
        const ob::SO3StateSpace::StateType *sM0_SE3_rotation = &sM0_SE3->rotation();
        const ob::RealVectorStateSpace::StateType *sM0_RM = qM0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

        const ob::RealVectorStateSpace::StateType *sC1 = qC1->as<RealVectorStateSpace::StateType>();

        sM1_SE3->setXYZ( sM0_SE3->getX(), sM0_SE3->getY(), sM0_SE3->getZ());
        sM1_SE3_rotation->x = sM0_SE3_rotation->x;
        sM1_SE3_rotation->y = sM0_SE3_rotation->y;
        sM1_SE3_rotation->z = sM0_SE3_rotation->z;
        sM1_SE3_rotation->w = sM0_SE3_rotation->w;

        //[X Y Z YAW PITCH ROLL] [1...M-1][M...N-1]
        //SE3                    RN
        uint M = M1_dimension-C1_dimension-6;
        uint N = C1_dimension;

        for(uint k = 0; k < M; k++){
          sM1_RN->values[k] = sM0_RM->values[k];
        }
        for(uint k = M; k < M+N; k++){
          sM1_RN->values[k] = sC1->values[k-M];
        }
        break;
      }
    default:
      {
        std::cout << "cannot merge states" << std::endl;
        exit(0);
      }

  }
}
bool Quotient::Sample(ob::State *q_random)
{
  if(previous == nullptr){
    return sampler_->sample(q_random);
  }else{
    //Adjusted sampling function: Sampling in G0 x C1

    ob::SpaceInformationPtr M0 = previous->getSpaceInformation();
    base::State *s_C1 = C1->allocState();
    base::State *s_M0 = M0->allocState();
    C1_sampler->sampleUniform(s_C1);
    previous->SampleGraph(s_M0);
    mergeStates(s_M0, s_C1, q_random);

    return M1->isValid(q_random);
  }
}
void Quotient::ExtractC1Subspace( ob::State* q, ob::State* qC1 ) const
{
  switch(type){
    case RN_RM:
      {
        const ob::RealVectorStateSpace::StateType *sM1 = q->as<RealVectorStateSpace::StateType>();
        ob::RealVectorStateSpace::StateType *sC1 = qC1->as<RealVectorStateSpace::StateType>();

        for(uint k = M0_dimension; k<M1_dimension; k++){
          sC1->values[k-M0_dimension] = sM1->values[k];
        }
        break;
      }
    case SE2_R2:
      {
        const ob::SE2StateSpace::StateType *sM1 = q->as<SE2StateSpace::StateType>();
        ob::SO2StateSpace::StateType *sC1 = qC1->as<SO2StateSpace::StateType>();
        sC1->value = sM1->getYaw();
        break;
      }
    case SE3_R3:
      {
        const ob::SE3StateSpace::StateType *sM1 = q->as<SE3StateSpace::StateType>();
        const ob::SO3StateSpace::StateType *sM1_rotation = &sM1->rotation();

        ob::SO3StateSpace::StateType *sC1 = qC1->as<SO3StateSpace::StateType>();

        sC1->x = sM1_rotation->x;
        sC1->y = sM1_rotation->y;
        sC1->z = sM1_rotation->z;
        sC1->w = sM1_rotation->w;

        break;
      }
    case SE3RN_SE3:
      {
        const ob::RealVectorStateSpace::StateType *sM1_RN = q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);
        ob::RealVectorStateSpace::StateType *sC1 = qC1->as<RealVectorStateSpace::StateType>();

        for(uint k = 0; k < C1_dimension; k++){
          sC1->values[k] = sM1_RN->values[k];
        }

        break;
      }
    case SE3RN_SE3RM:{
        const ob::RealVectorStateSpace::StateType *sM1_RN = q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

        const ob::RealVectorStateSpace::StateType *sC1 = qC1->as<RealVectorStateSpace::StateType>();

        uint N = M1_dimension - C1_dimension - 6;
        for(uint k = N; k < M1_dimension; k++){
          sC1->values[k-N] = sM1_RN->values[k];
        }
        break;
      }
    default:
      {
        std::cout << "cannot merge states" << std::endl;
        exit(0);
      }

  }
}

void Quotient::ExtractM0Subspace( ob::State* q, ob::State* qM0 ) const
{

  switch(type){
    case RN_RM:
      {
        const ob::RealVectorStateSpace::StateType *sM1 = q->as<RealVectorStateSpace::StateType>();
        ob::RealVectorStateSpace::StateType *sM0 = qM0->as<RealVectorStateSpace::StateType>();

        for(uint k = 0; k<M0_dimension; k++){
          sM0->values[k] = sM1->values[k];
        }
        break;
      }
    case SE2_R2:
      {
        const ob::SE2StateSpace::StateType *sM1 = q->as<SE2StateSpace::StateType>();
        ob::RealVectorStateSpace::StateType *sM0 = qM0->as<RealVectorStateSpace::StateType>();
        sM0->values[0] = sM1->getX();
        sM0->values[1] = sM1->getY();
        break;
      }
    case SE3_R3:
      {
        const ob::SE3StateSpace::StateType *sM1 = q->as<SE3StateSpace::StateType>();
        const ob::SO3StateSpace::StateType *sM1_rotation = &sM1->rotation();
        ob::RealVectorStateSpace::StateType *sM0 = qM0->as<RealVectorStateSpace::StateType>();

        sM0->values[0] = sM1->getX();
        sM0->values[1] = sM1->getY();
        sM0->values[2] = sM1->getZ();

        break;
      }
    case SE3RN_SE3:
      {
        const ob::SE3StateSpace::StateType *sM1_SE3 = q->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
        const ob::SO3StateSpace::StateType *sM1_SE3_rotation = &sM1_SE3->rotation();

        ob::SE3StateSpace::StateType *sM0 = qM0->as<SE3StateSpace::StateType>();
        ob::SO3StateSpace::StateType *sM0_rotation = &sM0->rotation();
        sM0->setXYZ( sM1_SE3->getX(), sM1_SE3->getY(), sM1_SE3->getZ());
        sM0_rotation->x = sM1_SE3_rotation->x;
        sM0_rotation->y = sM1_SE3_rotation->y;
        sM0_rotation->z = sM1_SE3_rotation->z;
        sM0_rotation->w = sM1_SE3_rotation->w;

        break;
      }
    case SE3RN_SE3RM:{
        const ob::SE3StateSpace::StateType *sM1_SE3 = q->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
        const ob::SO3StateSpace::StateType *sM1_SE3_rotation = &sM1_SE3->rotation();
        const ob::RealVectorStateSpace::StateType *sM1_RN = q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

        ob::SE3StateSpace::StateType *sM0_SE3 = qM0->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
        ob::SO3StateSpace::StateType *sM0_rotation = &sM0_SE3->rotation();
        sM0_SE3->setXYZ( sM1_SE3->getX(), sM1_SE3->getY(), sM1_SE3->getZ());
        sM0_rotation->x = sM1_SE3_rotation->x;
        sM0_rotation->y = sM1_SE3_rotation->y;
        sM0_rotation->z = sM1_SE3_rotation->z;
        sM0_rotation->w = sM1_SE3_rotation->w;

        ob::RealVectorStateSpace::StateType *sM0_RN = qM0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

        for(uint k = 0; k < M1_dimension-C1_dimension-6; k++){
          sM0_RN->values[k] = sM1_RN->values[k];
        }
        break;
      }
    default:
      {
        std::cout << "cannot merge states" << std::endl;
        exit(0);
      }

  }
}

bool Quotient::SampleGraph(ob::State *q_random)
{
  std::cout << "SampleGraph: NYI" << std::endl;
  exit(0);
}


double Quotient::GetSamplingDensity(){
  return (double)GetNumberOfVertices()/(double)si_->getSpaceMeasure();
}

namespace ompl{
  namespace geometric{
    std::ostream& operator<< (std::ostream& out, const Quotient& qtnt){
      if(qtnt.previous == nullptr){
        out << "M0: ";
        if( qtnt.M1->getStateSpace()->getType() == ob::STATE_SPACE_SE2 ){
          out << "SE(2)" << std::endl;
        }else if( qtnt.M1->getStateSpace()->getType() == ob::STATE_SPACE_SE3 ){
          out << "SE(3)" << std::endl;
        }else if( qtnt.M1->getStateSpace()->getType() == ob::STATE_SPACE_REAL_VECTOR ){
          out << "R^" << qtnt.M1_dimension << std::endl;
        }else{
          out << "unknown" << std::endl;
        }
      }
      switch (qtnt.type) {
        case Quotient::RN_RM:
          {
            out << "M0: R^"<< qtnt.M0_dimension << " | M1: R^" << qtnt.M1_dimension << " | C1: R^" << qtnt.M1_dimension-qtnt.M0_dimension<< std::endl;
            break;
          }
        case Quotient::SE2_R2:
          {
            out << "M0: SE(2) | M1: R^2 | C1: SO(2)" << std::endl;
            break;
          }
        case Quotient::SE3_R3:
          {
            out << "M0: SE(3) | M1: R^3 | C1: SO(3)" << std::endl;
            break;
          }
        case Quotient::SE3RN_SE3:
          {
            out << "M0: SE(3)xR^" << qtnt.M0_dimension << " | M1: SE(3) | C1: R^" <<qtnt.M0_dimension << std::endl;
            break;
          }
        case Quotient::SE3RN_SE3RM:
          {
            out << "M0: SE(3)xR^" << qtnt.M0_dimension << " | M1: SE(3)xR^"<<qtnt.M1_dimension << " | C1: R^" << qtnt.M1_dimension - qtnt.M0_dimension << std::endl;
            break;
          }
        default:
         {
           out << "unknown type: " << qtnt.type << std::endl;
         }

      }
      return out;

    }
  }
}
