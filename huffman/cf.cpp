#include <bits/stdc++.h>

using namespace std;

void solve(){
    long long n,k;
    cin>>n>>k;
    vector < long long > a(n);
    map < long long,long long > freq;
    for(long long i=0;i<n;i++){
        cin>>a[i];
        freq[a[i]]++;
    }
    for(long long i=0;i<n;i++){
        long long s = freq.size();
        
    }


}

int main(){
    int t;
    cin>>t;
    while(t--){
        solve();
    }
}