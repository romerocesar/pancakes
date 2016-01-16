#include "pancake.hpp"

char pancake::m_n=19;		// 11 to 19
char pancake::m_f=7;		// this is what uzi reported
char * pancake::m_pdb=NULL;
uint8 pancake::pdb_type_ = ORD;	// default to the state of the art.

// relative order abstractions
vector<uint32> pancake::hord_masks_;
uint8 pancake::ord_lookups_ = 0;

// duality
char pancake::m_dual=0;
uint8 pancake::dual_lookups_ = 0;
pair<bool,bool> pancake::m_side=make_pair(1,0); // first,second=regular?,sym?
short pancake::m_prev[][2]={{-1,-1},{-1,-1}}; // prev opers used while jumping
char pancake::m_jump=0;
pair<char,char> pancake::m_target=make_pair(0,0);
char pancake::bpmx=0;

// symmetries
char pancake::m_sym=0;
uint8 pancake::sym_lookups_ = 0;
vector<symmetry> pancake::m_syms;
char pancake::m_multi=0;
char * pancake::m_D;
char * pancake::m_E;

char pancake::__verb__=0;

pancake::pancake(const size_t n)
{
  m_n=n;
  m_pancakes=new char[n];
  for (char i = 0; i < n; ++i)
    m_pancakes[i]=i;
}

pancake::pancake(const vector<int>& v)
{
  m_n=v.size();
  m_pancakes=new char[v.size()];
  for (char i = 0; i < m_n; ++i)
      m_pancakes[i]=v[i];
}

pancake::pancake(const pancake& cp)
{
  m_pancakes=new char[cp.m_n];
  copy(cp.m_pancakes,cp.m_pancakes+cp.m_n,m_pancakes);
}

pancake::~pancake()
{
  delete [] m_pancakes;
}

// initializes the pancakes (to simplify creating the initial stage)
void pancake::init(const char * a)
{
  for (int i = 0; i < m_n; ++i)
    m_pancakes[i]=a[i];
}

void pancake::init(const vector<int>& a)
{
  m_n=a.size();
  for (int i = 0; i < m_n; ++i)
    m_pancakes[i]=a[i];
}

int pancake::move(const int i)
{
  for (int j=i; j > i/2 ; --j){
    char aux=m_pancakes[i-j];
    m_pancakes[i-j]=m_pancakes[j];
    m_pancakes[j]=aux;
  }
  return i;
}

int pancake::opers(char prev) const
{
  char p=prev&0x00ff; // previous move
  char pp=(prev&0xff00)>>8; // two moves ago

  int ans=0;
  for (int i = 1; i < m_n; ++i){
    if(i!=p)
      ans|=1<<i;
  }
  return ans;
}

int pancake::goal() const
{
  for (int i = 0; i < m_n; ++i)
    if(m_pancakes[i]!=i)
      return 0;
  
  return 1;
}


void pancake::randomwalk(size_t steps)
{
  srand(time(NULL));		// initialize random seed
  int value;			// random value
  char prev=-1;			// previous move
  for (int i = 0; i < steps; ++i){
    value=rand();
    char oper=(value % (m_n-1))+1;
    if(prev==oper) continue;
    this->move(oper);
    prev=oper;
  }
}

// interactive mode
void pancake::play()
{
  cout<<*this<<endl;
  short j;
  while(cin>>j){
    this->move(j);
    cout<<j<<" -> "<<*this<<endl;
  }
}

// RELATIVE ORDER ABSTRACTIONS.
// Generates a random mask with exactly m_f 1's in the m_n right-most
// bits.
// \return the mask
uint32 pancake::random_mask(uint32 seed)
{
  if(!seed) seed = time(NULL);
  uint32 mask = 0;
  uint32 value;
  uint8 f = m_f;
  while(f>0) {
    srand(seed);
    value = rand() % m_n;
    seed++;
    if (1<<value & mask) continue;
    mask |= (1<<value);
    f--;
  }
  return mask;
}

