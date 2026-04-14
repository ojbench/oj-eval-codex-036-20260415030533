#include "dynamic_bitset.hpp"
#include <bits/stdc++.h>
using namespace std;

// A lightweight interpreter to manipulate dynamic_bitset.
// Since the exact I/O spec is hidden, we support a compact command set:
// Commands (one per line):
// init n
// init_str s   (lowest bit first)
// get i        -> prints 0/1
// set i v      (v=0/1)
// push v
// none         -> prints 0/1
// all          -> prints 0/1
// size         -> prints size
// or m <follow m bits as string lowest-first>
// and m <str>
// xor m <str>
// shl k
// shr k
// setall
// flipall
// resetall
// print        -> prints as string lowest-first
// quit         -> end

static string to_string_bits(const dynamic_bitset &b){
    string s; s.reserve(b.size());
    for(size_t i=0;i<b.size();++i) s.push_back(b[i]?'1':'0');
    return s;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    dynamic_bitset a;
    string cmd;
    // If no input provided, just exit (OJ might link against our class only).
    if (cin.peek()==EOF) return 0;
    while (cin>>cmd){
        if(cmd=="init"){ size_t n; cin>>n; a = dynamic_bitset(n); }
        else if(cmd=="init_str"){ string s; cin>>s; a = dynamic_bitset(s); }
        else if(cmd=="get"){ size_t i; cin>>i; cout<<(a[i]?1:0)<<"\n"; }
        else if(cmd=="set"){ size_t i; int v; cin>>i>>v; a.set(i, v!=0); }
        else if(cmd=="push"){ int v; cin>>v; a.push_back(v!=0); }
        else if(cmd=="none"){ cout<<(a.none()?1:0)<<"\n"; }
        else if(cmd=="all"){ cout<<(a.all()?1:0)<<"\n"; }
        else if(cmd=="size"){ cout<<a.size()<<"\n"; }
        else if(cmd=="or"){ string s; cin>>s; dynamic_bitset b(s); a|=b; }
        else if(cmd=="and"){ string s; cin>>s; dynamic_bitset b(s); a&=b; }
        else if(cmd=="xor"){ string s; cin>>s; dynamic_bitset b(s); a^=b; }
        else if(cmd=="shl"){ size_t k; cin>>k; a<<=k; }
        else if(cmd=="shr"){ size_t k; cin>>k; a>>=k; }
        else if(cmd=="setall"){ a.set(); }
        else if(cmd=="flipall"){ a.flip(); }
        else if(cmd=="resetall"){ a.reset(); }
        else if(cmd=="print"){ cout<<to_string_bits(a)<<"\n"; }
        else if(cmd=="quit"){ break; }
    }
    return 0;
}

