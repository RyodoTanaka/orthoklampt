#include <stdio.h>
#include <ctime>
#include <KrisLibrary/GLdraw/GL.h>
#include <KrisLibrary/GLdraw/drawextra.h>

#include <Contact/Grasp.h> //class Grasp
#include <Planning/ContactCSpace.h>
#include <Modeling/MultiPath.h>
#include <Planning/PlannerSettings.h>
#include <KrisLibrary/planning/AnyMotionPlanner.h>
#include <Modeling/DynamicPath.h>
#include <Modeling/Paths.h>
#include <Control/PathController.h>

#include "util.h"
#include "info.h"
#include "controller.h"
#include "gui.h"
#include "environment_loader.h"
#include "planner/planner_ompl.h"
#include "planner/planner_ompl_irreducible.h"

int main(int argc,const char** argv) {

  std::string file = "data/snake_turbine_irreducible.xml";
  EnvironmentLoader env = EnvironmentLoader(file.c_str());
  MotionPlannerOMPLIrreducible planner(env.GetWorldPtr());

  PlannerInput pin = env.GetPlannerInput();
  std::cout << pin << std::endl;
  Config p_init = pin.q_init;
  Config p_goal = pin.q_goal;

  if(planner.solve(p_init, p_goal)){
    std::vector<Config> keyframes = planner.GetKeyframes();
    env.GetBackendPtr()->AddPath(keyframes);
  }

  env.GetBackendPtr()->VisualizeStartGoal(p_init, p_goal);
  env.GetBackendPtr()->VisualizePlannerTree(planner.GetTree());
  env.GetBackendPtr()->HidePlannerTree();
  env.GetBackendPtr()->Save("snake_turbine_irreducible.xml");

  GLUIForceFieldGUI gui(env.GetBackendPtr(),env.GetWorldPtr());
  gui.SetWindowTitle("SweptVolumePath");
  gui.Run();

  return 0;
}
