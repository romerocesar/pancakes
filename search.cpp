#include "search.hpp"

int symmetry::m_n=11;

int loadpdb(const char * fname, char ** _pdb)
{
  ifstream pdb(fname);
  // get file size
  pdb.seekg(0,ios::end);
  size_t n=pdb.tellg();
  if(n==-1)
    return 1;
  *_pdb = new char[n];
  pdb.seekg(0,ios::beg);
  pdb.read(*_pdb,n);
  pdb.close();

  cout<<"read pdb:"<<fname<<" n:"<<n<<endl;

  return pdb.bad();
}

symmetry::symmetry(int n)
{
  m_n=n;
  m_D=new int[n];
  m_E=new int[n];
}

symmetry::symmetry(const symmetry& cp)
{
  m_cost=cp.m_cost;
  m_n=cp.m_n;
  m_D=new int[m_n];
  copy(cp.m_D,cp.m_D+m_n,m_D);
  m_E=new int[m_n];
  copy(cp.m_E,cp.m_E+m_n,m_E);
}

symmetry::~symmetry()
{
  delete [] m_D;
  delete [] m_E;
}

ostream& operator<<(ostream& out, const symmetry& sym)
{
  out<<"D = ";
  copy(sym.m_D,sym.m_D+sym.m_n,ostream_iterator<int>(out," "));
  out<<endl<<"E = ";
  copy(sym.m_E,sym.m_E+sym.m_n,ostream_iterator<int>(out," "));
  out<<endl<<"cost:"<<sym.m_cost<<endl;

  return out;
}