void pancake::init_masks(uint8 n)
{
  hord_masks_.clear();
  ord_lookups_ = 0;		// add_mask will increase this correctly
  for (int i = 0; i < n; ++i){
    add_mask(random_mask(time(NULL)+i));
  }
}

void pancake::init_masks(const uint32* masks, const uint8 m)
{
  hord_masks_.clear();
  copy(masks, masks+m, back_inserter(hord_masks_)); 
}

// HEURISTIC-RELATED FUNCTIONS. 

// Computes the index to the database assuming a relative order
// abstraction.
// \param mask that indicates which pancakes to use to compute the index.
// \return the index
uint32 pancake::hord_index(uint32 mask) const
{
  uint32 index = 0;    // index to be returned
  uint8 sel[m_f];      // contains the pancakes in mask (in order)
  uint8 sub[m_n];      // arr[i] = sel[i] - sub[sel[i]]
  uint8 arr[m_f];      // on this array we compute the index
  uint8 inv[m_f];      // relevant pancake i is in relative pos inv[i]
  uint8 smaller[m_f];  // # of pancakes smaller i to its left
  uint32 bits = 0;     // to help count smaller and compute the index
  uint8 u = m_f;       // unseen 
  uint8 j = 0, k = 0;  // aux counters

  // first, compute sub by looking only at mask. we scan mask right to
  // left ans add one to k for each zero we see.
  for (int i = 0; i < m_n && j < m_f; ++i){
    if (1<<i & mask) {
      j++;			// we found another useful pancake
      sub[i] = k;
    } else {
      k++;			// how much to substract
    }
  }
  // second, find the important pancakes and build inv and arr
  j = 0;
  for (int i = 0; i < m_n && j < m_f; ++i){
    if (1<<m_pancakes[i] & mask) {
      sel[j] = m_pancakes[i];
      arr[j] = sel[j] - sub[sel[j]];
      inv[arr[j]] = j;
      j++;
    }
  }
  // third, keep track of how many pancakes smaller than i are left of it
  for (int i=0; i < m_f; i++) {	// for each pancake
    bits|=(1<<(m_f-arr[i]-1));	// mark relevant pancake i as seen, left to right
    smaller[arr[i]]=__builtin_popcount((bits>>(m_f-arr[i])));
  }

  // finally, compute the index
  for(int i = 0; i < m_f; ++i){	// for each fringe pancake
    index=index*u+inv[i]-smaller[i];
    u--;
  }

  return index;
}

int pancake::hashval(char first) const
{
  if(__verb__){
    cout<<"hashval{ ";
    cout<<*this;
    cout<<" f:"<<(int)m_f<<" first:"<<(int)first;
  }

  int index=0;			// hash index
  int bits=0;			// bitstring to keep track and count smaller faster
  short smaller[m_f];		// # of pancakes smaller than i to its left
  short u=m_n;                  // unassigned or available pos
  char inv[m_n];		// pancake i is in position inv[i]

  memset(inv,m_n,m_n);		// initialize with dummy values

  for(int i = 0; i < m_n; ++i){	// compute inv[]
    if(m_pancakes[i]==m_n) 	// ignore dummy pancakes (to build pdbs)
      continue;
    inv[m_pancakes[i]]=i;
  }

  for(int i=0;i<m_n;i++){	// for each pancake
    if(m_pancakes[i]==m_n)	// ignore dummies (in pdbs)
      continue;	 
    bits|=(1<<(m_n-m_pancakes[i]-1));	// mark pancake i as seen
    if(m_pancakes[i]>=m_f)      // skip pancakes not used in the index
      continue; 
    smaller[m_pancakes[i]]=__builtin_popcount((bits>>(m_n-m_pancakes[i])));
  }

  // finally compute the index
  for(int i = 0; i < m_f; ++i){	// for each fringe pancake
    index=index*u+inv[i]-smaller[i];
    u--;
  }
  
  if(__verb__){
    cout<<" return:"<<index;
    cout<<" }"<<endl;
  }
  // delete [] _pancakes;
  return index;
}

