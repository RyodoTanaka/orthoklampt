#include "common.h"
#include "qng.h"
#include "elements/plannerdata_vertex_annotated.h"
#include "planner/cspace/validitychecker/validity_checker_ompl.h"
#include <limits>
#include <boost/graph/astar_search.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/property_map/transform_value_property_map.hpp>
#include <boost/foreach.hpp>

#define foreach BOOST_FOREACH

using namespace ompl::geometric;

QNG::QNG(const base::SpaceInformationPtr &si, Quotient *parent ): BaseT(si, parent)
{
  setName("QNG"+std::to_string(id));
  if(parent != nullptr) verbose = 1;
}

QNG::~QNG(void)
{
}

std::vector<QNG::Configuration*> QNG::ExpandNeighborhood(Configuration *q_current, const int M_samples)
{
  std::vector<Configuration*> q_children;
  if(q_current->parent_neighbor == nullptr)
  {
    //if no parent neighbor is available, just sample on the neighborhood boundary
    for(int k = 0; k < M_samples; k++)
    {
      Configuration *q_random = new Configuration(Q1);
      SampleNeighborhoodBoundary(q_random, q_current);
      if(ComputeNeighborhood(q_random))
      {
        q_children.push_back(q_random);
      }
    }
  }else{
    //############################################################################
    // Get projected sample
    //############################################################################
    Configuration *q_proj = new Configuration(Q1);
    Configuration *q_last = q_current->parent_neighbor;
    const double radius_current = q_current->GetRadius();
    const double radius_last = q_last->GetRadius();
    const double step_size = (radius_last+radius_current)/radius_last;
    // Q1->getStateSpace()->interpolate(q_last->state, q_current->state, step_size, q_proj->state);

    Interpolate(q_last, q_current, step_size, q_proj);
    if(verbose>2) CheckConfigurationIsOnBoundary(q_proj, q_current);

    //############################################################################
    // (1) q_proj is feasible: 
    //    (1a) neighborhood is bigger than current neighborhood -> return q_proj
    //    (1b) otherwise compute other neighborhoods
    // (2) q_proj is infeasible:
    //    (2a) sample broadly
    //############################################################################
    if(ComputeNeighborhood(q_proj))
    {
      q_proj->parent_neighbor = q_current;
      q_children.push_back(q_proj);

      const double radius_proj = q_proj->GetRadius();
      const double radius_ratio = radius_proj / radius_current;
      if(radius_ratio < 1)
      {
        for(int k = 0; k < M_samples-1; k++)
        {
          Configuration *q_k = new Configuration(Q1);
          //should be intersected with the cover if we are on the upper level
          //space.

          Q1_sampler->sampleUniformNear(q_k->state, q_proj->state, 0.5*radius_current);

          std::cout << "interpolate qng q_k" << std::endl;
          Interpolate(q_current, q_k);
          std::cout << "done interpolate qng q_k" << std::endl;
          if(verbose>2) CheckConfigurationIsOnBoundary(q_k, q_current);

          if(ComputeNeighborhood(q_k))
          {
            if(q_k->GetRadius() > 0.1*q_current->GetRadius()){
              q_children.push_back(q_k);
            }
          }
        }
      }

    }else{
      for(int k = 0; k < M_samples; k++)
      {
        Configuration *q_k = new Configuration(Q1);
        SampleNeighborhoodBoundaryHalfBall(q_k, q_current);

        if(ComputeNeighborhood(q_k))
        {
          if(q_k->GetRadius() > 0.1*q_current->GetRadius()){
            q_children.push_back(q_k);
          }
        }
      }
    }
  }

  for(uint k = 0; k < q_children.size(); k++){
    q_children.at(k)->parent_neighbor = q_current;
  }

  return q_children;
}

QNG::Configuration* QNG::SampleUniformQuotientCover(ob::State *state) 
{
  double r = rng_.uniform01();
  Configuration *q_coset = nullptr;
  if(r<shortestPathBias)
  {
    if(shortest_path_start_goal_necessary_vertices.empty())
    {
      std::cout << "[WARNING] shortest path does not have any necessary vertices! -- should be detected on CS" << std::endl;
      exit(0);
    }
    int k = rng_.uniformInt(0, shortest_path_start_goal_necessary_vertices.size()-1);
    const Vertex vk = shortest_path_start_goal_necessary_vertices.at(k);
    q_coset = graph[vk];
    Q1_sampler->sampleUniformNear(state, q_coset->state, q_coset->GetRadius());
  }else{
    q_coset = pdf_necessary_configurations.sample(rng_.uniform01());
    Q1_sampler->sampleUniformNear(state, q_coset->state, q_coset->GetRadius());
  }
  return q_coset;
}
