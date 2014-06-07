#include <jsonrpc/rpc.h>
#include <iostream>

#include "abstractrpcstubserver.h"

using namespace jsonrpc;
using namespace std;

class MyStubServer : public AbstractRPCStubServer
{
    public:
        MyStubServer();

        std::string ShowMessage(const std::string& friendlyName, const std::string& msg, const int& x, const int& y);
};

MyStubServer::MyStubServer() :
    AbstractRPCStubServer(new HttpServer(8080))
{
}

std::string MyStubServer::ShowMessage(const std::string& friendlyName, 
	const std::string& msg, const int& x, const int& y)
{
	cout<<"Showing message\'"<<msg<<"\' at "<<x<<","<<y<<" named "<<friendlyName<<endl;
	return friendlyName;
}

int main()
{
    MyStubServer s;
    s.StartListening();

    getchar();

    s.StopListening();

    return 0;
}