// Compute an index to the hauristic function.
// \param mask specifies the pancakes to consider if using a relative
// order abstraction. Ignored if using a object location abstraction.
uint32 pancake::hindex(uint32 mask) const
{
  if(pdb_type_ == ORD) return hord_index(mask);
  else return hashval();
}

// Compute a heuristic estimate. TODO: reimplement jumpsa and write a
// test for the new way of combining lookups.
int pancake::h(short H)
{
  int ans = 0;			// to be returned
  int val = 0;			// aux variable for heuristic values

  m_jump=0;			// by default, don't jump
  m_target.first=0;		// reset target cause we don't have one yet
  m_target.second=0;

  // no pdb
  if(!m_f || m_pdb==NULL)
    return 0;

  uint32 mask = 0;
  uint8 cost = 0;
  char ol = (pdb_type_ == OL)?1:0; // at least one lookup when using OL
  for (int d = 0; d <= dual_lookups_; ++d){
    if (d) this->dual();
    for (int y = 0; y <= sym_lookups_; ++y){
      if (y) {this->sym(y-1); cost = m_syms[y-1].m_cost;}
      else cost = 0;
      for (int o = 0; o < ord_lookups_ + ol; ++o){
	// watch(o); watch(hord_masks_[o]);
  	mask = (pdb_type_ == ORD)?hord_masks_[o]:0;
	// watch(mask);
  	// TODO: mark necessary jumps for DIDA*
	// watch(hindex(mask));
  	val = m_pdb[hindex(mask)] - cost;
  	ans = max(ans, val);
  	if (ans > H) break;
      }
      // revert sym
      if(y) this->sym(y-1);
      if (ans > H) break;
    }
    // revert dual
    if(d) this->dual();
    if (ans > H) break;
  }

  return ans;			
}

// symmetry stuff
void pancake::dual()
{
  char sd[m_n];
  for (int i = 0; i < m_n; ++i)
    sd[m_pancakes[i]]=i;
  memcpy(m_pancakes,sd,m_n);
}

// jump to target destionation. This is used to perform a jump during
// searching like DIDA* does. Updates pside to reflect on which
// side/sym the resulting state is. NOTE: JTL should be used only with
// one symmetry; more than that results most likely in incorrect
// behaviour
void pancake::jump(pair<bool,bool> & pside)
{
  // uncomment for JIL
  // char first=0;			// JIL. jumping to renumbering is not an option here
  // char second=m_target.second&1; // JIL. don't jump to sym either
  // uncomment for JTL
  char first=m_target.first; // JTL. jump to sym i
  char second=m_target.second; // JTL. where to jump?

#ifndef NDEBUG
  if(!(m_target.second&1)){	// bug in JIL
    cout<<*this<<endl;
    watch(this->hashval());
    watch(pside.first);
    watcht(second,int);
    watcht(m_target.second,int);
  }
  assert(m_target.second==m_jump);
#endif


  if(second & 2) {
    this->sym(first);		// jump to symmetry
    pside.second=!pside.second; // update side. move to sym()?
  }
  if(second & 1) {
    this->dual();		// jump to dual
    pside.first=!pside.first; // update side. move to dual()?
  }

  // reset jumping variables
  m_jump=0;
  m_target.first=0;
  m_target.second=0;

}

// computes the symmetric state according to sym
void pancake::sym(int sym)
{
  char ans[m_n];
  for (int i = 0; i < m_n; ++i){
    ans[i]=m_syms[sym].m_E[m_pancakes[m_syms[sym].m_D[i]]];
  }
  memcpy(m_pancakes,ans,m_n);
}

