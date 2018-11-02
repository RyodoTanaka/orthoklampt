#include "environment_loader.h"
#include "controller/controller.h"
#include "file_io.h"
#include <boost/filesystem.hpp>

RobotWorld& EnvironmentLoader::GetWorld(){
  return world;
}
RobotWorld* EnvironmentLoader::GetWorldPtr(){
  return &world;
}
Robot* EnvironmentLoader::GetRobotPtr(){
  return world.robots[0];
}
PlannerMultiInput EnvironmentLoader::GetPlannerInput(){
  return pin;
}
PlannerBackendPtr EnvironmentLoader::GetBackendPtr(){
  return _backend;
}

EnvironmentLoader EnvironmentLoader::from_args(int argc, char** argv){
  std::string exec = argv[0];
  std::string file;
  std::vector<std::string> all_args;

  if (argc > 1) {
    file = argv[1];
    all_args.assign(argv + 1, argv + argc);
  }else{
    std::cout << "Usage: <xml world file>" << std::endl;
    exit(0);
  }
  file = util::GetExecFilePath()+"/"+file;
  return EnvironmentLoader(file.c_str());
}
EnvironmentLoader::EnvironmentLoader(const char *file_name_){
  file_name = file_name_;

  std::cout << "[EnvironmentLoader] loading from file " << file_name << std::endl;
  std::cout << std::string(80, '-') << std::endl;

  world.background = GLColor(1,1,1);

  _backend = new PlannerBackend(&world);
  if(!_backend->LoadAndInitSim(file_name.c_str())){
    std::cout << std::string(80, '-') << std::endl;
    std::cout << std::endl;
    std::cout << "ERROR:" << std::endl;
    std::cout << std::endl;
    std::cout << "XML file does not exists or corrupted: "<< file_name << std::endl;
    std::cout << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    exit(0);
  }

  uint Nrobots = world.robots.size();
  if(Nrobots>0){
    name_robot = world.robots[0]->name;
  //################################################################################
  //################################################################################

    if(!(world.robots[0]->joints[0].type == RobotJoint::Floating)){
      std::cout << "First joint of robot should be a free floating joint" << std::endl;
      std::cout << "But actual type is: " << world.robots[0]->joints[0].type << std::endl;
      //exit(0);
      //break;
    }

    // Robot *robot = world.robots[0];

    if(pin.Load(file_name.c_str())){

      uint ridx = pin.inputs.at(0)->robot_idx;
      Robot *robot = world.robots[ridx];
      Vector q_init = pin.inputs.at(0)->q_init;
      Vector q_goal = pin.inputs.at(0)->q_goal;
      Vector dq_init = pin.inputs.at(0)->dq_init;

      for(int i = 0; i < 6; i++){
        robot->qMin[i] = pin.inputs.at(0)->se3min[i];
        robot->qMax[i] = pin.inputs.at(0)->se3max[i];
      }

      pin.inputs.at(0)->qMin = robot->qMin;
      pin.inputs.at(0)->qMax = robot->qMax;
      uint N = robot->q.size();

      uint Ni = pin.inputs.at(0)->q_init.size();
      uint Ng = pin.inputs.at(0)->q_goal.size();
      if(Ni!=N){
        std::cout << std::string(80, '#') << std::endl;
        std::cout << "q_init has " << Ni << " dofs, but robot " << robot->name << " expects " << N << " dofs." << std::endl;
        std::cout << std::string(80, '#') << std::endl;
        exit(1);
      }
      if(Ng!=N){
        std::cout << std::string(80, '#') << std::endl;
        std::cout << "q_goal has " << Ng << " dofs, but robot " << robot->name << " expects " << N << " dofs." << std::endl;
        std::cout << std::string(80, '#') << std::endl;
        exit(1);
      }

      robot->q = q_init;
      robot->dq = dq_init;
      robot->UpdateFrames();

      //set oderobot to planner start pos
      ODERobot *simrobot = _backend->sim.odesim.robot(ridx);
      simrobot->SetConfig(q_init);
      simrobot->SetVelocities(dq_init);

      //set other nested robots
      for(uint k = 0; k < pin.inputs.at(0)->robot_idxs.size(); k++){
        uint ik = pin.inputs.at(0)->robot_idxs.at(k);
        if(ik>=world.robots.size()){
          std::cout << std::string(80, '>') << std::endl;
          std::cout << ">>> [ERROR] Robot with idx " << ik << " does not exists." << std::endl;
          std::cout << std::string(80, '>') << std::endl;
          exit(1);
        }
        Robot *rk= world.robots.at(ik);
        for(int i = 0; i < 6; i++){
          rk->qMin[i] = pin.inputs.at(0)->se3min[i];
          rk->qMax[i] = pin.inputs.at(0)->se3max[i];
        }
      }

      if(pin.inputs.at(0)->kinodynamic){
        LoadController(robot, *pin.inputs.at(0));
      }
      util::SetSimulatedRobot(robot, _backend->sim, pin.inputs.at(0)->q_init, pin.inputs.at(0)->dq_init);
    }else{
      std::cout << std::string(80, '-') << std::endl;
      std::cout << "No Planner Settings. No Planning" << std::endl;
      std::cout << std::string(80, '-') << std::endl;
    }

    for(uint i = 0; i < _backend->sim.odesim.numRobots(); i++){
      ODERobot *simrobot = _backend->sim.odesim.robot(i);
      simrobot->EnableSelfCollisions(true);
    }
  }

  _backend->wrenchfield.Load(file_name.c_str());
  _backend->wrenchfield.print();

}
void EnvironmentLoader::LoadController(Robot *robot, const PlannerInput &pin)
{
  std::cout << "Adding free float driver to robot " << name_robot << std::endl;
  vector<string>* driverNames = &robot->driverNames;
  vector<RobotJointDriver>* drivers = &robot->drivers;

  RobotJointDriver translation[3], rotation[3];

  // Rotation is eulerangles ZYX

  Config uMin = pin.uMin;
  Config uMax = pin.uMax;

  for(int i = 2; i >= 0; i--){
    rotation[i].type = RobotJointDriver::Rotation;
    rotation[i].linkIndices.push_back(i+3);
    rotation[i].linkIndices.push_back(5);
    rotation[i].qmin = -dInf;
    rotation[i].qmax = dInf;
    rotation[i].vmin = -dInf;
    rotation[i].vmax = dInf;
    rotation[i].amin = -dInf;
    rotation[i].amax = dInf;
    // rotation[i].tmin = uMin(i+3);
    // rotation[i].tmax = uMax(i+3);
    rotation[i].tmin = -dInf;
    rotation[i].tmax = +dInf;
    rotation[i].servoP = 0;
    rotation[i].servoI = 0;
    rotation[i].servoD = 0;
    rotation[i].dryFriction = 0;
    rotation[i].viscousFriction = 0;
    drivers->insert(drivers->begin(), rotation[i]);
    std::string dName = "rotation e"+to_string(i);
    driverNames->insert(driverNames->begin(), dName.c_str());
  }
  for(int i = 2; i >= 0; i--){
    translation[i].type = RobotJointDriver::Translation;
    translation[i].linkIndices.push_back(i);
    translation[i].linkIndices.push_back(5);
    translation[i].qmin = -dInf;
    translation[i].qmax = +dInf;
    translation[i].vmin = -dInf;
    translation[i].vmax = dInf;
    translation[i].amin = -dInf;
    translation[i].amax = dInf;
    // translation[i].tmin = uMin(i);
    // translation[i].tmax = uMax(i);
    translation[i].tmin = -dInf;
    translation[i].tmax = +dInf;
    translation[i].servoP = 0;
    translation[i].servoI = 0;
    translation[i].servoD = 0;
    translation[i].dryFriction = 0;
    translation[i].viscousFriction = 0;
    drivers->insert(drivers->begin(), translation[i]);
    std::string dName = "translation e"+to_string(i);
    driverNames->insert(driverNames->begin(), dName.c_str());
  }

  //  nd = (int) drivers.size();
  SmartPointer<RobotController> controller = new ContactStabilityController(*robot);
  RobotControllerFactory::Register("SE3Controller", controller);
  _backend->sim.SetController(0, controller);

  //reinit robot such that drivers are copied to actuators (so we can use them in Command)
  _backend->sim.controlSimulators[0].Init(robot, _backend->sim.odesim.robot(0), _backend->sim.robotControllers[0]);
  controller->Reset();
}

