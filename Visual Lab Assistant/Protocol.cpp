#include "Protocol.h"
#include <iostream>

using namespace std;


Protocol::Protocol()
{

}



// ----------------------------------------------------------------------------
// Transition functions
//

static void CallMaintenance() {
	cout << "*** calling maintenance ***" << endl;
}

static void CallFirefighters() {
	cout << "*** calling firefighters ***" << endl;
}

// ----------------------------------------------------------------------------
// State: Cleam
//

class Clean : public Protocol
{
	void react(LoopDippedInVial const & e) override {
		cout << "InoculationLoop id is now contaminated";

		transit<PreStreak>();
		//change state
		// This is where I left off.
	};
};

class PreStreak : public Protocol
{

};

class PostStreak : public Protocol
{

};

class InterClean : public Protocol
{

};

class Spoiled : public Protocol
{

};




Protocol::~Protocol()
{
}