//  I don't know how to automate this process for the pancake problem
// yet. I'll write a single symmetry for now and try it out. TODO: try
// other symmetries with different g' and do many lookups
void pancake::init_syms(vector<int> syms)
{
  unsigned n=syms.size();
  sym_lookups_ = n;
  // TODO: remove m_sym and m_multi once the new way of combining
  // lookups is stable.
  m_sym=1;			// activate the symmetry
  m_multi=n;
  if(__verb__) cout<<"init_sym:"<<"m_sym:"<<(int)m_sym<<endl;

  // These symmetries don't garantee the distance to the goal, so they
  // can only be used as a bound. H(s) := max { h(s), h(s')-c(g',g)}.

  for(int s = 0; s<n; ++s){
    symmetry _sym(m_n);
    switch(syms[s]){
    case 666:		    // this one doesn't seem to work
      // g'= 2 3 4 5 1 => c(g',g)=2 // symcost=2
      // for (int i = 0; i <m_n; ++i){
      // 	m_D[i]=i;
      // 	m_E[i]=(i+1)%m_n;			
      // }
      // m_symcost=2;
      break;
    case 3:
      // g'= 3 2 1 4 5 => c(g',g)=1 // symcost=1
      for (int i = 0; i <m_n; ++i){
	_sym.m_D[i]=i;
	_sym.m_E[i]=m_n-i-3;
      }
      _sym.m_E[m_n-1]=m_n-1;
      _sym.m_E[m_n-2]=m_n-2;
      _sym.m_cost=1;
      break;
    case 2:
      // g'= 4 3 2 1 5 => c(g',g)=1 // symcost=1
      for (int i = 0; i <m_n; ++i){
	_sym.m_D[i]=i;
	_sym.m_E[i]=m_n-i-2;
      }
      _sym.m_E[m_n-1]=m_n-1;
      _sym.m_cost=1;
      break;
    default:
    case 1:
      // g'= 5 4 3 2 1 => c(g',g)=1  // symcost=1
      for (int i = 0; i <m_n; ++i){
	_sym.m_D[i]=i;
	_sym.m_E[i]=m_n-1-i;			
      }
      _sym.m_cost=1;
      break;
    }    
    _sym.m_n=m_n;
    m_syms.push_back(_sym);
  }

}

// seters
void pancake::set_pdb(char* pdb, const uint8 fringe, const uint8 type)
{
  m_pdb=pdb;
  m_f=fringe;
  pdb_type_ = type;
}

void pancake::set_pdb(char* pdb, const uint8 fringe, const string type)
{
  if (type == "ord") set_pdb(pdb, fringe, ORD);
  else if (type == "ol") set_pdb(pdb, fringe, OL);
}

// operators
ostream& operator<<(ostream& out, const pancake& p)
{
  for (int i = 0; i < pancake::m_n; ++i)
    out<<(int)p[i]<<' ';

 return out;
}

char pancake::operator[](size_t i) const
{
  return m_pancakes[i];
}

// Pattern space begins here
// TODO: uzi used the f largest pancakes. Does this make a difference?
pcpattern::pcpattern(char N, char f):pancake(N)
{
  m_f=f;
  for (int i = 0; i < N; ++i)
    m_pancakes[i]=i<f?i:N;
}

// unrank constructor. Creates a pancake pattern n-f out of the ranked
// state r. This version assumes state[0]=0 always... I think this is
// no longer true because the first loop starts with i=0 and actually
// hashval al ready hashes a state where state[0]=0. This hasn't been
// tested for the pancake problem (code comes from topspin). It has to
// be modified to not make the assumption that state[0]=0 always. This
// constructor is onlynecessary to build a PDB using implicitBFS.
pcpattern::pcpattern(const unsigned r):pancake(m_n)
{
  memset(m_pancakes,m_n,m_n);

  unsigned long long fac=1;	// fac = number of possible configurations of the fringe
  for (int i = 0; i < m_f; ++i)	
    fac*=(m_n-i);  		// all possible configurations of the fringe pancakes
  
  unsigned p=0;			// temp var with the relative position
  char j;			// utility var to find the position
  for (int i = 0; i < m_f; ++i){ // for each token
    fac=fac/(m_n-i);		   // n-i = possible positions for i
    p=r/fac;
    p=p%(m_n-i);
    j=0;
    while(m_pancakes[j]!=m_n || p) { // find the pth empty spot
      if(m_pancakes[j]==m_n) p--;
      j++;
    } 
    m_pancakes[j]=i;
  }
}


pcpattern::pcpattern(const pcpattern& cp):pancake(cp) {}

