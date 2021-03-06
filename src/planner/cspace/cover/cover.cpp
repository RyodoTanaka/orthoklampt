#include "cover.h"

using namespace cover;
Cover::Cover()
{
}

bool Cover::IsInsideCover(ob::State *s)
{
  for(uint k = 0; k < opensets.size(); k++)
  {
    if(opensets.at(k)->IsInside(s))
    {
      return true;
    }
  }
  return false;
}

void Cover::Clear()
{
  opensets.clear();
}

void Cover::AddOpenSet( OpenSet* set )
{
  for(uint k = 0; k < opensets.size(); k++){
    if(set->IsSubsetOf(opensets.at(k))){
      rejected_openset++;
      return;
    }
  }
  opensets.push_back(set);
}
void Cover::AddStartOpenSet( OpenSet* set )
{
  AddOpenSet(set);
  startSet = opensets.size()-1;
}
void Cover::AddGoalOpenSet( OpenSet* set )
{
  AddOpenSet(set);
  goalSet = opensets.size()-1;
}

std::vector<OpenSet*> Cover::GetCover() const
{
  return opensets;
}
int Cover::GetStartSetIndex() const
{
  return startSet;
}
int Cover::GetGoalSetIndex() const
{
  return goalSet;
}

namespace cover{
  std::ostream& operator<< (std::ostream& out, const Cover& cvr)
  {
    std::cout << "Cover contains " << cvr.opensets.size() << " opensets." << std::endl;
    std::cout << "    rejected opensets: " << cvr.rejected_openset << std::endl;
    // for(uint k = 0; k < cvr.opensets.size(); k++){
    //   out << *cvr.opensets.at(k) << std::endl;
    // }
    return out;
  }
}
