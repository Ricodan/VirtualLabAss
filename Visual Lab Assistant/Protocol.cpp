#include "tinyfsm.hpp"
#include "Protocol.h"
#include <iostream>

using namespace std;


Protocol::Protocol()
{

}
class PreStreak;
class PostStreak;

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
		
	};
};

class InterClean : public Protocol
{
	void react(LoopDippedInVial const & e) override {
		cout << "InoculationLoop id is now Contaminated ";
		cout << "Dipped in Vial";

		transit<PreStreak>();

	};

	void react(Stow const & e) override {
		cout << "Beginning with a new Petri dish";


		transit<Clean>();

	};

	void react(Streak const & e) override {
		cout << "InoculationLoop id is now Contaminated ";
		cout << "Streaked";

		transit<PostStreak>();

	};
};

class PostStreak : public Protocol
{
	void react(LoopSterilize const & e) override {
		cout << "InoculationLoop id is now Sterilized ";
		cout << "Sterilization";

		transit<InterClean>();

	};
};

class PreStreak : public Protocol
{
	void react(Streak const & e) override {
		cout << "InoculationLoop id is now contaminated ";
		cout << "Streak";

		transit<PostStreak>();
	
	};
};





class Spoiled : public Protocol
{

};




//Protocol::~Protocol()
//{
//
//}
