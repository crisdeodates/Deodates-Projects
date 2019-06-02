//--------------------------------------------------------------------------//
//-- PROJECT Autonomous Fire Response & Monitoring Unit                          
//                                                                               
//-- PROGRAM GOALS                                                               
//-- 1. Control a ground vehicle autonomously or manually using mobile app       
//-- 2. Detect nasty things and provide warnings                                 
//                                                                               
//-- Programmers: Cris Thomas, Jiss Joseph Thomas                                
//-- References: Donkey Car                   
//-- Acknowledgements: Julian Oes, Shakthi Prashanth                             
//--------------------------------------------------------------------------//
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
using namespace mavsdk;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::seconds;
#define ERROR_CONSOLE_TEXT "\033[31m" // Turn text on console red
#define TELEMETRY_CONSOLE_TEXT "\033[34m" // Turn text on console blue
#define NORMAL_CONSOLE_TEXT "\033[0m" // Restore normal console colour
// Handles Action's result
inline void action_error_exit(Action::Result result, const std::string& message)
{
if (result != Action::Result::SUCCESS) {
std::cerr << ERROR_CONSOLE_TEXT << message << Action::result_str(result)
<< NORMAL_CONSOLE_TEXT << std::endl;
exit(EXIT_FAILURE);
}
}
// Handles Offboard's result
inline void offboard_error_exit(Offboard::Result result, const std::string& message)
{
if (result != Offboard::Result::SUCCESS) {
std::cerr << ERROR_CONSOLE_TEXT << message << Offboard::result_str(result)
<< NORMAL_CONSOLE_TEXT << std::endl;
exit(EXIT_FAILURE);
}
}
// Handles connection result
inline void connection_error_exit(ConnectionResult result, const std::string& message)
{
if (result != ConnectionResult::SUCCESS) {
std::cerr << ERROR_CONSOLE_TEXT << message << connection_result_str(result)
<< NORMAL_CONSOLE_TEXT << std::endl;
exit(EXIT_FAILURE);
}
}
// Logs during Offboard control
inline void offboard_log(const std::string& offb_mode, const std::string msg)
{
std::cout << "[" << offb_mode << "] " << msg << std::endl;
}
/**
* Does Offboard control using NED co-ordinates.
*
* returns true if everything went well in Offboard control, exits with a log otherwise.
*/
bool offb_ctrl_ned(std::shared_ptr<mavsdk::Offboard> offboard)
{
const std::string offb_mode = "NED";
// Send it once before starting offboard, otherwise it will be rejected.
offboard->set_velocity_ned({0.0f, 0.0f, 0.0f, 0.0f});
Offboard::Result offboard_result = offboard->start();
offboard_error_exit(offboard_result, "Offboard start failed");
offboard_log(offb_mode, "Offboard started");
offboard_log(offb_mode, "Turn to face East");
offboard->set_velocity_ned({0.0f, 0.0f, 0.0f, 90.0f});
sleep_for(seconds(1)); // Let yaw settle.
offboard_log(offb_mode, "Go up 2 m/s, turn to face South");
offboard->set_velocity_ned({0.0f, 0.0f, -2.0f, 180.0f});
sleep_for(seconds(4));
// Now, stop offboard mode.
offboard_result = offboard->stop();
offboard_error_exit(offboard_result, "Offboard stop failed: ");
offboard_log(offb_mode, "Offboard stopped");
return true;
}
/**
* Does Offboard control using body co-ordinates.
*
* returns true if everything went well in Offboard control, exits with a log otherwise.
*/
bool offb_ctrl_body(std::shared_ptr<mavsdk::Offboard> offboard)
{
const std::string offb_mode = "BODY";
// Send it once before starting offboard, otherwise it will be rejected.
offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
Offboard::Result offboard_result = offboard->start();
offboard_error_exit(offboard_result, "Offboard start failed: ");
offboard_log(offb_mode, "Offboard started");
offboard_log(offb_mode, "Wait for a bit");
offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
sleep_for(seconds(2));
offboard_log(offb_mode, "Fly a circle");
offboard->set_velocity_body({5.0f, 0.0f, 0.0f, 30.0f});
sleep_for(seconds(15));
offboard_log(offb_mode, "Wait for a bit");
offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
sleep_for(seconds(8));
offboard_result = offboard->stop();
offboard_error_exit(offboard_result, "Offboard stop failed: ");
offboard_log(offb_mode, "Offboard stopped");
return true;
}
/**
* Does Offboard control using attitude commands.
*
* returns true if everything went well in Offboard control, exits with a log otherwise.
*/
bool offb_ctrl_attitude(std::shared_ptr<mavsdk::Offboard> offboard)
{
const std::string offb_mode = "ATTITUDE";
// Send it once before starting offboard, otherwise it will be rejected.
offboard->set_attitude({30.0f, 0.0f, 0.0f, 0.6f});
Offboard::Result offboard_result = offboard->start();
offboard_error_exit(offboard_result, "Offboard start failed");
offboard_log(offb_mode, "Offboard started");
offboard_log(offb_mode, "ROLL 30");
offboard->set_attitude({30.0f, 0.0f, 0.0f, 0.6f});
sleep_for(seconds(2)); // rolling
offboard_log(offb_mode, "ROLL -30");
offboard->set_attitude({-30.0f, 0.0f, 0.0f, 0.6f});
sleep_for(seconds(2)); // Let yaw settle.
offboard_log(offb_mode, "ROLL 0");
offboard->set_attitude({0.0f, 0.0f, 0.0f, 0.6f});
sleep_for(seconds(2)); // Let yaw settle.
// Now, stop offboard mode.
offboard_result = offboard->stop();
offboard_error_exit(offboard_result, "Offboard stop failed: ");
offboard_log(offb_mode, "Offboard stopped");
return true;
}
void usage(std::string bin_name)
{
std::cout << NORMAL_CONSOLE_TEXT << "Usage : " << bin_name << " <connection_url>" << std::endl
<< "Connection URL format should be :" << std::endl
<< " For TCP : tcp://[server_host][:server_port]" << std::endl
<< " For UDP : udp://[bind_host][:bind_port]" << std::endl
<< " For Serial : serial:///path/to/serial/dev[:baudrate]" << std::endl
<< "For example, to connect to the simulator use URL: udp://:14540" << std::endl;
}
int main(int argc, char** argv)
{
Mavsdk dc;
std::string connection_url;
ConnectionResult connection_result;
if (argc == 2) {
connection_url = argv[1];
connection_result = dc.add_any_connection(connection_url);
} else {
usage(argv[0]);
return 1;
}
if (connection_result != ConnectionResult::SUCCESS) {
std::cout << ERROR_CONSOLE_TEXT
<< "Connection failed: " << connection_result_str(connection_result)
<< NORMAL_CONSOLE_TEXT << std::endl;
return 1;
}
// Wait for the system to connect via heartbeat
while (!dc.is_connected()) {
std::cout << "Wait for system to connect via heartbeat" << std::endl;
sleep_for(seconds(1));
}
// System got discovered.
System& system = dc.system();
auto action = std::make_shared<Action>(system);
auto offboard = std::make_shared<Offboard>(system);
auto telemetry = std::make_shared<Telemetry>(system);
while (!telemetry->health_all_ok()) {
std::cout << "Waiting for system to be ready" << std::endl;
sleep_for(seconds(1));
}
std::cout << "System is ready" << std::endl;
Action::Result arm_result = action->arm();
action_error_exit(arm_result, "Arming failed");
std::cout << "Armed" << std::endl;
Action::Result takeoff_result = action->takeoff();
action_error_exit(takeoff_result, "Takeoff failed");
std::cout << "In Air..." << std::endl;
sleep_for(seconds(5));
//  using attitude control
bool ret = offb_ctrl_attitude(offboard);
if (ret == false) {
return EXIT_FAILURE;
}
//  using local NED co-ordinates
ret = offb_ctrl_ned(offboard);
if (ret == false) {
return EXIT_FAILURE;
}
//  using body co-ordinates
ret = offb_ctrl_body(offboard);
if (ret == false) {
return EXIT_FAILURE;
}
const Action::Result land_result = action->land();
action_error_exit(land_result, "Landing failed");
// Check if vehicle is still in air
while (telemetry->in_air()) {
std::cout << "Vehicle is landing..." << std::endl;
sleep_for(seconds(1));
}
std::cout << "Landed!" << std::endl;
sleep_for(seconds(3));
std::cout << "Finished..." << std::endl;
return EXIT_SUCCESS;
}