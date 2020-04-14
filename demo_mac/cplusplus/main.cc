#include <stdio.h>
#include "../../src/apollo_engine_impl.h"

int main(int argc, char **argv){
    
    webrtc::ApolloEngineImpl apolloEngineTest;
    //apolloEngineTest.Initialize();
    apolloEngineTest.CreateSocketClient();
    
    while(1){
        sleep(10);
    }
    
    return 0;
}
