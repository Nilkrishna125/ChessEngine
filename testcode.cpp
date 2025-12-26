#include<iostream>
using namespace std;
int main(){
    int n;
    cin>>n;
    for(int i=0; i<n; i++){
        int x[4];
        for(int j=0;j<4;j++)cin>>x[j];
        int a,b,c,d;
        a=x[0];
        b=x[1];
        c=x[2];
        d=x[3];
        if(a>=b && c>=b)cout<<"Gellyfish"<<endl;
        else if(b>a && a>=d)cout<<"Gellyfish"<<endl;
        else cout<<"Flower"<<endl;
    }
    return 0;
}