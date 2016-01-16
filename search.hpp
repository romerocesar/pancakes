#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <algorithm>		// for swap
#include <iterator>
#include <vector>
#include <climits>
#include <cstring>
#include <deque>
#include <queue>
#include <utility>
#include <fstream>
#include <iostream>

#include "base.hpp"

#define watch(_x_) cout<<#_x_<<":"<<_x_<<endl

#define min(a,b) (a<b)?a:b

using namespace std;

int loadpdb(const char * fname, char ** _pdb);

template<class T>
size_t implicitbfs(T& gp, char * pdb, int N)
{
  cout<<"N:"<<N<<endl;
  memset(pdb,-1,N);
  pdb[gp.hashval()]=0;		// depth of goal is 0 always

  size_t d=0;			// depth
  char b=1;			// new states in this iteration?
  size_t entries=1;		// entries computed
  while(b){
    b=0;
    for (int i = 0; i < N; ++i){ // for each entry
      if(pdb[i]!=d) continue;
    
      T pattern(i);
      size_t op=pattern.opers();
      for (int j = 0; j < T::m_n; ++j){ // at most m_n operators
	if((1<<j)&op){
	  T child(pattern);
	  child.move(j);
	  size_t index=child.hashval();
	  if(pdb[index]==-1) {pdb[index]=d+1; b=1; entries++;}
	}
      }
    }
    if(!b) break; 		// finish if we didn't add new entries
    d++;			// check next depth
  }

  return entries;
}

template<class T>
int pdbgen(T& gp,char * _pdb){
  int gk=gp.hashval();
  _pdb[gk]=0;
  int ent=1;
  queue<pair<T,char> > q;
  //pair<vector<int> *,int> *par = new pair<vector<int> *,int>(s,0);
  q.push(make_pair(gp,0));
  while(!q.empty()){
    //pair<vector<int> *,int> *p(q.front());
    pair<T,char> pd(q.front());
    char dist=pd.second;

    size_t a=pd.first.opers();
    
    for(size_t i=0;i<T::m_n;++i){
      if((1<<i)&a){
	T child(pd.first);
	child.move(i);
	int key=child.hashval();

	#ifdef VERB
	cout<<"move:"<<i<<" "<<child<<" key:"<<key<<" dist:"<<dist+1<<endl;
	#endif

	// debugging
	if(key==1802)
	  int __key=key;

	// don't store the goal
	if(_pdb[key]||key==gk){
	  continue;
	}

	// cout<<(int)tmp[N]<<endl;
	_pdb[key]=dist+1;
	ent++;

	q.push(make_pair(child,dist+1));

      }
    }
    q.pop();
  }
  return ent;
}

// namespace for static variables
namespace hs{
  static deque<short> soln;	// solution
  static uint64 exp;
  static uint64 gen;
}

template<class T>
size_t ida(const T& s, short& t, short g, short prev=0xffff, int ph=-1)
{
  // ph is the parent heuristic value being propagated (only used for bpmx)
  if(s.goal()){
    return 1;
  }
  hs::exp++;
  int ch;			// child heuristic value for bpmx. not used otherwise.
  short _t;
  short f=g;
  size_t newt = INT_MAX;
  int opers=s.opers(prev);
  // 32 is for the number of bits. One for each possible operator
  for(int i=0;i<32;++i){
    if((1<<i)&opers){
      T child(s);
      child.move(i);
      hs::gen++;
      ch=child.h(t-g);

      if(T::bpmx){		// bmpx activated
	ph=max(ph,ch-1);
	ch=max(ph-1,ch);
      }

      f=g+1+ch;
      if(f>t) {newt = min(newt,f); continue;}
      _t=t;
      int res=ida<T>(child,_t,g+1,(prev<<8)|i,ch);
      if(res) {
	hs::soln.push_front((short)i);
	return 1;
      }
      newt = min(newt,_t);

      if(T::bpmx){		// bmpx activated
	ph=max(ph,ch-1);
	ch=max(ph-1,ch);
      }

    }
  }
  t=newt;
  return 0;
}

template <class T>
size_t idastar(T& s)
{
  hs::exp=0;
  hs::gen=0;
  hs::soln.clear();
  short t=s.h();
  while(!ida<T>(s,t,0));
  return t;
}

// Dual idastar material
// pside = <side,sym>. side refers to dual/regular
template<class T>
size_t ddfs(T& s, short& t, short g, pair<bool,bool> pside, vector<vector<short> > prev, int ph=-1)
{
  // ph is the parent heuristic value being propagated (only used for bpmx)
  if(s.goal()){
    return 1;
  }
  hs::exp++;
  if(T::m_jump){			// dual search jump
    s.jump(pside);		// jump updating side and prev
    T::m_jump=0;
  }

  int ch;			// child heuristic value for bpmx. not used otherwise.
  short _t;
  short f=g;
  size_t newt = INT_MAX;
  short lprev=prev[pside.first][pside.second];
  int opers=s.opers(lprev);
  int invop=-1;			// inverse operator
  // 32 is for the number of bits. One for each possible operator
  for(int i=0;i<32;++i){
    if((1<<i)&opers){
      T child(s);
      child.move(i);		// move and keep track of operation

      hs::gen++;
      ch=child.h(t-g);

      // Can we just use this unchanged?
      if(T::bpmx){		// bmpx activated
	ph=max(ph,ch-1);
	ch=max(ph-1,ch);
      }

      f=g+1+ch;
      if(f>t) {newt = min(newt,f); continue;}

      _t=t;
      prev[pside.first][pside.second]=i;
      int res=ddfs<T>(child,_t,g+1,pside,prev,ch);
      if(res) {
	if(pside.first)		// REGULAR side
	  hs::soln.push_front((short)i);
	else			// DUAL side
	  hs::soln.push_back((short)T::inv(i));
	return 1;
      }
      newt = min(newt,_t);

      if(T::bpmx){		// bmpx activated
	ph=max(ph,ch-1);
	ch=max(ph-1,ch);
      }
      // s.move(invop);		// return last move applied
    }
  }
  t=newt;

  return 0;
}

template <class T>
size_t didastar(T& s)
{
  hs::exp=0;
  hs::gen=0;
  short t=s.h();
  char j=T::m_jump;

  // initialize side and prev
  pair<bool,bool> lside=make_pair(1,0); // regular and no sym
  vector<vector<short> > lprev(2,vector<short>(2,-1));	// no prev at this point
  
  while(!ddfs<T>(s,t,0,lside,lprev));
  return t;
}

// symmetry stuff
class symmetry
{
public:
  static int m_n;		// size of the permutation
  int * m_D;
  int * m_E;
  int m_cost;			// c(g',g)
  // default consturctor
  symmetry(int n);
  symmetry(const symmetry& cp);
  virtual ~symmetry();
  // operators
  friend ostream& operator<<(ostream& out, const symmetry& sym);

};

#endif /* _SEARCH_H_ */
