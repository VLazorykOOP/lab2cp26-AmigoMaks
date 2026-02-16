#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
#include <cmath>
#include <chrono>
#include <string>

using namespace std;

mutex cout_mutex;

const double SIM_WIDTH = 1000.0;
const double SIM_HEIGHT = 1000.0;

enum class VehicleType { TRUCK, CAR };

struct Point {
    double x, y;
};

double getRandomDouble(double min, double max) {
    static thread_local mt19937 generator(random_device{}());
    uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

class Vehicle {
private:
    int id;
    VehicleType type;
    Point currentPos;
    Point destination;
    double speed;
    bool hasArrived;

    double getDistance(Point p1, Point p2) {
        return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
    }

    void safePrint(const string& message) {
        lock_guard<mutex> lock(cout_mutex);
        string typeStr = (type == VehicleType::TRUCK) ? "[Truck " : "[Car ";
        cout << typeStr << id << "] " << message << "\n";
    }

public:
    Vehicle(int id, VehicleType type, double speed) 
        : id(id), type(type), speed(speed), hasArrived(false) 
    {
        currentPos.x = getRandomDouble(0.0, SIM_WIDTH);
        currentPos.y = getRandomDouble(0.0, SIM_HEIGHT);

        if (type == VehicleType::TRUCK) {
            if (currentPos.x <= SIM_WIDTH / 2.0 && currentPos.y <= SIM_HEIGHT / 2.0) {
                destination = currentPos;
                hasArrived = true;
            } else {
                destination.x = getRandomDouble(0.0, SIM_WIDTH / 2.0);
                destination.y = getRandomDouble(0.0, SIM_HEIGHT / 2.0);
            }
        } 
        else if (type == VehicleType::CAR) {
            if (currentPos.x > SIM_WIDTH / 2.0 && currentPos.y > SIM_HEIGHT / 2.0) {
                destination = currentPos;
                hasArrived = true;
            } else {
                destination.x = getRandomDouble(SIM_WIDTH / 2.0, SIM_WIDTH);
                destination.y = getRandomDouble(SIM_HEIGHT / 2.0, SIM_HEIGHT);
            }
        }
    }

    void simulateMovement() {
        if (hasArrived) {
            safePrint("Generated in needed area. Staying in place: (" + 
                      to_string(currentPos.x) + ", " + to_string(currentPos.y) + ")");
            return;
        }

        safePrint("Start: (" + to_string(currentPos.x) + ", " + to_string(currentPos.y) + 
                  ") -> Destination: (" + to_string(destination.x) + ", " + to_string(destination.y) + ")");

        while (!hasArrived) {
            double distance = getDistance(currentPos, destination);

            if (distance <= speed) {
                currentPos = destination;
                hasArrived = true;
                safePrint("Arrived at destination");
                break;
            }

            double dx = (destination.x - currentPos.x) / distance;
            double dy = (destination.y - currentPos.y) / distance;

            currentPos.x += dx * speed;
            currentPos.y += dy * speed;

            this_thread::sleep_for(chrono::milliseconds(200));
        }
    }
};

int main() {
    cout << "Area size: " << SIM_WIDTH << "x" << SIM_HEIGHT << "\n\n";

    const double SPEED = 50.0;
    const int NUM_TRUCKS = 3;
    const int NUM_CARS = 3;

    vector<thread> threads;
    vector<Vehicle> vehicles;

    for (int i = 0; i < NUM_TRUCKS; ++i) {
        vehicles.emplace_back(i + 1, VehicleType::TRUCK, SPEED);
    }
    for (int i = 0; i < NUM_CARS; ++i) {
        vehicles.emplace_back(i + 1, VehicleType::CAR, SPEED);
    }

    for (auto& vehicle : vehicles) {
        threads.emplace_back(&Vehicle::simulateMovement, &vehicle);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}