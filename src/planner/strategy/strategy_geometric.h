#pragma once
#include "planner/strategy/strategy.h"
#include <omplapp/config.h>

namespace ob = ompl::base;
namespace oc = ompl::control;
namespace og = ompl::geometric;
namespace oa = ompl::app;
namespace ot = ompl::tools;

class StrategyGeometricMultiLevel: public Strategy{
  public:
    virtual void Plan( StrategyOutput &output) override;
    virtual void Step( StrategyOutput &output) override;
    virtual void Init( const StrategyInput &input) override;
    virtual void Clear() override;

    StrategyGeometricMultiLevel();

    ob::PlannerPtr GetPlanner(std::string algorithm,
        std::vector<ob::SpaceInformationPtr> si_vec, 
        std::vector<ob::ProblemDefinitionPtr> pdef_vec);
    void RunBenchmark(
        const StrategyInput& input,
        std::vector<ob::SpaceInformationPtr> si_vec, 
        std::vector<ob::ProblemDefinitionPtr> pdef_vec);

    template<class T_Algorithm>
    ob::PlannerPtr GetSharedMultiChartPtr( 
        std::vector<ob::SpaceInformationPtr> si_vec, 
        std::vector<ob::ProblemDefinitionPtr> pdef_vec);
    template<class T_Algorithm>
    ob::PlannerPtr GetSharedMultiQuotientPtr( 
        std::vector<ob::SpaceInformationPtr> si_vec, 
        std::vector<ob::ProblemDefinitionPtr> pdef_vec);
};

